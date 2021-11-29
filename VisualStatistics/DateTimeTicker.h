#ifndef DATETIMETICKER_H
#define DATETIMETICKER_H

#include "qcustomplot/qcustomplot.h"

class DateTimeTicker : public QObject, public QCPAxisTicker
{
    Q_OBJECT

public:
    DateTimeTicker(QCPAxis *parentAxis);

    void setDisplayUtc(bool displayUtc);
    void setOffsetFromUtc(int offset);

signals:
    void skippedTicksChanged(int skipped);

private:
    virtual int getSubTickCount(double tickStep) override;
    virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) override;
    virtual QVector<double> createTickVector(double tickStep, const QCPRange &range) override;

    static bool isValidOffsetFromUtc(int offset);

    bool _displayUtc;
    int _skippedTicks;
    int _offsetFromUtc;
    QString _dateTimeFmt;
    QCPAxis *_parentAxis;
};

#endif // DATETIMETICKER_H
