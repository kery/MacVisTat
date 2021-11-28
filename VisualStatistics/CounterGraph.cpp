#include "CounterGraph.h"

CounterData::CounterData() :
    data(new QCPGraphDataContainer())
{
}

const QChar CounterGraph::nameSeparator(',');

CounterGraph::CounterGraph(QCPAxis *keyAxis, QCPAxis *valueAxis) :
    QCPGraph(keyAxis, valueAxis),
    _suspectKeys(nullptr),
    _suspectScatterStyle(QCPScatterStyle::ssCross)
{
}

void CounterGraph::setPen(const QPen &pen)
{
    mPen = pen;
    mScatterStyle.setPen(pen);
    _suspectScatterStyle.setPen(pen);
}

void CounterGraph::setData(QSharedPointer<QCPGraphDataContainer> data, const QSet<double> *suspectKeys)
{
    QCPGraph::setData(data);
    _suspectKeys = suspectKeys;
}

QString CounterGraph::moduleName() const
{
    return _moduleName;
}

void CounterGraph::setModuleName(const QString &name)
{
    _moduleName = name;
}

void CounterGraph::setScatterVisible(bool visible)
{
    mScatterStyle.setShape(visible ? QCPScatterStyle::ssDisc : QCPScatterStyle::ssNone);
}

QString CounterGraph::getModuleName(const QString &fullName)
{
    int index = fullName.indexOf(nameSeparator);
    if (index > 0) {
        return fullName.left(index);
    }
    return QString();
}

QPair<QString, QString> CounterGraph::separateModuleName(const QString &fullName)
{
    QPair<QString, QString> result;
    int index = fullName.indexOf(nameSeparator);
    if (index > 0) {
        result.first = fullName.left(index);
        result.second = fullName.mid(index + 1);
    } else {
        result.second = fullName;
    }
    return result;
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

    scatters->reserve(data.size() - _suspectKeys->size());
    suspectScatters->reserve(_suspectKeys->size());
    if (keyAxis->orientation() == Qt::Vertical) {
        for (int i = 0; i < data.size(); ++i) {
            if (!qIsNaN(data.at(i).value)) {
                QPointF pos(valueAxis->coordToPixel(data.at(i).value),
                            keyAxis->coordToPixel(data.at(i).key));
                if (_suspectKeys->contains(data.at(i).key)) {
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
                if (_suspectKeys->contains(data.at(i).key)) {
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
        if (!finalScatterStyle.isNone() || !_suspectKeys->isEmpty()) {
            getScatters(&scatters, &suspectScatters, allSegments.at(i));
            drawScatterPlot(painter, scatters, finalScatterStyle);
            drawScatterPlot(painter, suspectScatters, _suspectScatterStyle);
        }
    }

    // draw other selection decoration that isn't just line/scatter pens and brushes:
    if (mSelectionDecorator) {
        mSelectionDecorator->drawDecoration(painter, selection());
    }
}

void CounterGraph::drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
{
    applyFillAntialiasingHint(painter);
    painter->fillRect(rect.adjusted(1, 1, -2, -2), mPen.color());
}
