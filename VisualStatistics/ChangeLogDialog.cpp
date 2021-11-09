#include "ChangeLogDialog.h"
#include "ui_ChangeLogDialog.h"

#include <QNetworkAccessManager>
#include <QNetworkProxyQuery>
#include <QPushButton>

ChangeLogDialog::ChangeLogDialog(QWidget *parent, bool update) :
    QDialog(parent),
    ui(new Ui::ChangeLogDialog),
    m_resizeMan(this)
{
    ui->setupUi(this);
    if (update) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Update"));
    } else {
        ui->label->setVisible(false);
    }

    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
    setSizeGripEnabled(true);

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

bool ChangeLogDialog::event(QEvent *event)
{
    if (event->type() == QEvent::ShowToParent && !m_resizeMan.showToParentHandled()) {
        m_resizeMan.resizeWidgetFromCharWidth(110, 1.1);
    }
    return QDialog::event(event);
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
