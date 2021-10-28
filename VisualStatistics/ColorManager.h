#ifndef COLORMANAGER_H
#define COLORMANAGER_H

#include <QColor>

#include <array>
#include <random>

class ColorManager
{
public:
    ColorManager();

    QColor getColor();
    void reset();

private:
    QColor randomColor();

private:
    unsigned int _usedPredefinedColors;
    std::default_random_engine _randomEngine;
    std::uniform_int_distribution<> _uniformIntDis;

    static const std::array<QColor, 8> s_predefinedColors;
};

#endif // COLORMANAGER_H
