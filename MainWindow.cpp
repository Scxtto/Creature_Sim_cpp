#include "MainWindow.h"
#include "CreaturePanel.h"
#include "DataStore.h"
#include "ResultsWindow.h"
#include "SimEnvironment.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDateTime>
#include <QProcess>
#include <QElapsedTimer>
#include <QHash>
#include <QByteArray>

#include <cmath>
#include <algorithm>

// ----------------------------
// SimWorker
// ----------------------------
SimWorker::SimWorker(QObject* parent)
    : QObject(parent)
{
}

void SimWorker::setInputs(const SimulationSettings& sim,
    const QVector<CreatureSettings>& creatures)
{
    m_sim = sim;
    m_creatures = creatures;
}

void SimWorker::requestStop()
{
    m_stopRequested.store(true, std::memory_order_relaxed);
}

void SimWorker::run()
{
    m_stopRequested.store(false, std::memory_order_relaxed);
    SimulationResult result = runSimulation(m_sim, m_creatures);
    emit finishedWithResult(result);
}

static void drawCircle(QByteArray& frame,
    int width,
    int height,
    int cx,
    int cy,
    int radius,
    const QColor& color)
{
    if (radius <= 0) {
        return;
    }

    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            if (x * x + y * y <= radius * radius) {
                const int px = cx + x;
                const int py = cy + y;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    const int idx = (py * width + px) * 3;
                    frame[idx] = static_cast<char>(color.red());
                    frame[idx + 1] = static_cast<char>(color.green());
                    frame[idx + 2] = static_cast<char>(color.blue());
                }
            }
        }
    }
}

static QByteArray generateFrame(const Environment& environment, int width, int height)
{
    QByteArray frame(width * height * 3, char(0));

    for (const auto* food : environment.foods) {
        drawCircle(frame,
            width,
            height,
            static_cast<int>(std::round(food->x())),
            static_cast<int>(std::round(food->y())),
            static_cast<int>(std::round(food->size())),
            QColor(255, 255, 255));
    }

    for (const auto* creature : environment.creatures) {
        drawCircle(frame,
            width,
            height,
            static_cast<int>(std::round(creature->x)),
            static_cast<int>(std::round(creature->y)),
            static_cast<int>(std::round(creature->size)),
            QColor(creature->colorR, creature->colorG, creature->colorB));
    }

    return frame;
}

