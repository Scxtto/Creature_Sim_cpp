#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    qRegisterMetaType<SimulationResult>("SimulationResult");
    MainWindow w;
    w.show();
    return app.exec();
}
