#!/bin/bash

############################################################################
function testRet()
{
		tRet=$?;
        if [  $tRet != "0" ] ; then
                echo -e "\033[41;30m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\033[0m"
                echo -e "\033[41;30m!qLcdDaemon错误,退出!\033[0m"
                echo -e "\033[41;30m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\033[0m"
                exit $tRet;
        fi
}

#############################################################################
#release path
REL=../../rel/
NAME=qLcdDaemon.dm
VER=1.2.5

cd qLcdDaemon;
export PATH=$PWD/../:$PATH;
#export PATH=$PWD/../qtSource/bin:$PATH;
export PATH=$PWD/../bin:$PATH;
export QMAKESPEC=$PWD/../mkspecs/linux-g++;
QMAKE_CROSS=$PWD/../qmake

cd qSandIME/;
$QMAKE_CROSS -spec $PWD/../../mkspecs/qws/linux-arm-gnueabi-g++ -o Makefile qSandIME.pro;
testRet;

cd ../qlcddaemon;
$QMAKE_CROSS -spec $PWD/../../mkspecs/qws/linux-arm-gnueabi-g++ -o Makefile qlcddaemon.pro;
testRet;

cd ..;
$QMAKE_CROSS -spec $PWD/../mkspecs/qws/linux-arm-gnueabi-g++ -o Makefile qLcdDaemon.pro;
testRet;

make clean;
testRet;
make;
testRet;
rm -rf $REL$NAME*
testRet;
arm-none-linux-gnueabi-strip qlcddaemon/qLcdDaemon 
mv qlcddaemon/qLcdDaemon $REL$NAME.$VER
testRet;


rm -rf $REL/image;
rm -rf $REL/qss;
cp -r qlcddaemon/image $REL;
cp -r qlcddaemon/qss $REL

