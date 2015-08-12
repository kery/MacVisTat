#include "mainwindow.h"
#include "parsedataworker.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Call these functions so that we can use default constructor of
    // QSettings
    QCoreApplication::setOrganizationName("Nokia");
    QCoreApplication::setApplicationName("VisualIntStat");

    // Call this function so that the following types can be used in
    // queued connection between threads
    qRegisterMetaType<ParseDataParam>("ParseDataParam");
    qRegisterMetaType<ParsedResult>("ParsedResult");

    MainWindow w;
    w.show();

    return a.exec();
}
