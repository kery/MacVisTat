#ifndef VALUETIPLABEL_H
#define VALUETIPLABEL_H

#include <qlabel.h>

// Reference:
// Qt source src/widgets/kernel/qtooltip.cpp

class ValueTipLabel : public QLabel
{
    Q_OBJECT
public:
    ValueTipLabel();

    void show(const QPoint &point, const QRect &rect, const QString &text);

private:
    void paintEvent(QPaintEvent *ev);
    void resizeEvent(QResizeEvent *e);

    void updateSize();
    void placeTip(const QPoint &point, const QRect &rect);
};

#endif // VALUETIPLABEL_H
