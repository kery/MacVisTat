#include "CounterDescription.h"
#include "CounterName.h"
#include "GzipFile.h"
#include <csv.h>

CounterId::CounterId(const QString &module, const QString &group, const QString &object) :
    mModule(module),
    mGroup(group),
    mObject(object)
{
}

bool CounterId::operator==(const CounterId &other) const
{
    return other.mModule == mModule && other.mGroup == mGroup && other.mObject == mObject;
}

uint qHash(const CounterId &cid, uint seed)
{
    return qHash(cid.mModule, seed) ^ qHash(cid.mModule, seed) ^ qHash(cid.mObject, seed);
}

void CounterDescription::load(const QString &path)
{
    GzipFile reader;
    if (!reader.open(path, GzipFile::ReadOnly)) {
        return;
    }

    CallbackUserData ud;
    ud.columns.reserve(4);
    ud.descHash = &mDescHash;

    struct csv_parser parser;
    csv_init(&parser, CSV_STRICT | CSV_STRICT_FINI | CSV_EMPTY_IS_NULL);

    std::string line;
    reader.readLineKeepCrLf(line); // Consume the header line
    while (reader.readLineKeepCrLf(line)) {
        csv_parse(&parser, line.c_str(), line.length(), libcsvCbEndOfField, libcsvCbEndOfRow, &ud);
    }

    csv_fini(&parser, libcsvCbEndOfField, libcsvCbEndOfRow, &ud);
    csv_free(&parser);
}

QString CounterDescription::getDescription(const QString &name) const
{
    CounterId cid = getCounterId(name);
    auto iter = mDescHash.find(cid);
    if (iter != mDescHash.end()) {
        return iter.value();
    }
    return QString();
}

void CounterDescription::libcsvCbEndOfField(void *field, size_t len, void *ud)
{
    auto ccud = static_cast<CallbackUserData *>(ud);
    ccud->columns.append(field ? QString::fromLatin1(static_cast<const char *>(field), static_cast<int>(len)) : QString());
}

void CounterDescription::libcsvCbEndOfRow(int, void *ud)
{
    auto ccud = static_cast<CallbackUserData *>(ud);
    if (ccud->columns.size() == 4) {
        CounterId cid(ccud->columns[0], ccud->columns[1], ccud->columns[2]);
        ccud->descHash->insert(cid, ccud->columns[3]);
    }
    ccud->columns.clear();
}

CounterId CounterDescription::getCounterId(const QString &name)
{
    QString module, group, object;
    int pos1 = name.indexOf(CounterName::sModuleSeparator);
    if (pos1 != -1) {
        module = name.mid(0, pos1);
        pos1 = name.indexOf(QLatin1String("GroupName="), pos1);
        if (pos1 != -1) {
            pos1 += 10;
            int pos2 = name.indexOf(CounterName::sGroupSeparator, pos1);
            if (pos2 != -1) {
                group = name.mid(pos1, pos2 - pos1);
            }
        }
        // NRD counter has no GroupName, so we continue to get the KPI-KCI Object field
        pos1 = name.lastIndexOf(CounterName::sIndexesSeparator);
        if (pos1 != -1) {
            object = name.mid(pos1 + 1);
        }
    }
    return CounterId(module, group, object);
}
