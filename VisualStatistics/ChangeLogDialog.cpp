#include "ChangeLogDialog.h"
#include "ui_ChangeLogDialog.h"
#include "Application.h"
#include <QNetworkReply>
#include <QPushButton>

ChangeLogDialog::ChangeLogDialog(QWidget *parent, bool update) :
    QDialog(parent),
    ui(new Ui::ChangeLogDialog),
    mResizeMan(this)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
    if (update) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Update"));
    } else {
        ui->label->setVisible(false);
    }

    Application *app = Application::instance();
    QUrl url = app->getUrl(Application::upChangeLog);
    QNetworkRequest request(url);
    QNetworkReply *reply = app->networkAccessManager().get(request);
    connect(reply, &QNetworkReply::finished, this, &ChangeLogDialog::fetchChangeLogFinished);
}

ChangeLogDialog::~ChangeLogDialog()
{
    delete ui;
}

bool ChangeLogDialog::event(QEvent *event)
{
    mResizeMan.resizeWidgetFromCharWidth(event, 110, 1.1);
    return QDialog::event(event);
}

void ChangeLogDialog::fetchChangeLogFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray allData = reply->readAll();
        ui->textEdit->setPlainText(QString(allData));
    } else {
        ui->textEdit->setPlainText("Failed to fetch change log: " + reply->errorString());
    }
}
