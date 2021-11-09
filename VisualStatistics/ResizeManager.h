#ifndef RESIZEMANAGER_H
#define RESIZEMANAGER_H

#include <QSize>

class QWidget;

class ResizeManager
{
public:
    ResizeManager(QWidget *widget);

    QSize getScreenSize() const;
    bool showToParentHandled() const;
    void resizeWidgetFromScreenSize(double wr, double hr);
    void resizeWidgetFromScreenHeight(double hr, double wr);
    void resizeWidgetFromCharWidth(double mcw, double hr);

private:
    void doResize(const QSizeF &newSize);

private:
    bool m_showToParentHandled;
    QWidget *m_widget;
};

#endif // RESIZEMANAGER_H
