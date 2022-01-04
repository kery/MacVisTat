#ifndef DATETIMETICKER_H
#define DATETIMETICKER_H

#include "QCustomPlot/src/axis/axisticker.h"

class QCPAxis;

class DateTimeTicker : public QObject, public QCPAxisTicker
{
    Q_OBJECT

public:
    DateTimeTicker(QCPAxis *parentAxis, const QVector<double> *dateTimeVector);

    int skippedTicks() const;
    bool utcDisplay() const;
    void setUtcDisplay(bool on);
    void setOffsetFromUtc(int offset);
    bool setBeginDateTime(const QDateTime &dateTime);
    bool setEndDateTime(const QDateTime &dateTime);

signals:
    void beginDateTimeChanged(const QDateTime &dateTime);
    void endDateTimeChanged(const QDateTime &dateTime);
    void skippedTicksChanged(int skipped);

private:
    virtual int getSubTickCount(double tickStep) override;
    virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) override;
    virtual QVector<double> createTickVector(double tickStep, const QCPRange &range) override;

    QDateTime dateTimeFromKey(double key) const;
    double dateTimeToKey(const QDateTime &dateTime) const;

    bool mUtcDisplay;
    int mSkippedTicks;
    int mOffsetFromUtc;
    QString mDateTimeFmt;
    QCPAxis *mParentAxis;
    const QVector<double> *mDateTimeVector;
    QDateTime mBeginDateTime, mEndDateTime;
};

#endif // DATETIMETICKER_H
