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
    unsigned int mUsedPredefinedColors;
    std::default_random_engine mRandomEngine;
    std::uniform_int_distribution<> mUniformIntDis;

    static const std::array<QColor, 8> sPredefinedColors;
};

#endif // COLORPOOL_H
