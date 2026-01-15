#pragma once

#include <vector>
#include <unordered_map>
#include <QString>
#include <QVector>

#include "SimCreature.h"
#include "SimFood.h"

/**
 * @brief Per-tick tracking data collected during simulation updates.
 */
struct Tracking {
    /** @brief Species names of deaths recorded this tick. */
    QVector<QString> deaths;
    /**
     * @brief Death cause counters for a single tick.
     */
    struct DeathCause {
        /** @brief Deaths from age. */
        int age = 0;
        /** @brief Deaths from hunger. */
        int hunger = 0;
        /** @brief Deaths from predation. */
        int predation = 0;
    } deathCause;
    /** @brief Species names of births recorded this tick. */
    QVector<QString> births;
    /** @brief Newly spawned creatures to add after the tick. */
    std::vector<Creature*> newborns;
    /** @brief Prey creature ids to remove after predation. */
    std::vector<int> preyToAdd;
    /** @brief Map of food id to number of competitors targeting it. */
    std::unordered_map<int, int> foodCompetitionMap;
};

/**
 * @brief Simulation environment container for creatures and food.
 */
class Environment {
public:
    /**
     * @brief Create an environment with food and bounds configuration.
     * @param foodRespawnBase Base replication count for food.
     * @param foodRespawnMultiplier Multiplier applied to \c foodRespawnBase.
     * @param foodEnergy Energy value assigned to each food item.
     * @param width Environment width.
     * @param height Environment height.
     */
    Environment(double foodRespawnBase,
                double foodRespawnMultiplier,
                double foodEnergy,
                int width,
                int height);
    /**
     * @brief Release owned creatures and food.
     * @note This destructor deletes all pointers stored in \c creatures and \c foods.
     */
    ~Environment();

    /**
     * @brief Add a creature to the environment.
     * @param creature Heap-allocated creature.
     * @note Ownership transfers to the environment; it will delete the creature.
     */
    void addCreature(Creature* creature);
    /**
     * @brief Add a food item to the environment.
     * @param food Heap-allocated food.
     * @note Ownership transfers to the environment; it will delete the food.
     */
    void addFood(Food* food);

    /**
     * @brief Populate initial food items.
     * @note The count is based on \c baseReplicationCount.
     */
    void setupFood();
    /**
     * @brief Populate initial creatures from configs.
     * @param creatures Creature configuration list.
     */
    void setupCreatures(const QVector<CreatureSettings>& creatures);
    /**
     * @brief Randomly replenish food.
     * @note Replenishment is probabilistic and may add multiple food items.
     */
    void replenishFood();
    /**
     * @brief Advance environment one tick and collect tracking info.
     * @param tracking Per-tick tracking accumulator.
     */
    void update(Tracking& tracking);

    std::vector<Creature*> creatures;
    std::vector<Food*> foods;

    int width = 0;
    int height = 0;
    double foodRespawnBase = 0.0;
    double foodEnergy = 0.0;
    int baseReplicationCount = 0;
    bool hasPredators = false;

    int creatureID = 1;
    int foodID = 1;
};
