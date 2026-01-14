#include "SimCreature.h"
#include "SimBehavior.h"
#include "SimFood.h"
#include "SimEnvironment.h"

Creature::Creature(int idValue, double xValue, double yValue, const CreatureSettings& config, double width, double height)
{
    id = idValue;
    x = xValue;
    y = yValue;
    colorR = config.colorR;
    colorG = config.colorG;
    colorB = config.colorB;
    baseSpeed = config.baseSpeed;
    metabolicRate = config.metabolicRate;
    fullnessLevel = config.initialFullness;
    fullnessCap = config.fullnessCap;
    energyStorageRate = config.energyStorageRate;
    reserveEnergy = config.reserveEnergy;
    dietType = config.dietType;
    dietPreference = config.dietPreference;
    reproductionCost = config.reproductionCost;
    matingHungerThreshold = config.matingHungerThreshold;
    reproductionCooldown = config.reproductionCooldown;
    reproductionCooldownCap = reproductionCooldown;
    litterSize = config.litterSize;
    size = config.size;
    health = config.health;
    age = config.age;
    ageCap = config.ageCap;
    ageRate = config.ageRate;
    speciesName = config.speciesName;
    speedMultiplier = config.speedMultiplier;
    metabolicBaseRate = config.metabolicBaseRate;
    envWidth = width;
    envHeight = height;
    mutationFactor = config.mutationFactor;
    attackPower = config.attackPower;
    defencePower = config.defencePower;
    fleeExhaustionRate = config.fleeExhaustion;
    fleeRecoveryFactor = config.fleeRecoveryFactor;
    skittishMultiplierBase = config.skittishMultiplierBase;
    skittishMultiplier = skittishMultiplierBase;
    skittishMultiplierScared = config.skittishMultiplierScared;
}

void Creature::update(Environment& environment, Tracking& tracking)
{
    CreatureBehaviour::updateAge(*this);
    CreatureBehaviour::updateCooldowns(*this);
    CreatureBehaviour::checkSurvival(*this);
    if (dead) {
        return;
    }

    CreatureBehaviour::checkSafety(*this, environment);
    CreatureBehaviour::checkState(*this);

    if (state == "hunting") {
        CreatureBehaviour::goHunt(*this, environment, tracking);
    } else if (state == "mating") {
        CreatureBehaviour::goMate(*this, environment, tracking);
    } else if (state == "fleeing") {
        CreatureBehaviour::goFlee(*this);
    } else if (state == "resting") {
        CreatureBehaviour::goRest(*this);
    } else {
        CreatureBehaviour::goExplore(*this);
    }
}

double Creature::getDistance(double xValue, double yValue) const
{
    const double dx = x - xValue;
    const double dy = y - yValue;
    return std::sqrt(dx * dx + dy * dy);
}

double Creature::getDistance(const TargetRef& target) const
{
    return getDistance(target.x(), target.y());
}

double Creature::getEnergyContent() const
{
    return size * 6.0 + (fullnessLevel / static_cast<double>(fullnessCap)) * 6.0;
}

Creature* Creature::makeBaby(const CreatureSettings& config, int newId, double xValue, double yValue)
{
    return new Creature(newId, xValue, yValue, config, envWidth, envHeight);
}

int TargetRef::id() const
{
    if (type == Type::Food && food) {
        return food->id();
    }
    if (type == Type::Creature && creature) {
        return creature->id;
    }
    return -1;
}

double TargetRef::x() const
{
    if (type == Type::Food && food) {
        return food->x();
    }
    if (type == Type::Creature && creature) {
        return creature->x;
    }
    return 0.0;
}

double TargetRef::y() const
{
    if (type == Type::Food && food) {
        return food->y();
    }
    if (type == Type::Creature && creature) {
        return creature->y;
    }
    return 0.0;
}
