#ifndef COLORGENERATOR_H
#define COLORGENERATOR_H

#include <QColor>
#include <QPair>

class ColorGenerator
{
public:
    ColorGenerator(int s, int l);
    QColor genColor();

    static int colorCount();

private:
    int calculateH();

private:
    int _count;
    int _s, _l;
    QList<QPair<int, int> > _hList1, _hList2, *_ptr;

    static const QColor _predefined[8];
};

#endif // COLORGENERATOR_H
