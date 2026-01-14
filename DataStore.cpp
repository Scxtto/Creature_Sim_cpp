#include "DataStore.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

static QJsonObject creatureToJson(const CreatureSettings& c)
{
    QJsonObject obj;
    obj["speciesName"] = c.speciesName;
    obj["baseSpeed"] = c.baseSpeed;
    obj["speedMultiplier"] = c.speedMultiplier;
    obj["health"] = c.health;
    obj["age"] = c.age;
    obj["ageCap"] = c.ageCap;
    obj["ageRate"] = c.ageRate;
    obj["initialPopulation"] = c.initialPopulation;
    obj["initialFullness"] = c.initialFullness;
    obj["fullnessCap"] = c.fullnessCap;
    obj["metabolicBaseRate"] = c.metabolicBaseRate;
    obj["metabolicRate"] = c.metabolicRate;
    obj["energyStorageRate"] = c.energyStorageRate;
    obj["reserveEnergy"] = c.reserveEnergy;
    obj["dietType"] = c.dietType;
    obj["dietPreference"] = c.dietPreference;
    obj["reproductionCost"] = c.reproductionCost;
    obj["matingHungerThreshold"] = c.matingHungerThreshold;
    obj["reproductionCooldown"] = c.reproductionCooldown;
    obj["litterSize"] = c.litterSize;
    obj["mutationFactor"] = c.mutationFactor;
    obj["colorR"] = c.colorR;
    obj["colorG"] = c.colorG;
    obj["colorB"] = c.colorB;
    obj["size"] = c.size;
    obj["skittishMultiplierBase"] = c.skittishMultiplierBase;
    obj["skittishMultiplierScared"] = c.skittishMultiplierScared;
    obj["attackPower"] = c.attackPower;
    obj["defencePower"] = c.defencePower;
    obj["fleeExhaustion"] = c.fleeExhaustion;
    obj["fleeRecoveryFactor"] = c.fleeRecoveryFactor;
    return obj;
}

static CreatureSettings creatureFromJson(const QJsonObject& obj)
{
    CreatureSettings c;
    c.speciesName = obj.value("speciesName").toString(c.speciesName);
    c.baseSpeed = obj.value("baseSpeed").toDouble(c.baseSpeed);
    c.speedMultiplier = obj.value("speedMultiplier").toDouble(c.speedMultiplier);
    c.health = obj.value("health").toInt(c.health);
    c.age = obj.value("age").toInt(c.age);
    c.ageCap = obj.value("ageCap").toInt(c.ageCap);
    c.ageRate = obj.value("ageRate").toDouble(c.ageRate);
    c.initialPopulation = obj.value("initialPopulation").toInt(c.initialPopulation);

    c.initialFullness = obj.value("initialFullness").toInt(c.initialFullness);
    c.fullnessCap = obj.value("fullnessCap").toInt(c.fullnessCap);
    c.metabolicBaseRate = obj.value("metabolicBaseRate").toDouble(c.metabolicBaseRate);
    c.metabolicRate = obj.value("metabolicRate").toDouble(c.metabolicRate);
    c.energyStorageRate = obj.value("energyStorageRate").toDouble(c.energyStorageRate);
    c.reserveEnergy = obj.value("reserveEnergy").toDouble(c.reserveEnergy);
    c.dietType = obj.value("dietType").toString(c.dietType);
    c.dietPreference = obj.value("dietPreference").toString(c.dietPreference);

    c.reproductionCost = obj.value("reproductionCost").toInt(c.reproductionCost);
    c.matingHungerThreshold = obj.value("matingHungerThreshold").toInt(c.matingHungerThreshold);
    c.reproductionCooldown = obj.value("reproductionCooldown").toInt(c.reproductionCooldown);
    c.litterSize = obj.value("litterSize").toInt(c.litterSize);
    c.mutationFactor = obj.value("mutationFactor").toDouble(c.mutationFactor);

    c.colorR = obj.value("colorR").toInt(c.colorR);
    c.colorG = obj.value("colorG").toInt(c.colorG);
    c.colorB = obj.value("colorB").toInt(c.colorB);
    c.size = obj.value("size").toDouble(c.size);

    c.skittishMultiplierBase = obj.value("skittishMultiplierBase").toDouble(c.skittishMultiplierBase);
    c.skittishMultiplierScared = obj.value("skittishMultiplierScared").toDouble(c.skittishMultiplierScared);
    c.attackPower = obj.value("attackPower").toDouble(c.attackPower);
    c.defencePower = obj.value("defencePower").toDouble(c.defencePower);
    c.fleeExhaustion = obj.value("fleeExhaustion").toDouble(c.fleeExhaustion);
    c.fleeRecoveryFactor = obj.value("fleeRecoveryFactor").toDouble(c.fleeRecoveryFactor);
    return c;
}

