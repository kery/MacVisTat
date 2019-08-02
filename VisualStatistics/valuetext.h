#ifndef VALUETEXT_H
#define VALUETEXT_H

#include <third_party/qcustomplot/qcustomplot.h>

class ValueText : public QCPItemText
{
    Q_OBJECT

public:
    ValueText(const QCPItemTracer *tracer);

protected:
    virtual void draw(QCPPainter *painter);
};

#endif // VALUETEXT_H
