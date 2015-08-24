#include "mainwindow.h"
#include "utils.h"
#include <QApplication>
#include <exception_handler.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Call these functions so that we can use default constructor of
    // QSettings
    // Should be called before getAppDataDir since is use these
    // informations
    QCoreApplication::setOrganizationName("Nokia");
    QCoreApplication::setApplicationName("VisualStatistics");

    google_breakpad::ExceptionHandler *exceptionHandler = NULL;
    if (QDir().mkpath(getAppDataDir())) {
#ifdef Q_OS_WIN
        exceptionHandler = new google_breakpad::ExceptionHandler(
                    getAppDataDir().toStdWString(),
                    NULL, NULL, NULL,
                    google_breakpad::ExceptionHandler::HANDLER_ALL,
                    MiniDumpNormal, (wchar_t*)NULL, NULL);
#endif
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QStringLiteral("Information"));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(QStringLiteral("Create application data directory failed!"));
        msgBox.exec();
    }

    MainWindow w;
    w.show();

    int retCode = a.exec();

    if (exceptionHandler) {
        delete exceptionHandler;
    }
    return retCode;
}
