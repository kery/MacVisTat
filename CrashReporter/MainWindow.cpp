#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileInfo>
#include <QPushButton>
#include <QNetworkAccessManager>

MainWindow::MainWindow(const QString &path, const QString &version,
                       const QString &ftpDir, const QString &ftpUser,
                       const QString &ftpPwd, QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _dumpFile(path),
    _version(version),
    _ftpDir(ftpDir),
    _ftpUser(ftpUser),
    _ftpPwd(ftpPwd)
{
    _ui->setupUi(this);
    setFixedSize(size());

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &MainWindow::buttonBoxAccepted);

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

QString MainWindow::getUploadFileName()
{
    QString fileName = QFileInfo(_dumpFile).fileName();
    if (!_version.isEmpty()) {
        int index = fileName.lastIndexOf('.');
        fileName.insert(qMax(index, 0), _version);
    }
    return fileName;
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

void MainWindow::buttonBoxAccepted()
{
    if (_dumpFile.open(QFile::ReadOnly)) {
        QUrl url(_ftpDir + getUploadFileName());
        url.setUserName(_ftpUser);
        url.setPassword(_ftpPwd);

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
