######################################################################
# Automatically generated by qmake (2.01a) ?? ?? 4 13:07:38 2008
######################################################################

TEMPLATE = app
TARGET = 
DESTDIR = ../../bin
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += .. ../libssh2/include/

QT += testlib

# Input
HEADERS += testnullfxp.h
SOURCES += testmain.cpp testnullfxp.cpp

SOURCES += ../basestorage.cpp ../utils.cpp ../globaloption.cpp \
../sshfileinfo.cpp

win32 {
    win32-g++ {
    } else {
        DEFINES += LIBSSH2_WIN32 _CRT_SECURE_NO_DEPRECATE
         CONFIG -= embed_manifest_exe
         CONFIG -= embed_manifest_dll
    }
}

win32:LIBS += -lQtTest -lws2_32  -lgdi32 
