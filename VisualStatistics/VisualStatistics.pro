#-------------------------------------------------
#
# Project created by QtCreator 2015-07-19T02:23:34
#
#-------------------------------------------------

QT       += core gui concurrent network

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
    third_party/lua/lapi.cpp \
    third_party/lua/lauxlib.cpp \
    third_party/lua/lbaselib.cpp \
    third_party/lua/lbitlib.cpp \
    third_party/lua/lcode.cpp \
    third_party/lua/lcorolib.cpp \
    third_party/lua/lctype.cpp \
    third_party/lua/ldblib.cpp \
    third_party/lua/ldebug.cpp \
    third_party/lua/ldo.cpp \
    third_party/lua/ldump.cpp \
    third_party/lua/lfunc.cpp \
    third_party/lua/lgc.cpp \
    third_party/lua/linit.cpp \
    third_party/lua/liolib.cpp \
    third_party/lua/llex.cpp \
    third_party/lua/lmathlib.cpp \
    third_party/lua/lmem.cpp \
    third_party/lua/loadlib.cpp \
    third_party/lua/lobject.cpp \
    third_party/lua/lopcodes.cpp \
    third_party/lua/loslib.cpp \
    third_party/lua/lparser.cpp \
    third_party/lua/lstate.cpp \
    third_party/lua/lstring.cpp \
    third_party/lua/lstrlib.cpp \
    third_party/lua/ltable.cpp \
    third_party/lua/ltablib.cpp \
    third_party/lua/ltm.cpp \
    third_party/lua/lundump.cpp \
    third_party/lua/lvm.cpp \
    third_party/lua/lzio.cpp \
    colormanager.cpp \
    luaenvironment.cpp \
    scriptwindow.cpp \
    statisticsfileparser.cpp \
    statistics.cpp \
    ValueTipLabel.cpp \
    CounterGraph.cpp

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
    third_party/lua/luaconf.h \
    third_party/lua/lualib.h \
    third_party/lua/lundump.h \
    third_party/lua/lvm.h \
    third_party/lua/lzio.h \
    colormanager.h \
    luaenvironment.h \
    scriptwindow.h \
    statisticsfileparser.h \
    statistics.h \
    ValueTipLabel.h \
    CounterGraph.h

FORMS    += mainwindow.ui \
    plotwindow.ui \
    aboutdialog.ui \
    scriptwindow.ui \
    progressdialog.ui

RESOURCES += \
    VisualStatistics.qrc

INCLUDEPATH += $$PWD/third_party/breakpad/
INCLUDEPATH += $$PWD/third_party/qcustomplot/

win32 {
    QT += winextras
    TARGET = ../../../VisualStatistics

    # Remove the warnings for calling function like strcmp
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
    TARGET = ../build/VisualStatistics

    # Generate debug info also in release mode so that it is possible to use breakpad
    # After dump symbol to a separate file, use strip --strip-debug to strip the debug
    # infos
    # QMAKE_CXXFLAGS_RELEASE += -g

    INCLUDEPATH += $$PWD/third_party/pcre/linux
    INCLUDEPATH += $$PWD/third_party/breakpad/client/linux/handler

    LIBS += -L$$PWD/third_party/pcre/linux/ -lpcre -lz
    LIBS += -L$$PWD/third_party/breakpad/lib/linux/ -lbreakpad_client
}

DISTFILES += \
    third_party/lua/Makefile
