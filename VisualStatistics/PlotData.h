#ifndef PLOTDATA_H
#define PLOTDATA_H

#include <memory>
#include "CounterData.h"

typedef QMap<QString, CounterData> CounterDataMap;

class PlotData
{
public:
    PlotData(int offsetFromUtc);
    PlotData(const PlotData &) = delete;
    PlotData(PlotData &&) = default;
    PlotData &operator=(const PlotData &) = delete;
    PlotData &operator=(PlotData &&) = default;

    int offsetFromUtc() const;
    int counterCount() const;
    int counterDataCount() const;
    QDateTime getDateTime(double key) const;
    double getSampleInterval() const;
    void setCounterDataMap(CounterDataMap &dataMap);
    QList<QString> counterNames() const;
    bool contains(const QString &name) const;
    QString firstCounterName() const;
    QSharedPointer<QCPGraphDataContainer> firstCounterData();
    QSharedPointer<QCPGraphDataContainer> counterData(const QString &name, bool delta=false);
    QSharedPointer<QCPGraphDataContainer> addCounterData(const QString &name);
    QSet<double> * suspectKeys(const QString &name);
    void removeCounterData(const QString &name);
    // Here we can't use QVector because PlotData is not copyable.
    // Use unique_ptr since QSharedPointer doesn't support array.
    std::unique_ptr<PlotData[]> split();
    const QVector<double> * dateTimeVector() const;

private:
    PlotData();

    int mOffsetFromUtc;
    CounterDataMap mDataMap;
    QVector<double> mDateTimeVector;
};

#endif // PLOTDATA_H
