#include "FilterValidator.h"

FilterValidator::FilterValidator(QObject *parent) :
    QValidator(parent)
{
}

void FilterValidator::fixup(QString &input) const
{
    input = input.trimmed();
}

QValidator::State FilterValidator::validate(QString &input, int &/*pos*/) const
{
    if (!input.isEmpty() && (input.constBegin()->isSpace() || (input.constEnd() - 1)->isSpace())) {
        return Intermediate;
    }
    return Acceptable;
}
