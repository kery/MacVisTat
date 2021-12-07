#include "DateTimeTicker.h"
#include "GlobalDefines.h"

DateTimeTicker::DateTimeTicker(QCPAxis *parentAxis) :
    mUtcDisplay(false),
    mSkippedTicks(-1),
    mOffsetFromUtc(0),
    mDateTimeFmt(DTFMT_DISPLAY),
    mParentAxis(parentAxis)
{
    mParentAxis->setTickLabelRotation(90);
}

int DateTimeTicker::skippedTicks() const
{
    return mSkippedTicks;
}

bool DateTimeTicker::utcDisplay() const
{
    return mUtcDisplay;
}

void DateTimeTicker::setUtcDisplay(bool on)
{
    mUtcDisplay = on;
}

void DateTimeTicker::setOffsetFromUtc(int offset)
{
    mOffsetFromUtc = offset;
}

void DateTimeTicker::setDateTimeVector(QVector<double> &&dtv)
{
    mDateTimeVector.swap(dtv);
}

bool DateTimeTicker::setBeginDateTime(const QDateTime &dateTime)
{
    double key = dateTimeToKey(dateTime);
    if (key > -1) {
        mParentAxis->setRangeLower(key);
        return true;
    }
    return false;
}

bool DateTimeTicker::setEndDateTime(const QDateTime &dateTime)
{
    double key = dateTimeToKey(dateTime);
    if (key > -1) {
        mParentAxis->setRangeUpper(key);
        return true;
    }
    return false;
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
    QDateTime dateTime = dateTimeFromKey(tick);
    return dateTime.toString(mDateTimeFmt);
}

QVector<double> DateTimeTicker::createTickVector(double tickStep, const QCPRange &range)
{
    Q_UNUSED(tickStep)
    QVector<double> result;
    QDateTime beginDateTime, endDateTime;
    QCustomPlot *plot = mParentAxis->parentPlot();
    if (plot->graphCount() > 0 && plot->graph(0)->dataCount() > 0) {
        QSharedPointer<QCPGraphDataContainer> data = plot->graph(0)->data();
        double fontHeight = QFontMetricsF(mParentAxis->tickLabelFont()).height();
        double prePos = -fontHeight;
        auto iterBegin = data->findBegin(range.lower, false), iterEnd = data->findEnd(range.upper, false);
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

        if (!result.isEmpty()) {
            beginDateTime = dateTimeFromKey(iterBegin->key);
        }
        if (iterBegin != iterEnd) {
            endDateTime = dateTimeFromKey((iterEnd - 1)->key);
        }
        if (beginDateTime != mBeginDateTime || beginDateTime.timeSpec() != mBeginDateTime.timeSpec()) {
            mBeginDateTime = beginDateTime;
            emit beginDateTimeChanged(mBeginDateTime);
        }
        if (endDateTime != mEndDateTime || endDateTime.timeSpec() != mEndDateTime.timeSpec()) {
            mEndDateTime = endDateTime;
            emit endDateTimeChanged(mEndDateTime);
        }
    }
    return result;
}

QDateTime DateTimeTicker::dateTimeFromKey(double key) const
{
    QDateTime dateTime;
    if (mDateTimeVector.isEmpty()) {
        dateTime = QDateTime::fromSecsSinceEpoch(key);
    } else {
        int index = static_cast<int>(key);
        if (index >= 0 && index < mDateTimeVector.size()) {
            dateTime = QDateTime::fromSecsSinceEpoch(mDateTimeVector[index]);
        }
    }
    if (!dateTime.isNull() && mUtcDisplay) {
        dateTime.setOffsetFromUtc(mOffsetFromUtc);
        return dateTime.toUTC();
    }
    return dateTime;
}

double DateTimeTicker::dateTimeToKey(const QDateTime &dateTime) const
{
    QDateTime localTime;
    if (dateTime.timeSpec() == Qt::UTC) {
        localTime = dateTime.toOffsetFromUtc(mOffsetFromUtc);
        localTime.setTimeSpec(Qt::LocalTime);
    } else {
        localTime = dateTime;
    }
    if (mDateTimeVector.isEmpty()) {
        return localTime.toSecsSinceEpoch();
    }
    auto iter = std::upper_bound(mDateTimeVector.constBegin(), mDateTimeVector.constEnd(), localTime.toSecsSinceEpoch());
    if (iter != mDateTimeVector.end()) {
        return iter - mDateTimeVector.begin() - 1;
    }
    return -1;
}
