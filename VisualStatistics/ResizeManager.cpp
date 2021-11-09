#include "ResizeManager.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

ResizeManager::ResizeManager(QWidget *widget) :
    m_showToParentHandled(false),
    m_widget(widget)
{
}

QSize ResizeManager::getScreenSize() const
{
    int screenNum = QApplication::desktop()->screenNumber(m_widget);
    QScreen *screen = QApplication::screens().at(screenNum);
    return screen->size();
}

bool ResizeManager::showToParentHandled() const
{
    return m_showToParentHandled;
}

#define CHECK_HANDLED if (m_showToParentHandled) {\
        return;\
    }\
    m_showToParentHandled = true;

void ResizeManager::resizeWidgetFromScreenSize(double wr, double hr)
{
    CHECK_HANDLED

    QSizeF screenSize = getScreenSize();
    QSizeF newSize(screenSize.width() * wr, screenSize.height() * hr);

    doResize(newSize);
}

void ResizeManager::resizeWidgetFromScreenHeight(double hr, double wr)
{
    CHECK_HANDLED

    QSizeF screenSize = getScreenSize();
    QSizeF newSize;
    newSize.setHeight(screenSize.height() * hr);
    newSize.setWidth(newSize.height() * wr);

    doResize(newSize);
}

void ResizeManager::resizeWidgetFromCharWidth(double mcw, double hr)
{
    CHECK_HANDLED

    QFontMetricsF fm = m_widget->fontMetrics();
    QSizeF newSize;
    newSize.setWidth(fm.averageCharWidth() * mcw);
    newSize.setHeight(newSize.width() * hr);

    doResize(newSize);
}

void ResizeManager::doResize(const QSizeF &newSize)
{
    QSizeF oldSize = m_widget->size();
    int dx = qRound((newSize.width() - oldSize.width())/2.0);
    int dy = qRound((newSize.height() - oldSize.height())/2.0);

    m_widget->setGeometry(m_widget->geometry().adjusted(-dx, -dy, dx, dy));
}
