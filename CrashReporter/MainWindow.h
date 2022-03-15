#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QUrl>

namespace Ui { class MainWindow; }

class QNetworkReply;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString &path, const QString &url, const QString &version);
    ~MainWindow();

private slots:
    void buttonBoxAccepted();
    void uploadFinished(QNetworkReply *reply);

private:
    void upload();
    QString uploadFileName() const;

    Ui::MainWindow *ui;
    QFile mDumpFile;
    QUrl mUrl;
    QString mVersion;
};

#endif // MAINWINDOW_H
