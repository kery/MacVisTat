#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &path, QWidget *parent = 0);
    ~MainWindow();

private slots:
    void uploadFinished(QNetworkReply *reply);
    void uploadError(QNetworkReply::NetworkError err);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);

    void on_buttonBox_accepted();

private:
    Ui::MainWindow *_ui;
    QString _dumpFilePath;
};

#endif // MAINWINDOW_H
