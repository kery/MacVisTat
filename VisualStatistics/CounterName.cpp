#include "CounterName.h"
#include "GlobalDefines.h"
#include <QSettings>

QChar CounterName::sModuleSeparator;
QChar CounterName::sGroupSeparator;
QChar CounterName::sIndexesSeparator;

void CounterName::initSeparators()
{
    QSettings setting;
    sModuleSeparator = setting.value(SETTING_KEY_MODULE_SEP, ',').toChar();
    sGroupSeparator = setting.value(SETTING_KEY_GROUP_SEP, ',').toChar();
    sIndexesSeparator = setting.value(SETTING_KEY_INDEXES_SEP, ',').toChar();
}

QString CounterName::getModuleName(const QString &name)
{
    int index = name.indexOf(sModuleSeparator);
    if (index > 0) {
        return name.left(index);
    }
    return QString();
}

QString CounterName::getObjectName(const QString &name)
{
    return name.mid(name.lastIndexOf(sIndexesSeparator) + 1);
}

QPair<QString, QString> CounterName::separateModuleName(const QString &name)
{
    QPair<QString, QString> result;
    int index = name.indexOf(sModuleSeparator);
    if (index > 0) {
        result.first = name.left(index);
        result.second = name.mid(index + 1);
    } else {
        result.second = name;
    }
    return result;
}
