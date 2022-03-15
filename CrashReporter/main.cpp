#include "MainWindow.h"
#include <QApplication>
#include <QFileInfo>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCommandLineParser parser;
    QCommandLineOption fileOption("file");
    QCommandLineOption urlOption("url");
    QCommandLineOption verOption("version");

    // These options want a value, so set value name. Otherwise the value will
    // not be parsed
    fileOption.setValueName("file");
    urlOption.setValueName("url");
    verOption.setValueName("version");

    parser.addOption(fileOption);
    parser.addOption(urlOption);
    parser.addOption(verOption);

    parser.process(a);
    QString dumpFile = parser.value(fileOption);
    if (!QFileInfo::exists(dumpFile)) {
        return 1;
    }

    MainWindow mainWnd(dumpFile, parser.value(urlOption), parser.value(verOption));
    mainWnd.show();
    return a.exec();
}
