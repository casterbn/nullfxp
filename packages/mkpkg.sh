#!/bin/sh


appname=`basename $0 | sed s,\.sh$,,`
dirname=`dirname $0`
if [ "${dirname:0:1}" != "/" ]; then
   dirname=$PWD/$dirname
fi

#LD_LIBRARY_PATH=$dirname:$dirname/lib
#export LD_LIBRARY_PATH
#$dirname/lib/$appname $*

mkdir -p $dirname/../nullfxp
mkdir -p $dirname/../nullfxp/bin
mkdir -p $dirname/../nullfxp/lib
mkdir -p $dirname/../nullfxp/share
mkdir -p $dirname/../nullfxp/doc
mkdir -p $dirname/../nullfxp/man
mkdir -p $dirname/../nullfxp/plugins

rm -v $dirname/../nullfxp/lib/*
cp -v $dirname/nullfxp $dirname/../nullfxp/bin/
chmod +x $dirname/../nullfxp/bin/nullfxp

cp -v $dirname/../bin/nullfxp $dirname/../nullfxp/lib/
cp -v $dirname/../bin/unitest $dirname/../nullfxp/bin/
cp -v $dirname/../bin/plink   $dirname/../nullfxp/lib/
strip -s -v $dirname/../nullfxp/lib/nullfxp
strip -s -v $dirname/../nullfxp/bin/unitest
strip -s -v $dirname/../nullfxp/lib/plink

VER_FILE=$dirname/../src/nullfxp-version.h
echo $VER_FILE
VERSION=`cat $VER_FILE|grep NullFXP|awk '{print $4}'`
echo $VERSION

USED_OPENSSL_SSL=`ldd $dirname/../nullfxp/lib/nullfxp|grep libssl|awk '{print $3}'` 
USED_OPENSSL_CRYPTO=`ldd $dirname/../nullfxp/lib/nullfxp|grep libcrypto|awk '{print $3}'` 
echo $USED_OPENSSL_SSL $USED_OPENSSL_CRYPTO
cp -v $USED_OPENSSL_SSL $dirname/../nullfxp/lib/
cp -v $USED_OPENSSL_CRYPTO $dirname/../nullfxp/lib/

USED_QT=`ldd $dirname/../nullfxp/lib/nullfxp|grep libQt|awk '{print $3}'`
echo $USED_QT
if [ x"$USED_QT" = x"" ]; then
    LINK_TYPE=static
else
    LINK_TYPE=shared
    for lib_name in $USED_QT 
    do
	#cp -v $lib_name $dirname/../nullfxp/lib/
	echo $lib_name
    done
fi

#echo $USE_QTCORE $LINK_TYPE
#LINK_TYPE=static
LINK_PLATFORM=`uname`

echo "package info: $LINK_PLATFORM $LINK_TYPE $VERSION"
tar jcvf $dirname/../nullfxp-$VERSION-$LINK_TYPE-qt4.i686.$LINK_PLATFORM.tar.bz2 $dirname/../nullfxp