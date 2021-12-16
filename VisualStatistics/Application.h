#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QNetworkAccessManager>

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);

    QNetworkAccessManager & networkAccessManager();

    enum UrlPath {
        upHelp,
        upCounterDescription,
        upChangeLog,
        upUsageReport,
        upRoot,
    };

    static QUrl getUrl(UrlPath up);
    static Application * instance();

private:
    QNetworkAccessManager mNetMan;
};

#endif // APPLICATION_H
