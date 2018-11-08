#include "ColorManager.h"

const std::array<QColor, 8> ColorManager::s_predefinedColors = {
    QColor(0, 0, 255),
    QColor(0, 255, 0),
    QColor(255, 0, 0),
    QColor(220, 220, 0),
    QColor(255, 0, 255),
    QColor(0, 255, 255),
    QColor(128, 0, 255),
    QColor(255, 128, 0)
};

ColorManager::ColorManager() :
    _usedPredefinedColors(0),
    _uniformIntDis(0, 255)
{
}

QColor ColorManager::getColor()
{
    if (_usedPredefinedColors < s_predefinedColors.size()) {
        return s_predefinedColors[_usedPredefinedColors++];
    } else {
        return randomColor();
    }
}

void ColorManager::reset()
{
    _usedPredefinedColors = 0;
}

QColor ColorManager::randomColor()
{
    return QColor(_uniformIntDis(_randomEngine),
        _uniformIntDis(_randomEngine),
        _uniformIntDis(_randomEngine));
}
