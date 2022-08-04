#include "BalloonTip.h"
#include <QPainter>

BalloonTip *BalloonTip::create()
{
    return new BalloonTip();
}

void BalloonTip::riseUp()
{
    mStartPos = QCursor::pos();
    move(mStartPos);
    show();
    mAnimation.start();
}

BalloonTip::BalloonTip() :
    QLabel(nullptr, Qt::ToolTip | Qt::BypassGraphicsProxyWidget)
{
    // We want rounded rectangle, this is mandatory to prevent each corner pixels
    // from being drawn.
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // Make cursor pass through this widget.
    setWindowFlag(Qt::WindowTransparentForInput);

    mAnimation.setDuration(1000);
    mAnimation.setStartValue(QVariant(1.0));
    mAnimation.setEndValue(QVariant(0.0));
    connect(&mAnimation, &QVariantAnimation::valueChanged, this, &BalloonTip::animationValueChange);
    connect(&mAnimation, &QVariantAnimation::finished, this, &BalloonTip::animationFinished);
}

QSize BalloonTip::sizeHint() const
{
    QSize result;
    QFontMetrics fontMetrics(font());
    QRect textRect = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextSingleLine | Qt::AlignCenter | Qt::TextDontClip,
                                              text());
    result.setWidth(textRect.width() + 2 * 6);
    result.setHeight(textRect.height() + 2 * 4);
    return result;
}

void BalloonTip::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    QLinearGradient gradient(0, 0, 0, 1);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(1, QColor(250, 250, 250));

    painter.save();
    painter.setBrush(gradient);
    painter.setPen(QColor(160, 160, 160));
    painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 2, 2);
    painter.restore();
    painter.drawText(rect(), Qt::TextSingleLine | Qt::AlignCenter | Qt::TextDontClip, text());
}

void BalloonTip::animationValueChange(const QVariant &value)
{
    double v = value.toDouble();
    QPoint newPos(mStartPos.x(), mStartPos.y() - (1 - v) * 50);
    move(newPos);
    setWindowOpacity(v);
}

void BalloonTip::animationFinished()
{
    deleteLater();
}
