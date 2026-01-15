#include "SimBehavior.h"
#include "SimRandom.h"

#include <cmath>
#include <limits>
#include <algorithm>

namespace {
constexpr double kPi = 3.141592653589793;

/**
 * @brief Distance between a creature and a point.
 * @param creature Creature to measure from.
 * @param x Target x coordinate.
 * @param y Target y coordinate.
 * @return Euclidean distance.
 */
double getDistance(const Creature& creature, double x, double y)
{
    const double dx = creature.x - x;
    const double dy = creature.y - y;
    return std::sqrt(dx * dx + dy * dy);
}

/**
 * @brief Calculate desirability score for a food target.
 * @param creature Creature selecting food.
 * @param food Food candidate.
 * @param tracking Per-tick tracking with food competition.
 * @return Desirability score (higher is better).
 */
double calculateFoodDesirability(Creature& creature, Food& food, Tracking& tracking)
{
    const double distance = getDistance(creature, food.x(), food.y());
    const double energyValue = food.energyContent();
    const double focus = (creature.targetFood.type == TargetRef::Type::Food && creature.targetFood.food == &food)
        ? 3.0
        : 1.0;
    const int competition = tracking.foodCompetitionMap.count(food.id())
        ? tracking.foodCompetitionMap[food.id()]
        : 0;

    return ((energyValue * focus) / distance) * (1.0 / (competition + 1));
}

/**
 * @brief Calculate desirability score for a prey target.
 * @param creature Predator creature.
 * @param prey Prey candidate.
 * @param environment Environment containing competitors.
 * @return Desirability score (higher is better).
 */
double calculatePreyDesirability(Creature& creature, Creature& prey, Environment& environment)
{
    const double distance = getDistance(creature, prey.x, prey.y);
    const double energyValue = prey.getEnergyContent();
    const double focus = (creature.targetFood.type == TargetRef::Type::Creature && creature.targetFood.creature == &prey)
        ? 1.5
        : 1.0;

    int competition = 0;
    for (auto* otherCreature : environment.creatures) {
        if (otherCreature->targetFood.type == TargetRef::Type::Creature &&
            otherCreature->targetFood.creature == &prey) {
            competition += 1;
        }
    }

    return ((energyValue * focus) / distance) * (1.0 / std::pow(competition + 1, 0.2));
}

/**
 * @brief Apply movement deltas with bounds and metabolic cost.
 * @param creature Creature to move.
 * @param xDelta X delta to apply.
 * @param yDelta Y delta to apply.
 * @note Movement is doubled when \c creature.state is "fleeing".
 */
void move(Creature& creature, double xDelta, double yDelta)
{
    if (creature.state == "fleeing") {
        xDelta *= 2.0;
        yDelta *= 2.0;
    }

    creature.x += xDelta;
    creature.y += yDelta;

    creature.fullnessLevel -= std::abs(xDelta) * creature.metabolicBaseRate * creature.metabolicRate;
    creature.fullnessLevel -= std::abs(yDelta) * creature.metabolicBaseRate * creature.metabolicRate;

    if (creature.x < 0) creature.x = 0;
    if (creature.x > creature.envWidth) creature.x = creature.envWidth;
    if (creature.y < 0) creature.y = 0;
    if (creature.y > creature.envHeight) creature.y = creature.envHeight;
}

/**
 * @brief Move toward a target position.
 * @param creature Creature to move.
 * @param xTarget Target x coordinate.
 * @param yTarget Target y coordinate.
 */
void moveTowards(Creature& creature, double xTarget, double yTarget)
{
    double xDiff = xTarget - creature.x;
    double yDiff = yTarget - creature.y;
    double angle = std::atan2(yDiff, xDiff);
    double xDelta = std::cos(angle) * creature.baseSpeed * creature.speedMultiplier;
    double yDelta = std::sin(angle) * creature.baseSpeed * creature.speedMultiplier;

    if (std::abs(xDelta) > std::abs(xDiff)) {
        xDelta = xDiff;
    }
    if (std::abs(yDelta) > std::abs(yDiff)) {
        yDelta = yDiff;
    }

    move(creature, xDelta, yDelta);
}

/**
 * @brief Find closest mate candidate of the same species.
 * @param creature Creature seeking a mate.
 * @param environment Environment containing candidates.
 * @return Closest matching creature or nullptr.
 */
Creature* findClosestCreature(Creature& creature, Environment& environment)
{
    Creature* closestCreature = nullptr;
    double minDistance = std::numeric_limits<double>::infinity();

    for (auto* other : environment.creatures) {
        if (other->id != creature.id &&
            other->speciesName == creature.speciesName &&
            other->reproductionCooldown <= 0) {
            const double distance = getDistance(creature, other->x, other->y);
            if (distance < minDistance) {
                minDistance = distance;
                closestCreature = other;
            }
        }
    }

    return closestCreature;
}

/**
 * @brief Find closest predator in the environment.
 * @param creature Creature checking for predators.
 * @param environment Environment containing predators.
 * @return Closest predator or nullptr.
 */
Creature* findClosestPredator(Creature& creature, Environment& environment)
{
    Creature* closestCreature = nullptr;
    double minDistance = std::numeric_limits<double>::infinity();

    if (environment.hasPredators) {
        for (auto* other : environment.creatures) {
            if (other->id != creature.id &&
                other->speciesName != creature.speciesName &&
                other->dietType != "herbivore") {
                const double distance = getDistance(creature, other->x, other->y);
                if (distance < minDistance) {
                    minDistance = distance;
                    closestCreature = other;
                }
            }
        }
    }

    return closestCreature;
}

/**
 * @brief Select the best food or prey target.
 * @param creature Creature selecting targets.
 * @param environment Environment containing food and prey.
 * @param tracking Per-tick tracking with competition data.
 * @return Target reference describing the best option.
 */
TargetRef findBestFood(Creature& creature, Environment& environment, Tracking& tracking)
{
    TargetRef best;
    double highestDesirability = -std::numeric_limits<double>::infinity();

    if (creature.targetFood.type == TargetRef::Type::Food) {
        bool exists = false;
        for (auto* food : environment.foods) {
            if (food->consumed()) {
                continue;
            }
            if (creature.targetFood.food == food) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            creature.targetFood = TargetRef();
        }
    }

    if (creature.dietType == "herbivore" || creature.dietType == "omnivore") {
        for (auto* food : environment.foods) {
            if (food->consumed()) {
                continue;
            }
            double desirability = calculateFoodDesirability(creature, *food, tracking);
            if (creature.dietType == "omnivore" && creature.preferredFoodType == "Plants") {
                desirability *= 2.0;
            }
            if (desirability > highestDesirability) {
                highestDesirability = desirability;
                best.type = TargetRef::Type::Food;
                best.food = food;
                best.creature = nullptr;
            }
        }
    }

    if (creature.dietType == "carnivore" || creature.dietType == "omnivore") {
        for (auto* potentialPrey : environment.creatures) {
            if (potentialPrey->speciesName != creature.speciesName && potentialPrey->health > 0) {
                double desirability = calculatePreyDesirability(creature, *potentialPrey, environment);
                if (creature.dietType == "omnivore" && creature.preferredFoodType == "Meat") {
                    desirability *= 2.0;
                }
                if (desirability > highestDesirability) {
                    highestDesirability = desirability;
                    best.type = TargetRef::Type::Creature;
                    best.creature = potentialPrey;
                    best.food = nullptr;
                }
            }
        }
    }

    return best;
}

/**
 * @brief Consume a food item and update creature state.
 * @param creature Creature consuming food.
 * @param food Food to consume.
 * @param environment Environment used to mark food consumed.
 */
void consumeFood(Creature& creature, Food& food, Environment& environment)
{
    creature.fullnessLevel += food.energyContent();
    for (auto it = environment.foods.begin(); it != environment.foods.end(); ++it) {
        if (*it == &food) {
            food.markConsumed();
            break;
        }
    }
    creature.targetFood = TargetRef();
    creature.tired = true;
    creature.recoveryNeeded = 2;
}

/**
 * @brief Consume a prey creature and update creature state.
 * @param creature Predator creature.
 * @param prey Prey creature.
 */
void consumePrey(Creature& creature, Creature& prey)
{
    creature.fullnessLevel = creature.fullnessLevel + prey.getEnergyContent();
    creature.targetFood = TargetRef();
    creature.tired = true;
    creature.recoveryNeeded = 60;
}

/**
 * @brief Mutate a numeric value by a percent factor.
 * @param value Original value.
 * @param mutationFactor Probability of mutation.
 * @param factor Maximum percent change (as fraction).
 * @return Mutated or original value.
 */
double mutateValuePercent(double value, double mutationFactor, double factor)
{
    if (SimRandom::urand() < mutationFactor) {
        const double mutation = SimRandom::urand() * (factor * 2) - factor;
        double newValue = value + value * mutation;
        if (newValue <= 0) {
            return value;
        }
        return newValue;
    }
    return value;
}

/**
 * @brief Mutate litter size when both parents mutate.
 * @param creature First parent.
 * @param otherCreature Second parent.
 * @return Litter size after mutation logic.
 */
int mutateBirth(Creature& creature, Creature& otherCreature)
{
    if (SimRandom::urand() < creature.mutationFactor && SimRandom::urand() < otherCreature.mutationFactor) {
        double newLitter = std::round((creature.litterSize + otherCreature.litterSize) / 2.0 + SimRandom::urand() * 2 - 1);
        if (newLitter < 1) {
            return 1;
        }
        return static_cast<int>(std::round(newLitter));
    }
    return static_cast<int>(std::floor((creature.litterSize + otherCreature.litterSize) / 2.0));
}

/**
 * @brief Produce a mutated child configuration for reproduction.
 * @param creature First parent.
 * @param otherCreature Second parent.
 * @return Child configuration with mutations applied.
 */
CreatureSettings reproduce(Creature& creature, Creature& otherCreature)
{
    struct Factors {
        double baseSpeed = 0.1;
        double speedMultiplier = 0.15;
        double health = 0.15;
        double ageCap = 0.05;
        double fullnessCap = 0.05;
        double metabolicBaseRate = 0.1;
        double metabolicRate = 0.15;
        double energyStorageRate = 0.1;
        double reproductionCost = 0.1;
        double matingHungerThreshold = 0.1;
        double reproductionCooldown = 0.05;
        double attackPower = 0.1;
        double defencePower = 0.1;
        double skittishMultiplierBase = 0.1;
        double skittishMultiplierScared = 0.1;
        double fleeExhaustionRate = 0.1;
        double fleeRecoveryFactor = 0.1;
    } factors;

    CreatureSettings config;
    config.speciesName = creature.speciesName;
    config.baseSpeed = (mutateValuePercent(creature.baseSpeed, creature.mutationFactor, factors.baseSpeed)
        + mutateValuePercent(otherCreature.baseSpeed, otherCreature.mutationFactor, factors.baseSpeed)) / 2.0;
    config.speedMultiplier = (mutateValuePercent(creature.speedMultiplier, creature.mutationFactor, factors.speedMultiplier)
        + mutateValuePercent(otherCreature.speedMultiplier, otherCreature.mutationFactor, factors.speedMultiplier)) / 2.0;
    config.health = static_cast<int>((mutateValuePercent(creature.health, creature.mutationFactor, factors.health)
        + mutateValuePercent(otherCreature.health, otherCreature.mutationFactor, factors.health)) / 2.0);
    config.age = 0;
    config.ageCap = (mutateValuePercent(creature.ageCap, creature.mutationFactor, factors.ageCap)
        + mutateValuePercent(otherCreature.ageCap, otherCreature.mutationFactor, factors.ageCap)) / 2.0;
    config.ageRate = creature.ageRate;
    config.fullnessCap = static_cast<int>((mutateValuePercent(creature.fullnessCap, creature.mutationFactor, factors.fullnessCap)
        + mutateValuePercent(otherCreature.fullnessCap, otherCreature.mutationFactor, factors.fullnessCap)) / 2.0);
    config.initialFullness = static_cast<int>(std::floor(
        (mutateValuePercent(creature.fullnessCap / 2.0, creature.mutationFactor, factors.fullnessCap)
            + mutateValuePercent(otherCreature.fullnessCap / 2.0, otherCreature.mutationFactor, factors.fullnessCap)) / 2.0));
    config.metabolicBaseRate = (mutateValuePercent(creature.metabolicBaseRate, creature.mutationFactor, factors.metabolicRate)
        + mutateValuePercent(otherCreature.metabolicBaseRate, otherCreature.mutationFactor, factors.metabolicRate)) / 2.0;
    config.metabolicRate = (mutateValuePercent(creature.metabolicRate, creature.mutationFactor, factors.metabolicBaseRate)
        + mutateValuePercent(otherCreature.metabolicRate, otherCreature.mutationFactor, factors.metabolicBaseRate)) / 2.0;
    config.energyStorageRate = (mutateValuePercent(creature.energyStorageRate, creature.mutationFactor, factors.energyStorageRate)
        + mutateValuePercent(otherCreature.energyStorageRate, otherCreature.mutationFactor, factors.energyStorageRate)) / 2.0;
    config.reserveEnergy = 0.0;
    config.dietType = creature.dietType == otherCreature.dietType ? creature.dietType : "omnivore";
    config.dietPreference = (SimRandom::urand() > 0.5) ? creature.dietPreference : otherCreature.dietPreference;
    config.reproductionCost = static_cast<int>((mutateValuePercent(creature.reproductionCost, creature.mutationFactor, factors.reproductionCost)
        + mutateValuePercent(otherCreature.reproductionCost, otherCreature.mutationFactor, factors.reproductionCost)) / 2.0);
    config.matingHungerThreshold = static_cast<int>((mutateValuePercent(creature.matingHungerThreshold, creature.mutationFactor, factors.matingHungerThreshold)
        + mutateValuePercent(otherCreature.matingHungerThreshold, otherCreature.mutationFactor, factors.matingHungerThreshold)) / 2.0);
    config.reproductionCooldown = static_cast<int>((mutateValuePercent(creature.reproductionCooldownCap, creature.mutationFactor, factors.reproductionCooldown)
        + mutateValuePercent(otherCreature.reproductionCooldownCap, otherCreature.mutationFactor, factors.reproductionCooldown)) / 2.0);
    config.litterSize = mutateBirth(creature, otherCreature);
    config.colorR = creature.colorR;
    config.colorG = creature.colorG;
    config.colorB = creature.colorB;
    config.size = creature.size;
    config.mutationFactor = creature.mutationFactor;
    config.attackPower = (mutateValuePercent(creature.attackPower, creature.mutationFactor, factors.attackPower)
        + mutateValuePercent(otherCreature.attackPower, otherCreature.mutationFactor, factors.attackPower)) / 2.0;
    config.defencePower = (mutateValuePercent(creature.defencePower, creature.mutationFactor, factors.defencePower)
        + mutateValuePercent(otherCreature.defencePower, otherCreature.mutationFactor, factors.defencePower)) / 2.0;
    config.skittishMultiplierBase = (mutateValuePercent(creature.skittishMultiplierBase, creature.mutationFactor, factors.skittishMultiplierBase)
        + mutateValuePercent(otherCreature.skittishMultiplierBase, otherCreature.mutationFactor, factors.skittishMultiplierBase)) / 2.0;
    config.skittishMultiplierScared = (mutateValuePercent(creature.skittishMultiplierScared, creature.mutationFactor, factors.skittishMultiplierScared)
        + mutateValuePercent(otherCreature.skittishMultiplierScared, otherCreature.mutationFactor, factors.skittishMultiplierScared)) / 2.0;
    config.fleeExhaustion = (mutateValuePercent(creature.fleeExhaustionRate, creature.mutationFactor, factors.fleeExhaustionRate)
        + mutateValuePercent(otherCreature.fleeExhaustionRate, otherCreature.mutationFactor, factors.fleeExhaustionRate)) / 2.0;
    config.fleeRecoveryFactor = (mutateValuePercent(creature.fleeRecoveryFactor, creature.mutationFactor, factors.fleeRecoveryFactor)
        + mutateValuePercent(otherCreature.fleeRecoveryFactor, otherCreature.mutationFactor, factors.fleeRecoveryFactor)) / 2.0;

    return config;
}

/**
 * @brief Update food competition counts for targeting changes.
 * @param tracking Per-tick tracking accumulator.
 * @param oldFoodID Previous food id (or -1).
 * @param newFoodID New food id (or -1).
 */
void updateFoodCompetitionMap(Tracking& tracking, int oldFoodID, int newFoodID)
{
    if (oldFoodID != -1) {
        auto it = tracking.foodCompetitionMap.find(oldFoodID);
        if (it != tracking.foodCompetitionMap.end()) {
            if (it->second > 1) {
                it->second -= 1;
            } else {
                tracking.foodCompetitionMap.erase(it);
            }
        }
    }

    if (newFoodID != -1) {
        auto it = tracking.foodCompetitionMap.find(newFoodID);
        if (it != tracking.foodCompetitionMap.end()) {
            it->second += 1;
        } else {
            tracking.foodCompetitionMap.emplace(newFoodID, 1);
        }
    }
}

/**
 * @brief Attack prey and register predation deaths.
 * @param creature Predator creature.
 * @param prey Prey creature.
 * @param tracking Per-tick tracking accumulator.
 */
void attackPrey(Creature& creature, Creature& prey, Tracking& tracking)
{
    prey.health = prey.health - creature.attackPower;
    if (prey.health <= 0) {
        const QString cause = "predation";
        tracking.deathCause.predation = tracking.deathCause.predation + 1;
        tracking.deaths.push_back(prey.speciesName);
        tracking.preyToAdd.push_back(prey.id);
        consumePrey(creature, prey);
    }
}
}

