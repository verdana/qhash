#-------------------------------------------------
#
# Project created by QtCreator 2013-10-05T18:44:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QHash
TEMPLATE = app


SOURCES +=  main.cpp\
          window.cpp \
    workerthread.cpp

HEADERS += window.h \
     workerthread.h

FORMS += window.ui

RESOURCES += QHash.qrc

RC_FILE = QHash.rc
