#ifndef PLOTDATA_H
#define PLOTDATA_H

#include "CounterData.h"

typedef QMap<QString, CounterData> CounterDataMap;

class PlotData
{
public:
    enum KeyType {
        ktUnknown,
        ktDateTime,
        ktIndex,
    };

    PlotData(int offsetFromUtc);
    PlotData(const PlotData &) = delete;
    PlotData(PlotData &&) = default;
    PlotData &operator=(const PlotData &) = delete;
    PlotData &operator=(PlotData &&) = default;

    KeyType keyType() const;
    int offsetFromUtc() const;
    QVector<double> dateTimeVector() const;
    int size() const;
    int dataCount() const;
    QDateTime dateTimeFromKey(double key) const;
    QDateTime dateTimeFromIndex(int index) const;
    double getSampleInterval() const;
    void setCounterDataMap(KeyType keyType, CounterDataMap &dataMap);
    QList<QString> counterNames() const;
    QString firstCounterName() const;
    bool contains(const QString &name) const;
    QSharedPointer<QCPGraphDataContainer> firstCounterData();
    QSharedPointer<QCPGraphDataContainer> counterData(const QString &name, bool delta = false);
    QSharedPointer<QCPGraphDataContainer> addCounterData(const QString &name);
    QSet<double> * suspectKeys(const QString &name);
    void removeCounterData(const QString &name);
    // Here we can't use QVector because PlotData is not copyable.
    // Use unique_ptr since QSharedPointer doesn't support array.
    std::unique_ptr<PlotData[]> split();

private:
    PlotData();

    KeyType mKeyType;
    int mOffsetFromUtc;
    QVector<double> mDateTimeVector;
    CounterDataMap mDataMap;
};

#endif // PLOTDATA_H
