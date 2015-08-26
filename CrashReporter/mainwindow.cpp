#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QNetworkAccessManager>

MainWindow::MainWindow(const QString &path, QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _dumpFilePath(path)
{
    _ui->setupUi(this);
    setFixedSize(size());

    _ui->textBrowser->setText("An unhandled exception occurred, which cause the program exit unexpectedly.\n\n"
                              "Do you want to upload the core dump file so that this bug can be fixed in future version?");
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::uploadFinished(QNetworkReply *reply)
{
    qDebug() << "finished";
    reply->deleteLater();
    sender()->deleteLater();
}

void MainWindow::uploadError(QNetworkReply::NetworkError err)
{
    qDebug() << err;
}

void MainWindow::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    _ui->progressBar->setValue(bytesSent);
}

void MainWindow::on_buttonBox_accepted()
{
    QFile *dumpFile = new QFile(_dumpFilePath);
    if (dumpFile->open(QFile::ReadOnly)) {
        _ui->progressBar->setRange(0, dumpFile->size());
        QUrl url(QStringLiteral("ftp://52.76.33.99/test.dmp"));
        url.setUserName(QStringLiteral("kery"));
        url.setPassword(QStringLiteral("test"));

        QNetworkAccessManager *nam = new QNetworkAccessManager();
        QNetworkReply *reply = nam->put(QNetworkRequest(url), dumpFile);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(uploadError(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(uploadProgress(qint64,qint64)));
        connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(uploadFinished(QNetworkReply*)));
    } else {
        close();
    }
}
