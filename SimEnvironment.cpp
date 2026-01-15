#include "SimEnvironment.h"
#include "SimRandom.h"

#include <cmath>


Environment::Environment(double respawnBase,
    double respawnMultiplier,
    double energy,
    int widthValue,
    int heightValue)
{
    width = widthValue;
    height = heightValue;
    foodRespawnBase = respawnBase;
    foodEnergy = energy;
    baseReplicationCount = static_cast<int>(std::floor(foodRespawnBase * respawnMultiplier));
}


Environment::~Environment()
{
    for (auto* creature : creatures) {
        delete creature;
    }
    for (auto* food : foods) {
        delete food;
    }
}


void Environment::addCreature(Creature* creature)
{
    creatures.push_back(creature);
}


void Environment::addFood(Food* food)
{
    foods.push_back(food);
}


void Environment::setupFood()
{
    for (int i = 0; i < baseReplicationCount; i++) {
        int x = static_cast<int>(std::floor(SimRandom::urand() * width));
        int y = static_cast<int>(std::floor(SimRandom::urand() * height));
        addFood(new Food(foodID++, x, y, foodEnergy));
    }
}

void Environment::setupCreatures(const QVector<CreatureSettings>& creaturesConfig)
{
    for (const auto& creatureConfig : creaturesConfig) {
        for (int i = 0; i < creatureConfig.initialPopulation; i++) {
            addCreature(new Creature(
                creatureID++,
                std::floor(SimRandom::urand() * width),
                std::floor(SimRandom::urand() * height),
                creatureConfig,
                width,
                height));
            if (creatureConfig.dietType != "herbivore") {
                hasPredators = true;
            }
        }
    }
}

void Environment::replenishFood()
{
    if (SimRandom::urand() > 0.5) {
        for (int i = 0; i < baseReplicationCount; i++) {
            int x = static_cast<int>(std::round(SimRandom::urand() * width));
            int y = static_cast<int>(std::round(SimRandom::urand() * height));
            addFood(new Food(foodID++, x, y, foodEnergy));
        }
    }
}

void Environment::update(Tracking& tracking)
{
    for (auto* food : foods) {
        food->update();
    }

    replenishFood();

    for (auto* creature : creatures) {
        if (creature->targetFood.type != TargetRef::Type::None) {
            int id = creature->targetFood.id();
            if (id >= 0) {
                tracking.foodCompetitionMap[id] += 1;
            }
        }
    }

    std::vector<int> creaturesToRemove;
    std::vector<int> foodToRemove;

    for (auto* creature : creatures) {
        creature->update(*this, tracking);

        if (!tracking.preyToAdd.empty()) {
            for (const auto& preyId : tracking.preyToAdd) {
                creaturesToRemove.push_back(preyId);
            }
        }

        if (creature->dead) {
            const QString cause = creature->deathCause;
            if (cause == "age") {
                tracking.deathCause.age += 1;
            } else if (cause == "hunger") {
                tracking.deathCause.hunger += 1;
            } else if (cause == "predation") {
                tracking.deathCause.predation += 1;
            }
            tracking.deaths.push_back(creature->speciesName);
            creaturesToRemove.push_back(creature->id);
        }
    }

    if (!creaturesToRemove.empty()) {
        for (auto* creature : creatures) {
            if (creature->targetFood.type == TargetRef::Type::Creature && creature->targetFood.creature) {
                for (int id : creaturesToRemove) {
                    if (creature->targetFood.creature->id == id) {
                        creature->targetFood = TargetRef();
                        break;
                    }
                }
            }
        }

        std::vector<Creature*> remaining;
        remaining.reserve(creatures.size());
        for (auto* creature : creatures) {
            bool remove = false;
            for (int id : creaturesToRemove) {
                if (creature->id == id) {
                    remove = true;
                    break;
                }
            }
            if (remove) {
                delete creature;
            } else {
                remaining.push_back(creature);
            }
        }
        creatures = std::move(remaining);
    }

    for (auto* food : foods) {
        if (food->consumed()) {
            foodToRemove.push_back(food->id());
        }
    }

    if (!foodToRemove.empty()) {
        for (auto* creature : creatures) {
            if (creature->targetFood.type == TargetRef::Type::Food && creature->targetFood.food) {
                for (int id : foodToRemove) {
                    if (creature->targetFood.food->id() == id) {
                        creature->targetFood = TargetRef();
                        break;
                    }
                }
            }
        }

        std::vector<Food*> remainingFoods;
        remainingFoods.reserve(foods.size());
        for (auto* food : foods) {
            bool remove = false;
            for (int id : foodToRemove) {
                if (food->id() == id) {
                    remove = true;
                    break;
                }
            }
            if (remove) {
                delete food;
            } else {
                remainingFoods.push_back(food);
            }
        }
        foods = std::move(remainingFoods);
    }

    if (!tracking.newborns.empty()) {
        for (auto* baby : tracking.newborns) {
            addCreature(baby);
        }
    }
}
