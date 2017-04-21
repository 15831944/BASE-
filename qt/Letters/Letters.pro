#-------------------------------------------------
#
# Project created by QtCreator 2016-01-24T14:22:45
#
#-------------------------------------------------

QT       += widgets

TARGET = Letters
TEMPLATE = lib

DEFINES += LETTERS_LIBRARY

SOURCES += Letters.cpp

HEADERS += Letters.h\
        letters_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
