#ifndef COUNTERGRAPH_H
#define COUNTERGRAPH_H

#include <qcustomplot.h>

class CounterGraph : public QCPGraph
{
    Q_OBJECT

public:
    CounterGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);

    void setPen(const QPen &pen);
    QString displayName() const;
    void setShortName(const QString &name);
    void setScatterVisible(bool visible);
    const QSet<double> *suspectKeys() const;
    void setSuspectKeys(const QSet<double> *suspectKeys);
    bool isSuspect(double key);

    virtual QCPRange getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain=QCP::sdBoth) const override;

public slots:
    void setSelected(bool selected);

private:
    void getScatters(QVector<QPointF> *scatters, QVector<QPointF> *suspectScatters, const QCPDataRange &dataRange) const;
    virtual void draw(QCPPainter *painter) override;
    virtual void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const override;

    QString mShortName;
    const QSet<double> *mSuspectKeys;
    QCPScatterStyle mSuspectScatterStyle;

    friend class CounterLegendItem;
};

#endif // COUNTERGRAPH_H
