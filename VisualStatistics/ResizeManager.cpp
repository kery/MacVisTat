#include "ResizeManager.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

ResizeManager::ResizeManager(QWidget *widget) :
    mShowToParentHandled(false),
    mWidget(widget)
{
}

QSizeF ResizeManager::screenSize() const
{
    int screenNum = QApplication::desktop()->screenNumber(mWidget);
    auto screens = QApplication::screens();
    return screens.at(screenNum)->size();
}

bool ResizeManager::resizeWidgetFromScreenSize(QEvent *event, double wr, double hr)
{
    if (updateState(event)) {
        QSizeF size = screenSize();
        QSizeF newSize(size.width() * wr, size.height() * hr);

        doResize(newSize);
        return true;
    }
    return false;
}

bool ResizeManager::resizeWidgetFromScreenHeight(QEvent *event, double hr, double wr)
{
    if (updateState(event)) {
        QSizeF newSize;
        newSize.setHeight(screenSize().height() * hr);
        newSize.setWidth(newSize.height() * wr);

        doResize(newSize);
        return true;
    }
    return false;
}

bool ResizeManager::resizeWidgetFromCharWidth(QEvent *event, double mcw, double hr)
{
    if (updateState(event)) {
        QFontMetricsF fm = mWidget->fontMetrics();
        QSizeF newSize;
        newSize.setWidth(fm.averageCharWidth() * mcw);
        newSize.setHeight(newSize.width() * hr);

        doResize(newSize);
        return true;
    }
    return false;
}

bool ResizeManager::updateState(QEvent *event)
{
    if (event->type() == QEvent::ShowToParent && !mShowToParentHandled) {
        mShowToParentHandled = true;
        return true;
    }
    return false;
}

void ResizeManager::doResize(const QSizeF &newSize)
{
    QSizeF oldSize = mWidget->size();
    int dx = qRound((newSize.width() - oldSize.width())/2.0);
    int dy = qRound((newSize.height() - oldSize.height())/2.0);

    mWidget->setGeometry(mWidget->geometry().adjusted(-dx, -dy, dx, dy));
}
