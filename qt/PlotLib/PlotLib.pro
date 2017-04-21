#-------------------------------------------------
#
# Project created by QtCreator 2015-11-02T17:43:06
#
#-------------------------------------------------

QT       += widgets

TARGET = PlotLib
TEMPLATE = lib
CONFIG += staticlib

DEFINES += PLOT_LIBRARY

DESTDIR = V:/Work/Project/qt/LIBS

SOURCES += \
    PlotAddFileDlg.cpp \
    PlotAttrDlg.cpp \
    PlotData.cpp \
    PlotHistDlg.cpp \
    PlotNewDlg.cpp \
    PlotProp.cpp \
    PlotSimpleListDlg.cpp \
    PlotDeletedDlg.cpp \
    DwgCmpSettingsDlg.cpp \
    CDwgLayout.cpp \
    DwgData.cpp \
    DwgLayoutData.cpp \
    PlotRightsDlg.cpp \
    PlotHistoryTree.cpp \
    PlotAddFilesTree.cpp \
    PlotAddFilesTreeItem.cpp \
    PlotHistTreeItem.cpp

HEADERS += \
    PlotAddFileDlg.h \
    PlotAttrDlg.h \
    PlotData.h \
    PlotHistDlg.h \
    PlotNewDlg.h \
    PlotProp.h \
    PlotSimpleListDlg.h \
    PlotDeletedDlg.h \
    DwgCmpSettingsDlg.h \
    CDwgLayout.h \
    DwgData.h \
    DwgLayoutData.h \
    PlotRightsDlg.h \
    PlotHistoryTree.h \
    PlotHistTreeItem.h \
    PlotAddFilesTree.h \
    PlotAddFilesTreeItem.h

FORMS += \
    PlotAddFileDlg.ui \
    PlotAttrDlg.ui \
    PlotHistDlg.ui \
    PlotNewDlg.ui \
    PlotProp.ui \
    PlotSimpleListDlg.ui \
    PlotDeletedDlg.ui \
    DwgCmpSettingsDlg.ui \
    PlotRightsDlg.ui

unix {
    target.path = /usr/lib
    INSTALLS += target
}

#Windows XP targeting in MSVC 2012; also changed INCLUDE, PATH and LIB
QMAKE_CXXFLAGS += /D_USING_V110_SDK71_
QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS,5.01