SimulationResult SimWorker::runSimulation(const SimulationSettings& sim,
    const QVector<CreatureSettings>& creatures)
{
    SimulationResult out;
    out.datetime = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    out.status = "success";
    out.nodeType = "local";

    const int width = 1280;
    const int height = 720;
    const int fps = 30;

    const int binSize = std::max(1, static_cast<int>(std::ceil(sim.simLength / 80.0)));

    Environment environment(sim.foodRespawnBase,
        sim.foodRespawnMultiplier,
        sim.foodEnergy,
        width,
        height);
    environment.setupFood();
    environment.setupCreatures(creatures);

    struct SpeciesBinData {
        int count = 0;
        int births = 0;
        int deaths = 0;
    };

    QHash<QString, int> speciesIndex;
    QHash<QString, SpeciesBinData> speciesBin;
    for (const auto& creature : creatures) {
        if (!speciesIndex.contains(creature.speciesName)) {
            SpeciesSeries series;
            series.name = creature.speciesName;
            series.color = QColor(creature.colorR, creature.colorG, creature.colorB);
            out.species.push_back(series);
            speciesIndex.insert(creature.speciesName, out.species.size() - 1);
            speciesBin.insert(creature.speciesName, SpeciesBinData());
        }
    }

    double creatureCountBin = 0.0;
    double foodCountBin = 0.0;
    double birthCountBin = 0.0;
    double deathCountBin = 0.0;
    Tracking::DeathCause deathTypeCountBin{};

    int binCounter = 0;

    const QString outputPath = DataStore::outputVideoPath();
    out.videoFile = outputPath;

    QStringList args = {
        "-y",
        "-f", "rawvideo",
        "-pixel_format", "rgb24",
        "-video_size", QString("%1x%2").arg(width).arg(height),
        "-r", QString::number(fps),
        "-i", "pipe:0",
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        outputPath
    };

    QProcess ffmpeg;
    ffmpeg.start("ffmpeg", args, QIODevice::WriteOnly);
    if (!ffmpeg.waitForStarted()) {
        out.status = "failed";
        out.failureReason = "Failed to start ffmpeg process.";
        out.videoFile.clear();
        return out;
    }

    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < sim.simLength; ++i) {
        if (m_stopRequested.load(std::memory_order_relaxed)) {
            out.status = "cancelled";
            out.failureReason = "Simulation cancelled.";
            break;
        }

        Tracking tracking;
        environment.update(tracking);

        if (environment.creatures.empty()) {
            break;
        }

        creatureCountBin += environment.creatures.size();
        foodCountBin += environment.foods.size();
        birthCountBin += tracking.births.size();
        deathCountBin += tracking.deaths.size();
        deathTypeCountBin.age += tracking.deathCause.age;
        deathTypeCountBin.hunger += tracking.deathCause.hunger;
        deathTypeCountBin.predation += tracking.deathCause.predation;

        for (auto it = speciesIndex.constBegin(); it != speciesIndex.constEnd(); ++it) {
            const QString& speciesName = it.key();
            int speciesCount = 0;
            int speciesBirths = 0;
            int speciesDeaths = 0;

            for (const auto* creature : environment.creatures) {
                if (creature->speciesName == speciesName) {
                    speciesCount += 1;
                }
            }
            for (const auto& birth : tracking.births) {
                if (birth == speciesName) {
                    speciesBirths += 1;
                }
            }
            for (const auto& death : tracking.deaths) {
                if (death == speciesName) {
                    speciesDeaths += 1;
                }
            }

            SpeciesBinData& bin = speciesBin[speciesName];
            bin.count += speciesCount;
            bin.births += speciesBirths;
            bin.deaths += speciesDeaths;
        }

        binCounter += 1;

        if (binCounter == binSize || i == sim.simLength - 1) {
            const double divisor = static_cast<double>(std::max(1, binCounter));
            out.creatureCount.push_back(creatureCountBin / divisor);
            out.foodCount.push_back(foodCountBin / divisor);
            out.birthCount.push_back(birthCountBin);
            out.deathCount.push_back(deathCountBin);
            out.deathAge += deathTypeCountBin.age;
            out.deathHunger += deathTypeCountBin.hunger;
            out.deathPredation += deathTypeCountBin.predation;

            for (auto it = speciesIndex.constBegin(); it != speciesIndex.constEnd(); ++it) {
                const QString& speciesName = it.key();
                const int index = it.value();
                const SpeciesBinData bin = speciesBin.value(speciesName);
                out.species[index].count.push_back(bin.count / divisor);
                out.species[index].births.push_back(bin.births);
                out.species[index].deaths.push_back(bin.deaths);
                speciesBin[speciesName] = SpeciesBinData();
            }

            creatureCountBin = 0.0;
            foodCountBin = 0.0;
            birthCountBin = 0.0;
            deathCountBin = 0.0;
            deathTypeCountBin = Tracking::DeathCause();
            binCounter = 0;
        }

        const QByteArray frame = generateFrame(environment, width, height);
        const qint64 written = ffmpeg.write(frame);
        if (written == -1) {
            out.status = "failed";
            out.failureReason = "Failed to write frame to ffmpeg.";
            break;
        }
        if (written < frame.size()) {
            ffmpeg.waitForBytesWritten(-1);
        }
    }

    ffmpeg.closeWriteChannel();
    ffmpeg.waitForFinished(-1);

    if (ffmpeg.exitStatus() != QProcess::NormalExit || ffmpeg.exitCode() != 0) {
        out.status = "failed";
        out.failureReason = "ffmpeg exited with an error.";
    }

    out.duration = timer.elapsed() / 1000.0;
    out.computeCost = (0.096 / 3600.0) * out.duration;
    out.resultSize = 0.0;

    const QByteArray resultJson = DataStore::serializeResult(out);
    out.resultSize = resultJson.size() / 1024.0 / 1024.0;

    return out;
}

