#include "ResultsWindow.h"
#include "DataStore.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QButtonGroup>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QUrl>
#include <QTimer>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>

ResultsWindow::ResultsWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("Simulation Results");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    auto* topRow = new QHBoxLayout();
    backBtn = new QPushButton("Back to Setup");
    connect(backBtn, &QPushButton::clicked, this, &ResultsWindow::onBack);
    statusLabel = new QLabel("Ready");
    topRow->addWidget(backBtn);
    topRow->addStretch();
    topRow->addWidget(statusLabel);
    root->addLayout(topRow);

    auto* videoBox = new QVBoxLayout();
    videoWidget = new QVideoWidget();
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    player->setVideoOutput(videoWidget);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &ResultsWindow::onMediaStatusChanged);

    playBtn = new QPushButton("Play");
    connect(playBtn, &QPushButton::clicked, this, &ResultsWindow::onTogglePlayback);
    playBtn->setEnabled(false);
    videoBox->addWidget(videoWidget, 1);

    auto* controlsRow = new QHBoxLayout();
    controlsRow->setSpacing(10);
    controlsRow->addWidget(playBtn);

    fpsButtonGroup = new QButtonGroup(this);
    fpsButtonGroup->setExclusive(true);

    const char* fpsStyle = R"(
        QPushButton {
            background: #d8d8d8;
            color: #555555;
            border: none;
            border-radius: 8px;
            padding: 6px 10px;
            font-weight: 600;
        }
        QPushButton:checked {
            background: #0078ff;
            color: white;
        }
        QPushButton:disabled {
            background: #e6e6e6;
            color: #9a9a9a;
        }
    )";

    auto makeFpsButton = [this, fpsStyle](const QString& label, int fps) {
        auto* button = new QPushButton(label);
        button->setCheckable(true);
        button->setStyleSheet(fpsStyle);
        fpsButtonGroup->addButton(button, fps);
        return button;
    };

    fps30Btn = makeFpsButton("30", 30);
    fps60Btn = makeFpsButton("60", 60);
    fps120Btn = makeFpsButton("120", 120);
    fps240Btn = makeFpsButton("240", 240);
    setFpsControlsEnabled(false);

    auto* fpsRow = new QHBoxLayout();
    fpsRow->setSpacing(6);
    fpsRow->addWidget(new QLabel("FPS"));
    fpsRow->addWidget(fps30Btn);
    fpsRow->addWidget(fps60Btn);
    fpsRow->addWidget(fps120Btn);
    fpsRow->addWidget(fps240Btn);

    controlsRow->addStretch();
    controlsRow->addLayout(fpsRow);
    videoBox->addLayout(controlsRow);

    connect(fpsButtonGroup, &QButtonGroup::idClicked, this, [this](int fps) {
        applyPlaybackFps(fps);
    });
    root->addLayout(videoBox, 2);

    chartsContainer = new QWidget();
    auto* grid = new QGridLayout(chartsContainer);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(12);
    root->addWidget(chartsContainer, 3);
}

void ResultsWindow::setResult(const SimulationResult& result)
{
    statusLabel->setText(QString("Status: %1").arg(result.status));
    playBtn->setText("Play");
    playBtn->setEnabled(false);
    player->stop();
    setFpsControlsEnabled(false);
    if (fps30Btn) {
        fps30Btn->setChecked(true);
        applyPlaybackFps(30);
    }

    pendingResult = result;
    hasPendingResult = true;
    chartsBuilt = false;

    if (!result.videoFile.isEmpty()) {
        player->setSource(QUrl::fromLocalFile(result.videoFile));
        statusLabel->setText(QString("Status: %1 (loading video)").arg(result.status));
    }

    QString error;
    DataStore::saveResult(result, &error);
    if (result.videoFile.isEmpty()) {
        buildCharts(result);
        chartsBuilt = true;
    }
}

