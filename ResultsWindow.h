#pragma once

#include <QWidget>
#include "MainWindow.h"

class QMediaPlayer;
class QVideoWidget;
class QAudioOutput;
class QPushButton;
class QLabel;

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

private:
    void buildCharts(const SimulationResult& result);

    QPushButton* backBtn = nullptr;
    QLabel* statusLabel = nullptr;
    QMediaPlayer* player = nullptr;
    QAudioOutput* audioOutput = nullptr;
    QVideoWidget* videoWidget = nullptr;
    QPushButton* playBtn = nullptr;

    QWidget* chartsContainer = nullptr;
};
