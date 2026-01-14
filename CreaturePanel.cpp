#include "CreaturePanel.h"

#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>

static QSpinBox* makeIntSpin(int value, int minV, int maxV, int step)
{
    auto* w = new QSpinBox();
    w->setRange(minV, maxV);
    w->setSingleStep(step);
    w->setValue(value);
    return w;
}

static QDoubleSpinBox* makeFloatSpin(double value,
    double minV,
    double maxV,
    double step,
    int decimals)
{
    auto* w = new QDoubleSpinBox();
    w->setRange(minV, maxV);
    w->setSingleStep(step);
    w->setDecimals(decimals);
    w->setValue(value);
    return w;
}

CreaturePanel::CreaturePanel(const CreatureSettings& settings, QWidget* parent)
    : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(6);

    toggleButton = new QToolButton();
    toggleButton->setCheckable(true);
    toggleButton->setChecked(true);
    toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toggleButton->setArrowType(Qt::DownArrow);
    toggleButton->setText(settings.speciesName);
    connect(toggleButton, &QToolButton::toggled, this, &CreaturePanel::onToggle);
    root->addWidget(toggleButton);

    contentWidget = new QWidget();
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(8, 8, 8, 8);
    contentLayout->setSpacing(10);

    // General Settings
    auto* generalBox = new QGroupBox("General");
    auto* generalGrid = new QGridLayout(generalBox);
    generalGrid->setContentsMargins(12, 12, 12, 12);
    generalGrid->setHorizontalSpacing(12);
    generalGrid->setVerticalSpacing(10);

    speciesName = new QLineEdit(settings.speciesName);
    connect(speciesName, &QLineEdit::textChanged, this, &CreaturePanel::onNameChanged);
    baseSpeed = makeFloatSpin(settings.baseSpeed, 0.0, 1000.0, 0.1, 4);
    speedMultiplier = makeFloatSpin(settings.speedMultiplier, 0.0, 1000.0, 0.1, 4);
    health = makeIntSpin(settings.health, 0, 1'000'000, 1);
    age = makeIntSpin(settings.age, 0, 1'000'000, 1);
    ageCap = makeIntSpin(settings.ageCap, 0, 1'000'000, 1);
    ageRate = makeFloatSpin(settings.ageRate, 0.0, 10.0, 0.01, 6);
    initialPopulation = makeIntSpin(settings.initialPopulation, 0, 1'000'000, 1);

    generalGrid->addWidget(makeLabeledField("Species Name", speciesName), 0, 0);
    generalGrid->addWidget(makeLabeledField("Base Speed", baseSpeed), 0, 1);
    generalGrid->addWidget(makeLabeledField("Speed Multiplier", speedMultiplier), 1, 0);
    generalGrid->addWidget(makeLabeledField("Max Health", health), 1, 1);
    generalGrid->addWidget(makeLabeledField("Initial Age", age), 2, 0);
    generalGrid->addWidget(makeLabeledField("Age Cap", ageCap), 2, 1);
    generalGrid->addWidget(makeLabeledField("Rate of Aging", ageRate), 3, 0);
    generalGrid->addWidget(makeLabeledField("Initial Population", initialPopulation), 3, 1);

    contentLayout->addWidget(generalBox);

    // Diet Settings
    auto* dietBox = new QGroupBox("Diet");
    auto* dietGrid = new QGridLayout(dietBox);
    dietGrid->setContentsMargins(12, 12, 12, 12);
    dietGrid->setHorizontalSpacing(12);
    dietGrid->setVerticalSpacing(10);

    initialFullness = makeIntSpin(settings.initialFullness, 0, 1'000'000, 1);
    fullnessCap = makeIntSpin(settings.fullnessCap, 0, 1'000'000, 1);
    metabolicBaseRate = makeFloatSpin(settings.metabolicBaseRate, 0.0, 1000.0, 0.01, 6);
    metabolicRate = makeFloatSpin(settings.metabolicRate, 0.0, 1000.0, 0.1, 4);
    energyStorageRate = makeFloatSpin(settings.energyStorageRate, 0.0, 1000.0, 0.01, 6);
    reserveEnergy = makeFloatSpin(settings.reserveEnergy, 0.0, 1'000'000.0, 1.0, 3);

    dietType = new QComboBox();
    dietType->addItems({ "herbivore", "carnivore", "omnivore" });
    dietType->setCurrentText(settings.dietType);

    dietPreference = new QComboBox();
    dietPreference->addItems({ "Plants", "Meat", "Any" });
    dietPreference->setCurrentText(settings.dietPreference);

    dietGrid->addWidget(makeLabeledField("Initial Fullness", initialFullness), 0, 0);
    dietGrid->addWidget(makeLabeledField("Fullness Cap", fullnessCap), 0, 1);
    dietGrid->addWidget(makeLabeledField("Metabolic Base Rate", metabolicBaseRate), 1, 0);
    dietGrid->addWidget(makeLabeledField("Metabolism Multiplier", metabolicRate), 1, 1);
    dietGrid->addWidget(makeLabeledField("Energy Storage Rate", energyStorageRate), 2, 0);
    dietGrid->addWidget(makeLabeledField("Initial Reserve Energy", reserveEnergy), 2, 1);
    dietGrid->addWidget(makeLabeledField("Diet Type", dietType), 3, 0);
    dietGrid->addWidget(makeLabeledField("Diet Preference", dietPreference), 3, 1);

    contentLayout->addWidget(dietBox);

    // Reproduction
    auto* reproBox = new QGroupBox("Reproduction");
    auto* reproGrid = new QGridLayout(reproBox);
    reproGrid->setContentsMargins(12, 12, 12, 12);
    reproGrid->setHorizontalSpacing(12);
    reproGrid->setVerticalSpacing(10);

    reproductionCost = makeIntSpin(settings.reproductionCost, 0, 1'000'000, 1);
    matingHungerThreshold = makeIntSpin(settings.matingHungerThreshold, 0, 1'000'000, 1);
    reproductionCooldown = makeIntSpin(settings.reproductionCooldown, 0, 1'000'000, 1);
    litterSize = makeIntSpin(settings.litterSize, 1, 1'000'000, 1);
    mutationFactor = makeFloatSpin(settings.mutationFactor, 0.0, 1'000'000.0, 0.01, 6);

    reproGrid->addWidget(makeLabeledField("Reproduction Cost", reproductionCost), 0, 0);
    reproGrid->addWidget(makeLabeledField("Hunger Threshold", matingHungerThreshold), 0, 1);
    reproGrid->addWidget(makeLabeledField("Reproduction Cooldown", reproductionCooldown), 1, 0);
    reproGrid->addWidget(makeLabeledField("Litter Size", litterSize), 1, 1);
    reproGrid->addWidget(makeLabeledField("Mutation Factor", mutationFactor), 2, 0);

    contentLayout->addWidget(reproBox);

    // Behavior
    auto* behaviorBox = new QGroupBox("Behavior");
    auto* behaviorGrid = new QGridLayout(behaviorBox);
    behaviorGrid->setContentsMargins(12, 12, 12, 12);
    behaviorGrid->setHorizontalSpacing(12);
    behaviorGrid->setVerticalSpacing(10);

    skittishMultiplierBase = makeFloatSpin(settings.skittishMultiplierBase, 0.0, 1'000'000.0, 0.1, 4);
    skittishMultiplierScared = makeFloatSpin(settings.skittishMultiplierScared, 0.0, 1'000'000.0, 0.1, 4);
    attackPower = makeFloatSpin(settings.attackPower, 0.0, 1'000'000.0, 1.0, 3);
    defencePower = makeFloatSpin(settings.defencePower, 0.0, 1'000'000.0, 1.0, 3);
    fleeExhaustion = makeFloatSpin(settings.fleeExhaustion, 0.0, 1'000'000.0, 0.01, 6);
    fleeRecoveryFactor = makeFloatSpin(settings.fleeRecoveryFactor, 0.0, 1'000'000.0, 0.1, 4);

    behaviorGrid->addWidget(makeLabeledField("Skittish (Base)", skittishMultiplierBase), 0, 0);
    behaviorGrid->addWidget(makeLabeledField("Skittish (Scared)", skittishMultiplierScared), 0, 1);
    behaviorGrid->addWidget(makeLabeledField("Attack Power", attackPower), 1, 0);
    behaviorGrid->addWidget(makeLabeledField("Defense Power", defencePower), 1, 1);
    behaviorGrid->addWidget(makeLabeledField("Flee Exhaustion", fleeExhaustion), 2, 0);
    behaviorGrid->addWidget(makeLabeledField("Flee Recovery", fleeRecoveryFactor), 2, 1);

    contentLayout->addWidget(behaviorBox);

    // Appearance
    auto* appearanceBox = new QGroupBox("Appearance");
    auto* appearanceGrid = new QGridLayout(appearanceBox);
    appearanceGrid->setContentsMargins(12, 12, 12, 12);
    appearanceGrid->setHorizontalSpacing(12);
    appearanceGrid->setVerticalSpacing(10);

    colorR = makeIntSpin(settings.colorR, 0, 255, 1);
    colorG = makeIntSpin(settings.colorG, 0, 255, 1);
    colorB = makeIntSpin(settings.colorB, 0, 255, 1);
    size = makeFloatSpin(settings.size, 1.0, 1000.0, 1.0, 3);

    appearanceGrid->addWidget(makeLabeledField("Color (R)", colorR), 0, 0);
    appearanceGrid->addWidget(makeLabeledField("Color (G)", colorG), 0, 1);
    appearanceGrid->addWidget(makeLabeledField("Color (B)", colorB), 1, 0);
    appearanceGrid->addWidget(makeLabeledField("Size", size), 1, 1);

    contentLayout->addWidget(appearanceBox);
    root->addWidget(contentWidget);
}

