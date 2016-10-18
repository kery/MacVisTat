#include "colorgenerator.h"

const std::array<QColor, 8> ColorGenerator::s_predefColors = {
    QColor(0, 0, 255),
    QColor(0, 255, 0),
    QColor(255, 0, 0),
    QColor(220, 220, 0),
    QColor(255, 0, 255),
    QColor(0, 255, 255),
    QColor(128, 0, 255),
    QColor(255, 128, 0),
};

ColorGenerator::ColorGenerator() :
    m_usedPredef(0),
    m_unifDistrib(0, 255)
{
}

QColor ColorGenerator::nextColor()
{
    if (m_usedPredef < (int)s_predefColors.size()) {
        return s_predefColors[m_usedPredef++];
    } else {
        return nextRandColor();
    }
}

void ColorGenerator::reset()
{
    m_usedPredef = 0;
}

QColor ColorGenerator::nextRandColor()
{
    return QColor(m_unifDistrib(m_randEngine),
                  m_unifDistrib(m_randEngine),
                  m_unifDistrib(m_randEngine));
}
