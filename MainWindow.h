#pragma once

#include <QWidget>
#include <QThread>
#include <QVector>
#include <QColor>
#include <QString>
#include <atomic>

class QSpinBox;
class QDoubleSpinBox;
class QPushButton;
class QVBoxLayout;

class CreaturePanel;
class ResultsWindow;

struct SimulationSettings {
    int simLength = 5400;
    double foodRespawnMultiplier = 1.0;
    double foodRespawnBase = 1.0;
    double foodEnergy = 15.0;
};

struct CreatureSettings {
    QString speciesName = "Creature";
    double baseSpeed = 1.5;
    double speedMultiplier = 1.0;
    int health = 100;
    int age = 0;
    int ageCap = 35;
    double ageRate = 0.04;
    int initialPopulation = 25;

    int initialFullness = 100;
    int fullnessCap = 100;
    double metabolicBaseRate = 1.0 / 16.0;
    double metabolicRate = 1.0;
    double energyStorageRate = 0.7;
    double reserveEnergy = 0.0;

    QString dietType = "herbivore";
    QString dietPreference = "Plants";

    int reproductionCost = 40;
    int matingHungerThreshold = 50;
    int reproductionCooldown = 100;
    int litterSize = 1;
    double mutationFactor = 0.05;

    int colorR = 155;
    int colorG = 255;
    int colorB = 55;
    double size = 5.0;

    double skittishMultiplierBase = 10.0;
    double skittishMultiplierScared = 20.0;
    double attackPower = 40.0;
    double defencePower = 10.0;
    double fleeExhaustion = 0.05;
    double fleeRecoveryFactor = 10.0;
};

struct SpeciesSeries {
    QString name;
    QColor color;
    QVector<double> count;
    QVector<double> births;
    QVector<double> deaths;
};

struct SimulationResult {
    QString videoFile;
    QVector<double> creatureCount;
    QVector<double> foodCount;
    QVector<double> birthCount;
    QVector<double> deathCount;
    int deathAge = 0;
    int deathHunger = 0;
    int deathPredation = 0;
    QVector<SpeciesSeries> species;
    double duration = 0.0;
    double computeCost = 0.0;
    double resultSize = 0.0;
    QString datetime;
    QString status;
    QString nodeType;
    QString failureReason;
};

Q_DECLARE_METATYPE(SimulationResult)

class SimWorker : public QObject {
    Q_OBJECT
public:
    explicit SimWorker(QObject* parent = nullptr);

    void setInputs(const SimulationSettings& sim, const QVector<CreatureSettings>& creatures);
    void requestStop();

signals:
    void finishedWithResult(const SimulationResult& result);

public slots:
    void run();

private:
    SimulationResult runSimulation(const SimulationSettings& sim, const QVector<CreatureSettings>& creatures);

    std::atomic_bool m_stopRequested{ false };
    SimulationSettings m_sim;
    QVector<CreatureSettings> m_creatures;
};

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onAddCreature();
    void onSaveCreatures();
    void onLoadCreatures();
    void onStartSimulation();
    void onSimFinished(const SimulationResult& result);
    void onBackFromResults();

private:
    SimulationSettings collectSimulationSettings() const;
    QVector<CreatureSettings> collectCreatureSettings() const;
    void setSimulationSettings(const SimulationSettings& settings);
    void setCreatureSettings(const QVector<CreatureSettings>& creatures);
    void addCreaturePanel(const CreatureSettings& settings = CreatureSettings());
    void clearCreaturePanels();

    // Simulation inputs
    QSpinBox* simLength = nullptr;
    QDoubleSpinBox* foodRespawnMultiplier = nullptr;
    QDoubleSpinBox* foodRespawnBase = nullptr;
    QDoubleSpinBox* foodEnergy = nullptr;

    // Creature list UI
    QVBoxLayout* creatureListLayout = nullptr;
    QVector<CreaturePanel*> creaturePanels;

    // Buttons
    QPushButton* addCreatureBtn = nullptr;
    QPushButton* saveCreaturesBtn = nullptr;
    QPushButton* loadCreaturesBtn = nullptr;
    QPushButton* startBtn = nullptr;

    // Threading
    QThread* simThread = nullptr;
    SimWorker* simWorker = nullptr;

    // Results window
    ResultsWindow* resultsWindow = nullptr;
};
