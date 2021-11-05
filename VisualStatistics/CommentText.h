#ifndef COMMENTTEXT_H
#define COMMENTTEXT_H

#include <qcustomplot.h>

class CommentText : public QCPItemText
{
    Q_OBJECT
public:
    CommentText(QCustomPlot *parentPlot);
    virtual ~CommentText();

    QSize size() const;
    void setGraphAndKey(QCPGraph *graph, double key);
    QCPGraph * graph() const;
    void updateTracerLineVisible();

    virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const;

private:
    QCPItemTracer *m_tracer;
    QCPItemLine *m_line;
};

#endif // COMMENTTEXT_H
