#ifndef VALUETIPITEM_H
#define VALUETIPITEM_H

#include "TextItem.h"

class CounterGraph;

class ValueTipItem : public TextItem
{
    Q_OBJECT

public:
    ValueTipItem(QCustomPlot *plot);

    void setSelected(bool selected);
    CounterGraph *tracerGraph() const;
    void setTracerGraph(CounterGraph *graph);
    double tracerGraphKey() const;
    void setTracerGraphKey(double key);
    void setTracerPen(const QPen &pen);
    void setValueInfo(const QString &name, const QString &dateTime, const QString &value, bool suspect);
    void showWithAnimation();
    void hideWithAnimation();

    static QString layerName();

private:
    void setVisible(bool on);

    Q_SLOT void animValueChange(const QVariant &value);
    Q_SLOT void animFinished();

    virtual void draw(QCPPainter *painter) override;

    QString mGraphName;
    QString mGraphValue;
    QCPItemTracer *mTracer;
    QPropertyAnimation mAnimation;
};

#endif // VALUETIPITEM_H
