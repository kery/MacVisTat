#include "CounterGraph.h"
#include "CounterNameModel.h"

CounterGraph::CounterGraph(QCPAxis *keyAxis, QCPAxis *valueAxis) :
    QCPGraph(keyAxis, valueAxis),
    mSuspectKeys(nullptr),
    mSuspectScatterStyle(QCPScatterStyle::ssCross)
{
}

void CounterGraph::setPen(const QPen &pen)
{
    mPen = pen;
    mSelectionDecorator->setPen(pen);
}

void CounterGraph::setScatterVisible(bool visible)
{
    mScatterStyle.setShape(visible ? QCPScatterStyle::ssDisc : QCPScatterStyle::ssNone);
}

const QSet<double> *CounterGraph::suspectKeys() const
{
    return mSuspectKeys;
}

void CounterGraph::setSuspectKeys(const QSet<double> *suspectKeys)
{
    mSuspectKeys = suspectKeys;
}

bool CounterGraph::isSuspect(double key)
{
    return mSuspectKeys->contains(key);
}

QCPRange CounterGraph::getKeyRange(bool &foundRange, QCP::SignDomain /*inSignDomain*/) const
{
    if (mDataContainer->isEmpty()) {
        foundRange = false;
        return QCPRange();
    }
    foundRange = true;

    // Always return the full range of keys no matter the value is NAN or not.
    QCPRange result;
    auto iter = mDataContainer->begin();
    result.lower = iter->key;

    iter = mDataContainer->end() - 1;
    result.upper = iter->key;
    return result;
}

void CounterGraph::setSelected(bool selected)
{
    setSelection(selected ? QCPDataSelection(QCPDataRange(0, 1)) : QCPDataSelection());
}

void CounterGraph::getScatters(QVector<QPointF> *scatters, QVector<QPointF> *suspectScatters, const QCPDataRange &dataRange) const
{
    QCPAxis *keyAxis = mKeyAxis.data();
    QCPAxis *valueAxis = mValueAxis.data();
    if (!keyAxis || !valueAxis) {
        qDebug() << Q_FUNC_INFO << "invalid key or value axis";
        scatters->clear();
        suspectScatters->clear();
        return;
    }

    QCPGraphDataContainer::const_iterator begin, end;
    getVisibleDataBounds(begin, end, dataRange);
    if (begin == end) {
        scatters->clear();
        suspectScatters->clear();
        return;
    }

    QVector<QCPGraphData> data;
    getOptimizedScatterData(&data, begin, end);

    // make sure key pixels are sorted ascending in data (significantly simplifies following processing)
    if (mKeyAxis->rangeReversed() != (mKeyAxis->orientation() == Qt::Vertical)) {
        std::reverse(data.begin(), data.end());
    }

    scatters->reserve(data.size() - mSuspectKeys->size());
    suspectScatters->reserve(mSuspectKeys->size());
    if (keyAxis->orientation() == Qt::Vertical) {
        for (int i = 0; i < data.size(); ++i) {
            if (!qIsNaN(data.at(i).value)) {
                QPointF pos(valueAxis->coordToPixel(data.at(i).value),
                            keyAxis->coordToPixel(data.at(i).key));
                if (mSuspectKeys->contains(data.at(i).key)) {
                    suspectScatters->append(pos);
                } else {
                    scatters->append(pos);
                }
            }
        }
    } else {
        for (int i = 0; i < data.size(); ++i) {
            if (!qIsNaN(data.at(i).value)) {
                QPointF pos(keyAxis->coordToPixel(data.at(i).key),
                            valueAxis->coordToPixel(data.at(i).value));
                if (mSuspectKeys->contains(data.at(i).key)) {
                    suspectScatters->append(pos);
                } else {
                    scatters->append(pos);
                }
            }
        }
    }
}

void CounterGraph::draw(QCPPainter *painter)
{
    if (!mKeyAxis || !mValueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
    if (mKeyAxis.data()->range().size() <= 0 || mDataContainer->isEmpty()) { return; }
    if (mLineStyle == lsNone && mScatterStyle.isNone()) { return; }

    // line and (if necessary) scatter pixel coordinates will be stored here while iterating over segments
    QVector<QPointF> lines, scatters, suspectScatters;

    // loop over and draw segments of unselected/selected data:
    QList<QCPDataRange> selectedSegments, unselectedSegments, allSegments;
    getDataSegments(selectedSegments, unselectedSegments);
    allSegments << unselectedSegments << selectedSegments;
    for (int i = 0; i < allSegments.size(); ++i) {
        bool isSelectedSegment = i >= unselectedSegments.size();
        // get line pixel points appropriate to line style:
        // unselected segments extend lines to bordering selected data point (safe to exceed total data bounds in first/last segment, getLines takes care)
        QCPDataRange lineDataRange = isSelectedSegment ? allSegments.at(i) : allSegments.at(i).adjusted(-1, 1);
        getLines(&lines, lineDataRange);

        // check data validity if flag set:
#ifdef QCUSTOMPLOT_CHECK_DATA
        QCPGraphDataContainer::const_iterator it;
        for (it = mDataContainer->constBegin(); it != mDataContainer->constEnd(); ++it) {
            if (QCP::isInvalidData(it->key, it->value)) {
                qDebug() << Q_FUNC_INFO << "Data point at" << it->key << "invalid." << "Plottable name:" << name();
            }
        }
#endif

        // draw fill of graph:
        if (isSelectedSegment && mSelectionDecorator) {
            mSelectionDecorator->applyBrush(painter);
        } else {
            painter->setBrush(mBrush);
        }
        painter->setPen(Qt::NoPen);
        drawFill(painter, &lines);

        // draw line:
        if (mLineStyle != lsNone) {
            if (isSelectedSegment && mSelectionDecorator) {
                mSelectionDecorator->applyPen(painter);
            } else {
                painter->setPen(mPen);
            }
            painter->setBrush(Qt::NoBrush);
            if (mLineStyle == lsImpulse) {
                drawImpulsePlot(painter, lines);
            } else {
                drawLinePlot(painter, lines); // also step plots can be drawn as a line plot
            }
        }

        // draw scatters:
        QCPScatterStyle finalScatterStyle = mScatterStyle;
        if (isSelectedSegment && mSelectionDecorator) {
            finalScatterStyle = mSelectionDecorator->getFinalScatterStyle(mScatterStyle);
        }
        if (!finalScatterStyle.isNone() || !mSuspectKeys->isEmpty()) {
            getScatters(&scatters, &suspectScatters, allSegments.at(i));
            drawScatterPlot(painter, scatters, finalScatterStyle);
            drawScatterPlot(painter, suspectScatters, mSuspectScatterStyle);
        }
    }

    // draw other selection decoration that isn't just line/scatter pens and brushes:
    if (mSelectionDecorator) {
        mSelectionDecorator->drawDecoration(painter, selection());
    }
}

void CounterGraph::drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
{
    painter->fillRect(rect, mPen.color());
}
