#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCommandLineParser parser;

    QCommandLineOption fileOption(QStringLiteral("file"));
    // This option wants a value, so set value name otherwise the value will
    // not be parsed
    fileOption.setValueName(QStringLiteral("file"));
    parser.addOption(fileOption);

    parser.process(a);
    QString dumpFile = parser.value(fileOption);
    if (dumpFile.isEmpty()) {
        return 1;
    }

    MainWindow w;
    w.setDumpFilePath(dumpFile);
    w.show();

    return a.exec();
}
