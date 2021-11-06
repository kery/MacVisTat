#include "ResizeManager.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

ResizeManager::ResizeManager(QWidget *widget) :
    m_scale(0.0),
    m_showToParentHandled(false),
    m_widget(widget)
{
}

void ResizeManager::setScale(double scale)
{
    m_scale = scale;
}

double ResizeManager::scale() const
{
    return m_scale;
}

bool ResizeManager::showToParentHandled() const
{
    return m_showToParentHandled;
}

double ResizeManager::currentScreenScale() const
{
    int screenNum = QApplication::desktop()->screenNumber(m_widget);
    QScreen *screen = QApplication::screens().at(screenNum);
    return screen->logicalDotsPerInch()/96;
}

bool ResizeManager::resizeWidgetOnShowToParent()
{
    if (m_showToParentHandled) {
        return false;
    }
    m_showToParentHandled = true;
    m_scale = currentScreenScale();
    if (qFuzzyCompare(m_scale, 1.0)) {
        return false;
    }

    QSizeF oldSize = m_widget->size();
    QSizeF newSize = oldSize * m_scale;
    int dx = qRound((newSize.width() - oldSize.width())/2.0);
    int dy = qRound((newSize.height() - oldSize.height())/2.0);
    m_widget->setGeometry(m_widget->geometry().adjusted(-dx, -dy, dx, dy));
    return true;
}
