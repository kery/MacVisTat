#include "colorgenerator.h"

const QColor ColorGenerator::_predefined[8] = {
    QColor(255, 0, 0),
    QColor(0, 255, 0),
    QColor(0, 0, 255),
    QColor(220, 220, 0),
    QColor(255, 0, 255),
    QColor(0, 255, 255),
    QColor(128, 0, 255),
    QColor(255, 128, 0),
};

int ColorGenerator::colorCount()
{
    return 360 + sizeof(_predefined)/sizeof(_predefined[0]);
}

ColorGenerator::ColorGenerator(int s, int l) :
    _count(0), _s(s), _l(l), _ptr(&_hList1)
{
    _hList1 << QPair<int, int>(0, 359);
}

QColor ColorGenerator::genColor()
{
    if (_count < sizeof(_predefined)/sizeof(_predefined[0])) {
        return _predefined[_count++];
    }
    int h = calculateH();
    return QColor::fromHsl(h, _s, _l);
}

int ColorGenerator::calculateH()
{
    if (_ptr->size() > 0) {
        const QPair<int, int> pair = _ptr->takeFirst();
        int h;
        if (pair.second - pair.first > 1) {
            h = (pair.first + pair.second - 1) / 2 + 1;
            QPair<int, int> part1(pair.first, h - 1), part2(h + 1, pair.second);
            if (_ptr == &_hList1) {
                _hList2 << part1 << part2;
            } else {
                _hList1 << part1 << part2;
            }
        } else {
            h = pair.second;
            if (pair.first != pair.second) {
                if (_ptr == &_hList1) {
                    _hList2 << QPair<int, int>(pair.first, pair.first);
                } else {
                    _hList1 << QPair<int, int>(pair.first, pair.first);
                }
            }
        }
        if (_ptr->isEmpty()) {
            _ptr = _ptr == &_hList1 ? &_hList2  : &_hList1;
        }
        return h;
    }
    return 0;
}
