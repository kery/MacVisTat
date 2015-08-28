#-------------------------------------------------
#
# Project created by QtCreator 2015-07-19T02:23:34
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

TEMPLATE = app
TARGET = ../../../VisualStatistics

OBJECTS_DIR = obj
MOC_DIR = moc
RCC_DIR = rcc
UI_DIR = ui

SOURCES += main.cpp\
        mainwindow.cpp \
    plotwindow.cpp \
    third_party/qcustomplot/qcustomplot.cpp \
    gzipfile.cpp \
    utils.cpp \
    statisticsnamemodel.cpp \
    aboutdialog.cpp

HEADERS  += mainwindow.h \
    plotwindow.h \
    third_party/qcustomplot/qcustomplot.h \
    gzipfile.h \
    utils.h \
    statisticsnamemodel.h \
    aboutdialog.h \
    version.h

FORMS    += mainwindow.ui \
    plotwindow.ui \
    aboutdialog.ui

RESOURCES += \
    VisualStatistics.qrc

INCLUDEPATH += $$PWD/third_party/breakpad/

win32 {
    RC_FILE = VisualStatistics.rc

    INCLUDEPATH += $$PWD/third_party/pcre/win
    INCLUDEPATH += $$PWD/third_party/breakpad/client/windows/handler

    LIBS += -L$$PWD/third_party/pcre/win/ -lpcre
}

win32:CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/third_party/breakpad/lib/win/debug -lexception_handler -lcrash_generation_client -lcommon
}

win32:CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF /OPT:ICF

    LIBS += -L$$PWD/third_party/breakpad/lib/win/release -lexception_handler -lcrash_generation_client -lcommon
}

win32:CONFIG(profiling) {
    QMAKE_LFLAGS_RELEASE += /PROFILE
}

unix:!macx {
    INCLUDEPATH += $$PWD/third_party/pcre/linux

    LIBS += -L$$PWD/third_party/pcre/linux/ -lpcre
}
