#include "MainWindow.h"
#include "CounterName.h"
#include "Application.h"
#include "Utils.h"
#include <exception_handler.h>
#include <QFile>
#include <QDir>
#include <QStyleFactory>
#if defined(Q_OS_WIN)
#include <Shlwapi.h>

static wchar_t crashReporterPath[MAX_PATH];
static wchar_t uploadUrl[MAX_PATH];

static void initCrashReporterPathAndUploadUrl(const Application &app)
{
    GetModuleFileNameW(NULL, crashReporterPath, MAX_PATH);
    PathRemoveFileSpecW(crashReporterPath);
    PathAppendW(crashReporterPath, L"CrashReporter.exe");

    std::wstring urlStr = app.getUrl(Application::upUpload).toString().toStdWString();
    wcscpy(uploadUrl, urlStr.c_str());
}

static bool minidumpCallback(const wchar_t* dump_path,
                             const wchar_t* minidump_id,
                             void* /*context*/,
                             EXCEPTION_POINTERS* /*exinfo*/,
                             MDRawAssertionInfo* /*assertion*/,
                             bool succeeded)
{
    if (succeeded) {
        wchar_t param[MAX_PATH];
        wcscpy_s(param, L"--file \"");
        wcscat_s(param, dump_path);
        if (dump_path[wcslen(dump_path) - 1] != L'/') {
            wcscat_s(param, L"/");
        }
        wcscat_s(param, minidump_id);
        wcscat_s(param, L".dmp\" --url ");
        wcscat_s(param, uploadUrl);

        ShellExecuteW(NULL, NULL, crashReporterPath, param, NULL, SW_SHOW);
    }
    return succeeded;
}
#else
#include "Version.h"

static char crashReporterPath[PATH_MAX];
static char uploadUrl[PATH_MAX];

static void initCrashReporterPathAndUploadUrl(const Application &app)
{
    QString path = QDir(Application::applicationDirPath()).filePath(QStringLiteral("CrashReporter"));
    strcpy(crashReporterPath, path.toStdString().c_str());

    strcpy(uploadUrl, app.getUrl(Application::upUpload).toString().toStdString().c_str());
}

static bool minidumpCallback(const google_breakpad::MinidumpDescriptor &descriptor,
                             void */*context*/,
                             bool succeeded)
{
    if (succeeded) {
        pid_t pid = sys_fork();
        if (pid == 0) { // Child process
            const char * const argv[] = {
                "CrashReporter",
                "--file", descriptor.path(),
                "--url", uploadUrl,
                "--version", VER_FILEVERSION_STR,
                nullptr
            };
            sys_execv(crashReporterPath, argv);
        }
    }
    return succeeded;
}
#endif

static void loadStyleSheet(QApplication &app)
{
#ifdef Q_OS_WIN
    QFile file(QStringLiteral(":/qss/win.qss"));
#else
    QFile file(QStringLiteral(":/qss/default.qss"));
#endif
    if (file.open(QFile::ReadOnly)) {
        app.setStyleSheet(file.readAll());
    }
}

int main(int argc, char *argv[])
{
    Application app(argc, argv);
    app.setStyle(QStyleFactory::create(QStringLiteral("fusion")));
    loadStyleSheet(app);

    // Call these functions so that we can use default constructor of
    // QSettings.
    Application::setOrganizationName("Nokia");
    Application::setApplicationName("VisualStatistics");
    CounterName::initSeparators();

    // Close stdin so that calling lua functions, which reads stdin, from
    // script window will not hang UI thread.
    fclose(stdin);

    initCrashReporterPathAndUploadUrl(app);

#if defined(Q_OS_WIN)
    google_breakpad::ExceptionHandler eh(QDir::tempPath().toStdWString(),
                                         nullptr,
                                         minidumpCallback,
                                         nullptr,
                                         google_breakpad::ExceptionHandler::HANDLER_ALL,
                                         MiniDumpNormal,
                                         static_cast<wchar_t *>(nullptr),
                                         nullptr);
#else
    google_breakpad::ExceptionHandler(google_breakpad::MinidumpDescriptor(QDir::tempPath().toStdString()),
                                      nullptr,
                                      minidumpCallback,
                                      nullptr,
                                      true,
                                      -1);
#endif

    MainWindow mainWnd;
    mainWnd.show();

    return app.exec();
}
