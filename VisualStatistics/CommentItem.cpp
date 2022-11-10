#include "CommentItem.h"
#include "CounterGraph.h"
#include "QCustomPlot/src/items/item-tracer.h"
#include "QCustomPlot/src/items/item-line.h"

CommentItem::CommentItem(QCustomPlot *parent) :
    TextItem(parent),
    mTracer(nullptr),
    mLine(nullptr)
{
}

CommentItem::~CommentItem()
{
    // If this comment item object is destroyed in QCustomPlot::clearItems, mTracer and
    // mLine will be destroyed before this object. So, it's necessary to use hasItem to
    // check they still exist.
    if (mTracer != nullptr && mParentPlot->hasItem(mTracer)) {
        mParentPlot->removeItem(mTracer);
    }
    if (mLine != nullptr && mParentPlot->hasItem(mLine)) {
        mParentPlot->removeItem(mLine);
    }
}

QSizeF CommentItem::size() const
{
    QPointF tl = topLeft->pixelPosition();
    QPointF br = bottomRight->pixelPosition();
    return QSize(qRound(br.x() - tl.x()), qRound(br.y() - tl.y()));
}

void CommentItem::setGraphAndKey(CounterGraph *graph, double key)
{
    mTracer = new QCPItemTracer(mParentPlot);
    mTracer->setVisible(false);
    // call setGraphKey first so that the position can be updated by
    // updatePosition which is called in setGraph.
    mTracer->setGraphKey(key);
    mTracer->setGraph(graph);

    mLine = new QCPItemLine(mParentPlot);
    mLine->end->setParentAnchor(mTracer->position);
    mLine->setHead(QCPLineEnding::esSpikeArrow);
    mLine->setSelectable(false);

    QPen pen = mLine->pen();
    pen.setStyle(Qt::DashLine);
    mLine->setPen(pen);
}

CounterGraph * CommentItem::graph() const
{
    if (mTracer != nullptr) {
        return qobject_cast<CounterGraph *>(mTracer->graph());
    }
    return nullptr;
}

void CommentItem::updateLineStartAnchor()
{
    if (mLine == nullptr) {
        return;
    }

    QPointF tracerPos = mTracer->position->pixelPosition();
    QSizeF ciSize = size();
    QPointF leftPos = left->pixelPosition(), rightPos = right->pixelPosition();
    QPointF topPos = top->pixelPosition(), bottomPos = bottom->pixelPosition();
    if (tracerPos.x() < leftPos.x() + ciSize.width() / 4) {
        if (tracerPos.y() < topPos.y() + ciSize.height() / 4) {
            mLine->start->setParentAnchor(topLeft);
        } else if (tracerPos.y() < bottomPos.y() - ciSize.height() / 4) {
            mLine->start->setParentAnchor(left);
        } else {
            mLine->start->setParentAnchor(bottomLeft);
        }
    } else if (tracerPos.x() < rightPos.x() - ciSize.width() / 4) {
        if (tracerPos.y() < topPos.y()) {
            mLine->start->setParentAnchor(top);
        } else {
            mLine->start->setParentAnchor(bottom);
        }
    } else {
        if (tracerPos.y() < topPos.y() + ciSize.height() / 4) {
            mLine->start->setParentAnchor(topRight);
        } else if (tracerPos.y() < bottomPos.y() - ciSize.height() / 4) {
            mLine->start->setParentAnchor(right);
        } else {
            mLine->start->setParentAnchor(bottomRight);
        }
    }
}

void CommentItem::setVisible(bool on)
{
    if (mLine) {
        mLine->setVisible(on);
    }
    TextItem::setVisible(on);
}

double CommentItem::selectTest(const QPointF &pos, bool onlySelectable, QVariant * /*details*/) const
{
    if (onlySelectable && !mSelectable) {
        return -1;
    }

    // The rect may be rotated, so we transform the actual clicked pos to the rotated
    // coordinate system, so we can use the normal rectSelectTest function for non-rotated rects:
    QPointF positionPixels(position->pixelPosition());
    QTransform inputTransform;
    inputTransform.translate(positionPixels.x(), positionPixels.y());
    inputTransform.rotate(-mRotation);
    inputTransform.translate(-positionPixels.x(), -positionPixels.y());
    QPointF rotatedPos = inputTransform.map(pos);
    QFontMetrics fontMetrics(mFont);
    QRect textRect = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip | mTextAlignment, mText);
    QRect textBoxRect = textRect.adjusted(-mPadding.left(), -mPadding.top(), mPadding.right(), mPadding.bottom());
    QPointF textPos = getTextDrawPoint(positionPixels, textBoxRect, mPositionAlignment);
    textBoxRect.moveTopLeft(textPos.toPoint());

    return textBoxRect.contains(rotatedPos.toPoint()) ? mParentPlot->selectionTolerance() * 0.99 : -1;
}
