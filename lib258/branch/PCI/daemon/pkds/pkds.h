#ifndef __PKDS_H_
#define __PKDS_H_


#ifdef __cplusplus
extern "C" {
#endif


#define RFID_IOCTL		0x01

#define RFID_READ		0x02

#define ICC_IOCTL		0x03

#define ICC_READ		0x04

#define WAIT_EVENT		0x05

#define WAIT_NOKEY		0x06


#define PID_VERIFIED	0x5AA5

#define APP_VERIFIED	0xA55A

#define FUNC_SUCCESS	0x5A5A


#ifdef __cplusplus
}
#endif

#endif
