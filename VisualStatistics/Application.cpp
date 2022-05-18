#include "Application.h"
#include <QNetworkProxyQuery>

Application::Application(int &argc, char **argv) :
    QApplication(argc, argv)
{
    QNetworkProxyQuery npq(getUrl(upRoot));
    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(npq);
    if (!proxies.isEmpty()) {
        mNetMan.setProxy(proxies[0]);
    }
}

QNetworkAccessManager & Application::networkAccessManager()
{
    return mNetMan;
}

QUrl Application::getUrl(UrlPath up)
{
    QUrl root(QStringLiteral("http://sdu.int.nokia-sbell.com:4099/"));

    switch (up) {
    case upRoot:
        break;
    case upHelp:
        return root.resolved(QStringLiteral("/help.html"));
    case upCounterDescription:
        return root.resolved(QStringLiteral("/counters.zip"));
    case upChangeLog:
        return root.resolved(QStringLiteral("/changelog.txt"));
    case upUsageReport:
        return root.resolved(QStringLiteral("/report"));
    case upUpload:
        return root.resolved(QStringLiteral("/upload"));
    }
    return root;
}

Application * Application::instance()
{
    return qobject_cast<Application *>(QApplication::instance());
}
