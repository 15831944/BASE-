#-------------------------------------------------
#
# Project created by QtCreator 2016-06-08T17:11:47
#
#-------------------------------------------------

QT       += widgets

TARGET = SaveLoadLib
TEMPLATE = lib

DEFINES += SAVELOADLIB_LIBRARY
DEFINES += VPROJECT_MAIN_IMPORT

SOURCES += \
    LoadImagesDlg.cpp \
    TreeWidgetItemLoad.cpp \
    LISettingsDlg.cpp \
    wdSettingsImage.cpp \
    VPImageViewer.cpp

HEADERS +=\
        saveloadlib_global.h \
    LoadImagesDlg.h \
    TreeWidgetItemLoad.h \
    LISettingsDlg.h \
    wdSettingsImage.h \
    VPImageViewer.h

DESTDIR = V:/Work/Project/qt/LIBS
DLLDESTDIR = V:/Project/bin/exe

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    LoadImagesDlg.ui \
    LISettingsDlg.ui \
    wdSettingsImage.ui \
    VPImageViewer.ui

win32: LIBS += -lQt5Sql
LIBS += -lV:\Work\Project\qt\LIBS\Login
LIBS += -lV:\Work\Project\qt\LIBS\Logger
LIBS += -lV:\Work\Project\qt\LIBS\VProject

#Windows XP targeting in MSVC 2012; also changed INCLUDE, PATH and LIB
QMAKE_CXXFLAGS += /D_USING_V110_SDK71_
QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS,5.01
