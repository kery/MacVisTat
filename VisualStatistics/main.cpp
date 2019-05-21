#include <exception_handler.h>
#include "mainwindow.h"
#include "utils.h"
#include <QApplication>
#include <memory>

#if defined(Q_OS_WIN)

static wchar_t* getCrashReporterPath()
{
    static wchar_t crPath[MAX_PATH];

    GetModuleFileNameW(NULL, crPath, MAX_PATH);
    wchar_t *lastBackSlash = wcsrchr(crPath, L'\\');
    wcscpy_s(lastBackSlash + 1, MAX_PATH - (lastBackSlash - crPath) - 1, L"CrashReporter.exe");
    return crPath;
}

static bool minidumpCallback(const wchar_t* dump_path,
                             const wchar_t* minidump_id,
                             void* context,
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
        wcscat_s(param, L".dmp\" --ftpdir ftp://sdu.int.nokia-sbell.com/visualstatistics/ --ftpuser sdu --ftppwd chengdusdu123");

        wchar_t *crPath = static_cast<wchar_t*>(context);
        ShellExecuteW(NULL, NULL, crPath, param, NULL, SW_SHOW);
    }

    return succeeded;
}

#elif defined(Q_OS_LINUX)

#include "version.h"
#include <third_party/lss/linux_syscall_support.h>

static char* getCrashReporterPath()
{
    static char crPath[PATH_MAX];

    QString appDir = QCoreApplication::applicationDirPath();
    QString path = QDir(appDir).filePath(QStringLiteral("CrashReporter"));
    strcpy(crPath, path.toStdString().c_str());
    return crPath;
}

static bool minidumpCallback(const google_breakpad::MinidumpDescriptor &descriptor,
                             void *context, bool succeeded)
{
    if (succeeded) {
        pid_t pid = sys_fork();
        if (pid == 0) { // Child process
            const char * const argv[] = {"CrashReporter",
                "--file", descriptor.path(),
                "--version", VER_FILEVERSION_STR,
                NULL};
            char *crPath = static_cast<char*>(context);
            sys_execv(crPath, argv);
        }
    }
    return succeeded;
}

#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // Call these functions so that we can use default constructor of
    // QSettings.
    // Should be called before getAppDataDir since it use these
    // informations
    QCoreApplication::setOrganizationName("Nokia");
    QCoreApplication::setApplicationName("VisualStatistics");

    std::unique_ptr<google_breakpad::ExceptionHandler> exceptionHandler;

    if (QDir().mkpath(getAppDataDir())) {
#if defined(Q_OS_WIN)
        exceptionHandler.reset(new google_breakpad::ExceptionHandler(
                    QDir::toNativeSeparators(getAppDataDir()).toStdWString(),
                    NULL, minidumpCallback, getCrashReporterPath(),
                    google_breakpad::ExceptionHandler::HANDLER_ALL,
                    MiniDumpNormal, (wchar_t*)NULL, NULL));
#elif defined(Q_OS_LINUX)
        exceptionHandler.reset(new google_breakpad::ExceptionHandler(
                    google_breakpad::MinidumpDescriptor(getAppDataDir().toStdString()),
                    NULL, minidumpCallback, getCrashReporterPath(), true, -1));
#endif
    } else {
        showInfoMsgBox(NULL, QStringLiteral("Create application data directory failed!"));
    }

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
