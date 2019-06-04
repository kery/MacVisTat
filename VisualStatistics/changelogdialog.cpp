#include "changelogdialog.h"
#include "ui_changelogdialog.h"

#include <QNetworkAccessManager>
#include <QNetworkProxyQuery>

ChangeLogDialog::ChangeLogDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeLogDialog)
{
    ui->setupUi(this);

    setFixedSize(size());
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

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

void ChangeLogDialog::hideLabel()
{
    ui->label->hide();
}

void ChangeLogDialog::fetchChangeLogFinished(QNetworkReply *reply)
{
    sender()->deleteLater();
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray allData = reply->readAll();
        ui->plainTextEdit->setPlainText(QString(allData));
    } else {
        ui->plainTextEdit->setPlainText(QStringLiteral("Failed to fetch change log."));
    }
}