// ----------------------------
// MainWindow
// ----------------------------
MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("Creature Simulator");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    setStyleSheet(R"(
        QGroupBox {
            font-weight: 600;
            border: 1px solid #d0d0d0;
            border-radius: 8px;
            margin-top: 12px;
            background: #f3f3f3;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
        }
        QSpinBox, QDoubleSpinBox, QLineEdit, QComboBox {
            background: white;
            padding: 4px;
            border: 1px solid #cfcfcf;
            border-radius: 6px;
            min-height: 24px;
        }
        QPushButton {
            background: #0078ff;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 12px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: #006ae3;
        }
    )");

    auto* simBox = new QGroupBox("Simulation Settings");
    auto* simGrid = new QGridLayout(simBox);
    simGrid->setContentsMargins(16, 18, 16, 16);
    simGrid->setHorizontalSpacing(18);
    simGrid->setVerticalSpacing(12);

    simLength = new QSpinBox();
    simLength->setRange(1, 10'000'000);
    simLength->setSingleStep(60);
    simLength->setValue(5400);

    foodRespawnMultiplier = new QDoubleSpinBox();
    foodRespawnMultiplier->setRange(0.0, 1000.0);
    foodRespawnMultiplier->setDecimals(4);
    foodRespawnMultiplier->setSingleStep(0.1);
    foodRespawnMultiplier->setValue(1.0);

    foodRespawnBase = new QDoubleSpinBox();
    foodRespawnBase->setRange(0.0, 1000.0);
    foodRespawnBase->setDecimals(4);
    foodRespawnBase->setSingleStep(0.1);
    foodRespawnBase->setValue(1.0);

    foodEnergy = new QDoubleSpinBox();
    foodEnergy->setRange(0.0, 1'000'000.0);
    foodEnergy->setDecimals(3);
    foodEnergy->setSingleStep(1.0);
    foodEnergy->setValue(15.0);

    simGrid->addWidget(new QLabel("Simulation Length"), 0, 0);
    simGrid->addWidget(simLength, 0, 1);
    simGrid->addWidget(new QLabel("Food Respawn Multiplier"), 1, 0);
    simGrid->addWidget(foodRespawnMultiplier, 1, 1);
    simGrid->addWidget(new QLabel("Food Respawn Base"), 2, 0);
    simGrid->addWidget(foodRespawnBase, 2, 1);
    simGrid->addWidget(new QLabel("Energy per Food"), 3, 0);
    simGrid->addWidget(foodEnergy, 3, 1);

    root->addWidget(simBox);

    auto* creaturesBox = new QGroupBox("Creatures");
    auto* creaturesLayout = new QVBoxLayout(creaturesBox);
    creaturesLayout->setContentsMargins(12, 16, 12, 16);
    creaturesLayout->setSpacing(10);

    auto* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    auto* creatureContainer = new QWidget();
    creatureListLayout = new QVBoxLayout(creatureContainer);
    creatureListLayout->setSpacing(10);
    creatureListLayout->addStretch();
    scrollArea->setWidget(creatureContainer);

    creaturesLayout->addWidget(scrollArea);

    addCreatureBtn = new QPushButton("Add Creature");
    connect(addCreatureBtn, &QPushButton::clicked, this, &MainWindow::onAddCreature);
    creaturesLayout->addWidget(addCreatureBtn);

    root->addWidget(creaturesBox, 1);

    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);
    saveCreaturesBtn = new QPushButton("Save Creatures");
    loadCreaturesBtn = new QPushButton("Load Creatures");
    btnRow->addWidget(saveCreaturesBtn);
    btnRow->addWidget(loadCreaturesBtn);
    root->addLayout(btnRow);

    connect(saveCreaturesBtn, &QPushButton::clicked, this, &MainWindow::onSaveCreatures);
    connect(loadCreaturesBtn, &QPushButton::clicked, this, &MainWindow::onLoadCreatures);

    startBtn = new QPushButton("Start Simulation");
    startBtn->setMinimumHeight(46);
    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStartSimulation);
    root->addWidget(startBtn);

    addCreaturePanel();
    resize(1200, 900);
}

MainWindow::~MainWindow()
{
    if (simWorker) {
        simWorker->requestStop();
    }

    if (simThread) {
        simThread->quit();
        simThread->wait(1500);
    }
}

