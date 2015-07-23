#-------------------------------------------------
#
# Project created by QtCreator 2015-07-19T02:23:34
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = VisualIntStat
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    plotwindow.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    plotwindow.h

FORMS    += mainwindow.ui \
    plotwindow.ui

RESOURCES += \
    visualintstat.qrc
