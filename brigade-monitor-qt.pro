#-------------------------------------------------
#
# Project created by QtCreator 2014-01-15T10:49:31
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = brigade-monitor-qt
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    cgminerapi.cpp \
updater.cpp

HEADERS  += mainwindow.h \
    cgminerapi.h \
updater.h

FORMS    += mainwindow.ui

release: DESTDIR = build/release
debug:   DESTDIR = build/debug

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

RESOURCES += \
    monitor.qrc
