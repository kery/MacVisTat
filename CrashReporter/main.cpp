#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCommandLineParser parser;

    QCommandLineOption fileOption(QStringLiteral("file"));
    QCommandLineOption versionOption(QStringLiteral("version"));
    // This option wants a value, so set value name otherwise the value will
    // not be parsed
    fileOption.setValueName(QStringLiteral("file"));
    versionOption.setValueName(QStringLiteral("version"));
    parser.addOption(fileOption);
    parser.addOption(versionOption);

    parser.process(a);
    QString dumpFile = parser.value(fileOption);
    QString version = parser.value(versionOption);
    if (dumpFile.isEmpty()) {
        return 1;
    }

    MainWindow w(dumpFile, version);
    w.show();

    return a.exec();
}
