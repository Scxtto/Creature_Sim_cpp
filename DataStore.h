#pragma once

#include <QString>
#include <QVector>
#include <QByteArray>
#include "MainWindow.h"

class DataStore {
public:
    static QString dataDir();
    static QString outputDir();
    static QString outputVideoPath();

    static bool saveCreatures(const SimulationSettings& sim,
                              const QVector<CreatureSettings>& creatures,
                              QString* error);
    static bool loadCreatures(SimulationSettings& sim,
                              QVector<CreatureSettings>& creatures,
                              QString* error);

    static bool saveResult(const SimulationResult& result, QString* error);
    static QByteArray serializeResult(const SimulationResult& result);
};
