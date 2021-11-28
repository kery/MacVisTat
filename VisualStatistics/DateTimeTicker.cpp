#include "DateTimeTicker.h"
#include "Utils.h"

DateTimeTicker::DateTimeTicker(QCPAxis *parentAxis) :
    _showUtcTime(false),
    _skippedTicks(-1),
    _offsetFromUtc(0),
    _dateTimeFmt(DTFMT_DISPLAY),
    _parentAxis(parentAxis)
{
    _parentAxis->setTickLabelRotation(90);
}

void DateTimeTicker::setShowUtcTime(bool showUtc)
{
    _showUtcTime = showUtc;
}

void DateTimeTicker::setOffsetFromUtc(int offset)
{
    if (isValidOffsetFromUtc(offset)) {
        _offsetFromUtc = offset;
    }
}

int DateTimeTicker::getSubTickCount(double tickStep)
{
    Q_UNUSED(tickStep)
    return 0;
}

QString DateTimeTicker::getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision)
{
    Q_UNUSED(locale)
    Q_UNUSED(formatChar)
    Q_UNUSED(precision)
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(tick);
    if (_showUtcTime) {
        dateTime.setOffsetFromUtc(_offsetFromUtc);
        dateTime = dateTime.toUTC();
    }
    return dateTime.toString(_dateTimeFmt);
}

QVector<double> DateTimeTicker::createTickVector(double tickStep, const QCPRange &range)
{
    Q_UNUSED(tickStep)
    QVector<double> result;
    QCustomPlot *plot = _parentAxis->parentPlot();
    if (plot->graphCount() > 0 && plot->graph(0)->dataCount() > 0) {
        QSharedPointer<QCPGraphDataContainer> data = plot->graph(0)->data();
        double fontHeight = QFontMetricsF(_parentAxis->tickLabelFont()).height();
        double prePos = -fontHeight;
        auto iterBegin = data->findBegin(range.lower, false), iterEnd = data->findEnd(range.upper);
        for (auto iter = iterBegin; iter != iterEnd; ++iter) {
            double curPos = _parentAxis->coordToPixel(iter->key);
            if (curPos - prePos >= fontHeight) {
                result.append(iter->key);
                prePos = curPos;
            }
        }
        int skippedTicks = iterEnd - iterBegin - result.size();
        if (skippedTicks != _skippedTicks) {
            _skippedTicks = skippedTicks;
            emit skippedTicksChanged(_skippedTicks);
        }
    }
    return result;
}

bool DateTimeTicker::isValidOffsetFromUtc(int offset)
{
    // https://en.wikipedia.org/wiki/List_of_UTC_time_offsets
    // >= -12:00 <= +14:00
    return offset >= -(12 * 3600) && offset <= (14 * 3600);
}
