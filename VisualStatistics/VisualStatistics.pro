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

win32 {
    HEADERS += third_party/pcre/win/pcre.h

    RC_FILE = VisualStatistics.rc

    LIBS += -L$$PWD/third_party/pcre/win/ -lpcre
    INCLUDEPATH += $$PWD/third_party/pcre/win

    INCLUDEPATH += $$PWD/../Common/third_party/breakpad/include
}

win32:CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/../Common/third_party/breakpad/lib/debug -lexception_handler -lcrash_generation_client -lcommon
}

win32:CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF /OPT:ICF

    LIBS += -L$$PWD/../Common/third_party/breakpad/lib/release -lexception_handler -lcrash_generation_client -lcommon
}

win32:CONFIG(profiling) {
    QMAKE_LFLAGS_RELEASE += /PROFILE
}

unix:!macx {
    HEADERS += third_party/pcre/linux/pcre.h

    LIBS += -L$$PWD/third_party/pcre/linux/ -lpcre
    INCLUDEPATH += $$PWD/third_party/pcre/linux
}
