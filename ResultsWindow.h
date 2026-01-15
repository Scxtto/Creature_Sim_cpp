#pragma once

#include <QMediaPlayer>
#include <QWidget>
#include "MainWindow.h"

class QMediaPlayer;
class QVideoWidget;
class QAudioOutput;
class QPushButton;
class QLabel;
class QButtonGroup;

class ResultsWindow : public QWidget {
    Q_OBJECT
public:
    explicit ResultsWindow(QWidget* parent = nullptr);

    void setResult(const SimulationResult& result);

signals:
    void backRequested();

private slots:
    void onBack();
    void onTogglePlayback();
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    void buildCharts(const SimulationResult& result);
    void applyPlaybackFps(int fps);
    void setFpsControlsEnabled(bool enabled);

    QPushButton* backBtn = nullptr;
    QLabel* statusLabel = nullptr;
    QMediaPlayer* player = nullptr;
    QAudioOutput* audioOutput = nullptr;
    QVideoWidget* videoWidget = nullptr;
    QPushButton* playBtn = nullptr;
    QButtonGroup* fpsButtonGroup = nullptr;
    QPushButton* fps30Btn = nullptr;
    QPushButton* fps60Btn = nullptr;
    QPushButton* fps120Btn = nullptr;
    QPushButton* fps240Btn = nullptr;

    QWidget* chartsContainer = nullptr;

    SimulationResult pendingResult;
    bool hasPendingResult = false;
    bool chartsBuilt = false;
};
