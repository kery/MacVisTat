#ifndef COUNTERGRAPH_H
#define COUNTERGRAPH_H

#include "qcustomplot/qcustomplot.h"

struct CounterData
{
    QSet<double> suspectKeys;
    QCPGraphDataContainer data;

    static void dummyDeleter(QCPGraphDataContainer *data);
    static bool isAllZero(QSharedPointer<QCPGraphDataContainer> data);
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
    bool isSuspect(double key);

    static const QChar sNameSeparator;
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

    QString mModuleName;
    QString mFullName;
    const QSet<double> *mSuspectKeys;
    QCPScatterStyle mSuspectScatterStyle;

    friend class CounterLegendItem;
};

#endif // COUNTERGRAPH_H
