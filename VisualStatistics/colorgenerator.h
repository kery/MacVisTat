#ifndef COLORGENERATOR_H
#define COLORGENERATOR_H

#include <QColor>
#include <array>
#include <random>

class ColorGenerator
{
public:
    ColorGenerator();
    ColorGenerator(const ColorGenerator &) = delete;
    ColorGenerator& operator=(const ColorGenerator &) = delete;
    ~ColorGenerator() = default;

    QColor nextColor();
    void reset();

private:
    QColor nextRandColor();

private:
    int m_usedPredef;
    std::default_random_engine m_randEngine;
    std::uniform_int_distribution<> m_unifDistrib;

    static const std::array<QColor, 8> s_predefColors;
};

#endif // COLORGENERATOR_H
