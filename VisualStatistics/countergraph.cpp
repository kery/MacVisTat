#include "countergraph.h"
#include "counterlegenditem.h"

double SCATTER_SIZE = 6.0;

CounterGraph::CounterGraph(QCPAxis *keyAxis, QCPAxis *valueAxis, const QString &module, const QString &name) :
    m_showModule(false),
    m_module(module),
    m_name(name),
    m_ssSuspect(QCPScatterStyle::ssNone, SCATTER_SIZE),
    QCPGraph(keyAxis, valueAxis)
{
}

void CounterGraph::setShowModule(bool show) {
    m_showModule = show;
}

QString CounterGraph::displayName() const
{
    if (m_showModule && !m_module.isEmpty()) {
        return m_module + ',' + m_name;
    }
    return m_name;
}

void CounterGraph::enableSuspectFlag(bool enable)
{
    m_ssSuspect.setShape(enable ? QCPScatterStyle::ssCross : QCPScatterStyle::ssNone);
}

bool CounterGraph::addToLegend()
{
    if (!mParentPlot || !mParentPlot->legend)
      return false;

    if (!mParentPlot->legend->hasItemWithPlottable(this))
    {
      mParentPlot->legend->addItem(new CounterLegendItem(mParentPlot->legend, this));
      return true;
    } else
      return false;
}

void CounterGraph::draw(QCPPainter *painter)
{
    if (!mKeyAxis || !mValueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
    if (mKeyAxis.data()->range().size() <= 0 || mData->isEmpty()) return;
    if (mLineStyle == lsNone && mScatterStyle.isNone() && m_ssSuspect.isNone()) return;

    // allocate line and (if necessary) point vectors:
    QVector<QPointF> *lineData = new QVector<QPointF>;
    QVector<QCPData> *scatterData = 0;
    if (!mScatterStyle.isNone() || !m_ssSuspect.isNone())
      scatterData = new QVector<QCPData>;

    // fill vectors with data appropriate to plot style:
    getPlotData(lineData, scatterData);

    // check data validity if flag set:
  #ifdef QCUSTOMPLOT_CHECK_DATA
    QCPDataMap::const_iterator it;
    for (it = mData->constBegin(); it != mData->constEnd(); ++it)
    {
      if (QCP::isInvalidData(it.value().key, it.value().value) ||
          QCP::isInvalidData(it.value().keyErrorPlus, it.value().keyErrorMinus) ||
          QCP::isInvalidData(it.value().valueErrorPlus, it.value().valueErrorPlus))
        qDebug() << Q_FUNC_INFO << "Data point at" << it.key() << "invalid." << "Plottable name:" << name();
    }
  #endif

    // draw fill of graph:
    if (mLineStyle != lsNone)
      drawFill(painter, lineData);

    // draw line:
    if (mLineStyle == lsImpulse)
      drawImpulsePlot(painter, lineData);
    else if (mLineStyle != lsNone)
      drawLinePlot(painter, lineData); // also step plots can be drawn as a line plot

    // draw scatters:
    if (scatterData)
      drawScatterPlot(painter, scatterData);

    // free allocated line and point vectors:
    delete lineData;
    if (scatterData)
      delete scatterData;
}

void CounterGraph::drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
{
    QSizeF iconSize = parentPlot()->legend->iconSize();
    QRectF textRect = painter->fontMetrics().boundingRect(0, 0, 0, iconSize.height(), Qt::TextDontClip, displayName());
    if (textRect.height() > iconSize.height()) {
        const_cast<QRectF &>(rect).translate(0, ((textRect.height() - iconSize.height()) / 2 + 1));
        painter->setClipRect(rect);
        painter->fillRect(rect, mPen.color());
    } else {
        painter->fillRect(rect, mPen.color());
    }
}

void CounterGraph::drawScatterPlot(QCPPainter *painter, QVector<QCPData> *scatterData) const
{
    QCPAxis *keyAxis = mKeyAxis.data();
    QCPAxis *valueAxis = mValueAxis.data();
    if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }

    // ignore the error bars, and the valueErrorMinus member of QCPData is used to determin whether to draw
    // a special scatter for suspect value (<suspect>true</suspect>)

    // draw scatter point symbols:
    applyScattersAntialiasingHint(painter);
    mScatterStyle.applyTo(painter, mPen);
    if (keyAxis->orientation() == Qt::Vertical) {
      for (int i = 0; i < scatterData->size(); ++i) {
          const QCPData &data = scatterData->at(i);
          if (qIsNaN(data.value)) {
              continue;
          }
          if (data.valueErrorMinus > 0 && !m_ssSuspect.isNone()) {
              m_ssSuspect.drawShape(painter, valueAxis->coordToPixel(data.value), keyAxis->coordToPixel(data.key));
          } else {
              mScatterStyle.drawShape(painter, valueAxis->coordToPixel(data.value), keyAxis->coordToPixel(data.key));
          }
      }
    } else {
      for (int i = 0; i < scatterData->size(); ++i) {
          const QCPData &data = scatterData->at(i);
          if (qIsNaN(data.value)) {
              continue;
          }
          if (data.valueErrorMinus > 0 && !m_ssSuspect.isNone()) {
              m_ssSuspect.drawShape(painter, keyAxis->coordToPixel(data.key), valueAxis->coordToPixel(data.value));
          } else {
              mScatterStyle.drawShape(painter, keyAxis->coordToPixel(data.key), valueAxis->coordToPixel(data.value));
          }
      }
    }
}
