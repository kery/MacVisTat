#-------------------------------------------------
#
# Project created by QtCreator 2015-07-19T02:23:34
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

Debug:TARGET = ../VisualStatistics_d
Release:TARGET = ../VisualStatistics

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
    gzipfile.h \
    parsedataworker.h

FORMS    += mainwindow.ui \
    plotwindow.ui

RESOURCES += \
    VisualStatistics.qrc

win32 {
    SOURCES += third_party/pcre/win/pcre.h

    RC_FILE = VisualStatistics.rc

    LIBS += -L$$PWD/third_party/pcre/win/ -lpcre16

    INCLUDEPATH += $$PWD/third_party/pcre/win
    DEPENDPATH += $$PWD/third_party/pcre/win
}

unix:!macx {
    SOURCES += third_party/pcre/linux/pcre.h

    LIBS += -L$$PWD/third_party/pcre/linux/ -lpcre16
    INCLUDEPATH += $$PWD/third_party/pcre/linux
    DEPENDPATH += $$PWD/third_party/pcre/linux

    PRE_TARGETDEPS += $$PWD/third_party/pcre/linux/libpcre16.a
}
