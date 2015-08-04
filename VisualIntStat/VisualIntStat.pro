#-------------------------------------------------
#
# Project created by QtCreator 2015-07-19T02:23:34
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

Debug:TARGET = ../VisualIntStat_d
Release:TARGET = ../VisualIntStat
TEMPLATE = app

Debug:DESTDIR = debug
Debug:OBJECTS_DIR = debug/.obj
Debug:MOC_DIR = debug/.moc
Debug:RCC_DIR = debug/.rcc
Debug:UI_DIR = debug/.ui

Release:DESTDIR = release
Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR = release/.moc
Release:RCC_DIR = release/.rcc
Release:UI_DIR = release/.ui

SOURCES += main.cpp\
        mainwindow.cpp \
    plotwindow.cpp \
    third_party/qcustomplot/qcustomplot.cpp \
    statnamelistmodel.cpp \
    gzipfile.cpp \
    parsedataworker.cpp

HEADERS  += mainwindow.h \
    plotwindow.h \
    third_party/qcustomplot/qcustomplot.h \
    statnamelistmodel.h \
    third_party/pcre/pcre.h \
    gzipfile.h \
    parsedataworker.h

FORMS    += mainwindow.ui \
    plotwindow.ui

RESOURCES += \
    visualintstat.qrc

win32: LIBS += -L$$PWD/third_party/pcre/ -lpcre16

INCLUDEPATH += $$PWD/third_party/pcre
DEPENDPATH += $$PWD/third_party/pcre
