#!/bin/bash

############################################################################
function testRet()
{
		tRet=$?;
        if [  $tRet != "0" ] ; then
                echo -e "\033[41;30m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\033[0m"
                echo -e "\033[41;30m!keyboard错误,退出!\033[0m"
                echo -e "\033[41;30m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\033[0m"
                exit $tRet;
        fi
}

#############################################################################
#release path
CCPATH=/opt/linux_os/soft/trunk/daemon/qLcdDaemon

export PATH=$CCPATH:$PATH;
QMAKE_CROSS=$CCPATH/qmake

$QMAKE_CROSS -spec $CCPATH/mkspecs/qws/linux-arm-gnueabi-g++ -o Makefile mx25Keyboard.pro;
testRet;

make clean;
testRet;
make;
testRet;

