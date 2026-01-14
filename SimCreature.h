#pragma once

#include <QString>
#include <limits>
#include "MainWindow.h"

class Food;
class Environment;
struct Tracking;

struct TargetRef {
    enum class Type {
        None,
        Food,
        Creature
    };

    Type type = Type::None;
    Food* food = nullptr;
    class Creature* creature = nullptr;

    int id() const;
    double x() const;
    double y() const;
};

class Creature {
public:
    Creature(int id, double x, double y, const CreatureSettings& config, double envWidth, double envHeight);

    void update(Environment& environment, Tracking& tracking);
    double getDistance(double x, double y) const;
    double getDistance(const TargetRef& target) const;
    double getEnergyContent() const;
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
