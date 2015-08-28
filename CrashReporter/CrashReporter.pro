#-------------------------------------------------
#
# Project created by QtCreator 2015-08-25T13:40:13
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TEMPLATE = app

OBJECTS_DIR = obj
MOC_DIR = moc
RCC_DIR = rcc
UI_DIR = ui

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

win32 {
    TARGET = ../../../CrashReporter

    RC_FILE = CrashReporter.rc
}

unix:!macx {
    TARGET = ../../CrashReporter
}
