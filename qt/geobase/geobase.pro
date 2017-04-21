#-------------------------------------------------
#
# Project created by QtCreator 2015-05-07T15:22:37
#
#-------------------------------------------------

QT       += widgets sql

TARGET = geobase
TEMPLATE = lib

DEFINES += GEOBASE_LIBRARY
DEFINES += VPROJECT_MAIN_IMPORT

SOURCES += geobase.cpp \
    qgeobasetree.cpp \
    geobasedrawingprop.cpp \
    geobaseloadfiles.cpp \
    geobaseprop.cpp \
    geobasesettings.cpp \
    xreftypedata.cpp

HEADERS += geobase.h\
        geobase_global.h \
    qgeobasetree.h \
    geobasedrawingprop.h \
    geobaseloadfiles.h \
    geobaseprop.h \
    geobasesettings.h \
    xreftypedata.h

DESTDIR = V:/Work/Project/qt/LIBS
DLLDESTDIR = V:/Project/bin/exe

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    geobase.ui \
    geobasedrawingprop.ui \
    geobaseloadfiles.ui \
    geobaseprop.ui \
    geobasesettings.ui

RESOURCES += \
    resource1.qrc

win32: LIBS += -lQt5Sql
LIBS += -lV:\Work\Project\qt\LIBS\Login
LIBS += -lV:\Work\Project\qt\LIBS\Logger
LIBS += -lV:\Work\Project\qt\LIBS\VProject

#Windows XP targeting in MSVC 2012; also changed INCLUDE, PATH and LIB
QMAKE_CXXFLAGS += /D_USING_V110_SDK71_
QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS,5.01
