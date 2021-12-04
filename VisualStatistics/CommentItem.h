#ifndef COMMENTITEM_H
#define COMMENTITEM_H

#include "TextItem.h"

class CounterGraph;

class CommentItem : public TextItem
{
    Q_OBJECT

public:
    CommentItem(QCustomPlot *parent);
    ~CommentItem();

    QSizeF size() const;
    void setGraphAndKey(CounterGraph *graph, double key);
    CounterGraph *graph() const;
    void updateLineStartAnchor();
    void setVisible(bool on);

    virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;

private:
    QCPItemTracer *mTracer;
    QCPItemLine *mLine;
};

#endif // COMMENTITEM_H
