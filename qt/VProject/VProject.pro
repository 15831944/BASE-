#-------------------------------------------------
#
# Project created by QtCreator 2013-04-29T13:30:04
#
#-------------------------------------------------

QT += core gui widgets network xml

TARGET = VProject
TEMPLATE = app

DEFINES += VPROJECT_MAIN

DESTDIR = V:/Project/bin/exe

QMAKE_LFLAGS += /INCREMENTAL:NO

#QMAKE_LFLAGS += /DELAYLOAD:Logger.dll
QMAKE_LFLAGS += /DELAYLOAD:contract-pkz.dll
QMAKE_LFLAGS += /DELAYLOAD:AcadSupFiles.dll
QMAKE_LFLAGS += /DELAYLOAD:geobase.dll
QMAKE_LFLAGS += /DELAYLOAD:SaveLoadLib.dll
QMAKE_LFLAGS += /DELAYLOAD:WorkByDateUser.dll

QMAKE_LFLAGS += /IMPLIB:V:/Work/Project/qt/LIBS/VProject.lib

SOURCES += main.cpp \
    PublishReport.cpp \
    auditreport.cpp \
    entersavename.cpp \
    oracle.cpp \
    typetreeselect.cpp \
    qfcdialog.cpp \
    SelectColumnsDlg.cpp \
    MadadFromSite.cpp \
    TaskProp.cpp \
    SaveDialog.cpp \
    acad.cpp \
    BlobMemCache.cpp \
    QXrefTreeWidget.cpp \
    DocTreeSettings.cpp \
    TreeData.cpp \
    MainWindow.cpp \
    GlobalSettings.cpp \
    CommonSettingsDlg.cpp \
    PlotListDlg.cpp \
    PlotListTree.cpp \
    PlotTree.cpp \
    AcadExchange.cpp \
    MSOffice.cpp \
    ForSaveData.cpp \
    TWIForSave.cpp \
    PlotListItemDelegate.cpp \
    XrefPropsData.cpp \
    AcadXchgDialog.cpp \
    ChooseAcadDlg.cpp \
    AuditPurgeDlg.cpp \
    PublishDlg.cpp \
    PlotListTreeChange.cpp \
    SentParamsDgl.cpp \
    WaitDlg.cpp \
    PlotListTree2.cpp \
    DocListSettingsDlg.cpp \
    FindDlg.cpp \
    PlotListTreeRefresh.cpp \
    FileUtils.cpp \
    UpdateThread.cpp \
    LoadXrefsDlg.cpp \
    PreloadParamsDlg.cpp \
    LoadXrefsListItem.cpp \
    QRoCheckBox.cpp \
    HomeData.cpp \
    QLineEditEn.cpp \
    QMyMdiSubWindow.cpp \
    PlotListTreeShort.cpp \
    wdSettings.cpp \
    XMLProcess.cpp \
    ReplaceTextDlg.cpp

HEADERS  += \
    PublishReport.h \
    common.h \
    auditreport.h \
    QRoCheckBox.h \
    entersavename.h \
    oracle.h \
    typetreeselect.h \
    qfcdialog.h \
    SelectColumnsDlg.h \
    TaskProp.h \
    SaveDialog.h \
    acad.h \
    QXrefTreeWidget.h \
    DocTreeSettings.h \
    CommonData.h \
    TreeData.h \
    MainWindow.h \
    GlobalSettings.h \
    CommonSettingsDlg.h \
    PlotListDlg.h \
    PlotListTree.h \
    PlotTree.h \
    BlobMemCache.h \
    AcadExchange.h \
    MSOffice.h \
    ForSaveData.h \
    TWIForSave.h \
    PlotListItemDelegate.h \
    XrefPropsData.h \
    AcadXchgDialog.h \
    ChooseAcadDlg.h \
    AuditPurgeDlg.h \
    PublishDlg.h \
    PlotListTreeChange.h \
    SentParamsDgl.h \
    WaitDlg.h \
    DocListSettingsDlg.h \
    FindDlg.h \
    FileUtils.h \
    UpdateThread.h \
    LoadXrefsDlg.h \
    PreloadParamsDlg.h \
    def_expimp.h \
    HomeData.h \
    QLineEditEn.h \
    QMyMdiSubWindow.h \
    wdSettings.h \
    XMLProcess.h \
    ReplaceTextDlg.h

FORMS    += \
    PublishReport.ui \
    auditreport.ui \
    entersavename.ui \
    typetreeselect.ui \
    SelectColumnsDlg.ui \
    TaskProp.ui \
    SaveDialog.ui \
    DocTreeSettings.ui \
    MainWindow.ui \
    CommonSettingsDlg.ui \
    PlotListDlg.ui \
    ChooseAcadDlg.ui \
    AuditPurgeDlg.ui \
    PublishDlg.ui \
    PlotListTreeChange.ui \
    SentParamsDgl.ui \
    WaitDlg.ui \
    DocListSettingsDlg.ui \
    FindDlg.ui \
    LoadXrefsDlg.ui \
    PreloadParamsDlg.ui \
    ReplaceTextDlg.ui

win32: LIBS += -lQt5Sql
LIBS += -luser32
LIBS += -ladvapi32
LIBS += -lwinspool
LIBS += -ldelayimp
LIBS += -lV:\Work\Project\qt\LIBS\Login
LIBS += -lV:\Work\Project\qt\LIBS\Logger
LIBS += -lV:\Work\Project\qt\LIBS\contract-pkz
LIBS += -lV:\Work\Project\qt\LIBS\AcadSupFiles
LIBS += -lV:\Work\Project\qt\LIBS\geobase
LIBS += -lV:\Work\Project\qt\LIBS\WorkByDateUser
#LIBS += -lV:\Work\Project\qt\LIBS\PrCa
LIBS += -lV:\Work\Project\qt\LIBS\UsersDlg
LIBS += -lV:\Work\Project\qt\LIBS\ProjectLib
LIBS += -lV:\Work\Project\qt\LIBS\PlotLib
LIBS += -lV:\Work\Project\qt\LIBS\SaveLoadLib

TRANSLATIONS += VProject_ru.ts VProject_he.ts

RESOURCES += resource1.qrc

RC_FILE = database_ico.rc

PATH += c:\ora10\bin;

#win32 {
#    CONFIG += embed_manifest_exe
#    QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'asInvoker\'
#}
#asInvoker
#highestAvailable
#requireAdministrator

#Windows XP targeting in MSVC 2012; also changed INCLUDE, PATH and LIB
QMAKE_CXXFLAGS += /D_USING_V110_SDK71_
QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS,5.01
