#ifndef COMMENTTEXT_H
#define COMMENTTEXT_H

#include "ItemText.h"

class CommentText : public ItemText
{
    Q_OBJECT
public:
    CommentText(QCustomPlot *parentPlot);
    virtual ~CommentText();

    QSize size() const;
    void setGraphAndKey(QCPGraph *graph, double key);
    QCPGraph * graph() const;
    void updateLineStartAnchor();
    void setVisible (bool on);

    virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;

private:
    QCPItemTracer *m_tracer;
    QCPItemLine *m_line;
};

#endif // COMMENTTEXT_H
