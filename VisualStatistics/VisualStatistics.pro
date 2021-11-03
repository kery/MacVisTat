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
    libcsv/libcsv.c \
    qcustomplot/qcustomplot.cpp \
    lua/lapi.cpp \
    lua/lauxlib.cpp \
    lua/lbaselib.cpp \
    lua/lbitlib.cpp \
    lua/lcode.cpp \
    lua/lcorolib.cpp \
    lua/lctype.cpp \
    lua/ldblib.cpp \
    lua/ldebug.cpp \
    lua/ldo.cpp \
    lua/ldump.cpp \
    lua/lfunc.cpp \
    lua/lgc.cpp \
    lua/linit.cpp \
    lua/liolib.cpp \
    lua/llex.cpp \
    lua/lmathlib.cpp \
    lua/lmem.cpp \
    lua/loadlib.cpp \
    lua/lobject.cpp \
    lua/lopcodes.cpp \
    lua/loslib.cpp \
    lua/lparser.cpp \
    lua/lstate.cpp \
    lua/lstring.cpp \
    lua/lstrlib.cpp \
    lua/ltable.cpp \
    lua/ltablib.cpp \
    lua/ltm.cpp \
    lua/lundump.cpp \
    lua/lvm.cpp \
    lua/lzio.cpp \
    AboutDialog.cpp \
    AutoCompletionSrcPlotAPIs.cpp \
    ChangeLogDialog.cpp \
    ColorManager.cpp \
    CounterGraph.cpp \
    CounterLegendItem.cpp \
    DraggablePlot.cpp \
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
    DraggablePlot.h \
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
