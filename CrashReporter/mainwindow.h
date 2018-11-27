#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>
#include <QFile>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &path, const QString &version, QWidget *parent = 0);
    ~MainWindow();

private:
    QString getUploadFileName();

private slots:
    void uploadFinished(QNetworkReply *reply);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);

    void on_buttonBox_accepted();

private:
    Ui::MainWindow *_ui;
    QFile _dumpFile;
    QString _version;
    QTimer _timer;
};

#endif // MAINWINDOW_H