namespace CreatureBehaviour {
void updateAge(Creature& creature)
{
    creature.age += creature.ageRate;
}

void updateCooldowns(Creature& creature)
{
    if (creature.reproductionCooldown > 0) {
        creature.reproductionCooldown--;
    }
}

static void checkHunger(Creature& creature)
{
    if (creature.fullnessLevel <= 0 && creature.reserveEnergy <= 0) {
        creature.health -= std::abs(creature.fullnessLevel);
    } else if (creature.fullnessLevel <= 0 && creature.reserveEnergy > 0) {
        creature.reserveEnergy -= std::abs(creature.fullnessLevel);
        creature.fullnessLevel = 0;
    } else if (creature.fullnessLevel > creature.fullnessCap) {
        creature.reserveEnergy += (creature.fullnessLevel - creature.fullnessCap) * creature.energyStorageRate;
        creature.fullnessLevel = creature.fullnessCap;
    }
}

static void checkHealth(Creature& creature)
{
    if (creature.health <= 0) {
        creature.dead = true;
        creature.deathCause = "hunger";
    }
}

static void checkAge(Creature& creature)
{
    if (creature.age >= creature.ageCap) {
        const double ageExcess = creature.age - creature.ageCap;
        const double deathProbability = std::min(1.0, ageExcess * 0.1);
        if (SimRandom::urand() < deathProbability) {
            creature.dead = true;
            creature.deathCause = "age";
        }
    }
}

void checkSurvival(Creature& creature)
{
    checkHunger(creature);
    checkAge(creature);
    checkHealth(creature);
}

void checkSafety(Creature& creature, Environment& environment)
{
    if (creature.state == "fleeing") {
        creature.skittishMultiplier = creature.skittishMultiplierScared;
    } else {
        creature.skittishMultiplier = creature.skittishMultiplierBase;
    }

    Creature* closestPredator = findClosestPredator(creature, environment);
    if (closestPredator) {
        if (creature.getDistance(closestPredator->x, closestPredator->y) <=
            creature.baseSpeed * creature.speedMultiplier * creature.skittishMultiplier) {
            creature.state = "fleeing";
            creature.predator = closestPredator;
            return;
        }
        creature.state = "";
        creature.predator = nullptr;
        return;
    }

    if (creature.state == "fleeing") {
        creature.state = "";
        creature.predator = nullptr;
    }
}

void checkState(Creature& creature)
{
    if (creature.state == "fleeing") {
        creature.fleeCount = creature.fleeCount + 1;
        creature.fleeRecoverycooldown = creature.fleeRecoverycooldown + 1;
        return;
    }

    if (creature.fleeRecoverycooldown > 0) {
        creature.fleeRecoverycooldown = creature.fleeRecoverycooldown - 1;
    } else if (creature.fleeRecoverycooldown <= 0 && creature.fleeCount > 0) {
        creature.fleeCount = creature.fleeCount - 1;
    }

    if (creature.tired) {
        creature.state = "resting";
        return;
    }

    if (creature.fullnessLevel > creature.matingHungerThreshold && creature.reproductionCooldown <= 0) {
        creature.state = "mating";
        return;
    }

    if (creature.fullnessLevel < creature.fullnessCap) {
        creature.state = "hunting";
        return;
    }

    creature.state = "exploring";
}

void goMate(Creature& creature, Environment& environment, Tracking& tracking)
{
    Creature* closestCreature = findClosestCreature(creature, environment);
    if (closestCreature) {
        moveTowards(creature, closestCreature->x, closestCreature->y);
        if (creature.getDistance(closestCreature->x, closestCreature->y) <= creature.size + creature.size / 2.0) {
            creature.fullnessLevel -= creature.reproductionCost;
            closestCreature->fullnessLevel -= closestCreature->reproductionCost;

            for (int i = 0; i < creature.litterSize; i++) {
                CreatureSettings babyConfig = reproduce(creature, *closestCreature);
                Creature* baby = creature.makeBaby(babyConfig, environment.creatureID++, creature.x, closestCreature->y);
                tracking.newborns.push_back(baby);
                tracking.births.push_back(creature.speciesName);
            }

            creature.reproductionCooldown = creature.reproductionCooldownCap;
            closestCreature->reproductionCooldown = closestCreature->reproductionCooldownCap;
            return;
        }
        return;
    }

    goExplore(creature);
}

void goHunt(Creature& creature, Environment& environment, Tracking& tracking)
{
    int targetID = creature.targetFood.type != TargetRef::Type::None ? creature.targetFood.id() : -1;
    TargetRef bestFood = findBestFood(creature, environment, tracking);
    if (bestFood.type != TargetRef::Type::None) {
        updateFoodCompetitionMap(tracking, targetID, bestFood.id());
        creature.targetFood = bestFood;
        moveTowards(creature, bestFood.x(), bestFood.y());
        double targetSize = 0.0;
        if (bestFood.type == TargetRef::Type::Food && bestFood.food) {
            targetSize = bestFood.food->size();
        } else if (bestFood.type == TargetRef::Type::Creature && bestFood.creature) {
            targetSize = bestFood.creature->size;
        }
        if (creature.getDistance(bestFood) <= targetSize + creature.size / 2.0) {
            if (bestFood.type == TargetRef::Type::Food && bestFood.food) {
                consumeFood(creature, *bestFood.food, environment);
            } else if (bestFood.type == TargetRef::Type::Creature && bestFood.creature) {
                attackPrey(creature, *bestFood.creature, tracking);
            }
        }
    } else {
        const double angle = SimRandom::urand() * 2.0 * kPi;
        const double xDelta = std::cos(angle) * creature.baseSpeed * creature.speedMultiplier;
        const double yDelta = std::sin(angle) * creature.baseSpeed * creature.speedMultiplier;
        move(creature, xDelta, yDelta);
    }
}

void goFlee(Creature& creature)
{
    const double angle = std::atan2(creature.y - creature.predator->y, creature.x - creature.predator->x);
    const double xDelta = std::cos(angle) * creature.baseSpeed * creature.speedMultiplier;
    const double yDelta = std::sin(angle) * creature.baseSpeed * creature.speedMultiplier;
    move(creature, xDelta, yDelta);
}

void goRest(Creature& creature)
{
    creature.recoveryNeeded--;
    if (creature.recoveryNeeded <= 0) {
        creature.tired = false;
        creature.recoveryNeeded = 0;
        creature.state = "exploring";
    }
}

void goExplore(Creature& creature)
{
    const double maxTurnAngle = (18.0 * kPi) / 180.0;
    if (creature.hasLastDirection) {
        const double angleChange = SimRandom::urand() * 2 * maxTurnAngle - maxTurnAngle;
        const double angle = creature.lastDirection + angleChange;
        creature.lastDirection = angle;
        const double xDelta = std::cos(angle) * creature.baseSpeed * creature.speedMultiplier;
        const double yDelta = std::sin(angle) * creature.baseSpeed * creature.speedMultiplier;
        move(creature, xDelta, yDelta);
    } else {
        const double angle = SimRandom::urand() * 2 * kPi;
        creature.lastDirection = angle;
        creature.hasLastDirection = true;
        const double xDelta = std::cos(angle) * creature.baseSpeed * creature.speedMultiplier;
        const double yDelta = std::sin(angle) * creature.baseSpeed * creature.speedMultiplier;
        move(creature, xDelta, yDelta);
    }
}
}
