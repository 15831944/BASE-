#-------------------------------------------------
#
# Project created by QtCreator 2015-06-30T18:13:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Login
TEMPLATE = lib

DEFINES += LOGIN_LIBRARY

SOURCES += LoginDlg.cpp \
    OracleHelper.cpp \
    ../VProject/QLineEditEn.cpp

HEADERS  += LoginDlg.h \
    Login.h \
    OracleHelper.h \
    ../VProject/QLineEditEn.h

FORMS    += LoginDlg.ui

DESTDIR = V:/Work/Project/qt/LIBS
DLLDESTDIR = V:/Project/bin/exe

win32: LIBS += -lQt5Sql
LIBS += -luser32
#LIBS += -ladvapi32
#LIBS += -lwinspool
#LIBS += -ldelayimp
LIBS += -lV:\Work\Project\qt\LIBS\Logger
#LIBS += -lV:\Work\Project\qt\LIBS\VProject

TRANSLATIONS += Login_ru.ts Login_he.ts

#Windows XP targeting in MSVC 2012; also changed INCLUDE, PATH and LIB
QMAKE_CXXFLAGS += /D_USING_V110_SDK71_
QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS,5.01
