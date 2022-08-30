#include "FileDialog.h"
#include "GlobalDefines.h"
#include <QSettings>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif

FileDialog::FileDialog(QWidget *parent) :
    QFileDialog(parent)
{
    QSettings setting;
    setDirectory(setting.value(SETTING_KEY_OPENDLG_DIR).toString());
}

void FileDialog::selectNameFilter(int index)
{
    QStringList filters = nameFilters();
    if (filters.isEmpty() || index < 0 || index >= filters.size()) {
        return;
    }

    QFileDialog::selectNameFilter(filters[index]);

#ifdef Q_OS_WIN
    // On Windows platform, above code only update the selected filter in
    // combobox, to reflect the filter change in file dialog list view we
    // must notify the filter dialog that the filter has been changed.
    HWND hDlg = GetWindow((HWND)winId(), GW_HWNDPREV);
    HWND hCmb = GetDlgItem(hDlg, cmb1);

    if (GetWindowLong(hCmb, GWL_STYLE) & CBS_SIMPLE) {
        SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(cmb1, CBN_SELENDOK), (LPARAM)hCmb);
    }
    SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(cmb1, CBN_SELCHANGE), (LPARAM)hCmb);
#endif
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
