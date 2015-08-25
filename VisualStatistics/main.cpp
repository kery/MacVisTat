#include "mainwindow.h"
#include "utils.h"
#include <QApplication>
#include <exception_handler.h>

#ifdef Q_OS_WIN
static bool minidumpCallback(const wchar_t* dump_path,
                             const wchar_t* minidump_id,
                             void* context,
                             EXCEPTION_POINTERS* exinfo,
                             MDRawAssertionInfo* assertion,
                             bool succeeded)
{
    (void)context;
    (void)exinfo;
    (void)assertion;

    // Note: DO NOT USE heap since the heap may be corrupt
    if (succeeded) {
        wchar_t param[MAX_PATH];
        wcscpy_s(param, L"--file ");
        wcscat_s(param, dump_path);
        if (dump_path[wcslen(dump_path) - 1] != L'\\') {
            wcscat_s(param, L"\\");
        }
        wcscat_s(param, minidump_id);
        wcscat_s(param, L".dmp");
        ShellExecuteW(NULL, NULL, L"CrashReporter.exe", param, NULL, SW_SHOW);
    }

    return succeeded;
}
#endif

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
                    QDir::toNativeSeparators(getAppDataDir()).toStdWString(),
                    NULL, minidumpCallback, NULL,
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
