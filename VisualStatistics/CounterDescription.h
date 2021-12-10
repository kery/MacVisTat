#ifndef COUNTERDESCRIPTION_H
#define COUNTERDESCRIPTION_H

#include <QHash>
#include <QVector>

class CounterId
{
public:
    CounterId(const QString &module, const QString &group, const QString &object);

    bool operator==(const CounterId &other) const;

private:
    QString mModule, mGroup, mObject;

    friend uint qHash(const CounterId &cid, uint seed);
};

uint qHash(const CounterId &cid, uint seed);

class CounterDescription
{
public:
    void load(const QString &path);
    QString getDescription(const QString &name) const;

private:
    typedef QHash<CounterId, QString> DescriptionHash;

    struct CallbackUserData
    {
        QVector<QString> columns;
        DescriptionHash *descHash;
    };

    static void libcsvCbEndOfField(void *field, size_t len, void *ud);
    static void libcsvCbEndOfRow(int, void *ud);
    static CounterId getCounterId(const QString &name);

    DescriptionHash mDescHash;
};

#endif // COUNTERDESCRIPTION_H
