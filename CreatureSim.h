#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CreatureSim.h"

class CreatureSim : public QMainWindow
{
    Q_OBJECT

public:
    CreatureSim(QWidget *parent = nullptr);
    ~CreatureSim();

private:
    Ui::CreatureSimClass ui;
};

