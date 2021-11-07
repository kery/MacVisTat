#ifndef ITEMTEXT_H
#define ITEMTEXT_H

#include <qcustomplot.h>

class ItemText : public QCPItemText
{
    Q_OBJECT
public:
    ItemText(QCustomPlot *parentPlot);

protected:
    virtual void draw(QCPPainter *painter) override;
};

#endif // ITEMTEXT_H
