#!/bin/bash

PWD=.
SO_DIR=$PWD/appLib
DM_DIR=$PWD/daemon
LIB_DIR=$PWD/3rdPartyLib

############################################################################
function testRet()
{
	tRet=$?;
        if [ $tRet != "0" ] ; then
                echo -e "\033[41;30m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\033[0m"
                echo -e "\033[41;30m!检测到错误，清除缓存文件并退出!\033[0m"
                echo -e "\033[41;30m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\033[0m"
                exit $tRet;
        fi
}

#############################################################################
echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*更新SVN,请耐心等待。	       *\033[0m"
echo -e "\033[40;32m********************************\033[0m"

svn up;
testRet;

#############################################################################

rm -rf ./rel/bin;
testRet;
rm -rf ./rel/daemon;
testRet;
rm -rf ./rel/lib;
testRet;
rm -rf ./rel/etc;
testRet;
rm -rf ./rel/app;
testRet;
echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*已清除目录                    *\033[0m"
echo -e "\033[40;32m********************************\033[0m"

return ;

echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*开始复制第三方工具和库        *\033[0m"
echo -e "\033[40;32m********************************\033[0m"

cd $LIB_DIR/rel;
testRet;

cp -rf bin ../../rel/
testRet;
cp -rf lib ../../rel/
testRet;
cp -rf etc ../../rel/
testRet;
mkdir ../../rel/daemon
testRet;
mkdir ../../rel/app
testRet;

echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*完成三方库和目录重构	       *\033[0m"
echo -e "\033[40;32m********************************\033[0m"




echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*开始编译库                    *\033[0m"
echo -e "\033[40;32m********************************\033[0m"

cd ../../$SO_DIR;
testRet;

cd appApi;
testRet;
make clean;
testRet;
make;
testRet;
echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*完成applib                    *\033[0m"
echo -e "\033[40;32m********************************\033[0m"

cd ../commsEncrypt;
testRet;
make clean;
testRet;
make;
testRet;
echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*完成commsEncrypt              *\033[0m"
echo -e "\033[40;32m********************************\033[0m"

cd ../business;
testRet;
make clean;
testRet;
make;
testRet;
echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*完成emv smc vikang qpboc      *\033[0m"
echo -e "\033[40;32m********************************\033[0m"


cd ../rel;
testRet;
for f in *.so.*
do
	cp $f ../../rel/lib/
done


cd ../../;
testRet
cp ./readme ./rel/daemon/
testRet;


#############################################################################

echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*开始编译进程                  *\033[0m"
echo -e "\033[40;32m********************************\033[0m"

cd $DM_DIR;
testRet;

cd qLcdDaemon;
./qlcddaemon_rel.sh;
testRet;
echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*完成qlcddaemon              *\033[0m"
echo -e "\033[40;32m********************************\033[0m"
cd ..;

cd dialer;
testRet;
make clean;
testRet;
make;
testRet;
echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*完成dialer                    *\033[0m"
echo -e "\033[40;32m********************************\033[0m"

cd ../updateService;
testRet;
make clean;
testRet;
make;
testRet;
echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*完成updateService             *\033[0m"
echo -e "\033[40;32m********************************\033[0m"


cd ../linuxpos;
testRet;
make clean;
testRet;
make;
testRet;
echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*完成linuxpos                  *\033[0m"
echo -e "\033[40;32m********************************\033[0m"

cd ../cln;
testRet;
make clean;
testRet;
make;
testRet;
echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*完成cln                       *\033[0m"
echo -e "\033[40;32m********************************\033[0m"

cd ../sysSetting;
./syssetting_rel.sh;
testRet;
echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*完成sysSetting                *\033[0m"
echo -e "\033[40;32m********************************\033[0m"

cd ../devTest;
./devtest_rel.sh;
testRet;
echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*完成Test                      *\033[0m"
echo -e "\033[40;32m********************************\033[0m"

cd ../sysConfigFile;
cp -rf setting.ini ../../rel/daemon/
testRet;
echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*完成setting.ini               *\033[0m"
echo -e "\033[40;32m********************************\033[0m"

cd ../rel;
testRet;
cp -rf ./00001/ ../../rel/app/
testRet;
cp -rf ./00002/ ../../rel/app/
testRet;
cp -rf ./image/	../../rel/daemon/
testRet;
cp -rf ./qss/	../../rel/daemon/
testRet;


for f in *.dm.*
do
	cp $f ../../rel/daemon/
done
testRet;






echo -e "\033[40;32m********************************\033[0m"
echo -e "\033[40;32m*                              *\033[0m"
echo -e "\033[40;32m*                              *\033[0m"
echo -e "\033[40;32m*                              *\033[0m"
echo -e "\033[40;32m*                              *\033[0m"
echo -e "\033[40;32m*            完成！            *\033[0m"
echo -e "\033[40;32m*                              *\033[0m"
echo -e "\033[40;32m*                              *\033[0m"
echo -e "\033[40;32m*                              *\033[0m"
echo -e "\033[40;32m*                              *\033[0m"
echo -e "\033[40;32m********************************\033[0m"
