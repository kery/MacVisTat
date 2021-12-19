#include "BalloonTip.h"
#include <QPainter>

BalloonTip::BalloonTip() :
    QLabel(nullptr, Qt::ToolTip | Qt::BypassGraphicsProxyWidget)
{
    // We want rounded rectangle, this is mandatory to prevent each corner pixels
    // from being drawn.
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setWindowFlag(Qt::WindowTransparentForInput);
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
    QLinearGradient gradiend(0, 0, 0, 1);
    gradiend.setCoordinateMode(QGradient::ObjectBoundingMode);
    gradiend.setColorAt(0, Qt::white);
    gradiend.setColorAt(1, QColor(250, 250, 250));

    painter.save();
    painter.setBrush(gradiend);
    painter.setPen(QColor(160, 160, 160));
    painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 2, 2);
    painter.restore();
    painter.drawText(rect(), Qt::TextSingleLine | Qt::AlignCenter | Qt::TextDontClip, text());
}
