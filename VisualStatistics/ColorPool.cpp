#include "ColorPool.h"

const std::array<QColor, 8> ColorPool::sPredefinedColors = {
    QColor(0, 0, 255),
    QColor(0, 255, 0),
    QColor(255, 0, 0),
    QColor(220, 220, 0),
    QColor(255, 0, 255),
    QColor(0, 255, 255),
    QColor(128, 0, 255),
    QColor(255, 128, 0)
};

ColorPool::ColorPool() :
    mUsedPredefinedColors(0),
    mUniformIntDis(0, 255)
{
}

QColor ColorPool::getColor()
{
    if (mUsedPredefinedColors < sPredefinedColors.size()) {
        return sPredefinedColors[mUsedPredefinedColors++];
    } else {
        return QColor(mUniformIntDis(mRandomEngine), mUniformIntDis(mRandomEngine), mUniformIntDis(mRandomEngine));
    }
}