QWidget* CreaturePanel::makeLabeledField(const QString& label, QWidget* input)
{
    auto* wrap = new QWidget();
    auto* layout = new QVBoxLayout(wrap);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto* lbl = new QLabel(label);
    layout->addWidget(lbl);
    layout->addWidget(input);
    return wrap;
}

void CreaturePanel::onToggle(bool checked)
{
    contentWidget->setVisible(checked);
    toggleButton->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
}

void CreaturePanel::onNameChanged(const QString& text)
{
    toggleButton->setText(text.isEmpty() ? "Creature" : text);
}

CreatureSettings CreaturePanel::collect() const
{
    CreatureSettings c;
    c.speciesName = speciesName->text();
    c.baseSpeed = baseSpeed->value();
    c.speedMultiplier = speedMultiplier->value();
    c.health = health->value();
    c.age = age->value();
    c.ageCap = ageCap->value();
    c.ageRate = ageRate->value();
    c.initialPopulation = initialPopulation->value();

    c.initialFullness = initialFullness->value();
    c.fullnessCap = fullnessCap->value();
    c.metabolicBaseRate = metabolicBaseRate->value();
    c.metabolicRate = metabolicRate->value();
    c.energyStorageRate = energyStorageRate->value();
    c.reserveEnergy = reserveEnergy->value();

    c.dietType = dietType->currentText();
    c.dietPreference = dietPreference->currentText();

    c.reproductionCost = reproductionCost->value();
    c.matingHungerThreshold = matingHungerThreshold->value();
    c.reproductionCooldown = reproductionCooldown->value();
    c.litterSize = litterSize->value();
    c.mutationFactor = mutationFactor->value();

    c.colorR = colorR->value();
    c.colorG = colorG->value();
    c.colorB = colorB->value();
    c.size = size->value();

    c.skittishMultiplierBase = skittishMultiplierBase->value();
    c.skittishMultiplierScared = skittishMultiplierScared->value();
    c.attackPower = attackPower->value();
    c.defencePower = defencePower->value();
    c.fleeExhaustion = fleeExhaustion->value();
    c.fleeRecoveryFactor = fleeRecoveryFactor->value();
    return c;
}

