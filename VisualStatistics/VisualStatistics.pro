QT += core gui widgets concurrent network printsupport

CONFIG += c++11 qscintilla2

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AboutDialog.cpp \
    Application.cpp \
    AutoCompletionSrcPlotAPIs.cpp \
    BalloonTip.cpp \
    ChangeLogDialog.cpp \
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
    FileDialog.cpp \
    FilterValidator.cpp \
    GzipFile.cpp \
    KpiKciFileParser.cpp \
    LogTextEdit.cpp \
    LuaEnvironment.cpp \
    MultiLineInputDialog.cpp \
    OptionsDialog.cpp \
    PlotData.cpp \
    PlotWindow.cpp \
    ProgressDialog.cpp \
    QCustomPlot/src/axis/axis.cpp \
    QCustomPlot/src/axis/axisticker.cpp \
    QCustomPlot/src/axis/axistickerdatetime.cpp \
    QCustomPlot/src/axis/axistickerfixed.cpp \
    QCustomPlot/src/axis/axistickerlog.cpp \
    QCustomPlot/src/axis/axistickerpi.cpp \
    QCustomPlot/src/axis/axistickertext.cpp \
    QCustomPlot/src/axis/axistickertime.cpp \
    QCustomPlot/src/axis/labelpainter.cpp \
    QCustomPlot/src/axis/range.cpp \
    QCustomPlot/src/colorgradient.cpp \
    QCustomPlot/src/core.cpp \
    QCustomPlot/src/item.cpp \
    QCustomPlot/src/items/item-bracket.cpp \
    QCustomPlot/src/items/item-curve.cpp \
    QCustomPlot/src/items/item-ellipse.cpp \
    QCustomPlot/src/items/item-line.cpp \
    QCustomPlot/src/items/item-pixmap.cpp \
    QCustomPlot/src/items/item-rect.cpp \
    QCustomPlot/src/items/item-straightline.cpp \
    QCustomPlot/src/items/item-text.cpp \
    QCustomPlot/src/items/item-tracer.cpp \
    QCustomPlot/src/layer.cpp \
    QCustomPlot/src/layout.cpp \
    QCustomPlot/src/layoutelements/layoutelement-axisrect.cpp \
    QCustomPlot/src/layoutelements/layoutelement-colorscale.cpp \
    QCustomPlot/src/layoutelements/layoutelement-legend.cpp \
    QCustomPlot/src/layoutelements/layoutelement-textelement.cpp \
    QCustomPlot/src/lineending.cpp \
    QCustomPlot/src/paintbuffer.cpp \
    QCustomPlot/src/painter.cpp \
    QCustomPlot/src/plottable.cpp \
    QCustomPlot/src/plottables/plottable-bars.cpp \
    QCustomPlot/src/plottables/plottable-colormap.cpp \
    QCustomPlot/src/plottables/plottable-curve.cpp \
    QCustomPlot/src/plottables/plottable-errorbar.cpp \
    QCustomPlot/src/plottables/plottable-financial.cpp \
    QCustomPlot/src/plottables/plottable-graph.cpp \
    QCustomPlot/src/plottables/plottable-statisticalbox.cpp \
    QCustomPlot/src/polar/layoutelement-angularaxis.cpp \
    QCustomPlot/src/polar/polargraph.cpp \
    QCustomPlot/src/polar/polargrid.cpp \
    QCustomPlot/src/polar/radialaxis.cpp \
    QCustomPlot/src/scatterstyle.cpp \
    QCustomPlot/src/selection.cpp \
    QCustomPlot/src/selectiondecorator-bracket.cpp \
    QCustomPlot/src/selectionrect.cpp \
    QCustomPlot/src/vector2d.cpp \
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
    MainWindow.cpp

