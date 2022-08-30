#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QFileDialog>

class FileDialog : public QFileDialog
{
    Q_OBJECT

public:
    FileDialog(QWidget *parent);

    void selectNameFilter(int index);

    Q_SLOT virtual int exec() override;

    static QString getOpenFileName(QWidget *parent, const QString &filter);
    static QString getSaveFileName(QWidget *parent, const QString &fileName, const QString &filter);
};

#endif // FILEDIALOG_H
