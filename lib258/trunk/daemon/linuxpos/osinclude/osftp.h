#ifndef _FTP_H_
#define _FTP_H_
enum {
  WAIT_FOR_READ = 1,
  WAIT_FOR_WRITE = 2
};


#define FTP_TIME_READ	900

#define RE_USER_OK		331
#define RE_PASS_OK		230
#define RE_SIZE_OK		213
#define RE_PASV_OK		227
#define RE_RETR_OK		150
#define RE_STOR_OK		150
#define RE_LIST_OK		150
#define RE_CWD_OK		250
#define RE_PWD_OK		257

#define IN_CMD_LEN		64	

#define ACCOUNT_LEN		32
#define PASSWD_LEN		32
	
#define FTP_MAX_RETRY	3

#define FTP_CMD_NULL	0xff
#define FTP_CMD_BYE		0x00
#define FTP_CMD_HELP	0x01
#define FTP_CMD_LIST	0x02
#define FTP_CMD_GET		0x03
#define FTP_CMD_PUT		0x04
#define FTP_CMD_CD		0x05
#define FTP_CMD_PWD		0x06


#endif
