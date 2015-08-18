#-------------------------------------------------
#
# Project created by QtCreator 2015-07-19T02:23:34
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

TEMPLATE = app

CONFIG(debug, debug|release) {
    TARGET = ../VisualStatistics_d
    DESTDIR = debug
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    UI_DIR = debug/.ui
}

CONFIG(release, debug|release) {
    TARGET = ../VisualStatistics
    DESTDIR = release
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = release/.ui
}

SOURCES += main.cpp\
        mainwindow.cpp \
    plotwindow.cpp \
    third_party/qcustomplot/qcustomplot.cpp \
    gzipfile.cpp \
    utils.cpp \
    statisticsnamemodel.cpp

HEADERS  += mainwindow.h \
    plotwindow.h \
    third_party/qcustomplot/qcustomplot.h \
    gzipfile.h \
    utils.h \
    statisticsnamemodel.h

FORMS    += mainwindow.ui \
    plotwindow.ui

RESOURCES += \
    VisualStatistics.qrc

win32 {
    HEADERS += third_party/pcre/win/pcre.h

    RC_FILE = VisualStatistics.rc

    LIBS += -L$$PWD/third_party/pcre/win/ -lpcre

    INCLUDEPATH += $$PWD/third_party/pcre/win
    DEPENDPATH += $$PWD/third_party/pcre/win
}

unix:!macx {
    HEADERS += third_party/pcre/linux/pcre.h

    LIBS += -L$$PWD/third_party/pcre/linux/ -lpcre
    INCLUDEPATH += $$PWD/third_party/pcre/linux
    DEPENDPATH += $$PWD/third_party/pcre/linux

    PRE_TARGETDEPS += $$PWD/third_party/pcre/linux/libpcre.a
}