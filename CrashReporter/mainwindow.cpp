#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileInfo>
#include <QPushButton>
#include <QNetworkAccessManager>

// It is necessary to know VisualStatistics executable's version that
// corresponding to the dump file.
// On Windows platform this version is included in dump file which use
// the "File version" of executable's resource
// On linux platform it seems that there is no such kind of mechanism
// so here append the version to the dump file name
#if defined(Q_OS_LINUX)
#include "../VisualStatistics/version.h"
#endif

MainWindow::MainWindow(const QString &path, QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _dumpFile(path)
{
    _ui->setupUi(this);
    setFixedSize(size());

    QPushButton *yesButton = _ui->buttonBox->button(QDialogButtonBox::Yes);
    yesButton->setAutoDefault(true);
    yesButton->setDefault(true);
    yesButton->setFocus();

    QPushButton *noButton = _ui->buttonBox->button(QDialogButtonBox::No);
    noButton->setAutoDefault(true);

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
#if defined(Q_OS_LINUX)
        QUrl url(QStringLiteral("ftp://135.242.202.254/data/tmp/visualstatistics/coredump/%1.%2").arg(
                     QFileInfo(_dumpFile).fileName()).arg(VER_FILEVERSION_NUM));
#elif defined(Q_OS_WIN)
        QUrl url(QStringLiteral("ftp://135.242.202.254/data/tmp/visualstatistics/coredump/%1").arg(
                     QFileInfo(_dumpFile).fileName()));
#endif
        url.setUserName(QStringLiteral("sdu"));
        url.setPassword(QStringLiteral("sdu"));

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
