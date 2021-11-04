#ifndef VALUETEXT_H
#define VALUETEXT_H

#include <qcustomplot.h>

class ValueText : public QCPItemText
{
    Q_OBJECT

public:
    ValueText(const QCPItemTracer *tracer);

    void setValueInfo(const QString &name, const QString &dt, const QString &value, bool suspectFlag);
    QString graphName() const;
    QString graphValue() const;

protected:
    virtual void draw(QCPPainter *painter);

private:
    bool m_suspectFlag;
    QString m_graphName;
    QString m_dateTime;
    QString m_graphValue;

    static const double PosOffset;
};

#endif // VALUETEXT_H
