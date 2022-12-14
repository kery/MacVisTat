#ifndef VALUETIPITEM_H
#define VALUETIPITEM_H

#include "TextItem.h"

class CounterGraph;
class QCPItemTracer;

class ValueTipItem : public TextItem
{
    Q_OBJECT

public:
    ValueTipItem(QCustomPlot *plot);

    QString graphName() const;
    QString graphValue() const;
    void setSelected(bool selected);
    CounterGraph * tracerGraph() const;
    QCPItemPosition * tracerPosition() const;
    void setTracerGraph(CounterGraph *graph);
    double tracerGraphKey() const;
    void setTracerGraphKey(double key);
    void setTracerPen(const QPen &pen);
    void setValueInfo(const QString &name, const QDateTime &dateTime, const QString &value, bool suspect);
    void showWithAnimation();
    void hideWithAnimation();

    static const QString sLayerName;

private:
    void setVisible(bool on);

    Q_SLOT void animationValueChange(const QVariant &value);
    Q_SLOT void animationFinished();

    virtual void draw(QCPPainter *painter) override;

    QString mGraphName;
    QString mGraphValue;
    QCPItemTracer *mTracer;
    QPropertyAnimation mAnimation;
};

#endif // VALUETIPITEM_H
