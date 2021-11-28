#include "MainWindow.h"
#include <QFile>
#include <QStyleFactory>
#include <QApplication>

static void loadStyleSheet(QApplication &app)
{
    QFile file(QStringLiteral(":/qss/default.qss"));
    if (file.open(QFile::ReadOnly)) {
        app.setStyleSheet(file.readAll());
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create(QStringLiteral("fusion")));
    loadStyleSheet(app);

    // Call these functions so that we can use default constructor of
    // QSettings.
    // Should be called before getAppDataDir since it use these
    // informations
    QCoreApplication::setOrganizationName("Nokia");
    QCoreApplication::setApplicationName("VisualStatistics");

    // Close stdin so that calling lua functions, which reads stdin, from
    // script window will not hang UI thread.
    fclose(stdin);

    MainWindow mainWnd;
    mainWnd.show();
    return app.exec();
}
