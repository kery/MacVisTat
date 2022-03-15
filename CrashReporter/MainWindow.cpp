#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileInfo>
#include <QPushButton>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QNetworkProxyQuery>
#include <QNetworkAccessManager>

MainWindow::MainWindow(const QString &path, const QString &url, const QString &version) :
    QMainWindow(nullptr),
    ui(new Ui::MainWindow),
    mDumpFile(path),
    mUrl(url),
    mVersion(version)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &MainWindow::buttonBoxAccepted);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::buttonBoxAccepted()
{
    if (mDumpFile.open(QFile::ReadOnly)) {
        upload();
        hide();
    } else {
        close();
    }
}

void MainWindow::uploadFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    sender()->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        mDumpFile.remove();
    }
    close();
}

void MainWindow::upload()
{
    QString dispHeader("form-data; name=\"file\"; filename=\"");
    dispHeader += uploadFileName();
    dispHeader += "\"";

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(dispHeader));
    filePart.setBodyDevice(&mDumpFile);

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    multiPart->append(filePart);

    QNetworkProxyQuery npq(mUrl);
    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(npq);
    QNetworkAccessManager *netMan = new QNetworkAccessManager();
    if (!proxies.isEmpty()) {
        netMan->setProxy(proxies[0]);
    }
    QNetworkReply *reply = netMan->post(QNetworkRequest(mUrl), multiPart);
    multiPart->setParent(reply);
    connect(netMan, &QNetworkAccessManager::finished, this, &MainWindow::uploadFinished);
}

QString MainWindow::uploadFileName() const
{
    QString result = mDumpFile.fileName();
    if (!mVersion.isEmpty()) {
        int pos = result.lastIndexOf('.');
        if (pos >= 0) {
            result.insert(pos, "_" + mVersion);
        } else {
            result.append("_" + mVersion);
        }
    }
    return result;
}
