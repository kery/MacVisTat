QT += core gui network widgets

CONFIG += c++11

TEMPLATE = app

SOURCES += main.cpp\
        MainWindow.cpp

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

RC_FILE = CrashReporter.rc

win32 {
    TARGET = ../CrashReporter
}

unix:!macx {
    TARGET = CrashReporter
}
