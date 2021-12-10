#ifndef COUNTERNAME_H
#define COUNTERNAME_H

#include <QPair>
#include <QString>

class CounterName
{
public:
    static void initSeparators();
    static QString getModuleName(const QString &name);
    static QString getObjectName(const QString &name);
    static QPair<QString, QString> separateModuleName(const QString &name);

    // Module              Group                           Indexes                                                      KPI/KCI Object
    // -------------------,-------------------------------,------------------------------------------------------------,--------------
    // KPIReferencePointGX,KPI=ReferencePoint,GroupName=GX,vprnRouterName=vprn20,vrId=3,address=10.234.110.34,port=3868,VS.RxCcaI
    static QChar sModuleSeparator;
    static QChar sGroupSeparator;
    static QChar sIndexesSeparator;
};

#endif // COUNTERNAME_H
