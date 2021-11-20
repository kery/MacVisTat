#include "MainWindow.h"
#include "Utils.h"
#include <QApplication>
#include <memory>

#include <exception_handler.h>

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
        wcscat_s(param, L".dmp\" --ftpdir ftp://sdu.int.nokia-sbell.com/home/visualstat/coredump/ --ftpuser visualstat --ftppwd vtcore");

        wchar_t *crPath = static_cast<wchar_t*>(context);
        ShellExecuteW(NULL, NULL, crPath, param, NULL, SW_SHOW);
    }

    return succeeded;
}

static void loadStyleSheet(QApplication &app)
{
    QFile file(":/qss/default.qss");
    if (file.open(QFile::ReadOnly)) {
        QString qss = QString::fromLatin1(file.readAll());
        app.setStyleSheet(qss);
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    loadStyleSheet(app);
    // Call these functions so that we can use default constructor of
    // QSettings.
    // Should be called before getAppDataDir since it use these
    // informations
    QCoreApplication::setOrganizationName("Nokia");
    QCoreApplication::setApplicationName("VisualStatistics");

    QSettings settings;
    if (settings.value(QStringLiteral("fusionStyle"), true).toBool()) {
        app.setStyle(QStyleFactory::create("fusion"));
    }

    // Close stdin so that calling lua functions, which reads stdin, from
    // script window will not hang UI thread.
    fclose(stdin);

    std::unique_ptr<google_breakpad::ExceptionHandler> exceptionHandler;

    if (QDir().mkpath(getAppDataDir())) {
        exceptionHandler.reset(new google_breakpad::ExceptionHandler(
                    QDir::toNativeSeparators(getAppDataDir()).toStdWString(),
                    NULL, minidumpCallback, getCrashReporterPath(),
                    google_breakpad::ExceptionHandler::HANDLER_ALL,
                    MiniDumpNormal, (wchar_t*)NULL, NULL));
    } else {
        showInfoMsgBox(NULL, QStringLiteral("Create application data directory failed!"));
    }

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
