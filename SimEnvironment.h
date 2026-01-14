#pragma once

#include <vector>
#include <unordered_map>
#include <QString>
#include <QVector>

#include "SimCreature.h"
#include "SimFood.h"

struct Tracking {
    QVector<QString> deaths;
    struct DeathCause {
        int age = 0;
        int hunger = 0;
        int predation = 0;
    } deathCause;
    QVector<QString> births;
    std::vector<Creature*> newborns;
    std::vector<int> preyToAdd;
    std::unordered_map<int, int> foodCompetitionMap;
};

class Environment {
public:
    Environment(double foodRespawnBase,
                double foodRespawnMultiplier,
                double foodEnergy,
                int width,
                int height);
    ~Environment();

    void addCreature(Creature* creature);
    void addFood(Food* food);

    void setupFood();
    void setupCreatures(const QVector<CreatureSettings>& creatures);
    void replenishFood();
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
