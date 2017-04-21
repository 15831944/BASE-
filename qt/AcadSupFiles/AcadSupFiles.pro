#-------------------------------------------------
#
# Project created by QtCreator 2015-05-06T18:52:48
#
#-------------------------------------------------

QT       += widgets

TARGET = AcadSupFiles
TEMPLATE = lib

DEFINES += ACADSUPFILES_LIBRARY

SOURCES += AcadSupFiles.cpp \
    AcadSupFileRight.cpp \
    FileLoadData.cpp

HEADERS += AcadSupFiles.h\
        acadsupfiles_global.h \
    AcadSupFileRight.h \
    FileLoadData.h

DESTDIR = V:/Work/Project/qt/LIBS
DLLDESTDIR = V:/Project/bin/exe

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    AcadSupFiles.ui \
    AcadSupFileRight.ui


RESOURCES += \
    resource1.qrc

win32: LIBS += -lQt5Sql
LIBS += -lV:\Work\Project\qt\LIBS\Login
LIBS += -lV:\Work\Project\qt\LIBS\Logger
LIBS += -lV:\Work\Project\qt\LIBS\VProject
