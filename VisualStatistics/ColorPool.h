#ifndef COLORPOOL_H
#define COLORPOOL_H

#include <array>
#include <random>
#include <QColor>

class ColorPool
{
public:
    ColorPool();

    QColor getColor();

private:
    unsigned int _usedPredefinedColors;
    std::default_random_engine _randomEngine;
    std::uniform_int_distribution<> _uniformIntDis;

    static const std::array<QColor, 8> predefinedColors;
};

#endif // COLORPOOL_H
