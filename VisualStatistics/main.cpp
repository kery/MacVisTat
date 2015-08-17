#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("fusion")));

    QApplication a(argc, argv);

    // Call these functions so that we can use default constructor of
    // QSettings
    QCoreApplication::setOrganizationName("Nokia");
    QCoreApplication::setApplicationName("VisualStatistics");

    MainWindow w;
    w.show();

    return a.exec();
}
