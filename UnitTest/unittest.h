#ifndef UNITTEST_H
#define UNITTEST_H

#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include <utility>

#include "parsedstatistics.h"

class UnitTest : public QObject
{
    Q_OBJECT

public:
    UnitTest() {
    }

private Q_SLOTS:
    void testParsedStatistics_1()
    {
        ParsedStatistics::NodeNameDataMap nndm;

        nndm["AS7-0"]["shm_test1"].insert(190, QCPData(190, 12));
        nndm["AS7-0"]["shm_test1"].insert(191, QCPData(191, 10));
        nndm["AS7-0"]["shm_test1"].insert(192, QCPData(192, 19));

        ParsedStatistics ps(std::move(nndm));

        QCOMPARE(ps.getNodes().size(), 1);
        QCOMPARE(ps.getNodes().first(), QString("AS7-0"));
        QCOMPARE(ps.getNodeStr(), QString("AS7-0"));
        QCOMPARE(ps.getNames("AS7-0").size(), 1);
        QCOMPARE(ps.getNames("AS7-0").last(), QString("shm_test1"));
        QVERIFY(ps.getDataMap("AS7-0", "shm_test1") != nullptr);
        QCOMPARE(ps.formatName("AS7-0", "shm_test1"), QString("shm_test1"));
    }

    void testParsedStatistics_2()
    {
        ParsedStatistics::NodeNameDataMap nndm;

        nndm["AS7-0"]["shm_test1"].insert(190, QCPData(190, 12));
        nndm["AS7-0"]["shm_test1"].insert(192, QCPData(192, 10));
        nndm["AS7-0"]["shm_test1"].insert(194, QCPData(194, 19));
        nndm["AS7-0"]["shm_test2"].insert(190, QCPData(190, 10));
        nndm["AS7-0"]["shm_test2"].insert(192, QCPData(192, 12));
        nndm["AS7-0"]["shm_test2"].insert(194, QCPData(194, 19));
        nndm["AS9-1"]["shm_test1"].insert(185, QCPData(185, 12));
        nndm["AS9-1"]["shm_test1"].insert(189, QCPData(189, 10));
        nndm["AS9-1"]["shm_test1"].insert(191, QCPData(191, 19));
        nndm["AS9-1"]["shm_test2"].insert(193, QCPData(185, 10));
        nndm["AS9-1"]["shm_test2"].insert(194, QCPData(189, 12));
        nndm["AS9-1"]["shm_test2"].insert(196, QCPData(191, 19));

        ParsedStatistics ps(std::move(nndm));

        QCOMPARE(ps.dateTimeCount(), 6);
        QCOMPARE(ps.getDateTime(0), 185);
        QCOMPARE(ps.getDateTime(1), 189);
        QCOMPARE(ps.getDateTime(2), 190);
        QCOMPARE(ps.getDateTime(3), 191);
        QCOMPARE(ps.getDateTime(4), 192);
        QCOMPARE(ps.getDateTime(5), 194);
    }

    void testParsedStatistics_3()
    {
        ParsedStatistics::NodeNameDataMap nndm;

        nndm["AS7-0"]["shm_test1"].insert(190, QCPData(190, 12));
        nndm["AS7-0"]["shm_test1"].insert(192, QCPData(192, 10));
        nndm["AS7-0"]["shm_test1"].insert(194, QCPData(194, 19));
        nndm["AS7-0"]["shm_test2"].insert(190, QCPData(190, 10));
        nndm["AS7-0"]["shm_test2"].insert(192, QCPData(192, 12));
        nndm["AS7-0"]["shm_test2"].insert(194, QCPData(194, 19));
        nndm["AS9-1"]["shm_test1"].insert(185, QCPData(185, 21));
        nndm["AS9-1"]["shm_test1"].insert(189, QCPData(189, 90));
        nndm["AS9-1"]["shm_test1"].insert(191, QCPData(191, 17));
        nndm["AS9-1"]["shm_test2"].insert(193, QCPData(185, 10));
        nndm["AS9-1"]["shm_test2"].insert(194, QCPData(189, 12));
        nndm["AS9-1"]["shm_test2"].insert(196, QCPData(191, 19));

        ParsedStatistics ps(std::move(nndm));

        QCPDataMap *ptr = ps.getDataMap("AS9-1", "shm_test1");
        QVERIFY(ptr != nullptr);

        auto iter = ptr->begin();
        QCOMPARE((int)iter.key(), 0);
        QCOMPARE((int)iter.value().key, 0);
        QCOMPARE((int)iter.value().value, 21);
        ++iter;
        QCOMPARE((int)iter.key(), 1);
        QCOMPARE((int)iter.value().key, 1);
        QCOMPARE((int)iter.value().value, 90);
        ++iter;
        QCOMPARE((int)iter.key(), 3);
        QCOMPARE((int)iter.value().key, 3);
        QCOMPARE((int)iter.value().value, 17);
    }

    void testParsedStatistics_4()
    {
        ParsedStatistics::NodeNameDataMap nndm;

        nndm["AS7-0"]["shm_test1"].insert(190, QCPData(190, 12));
        nndm["AS7-0"]["shm_test1"].insert(192, QCPData(192, 10));
        nndm["AS7-0"]["shm_test1"].insert(194, QCPData(194, 19));
        nndm["AS7-0"]["shm_test2"].insert(190, QCPData(190, 10));
        nndm["AS7-0"]["shm_test2"].insert(192, QCPData(192, 12));
        nndm["AS7-0"]["shm_test2"].insert(194, QCPData(194, 19));
        nndm["AS9-1"]["shm_test1"].insert(185, QCPData(185, 21));
        nndm["AS9-1"]["shm_test1"].insert(189, QCPData(189, 90));
        nndm["AS9-1"]["shm_test1"].insert(191, QCPData(191, 17));
        nndm["AS9-1"]["shm_test2"].insert(193, QCPData(185, 10));
        nndm["AS9-1"]["shm_test2"].insert(194, QCPData(189, 12));
        nndm["AS9-1"]["shm_test2"].insert(196, QCPData(191, 19));

        ParsedStatistics ps(std::move(nndm));

        QCOMPARE(ps.getNodeStr(), QString("AS7-0,AS9-1"));
        QCOMPARE(ps.formatName("AS7-0", "shm_test1"), QString("AS7-0:shm_test1"));
        QCOMPARE(ps.totalNameCount(), 4);
    }
};

#endif // UNITTEST_H