static QJsonObject resultToJson(const SimulationResult& result)
{
    QJsonObject root;
    root["videoFile"] = result.videoFile;
    root["datetime"] = result.datetime;
    root["status"] = result.status;
    root["nodeType"] = result.nodeType;
    root["duration"] = result.duration;
    root["computeCost"] = result.computeCost;
    root["resultSize"] = result.resultSize;

    QJsonArray creatureCount;
    for (double v : result.creatureCount) {
        creatureCount.append(v);
    }
    root["creatureCount"] = creatureCount;

    QJsonArray foodCount;
    for (double v : result.foodCount) {
        foodCount.append(v);
    }
    root["foodCount"] = foodCount;

    QJsonArray birthCount;
    for (double v : result.birthCount) {
        birthCount.append(v);
    }
    root["birthCount"] = birthCount;

    QJsonArray deathCount;
    for (double v : result.deathCount) {
        deathCount.append(v);
    }
    root["deathCount"] = deathCount;

    QJsonObject deathTypes;
    deathTypes["age"] = result.deathAge;
    deathTypes["hunger"] = result.deathHunger;
    deathTypes["predation"] = result.deathPredation;
    root["deathTypeCount"] = deathTypes;

    QJsonArray speciesArray;
    for (const auto& series : result.species) {
        QJsonObject s;
        s["name"] = series.name;
        s["colorR"] = series.color.red();
        s["colorG"] = series.color.green();
        s["colorB"] = series.color.blue();

        QJsonArray counts;
        for (double v : series.count) {
            counts.append(v);
        }
        s["count"] = counts;

        QJsonArray births;
        for (double v : series.births) {
            births.append(v);
        }
        s["births"] = births;

        QJsonArray deaths;
        for (double v : series.deaths) {
            deaths.append(v);
        }
        s["deaths"] = deaths;

        speciesArray.append(s);
    }
    root["species"] = speciesArray;

    return root;
}

QString DataStore::dataDir()
{
    QDir dir(QCoreApplication::applicationDirPath());
    while (dir.dirName() != "cpp_sim" && dir.cdUp()) {
    }
    if (dir.dirName() != "cpp_sim") {
        dir = QDir(QCoreApplication::applicationDirPath());
    }
    dir.mkpath("data");
    dir.cd("data");
    return dir.absolutePath();
}

QString DataStore::outputDir()
{
    QDir dir(dataDir());
    dir.mkpath("output");
    dir.cd("output");
    return dir.absolutePath();
}

QString DataStore::outputVideoPath()
{
    const QString name = QString("simulation_%1.mp4")
                             .arg(QDateTime::currentMSecsSinceEpoch());
    QDir dir(outputDir());
    return dir.filePath(name);
}

bool DataStore::saveCreatures(const SimulationSettings& sim,
                              const QVector<CreatureSettings>& creatures,
                              QString* error)
{
    QJsonObject root;
    QJsonObject simObj;
    simObj["simLength"] = sim.simLength;
    simObj["foodRespawnMultiplier"] = sim.foodRespawnMultiplier;
    simObj["foodRespawnBase"] = sim.foodRespawnBase;
    simObj["foodEnergy"] = sim.foodEnergy;
    root["simulationSettings"] = simObj;

    QJsonArray creatureArray;
    for (const auto& creature : creatures) {
        creatureArray.append(creatureToJson(creature));
    }
    root["creatures"] = creatureArray;

    QJsonDocument doc(root);
    QFile file(QDir(dataDir()).filePath("creatures.json"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) {
            *error = "Unable to write creatures.json";
        }
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

bool DataStore::loadCreatures(SimulationSettings& sim,
                              QVector<CreatureSettings>& creatures,
                              QString* error)
{
    QFile file(QDir(dataDir()).filePath("creatures.json"));
    if (!file.exists()) {
        if (error) {
            *error = "No saved creatures found.";
        }
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = "Unable to read creatures.json";
        }
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        if (error) {
            *error = "Invalid creatures.json format.";
        }
        return false;
    }

    QJsonObject root = doc.object();
    QJsonObject simObj = root.value("simulationSettings").toObject();
    sim.simLength = simObj.value("simLength").toInt(sim.simLength);
    sim.foodRespawnMultiplier = simObj.value("foodRespawnMultiplier").toDouble(sim.foodRespawnMultiplier);
    sim.foodRespawnBase = simObj.value("foodRespawnBase").toDouble(sim.foodRespawnBase);
    sim.foodEnergy = simObj.value("foodEnergy").toDouble(sim.foodEnergy);

    creatures.clear();
    for (const auto& item : root.value("creatures").toArray()) {
        if (item.isObject()) {
            creatures.push_back(creatureFromJson(item.toObject()));
        }
    }

    if (creatures.isEmpty()) {
        if (error) {
            *error = "No creatures stored in creatures.json.";
        }
        return false;
    }

    return true;
}

bool DataStore::saveResult(const SimulationResult& result, QString* error)
{
    QFile file(QDir(dataDir()).filePath("last_result.json"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) {
            *error = "Unable to write last_result.json";
        }
        return false;
    }

    QJsonDocument doc(resultToJson(result));
    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

QByteArray DataStore::serializeResult(const SimulationResult& result)
{
    QJsonDocument doc(resultToJson(result));
    return doc.toJson(QJsonDocument::Compact);
}
