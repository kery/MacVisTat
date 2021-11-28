#ifndef DATETIMETICKER_H
#define DATETIMETICKER_H

#include "qcustomplot/qcustomplot.h"

class DateTimeTicker : public QCPAxisTicker
{
//    Q_OBJECT

public:
    DateTimeTicker(QCPAxis *parentAxis);

    void setShowUtcTime(bool showUtc);
    void setOffsetFromUtc(int offset);

private:
    virtual int getSubTickCount(double tickStep) override;
    virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) override;
    virtual QVector<double> createTickVector(double tickStep, const QCPRange &range) override;

    static bool isValidOffsetFromUtc(int offset);

    bool _showUtcTime;
    int _offsetFromUtc;
    QString _dateTimeFmt;
    QCPAxis *_parentAxis;
};

#endif // DATETIMETICKER_H
