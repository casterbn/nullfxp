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

win32:LIBS += -lQtTest
