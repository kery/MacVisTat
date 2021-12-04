#include "DateTimeTicker.h"
#include "Utils.h"

DateTimeTicker::DateTimeTicker(QCPAxis *parentAxis) :
    mDisplayUtc(false),
    mSkippedTicks(-1),
    mOffsetFromUtc(0),
    mDateTimeFmt(DTFMT_DISPLAY),
    mParentAxis(parentAxis)
{
    mParentAxis->setTickLabelRotation(90);
}

void DateTimeTicker::setDisplayUtc(bool displayUtc)
{
    mDisplayUtc = displayUtc;
}

void DateTimeTicker::setOffsetFromUtc(int offset)
{
    if (isValidOffsetFromUtc(offset)) {
        mOffsetFromUtc = offset;
    }
}

void DateTimeTicker::setDateTimeVector(QVector<double> &&dtv)
{
    mDateTimeVector.swap(dtv);
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
    QDateTime dateTime;
    if (mDateTimeVector.isEmpty()) {
        dateTime = QDateTime::fromSecsSinceEpoch(tick);
    } else {
        int index = static_cast<int>(tick);
        if (index >= 0 && index < mDateTimeVector.size()) {
            dateTime = QDateTime::fromSecsSinceEpoch(mDateTimeVector[index]);
        }
    }
    if (!dateTime.isNull()) {
        if (mDisplayUtc) {
            dateTime.setOffsetFromUtc(mOffsetFromUtc);
            dateTime = dateTime.toUTC();
        }
        return dateTime.toString(mDateTimeFmt);
    }
    return QString();
}

QVector<double> DateTimeTicker::createTickVector(double tickStep, const QCPRange &range)
{
    Q_UNUSED(tickStep)
    QVector<double> result;
    QCustomPlot *plot = mParentAxis->parentPlot();
    if (plot->graphCount() > 0 && plot->graph(0)->dataCount() > 0) {
        QSharedPointer<QCPGraphDataContainer> data = plot->graph(0)->data();
        double fontHeight = QFontMetricsF(mParentAxis->tickLabelFont()).height();
        double prePos = -fontHeight;
        auto iterBegin = data->findBegin(range.lower, false), iterEnd = data->findBegin(range.upper, false);
        for (auto iter = iterBegin; iter != iterEnd; ++iter) {
            double curPos = mParentAxis->coordToPixel(iter->key);
            if (curPos - prePos >= fontHeight) {
                result.append(iter->key);
                prePos = curPos;
            }
        }
        int skippedTicks = iterEnd - iterBegin - result.size();
        if (skippedTicks != mSkippedTicks) {
            mSkippedTicks = skippedTicks;
            emit skippedTicksChanged(mSkippedTicks);
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
