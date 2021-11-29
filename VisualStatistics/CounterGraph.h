#ifndef COUNTERGRAPH_H
#define COUNTERGRAPH_H

#include "qcustomplot/qcustomplot.h"

struct CounterData
{
    CounterData();

    QSet<double> suspectKeys;
    QSharedPointer<QCPGraphDataContainer> data;

    static bool isZeroData(QSharedPointer<QCPGraphDataContainer> data);
};

class CounterGraph : public QCPGraph
{
    Q_OBJECT

public:
    CounterGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);

    void setPen(const QPen &pen);

    QString moduleName() const;
    void setModuleName(const QString &name);
    QString fullName() const;
    void setFullName(const QString &fullName);
    void setScatterVisible(bool visible);
    void setSuspectKeys(const QSet<double> *suspectKeys);

    static const QChar nameSeparator;
    static QString getModuleName(const QString &fullName);
    static QString getNameRightPart(const QString &name);
    static QPair<QString, QString> separateModuleName(const QString &fullName);
    static const QPainterPath &suspectPainterPath();

public slots:
    void setSelected(bool selected);

private:
    void getScatters(QVector<QPointF> *scatters, QVector<QPointF> *suspectScatters, const QCPDataRange &dataRange) const;
    virtual void draw(QCPPainter *painter) override;
    virtual void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const override;

    QString _moduleName;
    QString _fullName;
    const QSet<double> *_suspectKeys;
    QCPScatterStyle _suspectScatterStyle;

    friend class CounterLegendItem;
};

#endif // COUNTERGRAPH_H
