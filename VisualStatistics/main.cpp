#include "mainwindow.h"
#include "utils.h"
#include <QApplication>
#include <exception_handler.h>

#if defined(Q_OS_WIN)

static wchar_t crashReporterPath[MAX_PATH];

static void getCrashReporterPath()
{
    GetModuleFileNameW(NULL, crashReporterPath, MAX_PATH);
    wchar_t *lastBackSlash = wcsrchr(crashReporterPath, L'\\');
    wcscpy_s(lastBackSlash + 1, MAX_PATH - (lastBackSlash - crashReporterPath) - 1, L"CrashReporter.exe");
}

static bool minidumpCallback(const wchar_t* dump_path,
                             const wchar_t* minidump_id,
                             void* /*context*/,
                             EXCEPTION_POINTERS* /*exinfo*/,
                             MDRawAssertionInfo* /*assertion*/,
                             bool succeeded)
{
    // Note: DO NOT USE heap since the heap may be corrupt
    if (succeeded) {
        wchar_t param[MAX_PATH];
        wcscpy_s(param, L"--file \"");
        wcscat_s(param, dump_path);
        if (dump_path[wcslen(dump_path) - 1] != L'\\') {
            wcscat_s(param, L"\\");
        }
        wcscat_s(param, minidump_id);
        wcscat_s(param, L".dmp\"");

        ShellExecuteW(NULL, NULL, crashReporterPath, param, NULL, SW_SHOW);
    }

    return succeeded;
}

#elif defined(Q_OS_LINUX)

#include <third_party/lss/linux_syscall_support.h>

static char crashReporterPath[PATH_MAX];

static void getCrashReporterPath()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString crPath = QDir(appDir).filePath(QStringLiteral("CrashReporter"));
    strcpy(crashReporterPath, crPath.toStdString().c_str());
}

static bool minidumpCallback(const google_breakpad::MinidumpDescriptor &descriptor,
                             void */*context*/, bool succeeded)
{
    if (succeeded) {
        pid_t pid = sys_fork();
        if (pid == 0) { // Child process
            const char * const argv[] = {"CrashReporter", "--file", descriptor.path(), NULL};
            sys_execv(crashReporterPath, argv);
        }
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
        getCrashReporterPath();
#if defined(Q_OS_WIN)
        exceptionHandler = new google_breakpad::ExceptionHandler(
                    QDir::toNativeSeparators(getAppDataDir()).toStdWString(),
                    NULL, minidumpCallback, NULL,
                    google_breakpad::ExceptionHandler::HANDLER_ALL,
                    MiniDumpNormal, (wchar_t*)NULL, NULL);
#elif defined(Q_OS_LINUX)
        exceptionHandler = new google_breakpad::ExceptionHandler(
                    google_breakpad::MinidumpDescriptor(getAppDataDir().toStdString()),
                    NULL, minidumpCallback, NULL, true, -1);
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
