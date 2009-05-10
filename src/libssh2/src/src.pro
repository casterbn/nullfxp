SOURCES += channel.c \
comp.c \
crypt.c \
hostkey.c \
kex.c \
mac.c \
misc.c \
#openssl.c \
#libgcrypt.c \
packet.c \
pem.c \
publickey.c \
scp.c \
session.c \
sftp.c \
transport.c \
userauth.c \
version.c 

HEADERS += libgcrypt.h \
libssh2_config.h \
libssh2_priv.h \
openssl.h

win32{
	#SOURCES += libgcrypt.c
	SOURCES += openssl.c	
} else {
	SOURCES += openssl.c
}


DESTDIR = .

TEMPLATE = lib

CONFIG += staticlib \
 console 

CONFIG -= qt 

win32 {
	CONFIG += release
} else {
	CONFIG += debug release
}




DEFINES += HAVE_CONFIG_H \
 LIBSSH2DEBUG=1 LIBSSH2_MD5=1

win32 {
    !win32-g++ {
        DEFINES += LIBSSH2_WIN32 _CRT_SECURE_NO_DEPRECATE
        INCLUDEPATH += Z:/librarys/vc-ssl/include Z:/librarys/vc-zlib/include
    }
}

TARGET = ssh2


QMAKE_CXXFLAGS_DEBUG += -g

QMAKE_CXXFLAGS_RELEASE += -g

INCLUDEPATH += ../include/

