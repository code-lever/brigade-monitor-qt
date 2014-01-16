#include "mainwindow.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("codelever");
    QCoreApplication::setApplicationName("brigade-monitor-qt");

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    MainWindow window;

    return app.exec();
}
