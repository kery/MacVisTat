#ifndef COUNTERGRAPH_H
#define COUNTERGRAPH_H

#include "third_party/qcustomplot/qcustomplot.h"

class CounterGraph : public QCPGraph
{
    Q_OBJECT

public:
    CounterGraph(QCPAxis *keyAxis, QCPAxis *valueAxis, const QString &node, const QString &module);

    void setShowNode(bool show);
    QString node() const;
    void setShowModule(bool show);
    void setDisplayName(const QString &name);
    QString displayName() const;
    QString realDisplayName() const;

    void setSuspectFlagScatterStyle(const QCPScatterStyle &ssSuspect);

    virtual bool addToLegend();

protected:
    virtual void draw(QCPPainter *painter);
    virtual void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const;
    virtual void drawScatterPlot(QCPPainter *painter, QVector<QCPData> *scatterData) const;

private:
    bool m_showNode;
    bool m_showModule;
    QString m_node;
    QString m_module;
    QString m_displayName;
    QCPScatterStyle m_ssSuspect; // scatter style for suspect (<suspect>true</suspect>) value

    friend class CounterLegendItem;
};

#endif // COUNTERGRAPH_H
