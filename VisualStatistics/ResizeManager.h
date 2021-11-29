#ifndef RESIZEMANAGER_H
#define RESIZEMANAGER_H

#include <QSize>

class QWidget;
class QEvent;

class ResizeManager
{
public:
    ResizeManager(QWidget *widget);

    QSizeF screenSize() const;
    bool resizeWidgetFromScreenSize(QEvent *event, double wr, double hr);
    bool resizeWidgetFromScreenHeight(QEvent *event, double hr, double wr);
    bool resizeWidgetFromCharWidth(QEvent *event, double mcw, double hr);

private:
    bool updateState(QEvent *event);
    void doResize(const QSizeF &newSize);

    bool mShowToParentHandled;
    QWidget *mWidget;
};

#endif // RESIZEMANAGER_H
