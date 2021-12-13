QT += core gui winextras widgets concurrent network printsupport

CONFIG += c++11 qscintilla2

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += _CRT_SECURE_NO_WARNINGS

SOURCES += \
    AutoCompletionSrcPlotAPIs.cpp \
    ColorPool.cpp \
    CommentItem.cpp \
    CounterData.cpp \
    CounterDescription.cpp \
    CounterFileParser.cpp \
    CounterGraph.cpp \
    CounterLegendItem.cpp \
    CounterName.cpp \
    CounterNameModel.cpp \
    CounterPlot.cpp \
    DateTimeTicker.cpp \
    GzipFile.cpp \
    LuaEnvironment.cpp \
    MultiLineInputDialog.cpp \
    PlotData.cpp \
    PlotWindow.cpp \
    ProgressDialog.cpp \
    ResizeManager.cpp \
    SciLexerLua5_2.cpp \
    ScriptWindow.cpp \
    TextItem.cpp \
    Utils.cpp \
    ValueTipItem.cpp \
    libcsv/libcsv.c \
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
    main.cpp \
    MainWindow.cpp \
    qcustomplot/qcustomplot.cpp

HEADERS += \
    AutoCompletionSrcPlotAPIs.h \
    ColorPool.h \
    CommentItem.h \
    CounterData.h \
    CounterDescription.h \
    CounterFileParser.h \
    CounterGraph.h \
    CounterLegendItem.h \
    CounterName.h \
    CounterNameModel.h \
    CounterPlot.h \
    DateTimeTicker.h \
    GlobalDefines.h \
    GzipFile.h \
    LuaEnvironment.h \
    MainWindow.h \
    MultiLineInputDialog.h \
    PlotData.h \
    PlotWindow.h \
    ProgressDialog.h \
    ResizeManager.h \
    SciLexerLua5_2.h \
    ScriptWindow.h \
    TextItem.h \
    Utils.h \
    ValueTipItem.h \
    Version.h \
    libcsv/csv.h \
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
    pcre/pcre.h \
    qcustomplot/qcustomplot.h

FORMS += \
    MainWindow.ui \
    PlotWindow.ui \
    ProgressDialog.ui \
    ScriptWindow.ui

LIBS += -L$$PWD/pcre/ -lpcre

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    VisualStatistics.qrc

RC_FILE = VisualStatistics.rc

TARGET = ../VisualStatistics
