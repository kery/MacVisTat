#ifndef BALLOONTIP_H
#define BALLOONTIP_H

#include <QLabel>

class BalloonTip : public QLabel
{
    Q_OBJECT

public:
    BalloonTip();

private:
    virtual QSize sizeHint() const override;
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // BALLOONTIP_H
