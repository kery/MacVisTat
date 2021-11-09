#include "CommentText.h"

CommentText::CommentText(QCustomPlot *parentPlot) :
    ItemText(parentPlot),
    m_tracer(nullptr),
    m_line(nullptr)
{
    parentPlot->addItem(this);
}

CommentText::~CommentText()
{
    if (m_line != nullptr) {
        delete m_line;
    }
    if (m_tracer != nullptr) {
        delete m_tracer;
    }
}

QSize CommentText::size() const
{
    QPointF tl = topLeft->pixelPoint();
    QPointF br = bottomRight->pixelPoint();
    return QSize(qRound(br.x() - tl.x()), qRound(br.y() - tl.y()));
}

void CommentText::setGraphAndKey(QCPGraph *graph, double key)
{
    Q_ASSERT(m_line == nullptr && m_tracer == nullptr);

    m_tracer = new QCPItemTracer(mParentPlot);
    m_tracer->setStyle(QCPItemTracer::tsCircle);
    m_tracer->setSize(6);
    m_tracer->setSelectable(false);
    // call setGraphKey first so that the position can be updated by
    // updatePosition which is called in setGraph.
    m_tracer->setGraphKey(key);
    m_tracer->setGraph(graph);

    m_line = new QCPItemLine(mParentPlot);
    m_line->end->setParentAnchor(m_tracer->position);
    m_line->setHead(QCPLineEnding::esSpikeArrow);

    QPen pen = m_line->pen();
    pen.setStyle(Qt::DashLine);
    m_line->setPen(pen);
}

QCPGraph * CommentText::graph() const
{
    if (m_tracer != nullptr) {
        return m_tracer->graph();
    }
    return nullptr;
}

void CommentText::updateLineStartAnchor()
{
    if (m_tracer == nullptr || m_line == nullptr) {
        return;
    }

    QPointF tracerPos = m_tracer->position->pixelPoint();
    QSize itemSize = size();
    QPointF leftPos = left->pixelPoint(), rightPos = right->pixelPoint();
    QPointF topPos = top->pixelPoint(), bottomPos = bottom->pixelPoint();
    if (tracerPos.x() < leftPos.x() + itemSize.width()/4) {
        if (tracerPos.y() < topPos.y() + itemSize.height()/4) {
            m_line->start->setParentAnchor(topLeft);
        } else if (tracerPos.y() < bottomPos.y() - itemSize.height()/4) {
            m_line->start->setParentAnchor(left);
        } else {
            m_line->start->setParentAnchor(bottomLeft);
        }
    } else if (tracerPos.x() < rightPos.x() - itemSize.width()/4) {
        if (tracerPos.y() < topPos.y()) {
            m_line->start->setParentAnchor(top);
        } else {
            m_line->start->setParentAnchor(bottom);
        }
    } else {
        if (tracerPos.y() < topPos.y() + itemSize.height()/4) {
            m_line->start->setParentAnchor(topRight);
        } else if (tracerPos.y() < bottomPos.y() - itemSize.height()/4) {
            m_line->start->setParentAnchor(right);
        } else {
            m_line->start->setParentAnchor(bottomRight);
        }
    }
}

void CommentText::setVisible (bool on)
{
    if (m_tracer) {
        m_tracer->setVisible(on);
    }
    if (m_line) {
        m_line->setVisible(on);
    }
    QCPItemText::setVisible(on);
}

double CommentText::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  Q_UNUSED(details)
  if (onlySelectable && !mSelectable)
    return -1;

  // The rect may be rotated, so we transform the actual clicked pos to the rotated
  // coordinate system, so we can use the normal rectSelectTest function for non-rotated rects:
  QPointF positionPixels(position->pixelPoint());
  QTransform inputTransform;
  inputTransform.translate(positionPixels.x(), positionPixels.y());
  inputTransform.rotate(-mRotation);
  inputTransform.translate(-positionPixels.x(), -positionPixels.y());
  QPointF rotatedPos = inputTransform.map(pos);
  QFontMetrics fontMetrics(mFont);
  QRect textRect = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip|mTextAlignment, mText);
  QRect textBoxRect = textRect.adjusted(-mPadding.left(), -mPadding.top(), mPadding.right(), mPadding.bottom());
  QPointF textPos = getTextDrawPoint(positionPixels, textBoxRect, mPositionAlignment);
  textBoxRect.moveTopLeft(textPos.toPoint());

  return textBoxRect.contains(rotatedPos.toPoint()) ? mParentPlot->selectionTolerance()*0.99 : -1;
}
