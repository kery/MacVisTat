QT += core gui winextras widgets concurrent network printsupport

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ColorPool.cpp \
    CommentItem.cpp \
    CounterFileParser.cpp \
    CounterGraph.cpp \
    CounterLegendItem.cpp \
    CounterNameModel.cpp \
    CounterPlot.cpp \
    DateTimeTicker.cpp \
    GzipFile.cpp \
    MultiLineInputDialog.cpp \
    PlotData.cpp \
    PlotWindow.cpp \
    ProgressDialog.cpp \
    ResizeManager.cpp \
    TextItem.cpp \
    Utils.cpp \
    ValueTipItem.cpp \
    main.cpp \
    MainWindow.cpp \
    qcustomplot/qcustomplot.cpp

HEADERS += \
    ColorPool.h \
    CommentItem.h \
    CounterFileParser.h \
    CounterGraph.h \
    CounterLegendItem.h \
    CounterNameModel.h \
    CounterPlot.h \
    DateTimeTicker.h \
    GzipFile.h \
    MainWindow.h \
    MultiLineInputDialog.h \
    PlotData.h \
    PlotWindow.h \
    ProgressDialog.h \
    ResizeManager.h \
    TextItem.h \
    Utils.h \
    ValueTipItem.h \
    Version.h \
    pcre/pcre.h \
    qcustomplot/qcustomplot.h

FORMS += \
    MainWindow.ui \
    PlotWindow.ui \
    ProgressDialog.ui

LIBS += -L$$PWD/pcre/ -lpcre

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    VisualStatistics.qrc

RC_FILE = VisualStatistics.rc

TARGET = ../VisualStatistics
