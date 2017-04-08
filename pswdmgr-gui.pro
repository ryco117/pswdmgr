#-------------------------------------------------
#
# Project created by QtCreator 2017-04-07T01:33:30
#
#-------------------------------------------------

QT       += core gui


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = pswdmgr-gui
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    pswdmgr.cpp \
    getpswd.cpp \
    getsiteuserpass.cpp \
    rmsitedialog.cpp

HEADERS  += mainwindow.h \
    pswdmgr.h \
    getpswd.h \
    getsiteuserpass.h \
    rmsitedialog.h

FORMS    += mainwindow.ui \
    getpswd.ui \
    getsiteuserpass.ui \
    rmsitedialog.ui

LIBS += -lcryptolibrary -lscrypt

unix: LIBS += -L/usr/local/lib/
INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

unix: PRE_TARGETDEPS += /usr/local/lib/libcryptolibrary.a
