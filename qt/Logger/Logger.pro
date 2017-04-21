#-------------------------------------------------
#
# Project created by QtCreator 2015-04-24T15:51:33
#
#-------------------------------------------------

QT       += widgets sql network

TARGET = Logger
TEMPLATE = lib

DEFINES += LOGGER_LIBRARY

SOURCES += logger.cpp \
    LoggerWnd.cpp

HEADERS += logger.h\
        logger_global.h \
    LoggerWnd.h

DESTDIR = V:/Work/Project/qt/LIBS
DLLDESTDIR = V:/Project/bin/exe

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    LoggerWnd.ui

TRANSLATIONS += Logger_ru.ts Logger_he.ts

#Windows XP targeting in MSVC 2012; also changed INCLUDE, PATH and LIB
QMAKE_CXXFLAGS += /D_USING_V110_SDK71_
QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS,5.01
