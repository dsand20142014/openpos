#!/bin/bash

############################################################################
function testRet()
{
		tRet=$?;
        if [  $tRet != "0" ] ; then
                echo -e "\033[41;30m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\033[0m"
                echo -e "\033[41;30m!sysSetting错误,退出!\033[0m"
                echo -e "\033[41;30m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\033[0m"
                exit $tRet;
        fi
}

#############################################################################
#release path
REL=../../../rel/
NAME=00001
VER=1.2.4

export PATH=$PWD/../qtSource/bin:$PATH;
cd qt/MySetting/;
export QMAKESPEC=$PWD/../../../qLcdDaemon/mkspecs/linux-g++;
QMAKE_CROSS=$PWD/../../../qLcdDaemon/qmake
CC=$PWD/../../../qLcdDaemon/mkspecs/qws/linux-arm-gnueabi-g++

$QMAKE_CROSS -spec $CC -o Makefile mysetting.pro;
testRet;

make clean;
testRet;
make;
testRet;
arm-none-linux-gnueabi-strip $NAME;
rm $REL$NAME/* -rf;
testRet;
mv $NAME $REL$NAME/;
testRet;

