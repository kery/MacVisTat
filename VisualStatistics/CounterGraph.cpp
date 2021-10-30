#include "CounterGraph.h"
#include "CounterLegendItem.h"

const double CounterGraph::ScatterSize = 6.0;

CounterGraph::CounterGraph(QCPAxis *keyAxis, QCPAxis *valueAxis, const QString &module, const QString &name) :
    m_showModule(false),
    m_module(module),
    m_name(name),
    m_ssSuspectFlag(QCPScatterStyle::ssNone, ScatterSize),
    m_ssDiscontinuousFlag(QCPScatterStyle::ssNone, ScatterSize),
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

void CounterGraph::enableScatter(bool enable)
{
    if (enable) {
        setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, pen().color(), ScatterSize));
    } else {
        setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone));
    }
}

void CounterGraph::enableSuspectFlag(bool enable)
{
    m_ssSuspectFlag.setShape(enable ? QCPScatterStyle::ssCross : QCPScatterStyle::ssNone);
}

void CounterGraph::enableDiscontinuousFlag(bool enable)
{
    m_ssDiscontinuousFlag.setShape(enable ? QCPScatterStyle::ssSquare : QCPScatterStyle::ssNone);
}

bool CounterGraph::addToLegend()
{
    if (!mParentPlot || !mParentPlot->legend)
      return false;

    if (!mParentPlot->legend->hasItemWithPlottable(this))
    {
        CounterLegendItem *item = new CounterLegendItem(mParentPlot->legend, this);
        mParentPlot->legend->addItem(item);
        connect(item, &CounterLegendItem::selectionChanged, this, &CounterGraph::setSelected);
        connect(this, &CounterGraph::selectionChanged, item, &CounterLegendItem::setSelected);
        return true;
    }
    return false;
}

void CounterGraph::draw(QCPPainter *painter)
{
    if (!mKeyAxis || !mValueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
    if (mKeyAxis.data()->range().size() <= 0 || mData->isEmpty()) return;
    if (mLineStyle == lsNone && mScatterStyle.isNone() && m_ssSuspectFlag.isNone() && m_ssDiscontinuousFlag.isNone()) return;

    // allocate line and (if necessary) point vectors:
    QVector<QPointF> *lineData = new QVector<QPointF>;
    QVector<QCPData> *scatterData = 0;
    if (!mScatterStyle.isNone() || !m_ssSuspectFlag.isNone() || !m_ssDiscontinuousFlag.isNone())
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

          if ((data.valueErrorMinus > 0 && !m_ssSuspectFlag.isNone()) || (data.valueErrorPlus > 0 && !m_ssDiscontinuousFlag.isNone())) {
              if (data.valueErrorMinus > 0 && !m_ssSuspectFlag.isNone()) {
                  m_ssSuspectFlag.drawShape(painter, valueAxis->coordToPixel(data.value), keyAxis->coordToPixel(data.key));
              }
              if (data.valueErrorPlus > 0 && !m_ssDiscontinuousFlag.isNone()) {
                  m_ssDiscontinuousFlag.drawShape(painter, valueAxis->coordToPixel(data.value), keyAxis->coordToPixel(data.key));
              }
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

          if ((data.valueErrorMinus > 0 && !m_ssSuspectFlag.isNone()) || (data.valueErrorPlus > 0 && !m_ssDiscontinuousFlag.isNone())) {
              if (data.valueErrorMinus > 0 && !m_ssSuspectFlag.isNone()) {
                m_ssSuspectFlag.drawShape(painter, keyAxis->coordToPixel(data.key), valueAxis->coordToPixel(data.value));
              }
              if (data.valueErrorPlus > 0 && !m_ssDiscontinuousFlag.isNone()) {
                m_ssDiscontinuousFlag.drawShape(painter, keyAxis->coordToPixel(data.key), valueAxis->coordToPixel(data.value));
              }
          } else {
              mScatterStyle.drawShape(painter, keyAxis->coordToPixel(data.key), valueAxis->coordToPixel(data.value));
          }
      }
    }
}

QCPRange CounterGraph::getValueRange(bool &foundRange, SignDomain inSignDomain) const
{
    return QCPGraph::getValueRange(foundRange, inSignDomain, false);
}