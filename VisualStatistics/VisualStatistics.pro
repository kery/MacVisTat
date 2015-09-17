#-------------------------------------------------
#
# Project created by QtCreator 2015-07-19T02:23:34
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

TEMPLATE = app

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
    aboutdialog.cpp \
    progressdialog.cpp \
    third_party/lua/lapi.c \
    third_party/lua/lauxlib.c \
    third_party/lua/lbaselib.c \
    third_party/lua/lbitlib.c \
    third_party/lua/lcode.c \
    third_party/lua/lcorolib.c \
    third_party/lua/lctype.c \
    third_party/lua/ldblib.c \
    third_party/lua/ldebug.c \
    third_party/lua/ldo.c \
    third_party/lua/ldump.c \
    third_party/lua/lfunc.c \
    third_party/lua/lgc.c \
    third_party/lua/linit.c \
    third_party/lua/liolib.c \
    third_party/lua/llex.c \
    third_party/lua/lmathlib.c \
    third_party/lua/lmem.c \
    third_party/lua/loadlib.c \
    third_party/lua/lobject.c \
    third_party/lua/lopcodes.c \
    third_party/lua/loslib.c \
    third_party/lua/lparser.c \
    third_party/lua/lstate.c \
    third_party/lua/lstring.c \
    third_party/lua/lstrlib.c \
    third_party/lua/ltable.c \
    third_party/lua/ltablib.c \
    third_party/lua/ltm.c \
    third_party/lua/lundump.c \
    third_party/lua/lvm.c \
    third_party/lua/lzio.c

HEADERS  += mainwindow.h \
    plotwindow.h \
    third_party/qcustomplot/qcustomplot.h \
    gzipfile.h \
    utils.h \
    statisticsnamemodel.h \
    aboutdialog.h \
    version.h \
    progressdialog.h \
    third_party/lua/lapi.h \
    third_party/lua/lauxlib.h \
    third_party/lua/lcode.h \
    third_party/lua/lctype.h \
    third_party/lua/ldebug.h \
    third_party/lua/ldo.h \
    third_party/lua/lfunc.h \
    third_party/lua/lgc.h \
    third_party/lua/llex.h \
    third_party/lua/llimits.h \
    third_party/lua/lmem.h \
    third_party/lua/lobject.h \
    third_party/lua/lopcodes.h \
    third_party/lua/lparser.h \
    third_party/lua/lstate.h \
    third_party/lua/lstring.h \
    third_party/lua/ltable.h \
    third_party/lua/ltm.h \
    third_party/lua/lua.h \
    third_party/lua/lua.hpp \
    third_party/lua/luaconf.h \
    third_party/lua/lualib.h \
    third_party/lua/lundump.h \
    third_party/lua/lvm.h \
    third_party/lua/lzio.h

FORMS    += mainwindow.ui \
    plotwindow.ui \
    aboutdialog.ui

RESOURCES += \
    VisualStatistics.qrc

INCLUDEPATH += $$PWD/third_party/breakpad/

win32 {
    QT += winextras
    TARGET = ../../../VisualStatistics

    DEFINES += _CRT_SECURE_NO_WARNINGS

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
    TARGET = ../../VisualStatistics

    # Generate debug info also in release mode so that it is possible to use breakpad
    # After dump symbol to a separate file, use strip --strip-debug to strip the debug
    # infos
    QMAKE_CXXFLAGS_RELEASE += -g

    INCLUDEPATH += $$PWD/third_party/pcre/linux
    INCLUDEPATH += $$PWD/third_party/breakpad/client/linux/handler

    LIBS += -L$$PWD/third_party/pcre/linux/ -lpcre
    LIBS += -L$$PWD/third_party/breakpad/lib/linux/ -lbreakpad_client
}

DISTFILES += \
    third_party/lua/Makefile
