#include "ColorPool.h"

const std::array<QColor, 8> ColorPool::predefinedColors = {
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
    _usedPredefinedColors(0),
    _uniformIntDis(0, 255)
{
}

QColor ColorPool::getColor()
{
    if (_usedPredefinedColors < predefinedColors.size()) {
        return predefinedColors[_usedPredefinedColors++];
    } else {
        return QColor(_uniformIntDis(_randomEngine), _uniformIntDis(_randomEngine), _uniformIntDis(_randomEngine));
    }
}
