#ifndef RESIZEMANAGER_H
#define RESIZEMANAGER_H

class QWidget;

class ResizeManager
{
public:
    ResizeManager(QWidget *widget);

    double scale() const;
    void setScale(double scale);
    bool showToParentHandled() const;
    double currentScreenScale() const;
    bool resizeWidgetOnShowToParent();

private:
    double m_scale;
    bool m_showToParentHandled;
    QWidget *m_widget;
};

#endif // RESIZEMANAGER_H
