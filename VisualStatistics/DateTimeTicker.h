#ifndef DATETIMETICKER_H
#define DATETIMETICKER_H

#include "qcustomplot/qcustomplot.h"

class DateTimeTicker : public QObject, public QCPAxisTicker
{
    Q_OBJECT

public:
    DateTimeTicker(QCPAxis *parentAxis);

    int skippedTicks() const;
    bool isUtcMode() const;
    void setUtcMode(bool on);
    void setOffsetFromUtc(int offset);
    void setDateTimeVector(QVector<double> &&dtv);

signals:
    void skippedTicksChanged(int skipped);

private:
    virtual int getSubTickCount(double tickStep) override;
    virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) override;
    virtual QVector<double> createTickVector(double tickStep, const QCPRange &range) override;

    static bool isValidOffsetFromUtc(int offset);

    bool mUtcMode;
    int mSkippedTicks;
    int mOffsetFromUtc;
    QString mDateTimeFmt;
    QCPAxis *mParentAxis;
    QVector<double> mDateTimeVector;
};

#endif // DATETIMETICKER_H
