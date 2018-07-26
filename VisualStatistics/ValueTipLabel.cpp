#include "ValueTipLabel.h"
#include <qstylepainter.h>
#include <qstyleoption.h>
#include <qtooltip.h>

ValueTipLabel::ValueTipLabel() :
    QLabel(NULL, Qt::ToolTip | Qt::BypassGraphicsProxyWidget)
{
    setForegroundRole(QPalette::ToolTipText);
    setBackgroundRole(QPalette::ToolTipBase);
    setPalette(QToolTip::palette());
    ensurePolished();
    setMargin(1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this));
    setFrameStyle(QFrame::NoFrame);
    setAlignment(Qt::AlignLeft);
    setIndent(1);
    setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / 255.0);
}

void ValueTipLabel::show(const QPoint &point, const QRect &rect, const QString &text)
{
    setText(text);
    updateSize();
    placeTip(point, rect);

    if (!isVisible()) {
        showNormal();
    }
}

void ValueTipLabel::paintEvent(QPaintEvent *ev)
{
    QStylePainter p(this);
    QStyleOptionFrame opt;
    opt.init(this);
    p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
    p.end();

    QLabel::paintEvent(ev);
}

void ValueTipLabel::resizeEvent(QResizeEvent *e)
{
    QStyleHintReturnMask frameMask;
    QStyleOption option;
    option.init(this);
    if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask))
        setMask(frameMask.region);

    QLabel::resizeEvent(e);
}

void ValueTipLabel::updateSize()
{
    QFontMetrics fm(font());
    QSize extra(1, 0);
    if (fm.descent() == 2 && fm.ascent() >= 11) {
        ++extra.rheight();
    }
    resize(sizeHint() + extra);
}

void ValueTipLabel::placeTip(const QPoint &point, const QRect &rect)
{
    const int OFFSET = 6;
    QPoint pos(point.x() + OFFSET, point.y() + OFFSET);
    
    if (pos.x() + width() > rect.right()) {
        pos.rx() = point.x() - OFFSET - width();
    }
    if (pos.y() + height() > rect.bottom()) {
        pos.ry() = point.y() - OFFSET - height();
    }

    move(pos);
}