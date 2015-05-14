V1087版本在之前基础上修改
1.因CSF文件中添加了write register命令，可以解除key-select soft lock， RTC counter soft lock，在dryice.c中不需要soft POR，
删除soft POR。
2.dryice第一次上电clock和temperature的tamper detect功能开启后会马上置位，需要在non-valid的模式下降状态标识清除，删除rtc-imxdi.c
中清除non-valod标识的功能，或则不能清除相关的tamper flag，在dryice中清除tamper位后，在清除non-valid的flag，后续正常开机相关状态位不会再置位。
3.清状态标识留有应用API的接口，原则上只能清除一次的动作，后续正常工作后不能再清除标识，否则设备异常的状态会被清掉。