void CreaturePanel::setFrom(const CreatureSettings& settings)
{
    speciesName->setText(settings.speciesName);
    baseSpeed->setValue(settings.baseSpeed);
    speedMultiplier->setValue(settings.speedMultiplier);
    health->setValue(settings.health);
    age->setValue(settings.age);
    ageCap->setValue(settings.ageCap);
    ageRate->setValue(settings.ageRate);
    initialPopulation->setValue(settings.initialPopulation);

    initialFullness->setValue(settings.initialFullness);
    fullnessCap->setValue(settings.fullnessCap);
    metabolicBaseRate->setValue(settings.metabolicBaseRate);
    metabolicRate->setValue(settings.metabolicRate);
    energyStorageRate->setValue(settings.energyStorageRate);
    reserveEnergy->setValue(settings.reserveEnergy);

    dietType->setCurrentText(settings.dietType);
    dietPreference->setCurrentText(settings.dietPreference);

    reproductionCost->setValue(settings.reproductionCost);
    matingHungerThreshold->setValue(settings.matingHungerThreshold);
    reproductionCooldown->setValue(settings.reproductionCooldown);
    litterSize->setValue(settings.litterSize);
    mutationFactor->setValue(settings.mutationFactor);

    colorR->setValue(settings.colorR);
    colorG->setValue(settings.colorG);
    colorB->setValue(settings.colorB);
    size->setValue(settings.size);

    skittishMultiplierBase->setValue(settings.skittishMultiplierBase);
    skittishMultiplierScared->setValue(settings.skittishMultiplierScared);
    attackPower->setValue(settings.attackPower);
    defencePower->setValue(settings.defencePower);
    fleeExhaustion->setValue(settings.fleeExhaustion);
    fleeRecoveryFactor->setValue(settings.fleeRecoveryFactor);
}
