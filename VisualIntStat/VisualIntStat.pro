#-------------------------------------------------
#
# Project created by QtCreator 2015-07-19T02:23:34
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = VisualIntStat
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    plotwindow.cpp \
    third_party/qcustomplot/qcustomplot.cpp \
    third_party/pcre/pcre_byte_order.c \
    third_party/pcre/pcre_chartables.c \
    third_party/pcre/pcre_compile.c \
    third_party/pcre/pcre_config.c \
    third_party/pcre/pcre_dfa_exec.c \
    third_party/pcre/pcre_exec.c \
    third_party/pcre/pcre_fullinfo.c \
    third_party/pcre/pcre_get.c \
    third_party/pcre/pcre_globals.c \
    third_party/pcre/pcre_jit_compile.c \
    third_party/pcre/pcre_maketables.c \
    third_party/pcre/pcre_newline.c \
    third_party/pcre/pcre_printint.c \
    third_party/pcre/pcre_refcount.c \
    third_party/pcre/pcre_string_utils.c \
    third_party/pcre/pcre_study.c \
    third_party/pcre/pcre_tables.c \
    third_party/pcre/pcre_ucd.c \
    third_party/pcre/pcre_version.c \
    third_party/pcre/pcre_xclass.c \
    third_party/pcre/pcre16_byte_order.c \
    third_party/pcre/pcre16_chartables.c \
    third_party/pcre/pcre16_compile.c \
    third_party/pcre/pcre16_config.c \
    third_party/pcre/pcre16_dfa_exec.c \
    third_party/pcre/pcre16_exec.c \
    third_party/pcre/pcre16_fullinfo.c \
    third_party/pcre/pcre16_get.c \
    third_party/pcre/pcre16_globals.c \
    third_party/pcre/pcre16_jit_compile.c \
    third_party/pcre/pcre16_maketables.c \
    third_party/pcre/pcre16_newline.c \
    third_party/pcre/pcre16_ord2utf16.c \
    third_party/pcre/pcre16_printint.c \
    third_party/pcre/pcre16_refcount.c \
    third_party/pcre/pcre16_string_utils.c \
    third_party/pcre/pcre16_study.c \
    third_party/pcre/pcre16_tables.c \
    third_party/pcre/pcre16_ucd.c \
    third_party/pcre/pcre16_utf16_utils.c \
    third_party/pcre/pcre16_valid_utf16.c \
    third_party/pcre/pcre16_version.c \
    third_party/pcre/pcre16_xclass.c

HEADERS  += mainwindow.h \
    plotwindow.h \
    third_party/qcustomplot/qcustomplot.h \
    third_party/pcre/config.h \
    third_party/pcre/pcre.h \
    third_party/pcre/pcre_internal.h

FORMS    += mainwindow.ui \
    plotwindow.ui

RESOURCES += \
    visualintstat.qrc