void MainWindow::addCreaturePanel(const CreatureSettings& settings)
{
    auto* panel = new CreaturePanel(settings);
    creaturePanels.push_back(panel);
    creatureListLayout->insertWidget(creatureListLayout->count() - 1, panel);
}

void MainWindow::clearCreaturePanels()
{
    for (auto* panel : creaturePanels) {
        panel->deleteLater();
    }
    creaturePanels.clear();
}

SimulationSettings MainWindow::collectSimulationSettings() const
{
    SimulationSettings s;
    s.simLength = simLength->value();
    s.foodRespawnMultiplier = foodRespawnMultiplier->value();
    s.foodRespawnBase = foodRespawnBase->value();
    s.foodEnergy = foodEnergy->value();
    return s;
}

QVector<CreatureSettings> MainWindow::collectCreatureSettings() const
{
    QVector<CreatureSettings> creatures;
    for (auto* panel : creaturePanels) {
        creatures.push_back(panel->collect());
    }
    return creatures;
}

void MainWindow::setSimulationSettings(const SimulationSettings& settings)
{
    simLength->setValue(settings.simLength);
    foodRespawnMultiplier->setValue(settings.foodRespawnMultiplier);
    foodRespawnBase->setValue(settings.foodRespawnBase);
    foodEnergy->setValue(settings.foodEnergy);
}

void MainWindow::setCreatureSettings(const QVector<CreatureSettings>& creatures)
{
    clearCreaturePanels();
    for (const auto& creature : creatures) {
        addCreaturePanel(creature);
    }
}

void MainWindow::onAddCreature()
{
    addCreaturePanel();
}

void MainWindow::onSaveCreatures()
{
    SimulationSettings sim = collectSimulationSettings();
    QVector<CreatureSettings> creatures = collectCreatureSettings();
    QString error;
    if (!DataStore::saveCreatures(sim, creatures, &error)) {
        QMessageBox::warning(this, "Save Failed", error);
        return;
    }

    QMessageBox::information(this, "Saved", "Creature settings saved.");
}

void MainWindow::onLoadCreatures()
{
    SimulationSettings sim;
    QVector<CreatureSettings> creatures;
    QString error;
    if (!DataStore::loadCreatures(sim, creatures, &error)) {
        QMessageBox::warning(this, "Load Failed", error);
        return;
    }

    setSimulationSettings(sim);
    setCreatureSettings(creatures);
}

void MainWindow::onStartSimulation()
{
    if (simThread && simThread->isRunning()) {
        return;
    }

    SimulationSettings sim = collectSimulationSettings();
    QVector<CreatureSettings> creatures = collectCreatureSettings();
    if (creatures.isEmpty()) {
        QMessageBox::warning(this, "Missing Creatures", "Add at least one creature.");
        return;
    }

    simThread = new QThread(this);
    simWorker = new SimWorker();
    simWorker->setInputs(sim, creatures);
    simWorker->moveToThread(simThread);

    connect(simThread, &QThread::started, simWorker, &SimWorker::run);
    connect(simWorker, &SimWorker::finishedWithResult, this, &MainWindow::onSimFinished);
    connect(simWorker, &SimWorker::finishedWithResult, simThread, &QThread::quit);
    connect(simThread, &QThread::finished, simWorker, &QObject::deleteLater);
    connect(simThread, &QThread::finished, simThread, &QObject::deleteLater);

    startBtn->setEnabled(false);
    simThread->start();
}

void MainWindow::onSimFinished(const SimulationResult& result)
{
    startBtn->setEnabled(true);
    simThread = nullptr;
    simWorker = nullptr;

    if (!resultsWindow) {
        resultsWindow = new ResultsWindow();
        connect(resultsWindow, &ResultsWindow::backRequested, this, &MainWindow::onBackFromResults);
    }

    resultsWindow->setResult(result);
    resultsWindow->show();
    hide();
}

void MainWindow::onBackFromResults()
{
    if (resultsWindow) {
        resultsWindow->hide();
    }

    show();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (simWorker) {
        simWorker->requestStop();
    }

    if (simThread) {
        simThread->quit();
        simThread->wait(1500);
    }

    event->accept();
}
