#ifndef TEXTITEM_H
#define TEXTITEM_H

#include "QCustomPlot/src/items/item-text.h"

class TextItem : public QCPItemText
{
public:
    TextItem(QCustomPlot *plot);

protected:
    virtual void draw(QCPPainter *painter) override;
};

#endif // TEXTITEM_H
