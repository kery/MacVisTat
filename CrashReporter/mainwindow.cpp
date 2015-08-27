#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileInfo>
#include <QNetworkAccessManager>

MainWindow::MainWindow(const QString &path, QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _dumpFile(path)
{
    _ui->setupUi(this);
    setFixedSize(size());

    _timer.setSingleShot(true);
    _timer.setInterval(10 * 1000);
    connect(&_timer, SIGNAL(timeout()), this, SLOT(close()));

    _ui->textBrowser->setText("An unhandled exception occurred, which cause the program exit unexpectedly.\n\n"
                              "Do you want to upload the core dump file so that this bug can be fixed in future version?");
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::uploadFinished(QNetworkReply *reply)
{
    _timer.stop();
    if (reply->error() == QNetworkReply::NoError) {
        _dumpFile.remove();
    }
    reply->deleteLater();
    sender()->deleteLater();
    close();
}

void MainWindow::uploadProgress(qint64 /*bytesSent*/, qint64 /*bytesTotal*/)
{
    _timer.start();
}

void MainWindow::on_buttonBox_accepted()
{
    if (_dumpFile.open(QFile::ReadOnly)) {
        QUrl url(QStringLiteral("ftp://careman4.emea.nsn-net.net/d/ftpserv/VisualStatistics/%1").arg(
                     QFileInfo(_dumpFile).fileName()));
        url.setUserName(QStringLiteral("micts"));
        url.setPassword(QStringLiteral("micts123"));

        QNetworkAccessManager *nam = new QNetworkAccessManager();
        QNetworkReply *reply = nam->put(QNetworkRequest(url), &_dumpFile);
        connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(uploadProgress(qint64,qint64)));
        connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(uploadFinished(QNetworkReply*)));

        _timer.start();
        hide();
    } else {
        close();
    }
}