void ResultsWindow::buildCharts(const SimulationResult& result)
{
    auto* grid = qobject_cast<QGridLayout*>(chartsContainer->layout());
    while (QLayoutItem* item = grid->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    auto makeLineChart = [](const QString& title, const QVector<double>& values, const QColor& color) {
        auto* series = new QLineSeries();
        for (int i = 0; i < values.size(); ++i) {
            series->append(i + 1, values[i]);
        }
        series->setColor(color);

        auto* chart = new QChart();
        chart->addSeries(series);
        chart->setTitle(title);
        chart->legend()->hide();

        auto* axisX = new QValueAxis();
        axisX->setTitleText("Bin");
        axisX->setLabelFormat("%d");
        const int maxBins = values.isEmpty() ? 1 : values.size();
        axisX->setRange(1, maxBins);

        auto* axisY = new QValueAxis();
        axisY->setTitleText("Value");

        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisX);
        series->attachAxis(axisY);

        auto* view = new QChartView(chart);
        view->setRenderHint(QPainter::Antialiasing);
        return view;
    };

    auto* creatureCountChart = makeLineChart("Total Creature Count", result.creatureCount, QColor(75, 192, 192));
    grid->addWidget(creatureCountChart, 0, 0);

    auto* foodChart = makeLineChart("Food Count", result.foodCount, QColor(153, 102, 255));
    grid->addWidget(foodChart, 0, 1);

    auto* birthChart = makeLineChart("Birth Count", result.birthCount, QColor(0, 123, 255));
    grid->addWidget(birthChart, 1, 0);

    auto* deathChart = makeLineChart("Death Count", result.deathCount, QColor(255, 99, 132));
    grid->addWidget(deathChart, 1, 1);

    // Species counts
    auto* speciesChart = new QChart();
    speciesChart->setTitle("Species Count");
    auto* axisX = new QValueAxis();
    axisX->setTitleText("Bin");
    axisX->setLabelFormat("%d");
    const int maxBins = result.creatureCount.isEmpty() ? 1 : result.creatureCount.size();
    axisX->setRange(1, maxBins);
    auto* axisY = new QValueAxis();
    axisY->setTitleText("Value");
    speciesChart->addAxis(axisX, Qt::AlignBottom);
    speciesChart->addAxis(axisY, Qt::AlignLeft);

    for (const auto& seriesData : result.species) {
        auto* series = new QLineSeries();
        series->setName(seriesData.name);
        for (int i = 0; i < seriesData.count.size(); ++i) {
            series->append(i + 1, seriesData.count[i]);
        }
        series->setColor(seriesData.color);
        speciesChart->addSeries(series);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }
    speciesChart->legend()->setVisible(true);
    speciesChart->legend()->setAlignment(Qt::AlignTop);
    auto* speciesView = new QChartView(speciesChart);
    speciesView->setRenderHint(QPainter::Antialiasing);
    grid->addWidget(speciesView, 2, 0);

    // Death breakdown pie
    auto* pieSeries = new QPieSeries();
    pieSeries->append("Age", result.deathAge);
    pieSeries->append("Starvation", result.deathHunger);
    pieSeries->append("Predation", result.deathPredation);
    auto* pieChart = new QChart();
    pieChart->addSeries(pieSeries);
    pieChart->setTitle("Death Breakdown");
    pieChart->legend()->setAlignment(Qt::AlignTop);
    auto* pieView = new QChartView(pieChart);
    pieView->setRenderHint(QPainter::Antialiasing);
    grid->addWidget(pieView, 2, 1);
}

void ResultsWindow::applyPlaybackFps(int fps)
{
    constexpr double baseFps = 30.0;
    const double rate = static_cast<double>(fps) / baseFps;
    player->setPlaybackRate(rate);
}

void ResultsWindow::setFpsControlsEnabled(bool enabled)
{
    if (fps30Btn) {
        fps30Btn->setEnabled(enabled);
    }
    if (fps60Btn) {
        fps60Btn->setEnabled(enabled);
    }
    if (fps120Btn) {
        fps120Btn->setEnabled(enabled);
    }
    if (fps240Btn) {
        fps240Btn->setEnabled(enabled);
    }
}

void ResultsWindow::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (!hasPendingResult) {
        return;
    }

    if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia) {
        playBtn->setEnabled(true);
        setFpsControlsEnabled(true);
        statusLabel->setText(QString("Status: %1").arg(pendingResult.status));
        if (!chartsBuilt) {
            chartsBuilt = true;
            QTimer::singleShot(0, this, [this]() { buildCharts(pendingResult); });
        }
        return;
    }

    if (status == QMediaPlayer::InvalidMedia) {
        playBtn->setEnabled(false);
        setFpsControlsEnabled(false);
        statusLabel->setText(QString("Status: %1 (video load failed)").arg(pendingResult.status));
        if (!chartsBuilt) {
            chartsBuilt = true;
            buildCharts(pendingResult);
        }
        return;
    }

    if (status == QMediaPlayer::LoadingMedia) {
        statusLabel->setText(QString("Status: %1 (loading video)").arg(pendingResult.status));
    }
}

void ResultsWindow::onBack()
{
    player->stop();
    emit backRequested();
}

void ResultsWindow::onTogglePlayback()
{
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        player->pause();
        playBtn->setText("Play");
    } else {
        player->play();
        playBtn->setText("Pause");
    }
}
