#!/bin/bash

############################################################################
function testRet()
{
		tRet=$?;
        if [  $tRet != "0" ] ; then
                echo -e "\033[41;30m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\033[0m"
                echo -e "\033[41;30m!devTest错误,退出!\033[0m"
                echo -e "\033[41;30m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\033[0m"
                exit $tRet;
        fi
}

#############################################################################
#release path
REL=../../../rel/
NAME=00002
VER=1.2.4

cd qt/testpro/;
export PATH=$PWD/../qtSource/bin:$PATH;
#export PATH=$PWD/../bin:$PATH;
export QMAKESPEC=$PWD/../../../qLcdDaemon/mkspecs/linux-g++;
QMAKE_CROSS=$PWD/../../../qLcdDaemon/qmake
CC=$PWD/../../../qLcdDaemon/mkspecs/qws/linux-arm-gnueabi-g++

$QMAKE_CROSS -spec $CC -o Makefile qtestapp.pro;
testRet;

make clean;
testRet;
make;
testRet;
rm -rf $REL$NAME/*;
testRet;
arm-none-linux-gnueabi-strip $NAME;
testRet;
mv $NAME $REL$NAME/;
testRet;