HEADERS += \
    AboutDialog.h \
    Application.h \
    AutoCompletionSrcPlotAPIs.h \
    BalloonTip.h \
    ChangeLogDialog.h \
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
    FileDialog.h \
    FilterValidator.h \
    GlobalDefines.h \
    GzipFile.h \
    KpiKciFileParser.h \
    LogTextEdit.h \
    LuaEnvironment.h \
    MainWindow.h \
    MultiLineInputDialog.h \
    OptionsDialog.h \
    PlotData.h \
    PlotWindow.h \
    ProgressDialog.h \
    QCustomPlot/src/axis/axis.h \
    QCustomPlot/src/axis/axisticker.h \
    QCustomPlot/src/axis/axistickerdatetime.h \
    QCustomPlot/src/axis/axistickerfixed.h \
    QCustomPlot/src/axis/axistickerlog.h \
    QCustomPlot/src/axis/axistickerpi.h \
    QCustomPlot/src/axis/axistickertext.h \
    QCustomPlot/src/axis/axistickertime.h \
    QCustomPlot/src/axis/labelpainter.h \
    QCustomPlot/src/axis/range.h \
    QCustomPlot/src/colorgradient.h \
    QCustomPlot/src/core.h \
    QCustomPlot/src/datacontainer.h \
    QCustomPlot/src/global.h \
    QCustomPlot/src/item.h \
    QCustomPlot/src/items/item-bracket.h \
    QCustomPlot/src/items/item-curve.h \
    QCustomPlot/src/items/item-ellipse.h \
    QCustomPlot/src/items/item-line.h \
    QCustomPlot/src/items/item-pixmap.h \
    QCustomPlot/src/items/item-rect.h \
    QCustomPlot/src/items/item-straightline.h \
    QCustomPlot/src/items/item-text.h \
    QCustomPlot/src/items/item-tracer.h \
    QCustomPlot/src/layer.h \
    QCustomPlot/src/layout.h \
    QCustomPlot/src/layoutelements/layoutelement-axisrect.h \
    QCustomPlot/src/layoutelements/layoutelement-colorscale.h \
    QCustomPlot/src/layoutelements/layoutelement-legend.h \
    QCustomPlot/src/layoutelements/layoutelement-textelement.h \
    QCustomPlot/src/lineending.h \
    QCustomPlot/src/paintbuffer.h \
    QCustomPlot/src/painter.h \
    QCustomPlot/src/plottable.h \
    QCustomPlot/src/plottable1d.h \
    QCustomPlot/src/plottables/plottable-bars.h \
    QCustomPlot/src/plottables/plottable-colormap.h \
    QCustomPlot/src/plottables/plottable-curve.h \
    QCustomPlot/src/plottables/plottable-errorbar.h \
    QCustomPlot/src/plottables/plottable-financial.h \
    QCustomPlot/src/plottables/plottable-graph.h \
    QCustomPlot/src/plottables/plottable-statisticalbox.h \
    QCustomPlot/src/polar/layoutelement-angularaxis.h \
    QCustomPlot/src/polar/polargraph.h \
    QCustomPlot/src/polar/polargrid.h \
    QCustomPlot/src/polar/radialaxis.h \
    QCustomPlot/src/scatterstyle.h \
    QCustomPlot/src/selection.h \
    QCustomPlot/src/selectiondecorator-bracket.h \
    QCustomPlot/src/selectionrect.h \
    QCustomPlot/src/vector2d.h \
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
    lua/lzio.h

FORMS += \
    AboutDialog.ui \
    ChangeLogDialog.ui \
    MainWindow.ui \
    OptionsDialog.ui \
    PlotWindow.ui \
    ProgressDialog.ui \
    ScriptWindow.ui

RESOURCES += \
    VisualStatistics.qrc

INCLUDEPATH += libexpat/expat/lib/ breakpad/src/

win32:CONFIG(debug, debug|release) {
    LIBS += -L../build-breakpad-Debug/lib/
}

win32:CONFIG(release, debug|release) {
    LIBS += -L../build-breakpad-Release/lib/

    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF /OPT:ICF
}

win32 {
    QT += winextras
    DEFINES += _CRT_SECURE_NO_WARNINGS
    RC_FILE = VisualStatistics.rc
    TARGET = ../VisualStatistics

    INCLUDEPATH += ../build-pcre2/
    INCLUDEPATH += breakpad/src/client/windows/handler/

    LIBS += -lShlwapi -lUser32
    LIBS += -L../build-pcre2/Release/
    LIBS += -L../build-libexpat/Release/ -llibexpat
    LIBS += -lexception_handler -lcrash_generation_client -lcommon
}

unix:!macx {
    TARGET = VisualStatistics

    INCLUDEPATH += pcre2/src/
    INCLUDEPATH += breakpad/ breakpad/src/client/linux/handler/

    LIBS += -L$$PWD/pcre2/.libs/
    LIBS += -L$$PWD/libexpat/expat/lib/.libs/ -lexpat
    LIBS += -L$$PWD/breakpad/src/client/linux/ -lbreakpad_client
    LIBS += -lz
}

LIBS += -lpcre2-8
