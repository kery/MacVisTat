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

QString CounterName::trimModuleName(const QString &name)
{
    int index = name.indexOf(sModuleSeparator);
    if (index > 0) {
        return name.mid(index + 1);
    }
    return name;
}
