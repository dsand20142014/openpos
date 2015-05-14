

test and setting 
write by huangdanli
2013-1-22

已改待验证
1.camera增加图片查看功能
2.ethernet增加提示信息
3.keyboard test的清除键现已正常显示
4.GSM设置保存一直报错Set failed现已正常
5.GSM设置页面选项可以修改，改好按ENTER键就OK
6.DATE/TIME显示已居中
7.签名退出崩溃已修正
8.GPRS设置DEFAULT BAND后现已正常，不会有段错误出现
9.WIFI连接成功会保存路由器参数
10.增加测试和设置的结果显示，如还有必要请详细说明，这边再加
11.GSM测试显示已改

正在解决中
1.密钥清除动态完成了，但静态API还没封好
2.BEEP测试界面，请外包专业人员再设计下吧
3.WIFI测试界面显示的IP地址还在修改中
4.SIGN IN效果差还在改进中
5.磁条卡要能反复测试，因为OS_WAITEVENT等待刷卡时按键无反应，这边界面不好处理，修改中


modified by lichao at 2013.2.1
1.在gui/include 路径下增加sysparam.h头文件，提供新的系统参数读取更改API。
2.注释掉SMART_NewReset函数的调用，解决上次丹荔提交不完全导致的编译通不过问题。
3.将屏幕背光亮度测试中老的API函数更改为新的API函数。