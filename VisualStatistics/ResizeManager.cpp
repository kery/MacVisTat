#include "ResizeManager.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

ResizeManager::ResizeManager(QWidget *widget) :
    _showToParentHandled(false),
    _widget(widget)
{
}

QSizeF ResizeManager::screenSize() const
{
    int screenNum = QApplication::desktop()->screenNumber(_widget);
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
        QFontMetricsF fm = _widget->fontMetrics();
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
    if (event->type() == QEvent::ShowToParent && !_showToParentHandled) {
        _showToParentHandled = true;
        return true;
    }
    return false;
}

void ResizeManager::doResize(const QSizeF &newSize)
{
    QSizeF oldSize = _widget->size();
    int dx = qRound((newSize.width() - oldSize.width())/2.0);
    int dy = qRound((newSize.height() - oldSize.height())/2.0);

    _widget->setGeometry(_widget->geometry().adjusted(-dx, -dy, dx, dy));
}
