#-------------------------------------------------
#
# Project created by QtCreator 2015-10-26T14:01:49
#
#-------------------------------------------------

QT       += widgets sql

TARGET = ProjectLib
TEMPLATE = lib
CONFIG += staticlib

DEFINES += PROJECT_LIBRARY

DESTDIR = V:/Work/Project/qt/LIBS

SOURCES += \
    ProjectData.cpp \
    ProjectTypeData.cpp \
    CodeFormingDlg.cpp \
    ProjectPropDlg.cpp \
    ContsrPropDlg.cpp \
    ProjectListDlg.cpp \
    ProjectTree.cpp \
    ProjectCard.cpp \
    ProjectCardReport.cpp \
    ProjectTypeDlg.cpp \
    ProjectRightsDlg.cpp

HEADERS += \
    ProjectData.h \
    CodeFormingDlg.h \
    ProjectPropDlg.h \
    ContsrPropDlg.h \
    ProjectListDlg.h \
    ProjectTree.h \
    ProjectCard.h \
    ProjectTypeDlg.h \
    ProjectTypeData.h \
    ProjectRightsDlg.h

DISTFILES +=

FORMS += \
    CodeFormingDlg.ui \
    ProjectPropDlg.ui \
    ContsrPropDlg.ui \
    ProjectListDlg.ui \
    ProjectCard.ui \
    ProjectTypeDlg.ui \
    ProjectRightsDlg.ui

#Windows XP targeting in MSVC 2012; also changed INCLUDE, PATH and LIB
QMAKE_CXXFLAGS += /D_USING_V110_SDK71_
QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS,5.01
