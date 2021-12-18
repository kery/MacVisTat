#include "CounterData.h"

void CounterData::dummyDeleter(QCPGraphDataContainer * /*data*/)
{
    // Do nothing
}

bool CounterData::isAllZero(QSharedPointer<QCPGraphDataContainer> data)
{
    for (auto iter = data->constBegin(); iter != data->constEnd(); ++iter) {
        if (!qIsNaN(iter->value) && !qFuzzyCompare(iter->value, 0.0)) {
            return false;
        }
    }
    return true;
}

bool CounterData::isConstant(QSharedPointer<QCPGraphDataContainer> data)
{
    auto iterFirst = data->constBegin();
    while (iterFirst != data->constEnd() && qIsNaN(iterFirst->value)) {
        ++iterFirst;
    }
    if (iterFirst == data->constEnd()) {
        return true; // All values are NAN, treat as constant data.
    }
    for (auto iter = iterFirst + 1; iter != data->constEnd(); ++iter) {
        if (!qIsNaN(iter->value) && !qFuzzyCompare(iterFirst->value, iter->value)) {
            return false;
        }
    }
    return true;
}
