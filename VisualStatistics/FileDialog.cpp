#include "FileDialog.h"
#include "GlobalDefines.h"
#include <QSettings>

FileDialog::FileDialog(QWidget *parent) :
    QFileDialog(parent)
{
    QSettings setting;
    setDirectory(setting.value(SETTING_KEY_OPENDLG_DIR).toString());
}

int FileDialog::exec()
{
    int result = QFileDialog::exec();
    if (result == QDialog::Accepted) {
        QSettings setting;
        setting.setValue(SETTING_KEY_OPENDLG_DIR, directory().absolutePath());
    }
    return result;
}

QString FileDialog::getOpenFileName(QWidget *parent, const QString &filter)
{
    QSettings setting;
    QString dir = setting.value(SETTING_KEY_OPENDLG_DIR).toString();
    QString result = QFileDialog::getOpenFileName(parent, QString(), dir, filter);
    if (!result.isEmpty()) {
        QFileInfo fileInfo(result);
        setting.setValue(SETTING_KEY_OPENDLG_DIR, fileInfo.absolutePath());
    }
    return result;
}

QString FileDialog::getSaveFileName(QWidget *parent, const QString &fileName, const QString &filter)
{
    QSettings setting;
    QString dir = setting.value(SETTING_KEY_OPENDLG_DIR).toString();
    if (!fileName.isEmpty()) {
        if (!dir.isEmpty() && !dir.endsWith('/')) {
            dir += '/';
        }
        dir += fileName;
    }
    QString result = QFileDialog::getSaveFileName(parent, QString(), dir, filter);
    if (!result.isEmpty()) {
        QFileInfo fileInfo(result);
        setting.setValue(SETTING_KEY_OPENDLG_DIR, fileInfo.absolutePath());
    }
    return result;
}
