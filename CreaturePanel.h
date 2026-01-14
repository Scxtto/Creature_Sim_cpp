#pragma once

#include <QWidget>
#include "MainWindow.h"

class QToolButton;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;

class CreaturePanel : public QWidget {
    Q_OBJECT
public:
    explicit CreaturePanel(const CreatureSettings& settings, QWidget* parent = nullptr);

    CreatureSettings collect() const;
    void setFrom(const CreatureSettings& settings);

private slots:
    void onToggle(bool checked);
    void onNameChanged(const QString& text);

private:
    QWidget* makeLabeledField(const QString& label, QWidget* input);

    QToolButton* toggleButton = nullptr;
    QWidget* contentWidget = nullptr;

    QLineEdit* speciesName = nullptr;
    QDoubleSpinBox* baseSpeed = nullptr;
    QDoubleSpinBox* speedMultiplier = nullptr;
    QSpinBox* health = nullptr;
    QSpinBox* age = nullptr;
    QSpinBox* ageCap = nullptr;
    QDoubleSpinBox* ageRate = nullptr;
    QSpinBox* initialPopulation = nullptr;

    QSpinBox* initialFullness = nullptr;
    QSpinBox* fullnessCap = nullptr;
    QDoubleSpinBox* metabolicBaseRate = nullptr;
    QDoubleSpinBox* metabolicRate = nullptr;
    QDoubleSpinBox* energyStorageRate = nullptr;
    QDoubleSpinBox* reserveEnergy = nullptr;
    QComboBox* dietType = nullptr;
    QComboBox* dietPreference = nullptr;

    QSpinBox* reproductionCost = nullptr;
    QSpinBox* matingHungerThreshold = nullptr;
    QSpinBox* reproductionCooldown = nullptr;
    QSpinBox* litterSize = nullptr;
    QDoubleSpinBox* mutationFactor = nullptr;

    QSpinBox* colorR = nullptr;
    QSpinBox* colorG = nullptr;
    QSpinBox* colorB = nullptr;
    QDoubleSpinBox* size = nullptr;

    QDoubleSpinBox* skittishMultiplierBase = nullptr;
    QDoubleSpinBox* skittishMultiplierScared = nullptr;
    QDoubleSpinBox* attackPower = nullptr;
    QDoubleSpinBox* defencePower = nullptr;
    QDoubleSpinBox* fleeExhaustion = nullptr;
    QDoubleSpinBox* fleeRecoveryFactor = nullptr;
};
