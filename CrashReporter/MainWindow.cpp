#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileInfo>
#include <QPushButton>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QNetworkAccessManager>

MainWindow::MainWindow(const QString &path, const QString &url) :
    QMainWindow(nullptr),
    ui(new Ui::MainWindow),
    mDumpFile(path),
    mUrl(url)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
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
    dispHeader += mDumpFile.fileName();
    dispHeader += "\"";

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(dispHeader));
    filePart.setBodyDevice(&mDumpFile);

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    multiPart->append(filePart);

    QNetworkAccessManager *netMan = new QNetworkAccessManager();
    QNetworkReply *reply = netMan->post(QNetworkRequest(mUrl), multiPart);
    multiPart->setParent(reply);
    connect(netMan, &QNetworkAccessManager::finished, this, &MainWindow::uploadFinished);
}
