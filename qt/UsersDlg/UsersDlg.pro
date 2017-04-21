#-------------------------------------------------
#
# Project created by QtCreator 2015-10-08T14:11:44
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = UsersDlg
TEMPLATE = lib
CONFIG += staticlib

DEFINES += USERSDLG_LIBRARY

DESTDIR = V:/Work/Project/qt/LIBS

SOURCES +=\
        UsersDlg.cpp \
    UserData.cpp \
    UserPropDlg.cpp \
    UserRight.cpp \
    DepartData.cpp \
    DepartDlg.cpp \
    UserRightsDlg.cpp \
    ChangePassDlg.cpp \
    NewUserDlg.cpp \
    CustomerData.cpp \
    OrganizationPropDlg.cpp \
    OrganizationsListDlg.cpp \
    OrgPersonDlg.cpp

HEADERS  += UsersDlg.h \
    UserData.h \
    UserPropDlg.h \
    UserRight.h \
    DepartData.h \
    DepartDlg.h \
    UserRightsDlg.h \
    ChangePassDlg.h \
    NewUserDlg.h \
    CustomerData.h \
    OrganizationPropDlg.h \
    OrganizationsListDlg.h \
    OrgPersonDlg.h

FORMS    += UsersDlg.ui \
    UserPropDlg.ui \
    DepartDlg.ui \
    UserRightsDlg.ui \
    ChangePassDlg.ui \
    NewUserDlg.ui \
    OrganizationPropDlg.ui \
    OrganizationsListDlg.ui \
    OrgPersonDlg.ui

#Windows XP targeting in MSVC 2012; also changed INCLUDE, PATH and LIB
QMAKE_CXXFLAGS += /D_USING_V110_SDK71_
QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS,5.01

RESOURCES += \
    resource1.qrc
