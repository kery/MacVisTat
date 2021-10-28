#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCommandLineParser parser;

    QCommandLineOption fileOption(QStringLiteral("file"));
    QCommandLineOption versionOption(QStringLiteral("version"));
    QCommandLineOption ftpDirOption(QStringLiteral("ftpdir"));
    QCommandLineOption ftpUserOption(QStringLiteral("ftpuser"));
    QCommandLineOption ftpPwdOption(QStringLiteral("ftppwd"));
    // This option wants a value, so set value name otherwise the value will
    // not be parsed
    fileOption.setValueName(QStringLiteral("file"));
    versionOption.setValueName(QStringLiteral("version"));
    ftpDirOption.setValueName(QStringLiteral("ftpdir"));
    ftpUserOption.setValueName(QStringLiteral("ftpuser"));
    ftpPwdOption.setValueName(QStringLiteral("ftppwd"));
    parser.addOption(fileOption);
    parser.addOption(versionOption);
    parser.addOption(ftpDirOption);
    parser.addOption(ftpUserOption);
    parser.addOption(ftpPwdOption);

    parser.process(a);
    QString dumpFile = parser.value(fileOption);
    if (dumpFile.isEmpty()) {
        return 1;
    }

    MainWindow w(dumpFile, parser.value(versionOption), parser.value(ftpDirOption),
                 parser.value(ftpUserOption), parser.value(ftpPwdOption));
    w.show();

    return a.exec();
}
