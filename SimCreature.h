#pragma once

#include <QString>
#include <limits>
#include "MainWindow.h"

class Food;
class Environment;
struct Tracking;

/**
 * @brief Lightweight reference to a target food or creature.
 */
struct TargetRef {
    /** @brief Type of target stored by \c TargetRef. */
    enum class Type {
        None,
        Food,
        Creature
    };

    Type type = Type::None;
    Food* food = nullptr;
    class Creature* creature = nullptr;

    /**
     * @brief Return target id or -1 when unset.
     * @return Target id when valid, otherwise -1.
     */
    int id() const;
    /**
     * @brief Return target x coordinate or 0 when unset.
     * @return Target x coordinate.
     */
    double x() const;
    /**
     * @brief Return target y coordinate or 0 when unset.
     * @return Target y coordinate.
     */
    double y() const;
};

/**
 * @brief Creature entity for the simulation.
 */
class Creature {
public:
    /**
     * @brief Create a creature from config at the given position and bounds.
     * @param id Unique creature id.
     * @param x Starting x coordinate.
     * @param y Starting y coordinate.
     * @param config Creature configuration values.
     * @param envWidth Environment width.
     * @param envHeight Environment height.
     */
    Creature(int id, double x, double y, const CreatureSettings& config, double envWidth, double envHeight);

    /**
     * @brief Update creature state for one simulation tick.
     * @param environment Environment owning the creature.
     * @param tracking Per-tick tracking accumulator.
     */
    void update(Environment& environment, Tracking& tracking);
    /**
     * @brief Distance to a point.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @return Euclidean distance to the point.
     */
    double getDistance(double x, double y) const;
    /**
     * @brief Distance to a target reference.
     * @param target Target to measure.
     * @return Euclidean distance to the target.
     */
    double getDistance(const TargetRef& target) const;
    /**
     * @brief Energy content used when consumed as prey.
     * @return Energy value for predators.
     */
    double getEnergyContent() const;
    /**
     * @brief Spawn a baby creature from config.
     * @param config New creature configuration.
     * @param newId Unique id for the baby.
     * @param x X coordinate for the baby.
     * @param y Y coordinate for the baby.
     * @return Newly allocated creature pointer.
     * @note Ownership transfers to the caller; \c Environment will delete it.
     */
    Creature* makeBaby(const CreatureSettings& config, int newId, double x, double y);

    int id = 0;
    double x = 0.0;
    double y = 0.0;

    int colorR = 0;
    int colorG = 0;
    int colorB = 0;
    double baseSpeed = 0.0;
    double metabolicRate = 0.0;
    double fullnessLevel = 0.0;
    int fullnessCap = 0;
    double energyStorageRate = 0.0;
    double reserveEnergy = 0.0;
    QString dietType;
    QString dietPreference;
    QString preferredFoodType;

    int reproductionCost = 0;
    int matingHungerThreshold = 0;
    int reproductionCooldown = 0;
    int reproductionCooldownCap = 0;
    int litterSize = 0;
    double size = 0.0;
    double health = 0.0;
    double age = 0.0;
    double ageCap = 0.0;
    double ageRate = 0.0;
    QString speciesName;
    double speedMultiplier = 0.0;
    double metabolicBaseRate = 0.0;
    double envWidth = 0.0;
    double envHeight = 0.0;
    double mutationFactor = 0.0;
    double attackPower = 0.0;
    double defencePower = 0.0;
    double fleeExhaustionRate = 0.0;
    double fleeRecoveryFactor = 0.0;
    double skittishMultiplierBase = 0.0;
    double skittishMultiplier = 0.0;
    double skittishMultiplierScared = 0.0;

    QString state = "hunting";
    bool dead = false;
    QString deathCause;
    bool tired = false;
    int recoveryNeeded = 0;
    TargetRef targetFood;
    Creature* predator = nullptr;
    int fleeCount = 0;
    double fleeRecoverycooldown = std::numeric_limits<double>::quiet_NaN();
    bool hasLastDirection = false;
    double lastDirection = 0.0;
};
