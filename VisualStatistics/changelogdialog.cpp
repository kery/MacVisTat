#include "changelogdialog.h"
#include "ui_changelogdialog.h"

#include <QNetworkAccessManager>
#include <QNetworkProxyQuery>
#include <QPushButton>

ChangeLogDialog::ChangeLogDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeLogDialog)
{
    ui->setupUi(this);
    ui->label->setVisible(false);

    setFixedSize(size());
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QUrl url("http://sdu.int.nokia-sbell.com:4099/changelog.txt");
    QNetworkProxyQuery npq(url);
    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(npq);
    if (proxies.size() > 0) {
        manager->setProxy(proxies[0]);
    }
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(fetchChangeLogFinished(QNetworkReply*)));

    QNetworkRequest request(url);
    manager->get(request);
}

ChangeLogDialog::~ChangeLogDialog()
{
    delete ui;
}

void ChangeLogDialog::setShownAfterCheckingUpdates()
{
    ui->label->setVisible(true);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Update"));
}

void ChangeLogDialog::fetchChangeLogFinished(QNetworkReply *reply)
{
    sender()->deleteLater();
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray allData = reply->readAll();
        ui->plainTextEdit->setPlainText(QString(allData));
    } else {
        ui->plainTextEdit->setPlainText("Failed to fetch change log: " + reply->errorString());
    }
}
