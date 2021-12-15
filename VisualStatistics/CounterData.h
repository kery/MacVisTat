#ifndef COUNTERDATA_H
#define COUNTERDATA_H

#include <qcustomplot.h>

struct CounterData
{
    QSet<double> suspectKeys;
    QCPGraphDataContainer data;

    static void dummyDeleter(QCPGraphDataContainer *data);
    static bool isAllZero(QSharedPointer<QCPGraphDataContainer> data);
    static bool isConstant(QSharedPointer<QCPGraphDataContainer> data);
};

#endif // COUNTERDATA_H
