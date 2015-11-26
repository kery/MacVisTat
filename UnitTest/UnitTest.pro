#-------------------------------------------------
#
# Project created by QtCreator 2015-10-15T16:01:01
#
#-------------------------------------------------

QT       += testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

TARGET = tst_unittesttest
CONFIG   += console
CONFIG   -= app_bundle

TARGET = ../../../unittest

OBJECTS_DIR = obj
MOC_DIR = moc
RCC_DIR = rcc
UI_DIR = ui


SOURCES += \
    ../VisualStatistics/parsedstatistics.cpp \
    ../VisualStatistics/third_party/qcustomplot/qcustomplot.cpp \
    unittest.cpp

HEADERS += \
    ../VisualStatistics/third_party/qcustomplot/qcustomplot.h \
    unittest.h

INCLUDEPATH += $$PWD/../VisualStatistics
