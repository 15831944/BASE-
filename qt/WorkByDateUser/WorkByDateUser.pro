#-------------------------------------------------
#
# Project created by QtCreator 2015-05-07T18:21:39
#
#-------------------------------------------------

QT       += widgets sql

TARGET = WorkByDateUser
TEMPLATE = lib

DEFINES += WORKBYDATEUSER_LIBRARY
DEFINES += VPROJECT_MAIN_IMPORT

SOURCES += workbydateuser.cpp \
    WorkByDateUserItem.cpp

HEADERS += workbydateuser.h\
        workbydateuser_global.h

DESTDIR = V:/Work/Project/qt/LIBS
DLLDESTDIR = V:/Project/bin/exe

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    workbydateuser.ui

win32: LIBS += -lQt5Sql
LIBS += -lV:\Work\Project\qt\LIBS\Login
LIBS += -lV:\Work\Project\qt\LIBS\Logger
LIBS += -lV:\Work\Project\qt\LIBS\VProject
#LIBS += -lV:\Work\Project\qt\LIBS\PlotLib
#LIBS += -lV:\Work\Project\qt\LIBS\UsersDlg
#LIBS += -lV:\Work\Project\qt\LIBS\ProjectLib

TRANSLATIONS += WorkByDateUser_ru.ts WorkByDateUser_he.ts

#Windows XP targeting in MSVC 2012; also changed INCLUDE, PATH and LIB
QMAKE_CXXFLAGS += /D_USING_V110_SDK71_
QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS,5.01
