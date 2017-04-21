#-------------------------------------------------
#
# Project created by QtCreator 2017-02-20T18:20:51
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SchedulerLib
TEMPLATE = lib


#DESTDIR = V:/Project/qt/LIBS
#DLLDESTDIR = V:/Project/bin/exe

######################################
#DEFINES += LETTERS_LIBRARY
DEFINES += VPROJECT_MAIN_IMPORT

win32: LIBS += -lQt5Sql
LIBS += -lV:\Project\qt\LIBS\VProject
LIBS += -lV:\Project\qt\LIBS\Login
LIBS += -lV:\Project\qt\LIBS\Logger
LIBS += -lV:\Project\qt\LIBS\UsersDlg


#######################################

SOURCES += main.cpp\
        schedulerlib.cpp \
    newmeeting.cpp \
    databaseconnection.cpp

HEADERS  += schedulerlib.h \
    newmeeting.h \
    databaseconnection.h

FORMS    += schedulerlib.ui \
    newmeeting.ui

QMAKE_CXXFLAGS += /D_USING_V110_SDK71_
QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS,5.01
