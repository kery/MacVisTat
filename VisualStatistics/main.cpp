#include "MainWindow.h"
#include "CounterName.h"
#include "Application.h"
#include <exception_handler.h>
#include <QFile>
#include <QDir>
#include <QStyleFactory>
#include <Shlwapi.h>

static void loadStyleSheet(QApplication &app)
{
    QFile file(QStringLiteral(":/qss/default.qss"));
    if (file.open(QFile::ReadOnly)) {
        app.setStyleSheet(file.readAll());
    }
}

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
    google_breakpad::ExceptionHandler eh(QDir::tempPath().toStdWString(),
                                         nullptr,
                                         minidumpCallback,
                                         nullptr,
                                         google_breakpad::ExceptionHandler::HANDLER_ALL,
                                         MiniDumpNormal,
                                         static_cast<wchar_t *>(nullptr),
                                         nullptr);
    MainWindow mainWnd;
    mainWnd.show();
    return app.exec();
}
