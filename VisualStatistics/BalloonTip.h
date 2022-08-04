#ifndef BALLOONTIP_H
#define BALLOONTIP_H

#include <QLabel>
#include <QVariantAnimation>

class BalloonTip : public QLabel
{
    Q_OBJECT

public:
    static BalloonTip *create();

    void riseUp();

private:
    BalloonTip();

    virtual QSize sizeHint() const override;
    virtual void paintEvent(QPaintEvent *event) override;

    Q_SLOT void animationValueChange(const QVariant &value);
    Q_SLOT void animationFinished();

    QPoint mStartPos;
    QVariantAnimation  mAnimation;
};

#endif // BALLOONTIP_H
