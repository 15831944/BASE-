#-------------------------------------------------
#
# Project created by QtCreator 2015-05-06T15:31:31
#
#-------------------------------------------------

QT       += widgets network sql xml

TARGET = contract-pkz
TEMPLATE = lib

DEFINES += CONTRACTPKZ_LIBRARY
DEFINES += VPROJECT_MAIN_IMPORT

SOURCES += ContractPkz.cpp \
    qcontracttree.cpp \
    contractprop.cpp \
    contractstageprop.cpp \
    contractcheck.cpp \
    ContractDefVAT.cpp \
    PayByCustParams.cpp \
    ContractReport.cpp \
    ContractReport2.cpp \
    ContractReportsOld.cpp \
    ContractReportLib.cpp \
    ContractReport3.cpp

HEADERS += ContractPkz.h\
        contract-pkz_global.h \
    qcontracttree.h \
    contractprop.h \
    contractstageprop.h \
    contractcheck.h \
    ContractDefVAT.h \
    PayByCustParams.h \
    contract-pkz_local.h \
    ContractReportLib.h

DESTDIR = V:/Work/Project/qt/LIBS
DLLDESTDIR = V:/Project/bin/exe

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    ContractPkz.ui \
    contractprop.ui \
    contractstageprop.ui \
    contractcheck.ui \
    ContractDefVAT.ui \
    PayByCustParams.ui

RESOURCES += \
    resource1.qrc

win32: LIBS += -lQt5Sql
LIBS += -lV:\Work\Project\qt\LIBS\Login
LIBS += -lV:\Work\Project\qt\LIBS\Logger
LIBS += -lV:\Work\Project\qt\LIBS\VProject

#Windows XP targeting in MSVC 2012; also changed INCLUDE, PATH and LIB
QMAKE_CXXFLAGS += /D_USING_V110_SDK71_
QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS,5.01
