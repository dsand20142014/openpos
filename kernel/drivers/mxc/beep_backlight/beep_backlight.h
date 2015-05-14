#ifndef __BEEP_BACKLIGHT_H_
#define __BEEP_BACKLIGHT_H_

#define DEVICE_NAME     "beep_backlight"
#define PLATFORM_NAME   "linux-beep_backlight"

#define MAG_MAJOR 0



#define BEEP_GATE_OPEN              1
#define BEEP_GATE_CLOSE             0

#define BACKLIGHT_GATE_OPEN         1
#define BACKLIGHT_GATE_CLOSE        0


#define BEEP_BACKLIGHT_IOC_NUM   'K'

#define BEEP_ON              _IO(BEEP_BACKLIGHT_IOC_NUM, 1)
#define BEEP_OFF             _IO(BEEP_BACKLIGHT_IOC_NUM, 2)


//背光亮度调节，0~255
#define BACKLIGHT_SET        _IOW(BEEP_BACKLIGHT_IOC_NUM, 3, int)

//按键音开关 1：开按键音；0：关按键音
#define BEEP_GATE            _IOW(BEEP_BACKLIGHT_IOC_NUM, 4, int)

//LCD背光开关 1：开背光； 0：关背光
#define BACKLIGHT_GATE       _IOW(BEEP_BACKLIGHT_IOC_NUM, 5, int)

//需要关闭电源的当前驱动模块
#define DRIVER_USED         _IOW(BEEP_BACKLIGHT_IOC_NUM, 6, int)


#define BEEP_BACKLIGHT_IOC_MAXNUM   10

#endif

