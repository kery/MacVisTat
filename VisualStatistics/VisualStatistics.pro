#-------------------------------------------------
#
# Project created by QtCreator 2015-07-19T02:23:34
#
#-------------------------------------------------

QT       += core gui concurrent network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11 qscintilla2

TEMPLATE = app

OBJECTS_DIR = obj
MOC_DIR = moc
RCC_DIR = rcc
UI_DIR = ui

SOURCES += main.cpp\
    CommentText.cpp \
    CustomPlot.cpp \
    MultiLineInputDialog.cpp \
    ResizeManager.cpp \
    libcsv/libcsv.c \
    qcustomplot/qcustomplot.cpp \
    lua/lapi.c \
    lua/lauxlib.c \
    lua/lbaselib.c \
    lua/lbitlib.c \
    lua/lcode.c \
    lua/lcorolib.c \
    lua/lctype.c \
    lua/ldblib.c \
    lua/ldebug.c \
    lua/ldo.c \
    lua/ldump.c \
    lua/lfunc.c \
    lua/lgc.c \
    lua/linit.c \
    lua/liolib.c \
    lua/llex.c \
    lua/lmathlib.c \
    lua/lmem.c \
    lua/loadlib.c \
    lua/lobject.c \
    lua/lopcodes.c \
    lua/loslib.c \
    lua/lparser.c \
    lua/lstate.c \
    lua/lstring.c \
    lua/lstrlib.c \
    lua/ltable.c \
    lua/ltablib.c \
    lua/ltm.c \
    lua/lundump.c \
    lua/lvm.c \
    lua/lzio.c \
    AboutDialog.cpp \
    AutoCompletionSrcPlotAPIs.cpp \
    ChangeLogDialog.cpp \
    ColorManager.cpp \
    CounterGraph.cpp \
    CounterLegendItem.cpp \
    GzipFile.cpp \
    LuaEnvironment.cpp \
    MainWindow.cpp \
    PlotWindow.cpp \
    ProgressDialog.cpp \
    SciLexerLua5_2.cpp \
    ScriptWindow.cpp \
    Statistics.cpp \
    StatisticsFileParser.cpp \
    StatisticsNameModel.cpp \
    Utils.cpp \
    ValueText.cpp

HEADERS  += \
    CommentText.h \
    CustomPlot.h \
    MultiLineInputDialog.h \
    ResizeManager.h \
    libcsv/csv.h \
    qcustomplot/qcustomplot.h \
    lua/lapi.h \
    lua/lauxlib.h \
    lua/lcode.h \
    lua/lctype.h \
    lua/ldebug.h \
    lua/ldo.h \
    lua/lfunc.h \
    lua/lgc.h \
    lua/llex.h \
    lua/llimits.h \
    lua/lmem.h \
    lua/lobject.h \
    lua/lopcodes.h \
    lua/lparser.h \
    lua/lstate.h \
    lua/lstring.h \
    lua/ltable.h \
    lua/ltm.h \
    lua/lua.h \
    lua/luaconf.h \
    lua/lualib.h \
    lua/lundump.h \
    lua/lvm.h \
    lua/lzio.h \
    AboutDialog.h \
    AutoCompletionSrcPlotAPIs.h \
    ChangeLogDialog.h \
    ColorManager.h \
    CounterGraph.h \
    CounterLegendItem.h \
    GzipFile.h \
    LuaEnvironment.h \
    MainWindow.h \
    PlotWindow.h \
    ProgressDialog.h \
    SciLexerLua5_2.h \
    ScriptWindow.h \
    Statistics.h \
    StatisticsFileParser.h \
    StatisticsNameModel.h \
    Utils.h \
    ValueText.h \
    Version.h

FORMS    += \
    AboutDialog.ui \
    ChangeLogDialog.ui \
    MainWindow.ui \
    PlotWindow.ui \
    ProgressDialog.ui \
    ScriptWindow.ui

RESOURCES += \
    VisualStatistics.qrc

INCLUDEPATH += $$PWD/qcustomplot/
INCLUDEPATH += $$PWD/lua/
INCLUDEPATH += $$PWD/expat

win32 {
    INCLUDEPATH += $$PWD/breakpad/

    QT += winextras
    TARGET = ../VisualStatistics

    # Remove the warnings for calling function like strcmp
    DEFINES += _CRT_SECURE_NO_WARNINGS

    RC_FILE = VisualStatistics.rc

    INCLUDEPATH += $$PWD/pcre/win
    INCLUDEPATH += $$PWD/breakpad/client/windows/handler

    LIBS += -L$$PWD/pcre/win/ -lpcre
    LIBS += -L$$PWD/expat/win/ -llibexpat
}

win32:CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/breakpad/lib/win/debug -lexception_handler -lcrash_generation_client -lcommon
}

win32:CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF /OPT:ICF

    LIBS += -L$$PWD/breakpad/lib/win/release -lexception_handler -lcrash_generation_client -lcommon
}

win32:CONFIG(profiling) {
    QMAKE_LFLAGS_RELEASE += /PROFILE
}

unix:!macx {
    TARGET = VisualStatistics

    # Generate debug info also in release mode so that it is possible to use breakpad
    # After dump symbol to a separate file, use strip --strip-debug to strip the debug
    # infos
    # QMAKE_CXXFLAGS_RELEASE += -g

    INCLUDEPATH += $$PWD/pcre/linux

    LIBS += -L$$PWD/pcre/linux/ -lpcre -lz
    LIBS += -L$$PWD/expat/linux/ -lexpat
}

DISTFILES += \
    lua/Makefile
