#ifndef VALUETEXT_H
#define VALUETEXT_H

#include <third_party/qcustomplot/qcustomplot.h>

class ValueText : public QCPItemText
{
    Q_OBJECT

public:
    ValueText(const QCPItemTracer *tracer);

    void setGraphName(const QString &name);
    void setDateTime(const QString &dt);
    void setGraphValue(const QString &value);
    QString graphName() const;
    QString graphValue() const;

    void updateText();

protected:
    virtual void draw(QCPPainter *painter);

private:
    QString m_graphName;
    QString m_dateTime;
    QString m_graphValue;

    static const double PosOffset;
};

#endif // VALUETEXT_H
