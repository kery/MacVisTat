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

    // These options want a value, so set value name. Otherwise the value will
    // not be parsed
    fileOption.setValueName("file");
    urlOption.setValueName("url");

    parser.addOption(fileOption);
    parser.addOption(urlOption);

    parser.process(a);
    QString dumpFile = parser.value(fileOption);
    if (!QFileInfo::exists(dumpFile)) {
        return 1;
    }

    MainWindow mainWnd(dumpFile, parser.value(urlOption));
    mainWnd.show();
    return a.exec();
}
