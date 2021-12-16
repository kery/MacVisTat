#include "MainWindow.h"
#include "CounterName.h"
#include "Application.h"
#include <QFile>
#include <QStyleFactory>

static void loadStyleSheet(QApplication &app)
{
    QFile file(QStringLiteral(":/qss/default.qss"));
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
    // Should be called before getAppDataDir since it use these
    // informations
    Application::setOrganizationName("Nokia");
    Application::setApplicationName("VisualStatistics");
    CounterName::initSeparators();

    // Close stdin so that calling lua functions, which reads stdin, from
    // script window will not hang UI thread.
    fclose(stdin);

    MainWindow mainWnd;
    mainWnd.show();
    return app.exec();
}
