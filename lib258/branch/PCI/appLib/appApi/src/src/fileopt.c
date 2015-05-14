/*******************************************************************************
*功能：文件操作函数功能
*author: zhangyong
*date:
* 说明：现规定文件全部存放到/mnt/pos目录下，按照各个应用程序编号创建子目录，如下图所示
 		/mnt/pos/appA/file1
                      file2
                      file3

				 appB/file1
                      file2
                      file3

                 ...........

* TODO: 增加对多应用的支持，即appA/B... 为应用程序编号

* init:2009-03-23
* modified :2010-04-26.liuguoming
			added reset_last_error()
            buglist No xxxx
            Reset the last error number before file operation function
*******************************************************************************/
#ifndef __LUXAPP_FILEOPS__
#define __LUXAPP_FILEOPS__

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ftw.h>
#include <string.h>
#include <errno.h>

#include "osapp.h"
#include "fileopt.h"
//#include "memwatch.h"

//#define EN_DBG

#ifdef EN_DBG
#define dbg Uart_Printf
#else
#define dbg
#endif

#define printf1 //Uart_Printf
#define DBGPress //Uart_Printf

unsigned short ucOSCurrAppID=12;

unsigned long FILE_TOTALSIZE=20*1024*1024; //配置、交易等文件的总大小，目前为1M
unsigned long FILE_TOTALSIZE_MAX = 20*1024*1024;

//extern 	int main_get_app_id(void);
unsigned char ucFileLastErr; //错误返回码

static void reset_last_error(void);
static void fd2pathname(int fd,char *pathname);



unsigned char Os__saved_swap (unsigned char PageNb)
{
}

/*
作用：拷贝数据到应用指定的数据页。
函数：void Os__saved_copy (unsigned char *ptSource, unsigned char *ptDest, unsigned short Length);
等待：是
说明： 指针'ptSource' 指向的数据源按长度'Length' 存储到指针'ptDest'指向的地址。
注意：原数据必须是EXTERN DATA或程序代码。
返回值：无
*/
void Os__saved_copy (unsigned char *ptSource, unsigned char *ptDest, unsigned short Length)
{
    memcpy( ptDest, ptSource, Length);
}

/*
作用：初始化数据存储区 SRAM.
函数： void Os__saved_set (unsigned char *ptDest, unsigned char Charac, unsigned short Length);
等待：是
说明：初始化数据存储区, 以开始地址 'ptDest' 按长度 'Length' 初始化为0x00.
返回值：无
*/
void Os__saved_set (unsigned char *ptDest, unsigned char Charac,  unsigned short Length)
{
    memset(ptDest, Charac, Length);
}



/*
return 0 文件存在
	   -1 文件不存在
*/
int CheckFileExist(char *FileName)
{
    int status;

    status = access(FileName,0);
    if (status == 0)
        dbg("%s File exists\n", FileName);
    else
        dbg("%s File doesn't exist\n", FileName);

    return status;
}

/*
判断文件夹是否存在，如不存在则创建文件夹
*/
int CheckFoldExist(char *foldname)
{
    DIR *foldfd;
    int ret;

    foldfd = opendir(foldname);
    if (foldfd == NULL)
    {
        Uart_Printf("foldfd=%x fold not exist\n", foldfd);
        ret=mkdir(foldname, 0666);
        return -1;
    }
    /*zy 0721
    * 未关闭导致大量的fd产生，达到进程限制的1024个就会致后继的文件无法打开
    */
    closedir(foldfd);
    if (ret<0)
        ucFileLastErr = FILESYSERR_NOFS;
    return 0;
}




/*根据杉德文件系统标志进行组合*/
int FileMode_to_ANSI(unsigned char mode)
{
    unsigned int  new_mode = 0;

    if (mode&O_READ)
		new_mode |= O_RDONLY;
	
    if (mode&O_WRITE)
		new_mode |= O_RDWR;
	
    if (mode&O_TRUNC)
		new_mode |= (O_RDWR|O_TRUNC);
	
    if (mode&O_CREATE)
		new_mode |= (O_RDWR|O_CREAT);

    return new_mode;
}

/*
*注意：
*	pcFileName[0]='/' 表示为OS底层配置文件
*	pcFileName[0]！='/' 表示为APP配置文件,此时需要增加应用号到路径中
*
*
*/
int FileName2FilePath(char *pcFileName, char *realPath)
{
    char folds[FILEFS_FILENAMELEN];
    int ret;

    if (!realPath)
        return -1;

    memset(folds, 0, sizeof(folds));
    memset(realPath, 0, sizeof(realPath));

    if (pcFileName[0]=='/')
        memcpy(realPath, pcFileName, strlen(pcFileName));
    else
    {
#if 0 //zy 0717    	
        ucOSCurrAppID = main_get_app_id();
#endif

        sprintf(folds, "%s/", OSFS_FLASHPREFIX);
        ret = CheckFoldExist(folds);
        
        sprintf(realPath, "%s/%s", OSFS_FLASHPREFIX, pcFileName);
    }


    dbg("%s pcFileName=%s realpath=%s\n", __func__, pcFileName, realPath);
}


int OSAPP_OpenFile(char *pcFileName,unsigned char ucMode)
{
    int fd, ret;
    char RealFileName[FILEFS_FILENAMELEN+strlen(OSFS_FLASHPREFIX)];
    int newMode;

    reset_last_error();

    if ( strlen(pcFileName) > FILEFS_FILENAMELEN || 0==strlen(pcFileName))
	{
            ucFileLastErr = FILESYSERR_FILENAME;
            return(-1);
    }

    //ret=CheckFoldExist("/mnt");
    //ret=CheckFoldExist("/mnt/pos");
    //ret=CheckFoldExist(OSFS_FLASHPREFIX);

    //判断并创建文件夹
    
    ret = FileName2FilePath(pcFileName, RealFileName);
    dbg("%s realfilename=%s\n", __func__, RealFileName);

    newMode=FileMode_to_ANSI(ucMode);
	newMode |= O_SYNC;
	umask(0);

	if (newMode&O_CREAT)
		fd=open(RealFileName, newMode,0666);
	else 
		fd=open(RealFileName, newMode);

    if (fd < 0)
    {
        ucFileLastErr = FILESYSERR_FILENOTOPEN;
        goto out;
    }

    ucFileLastErr = FILESYSERR_SUCCESS;

    return fd;

out:
    Uart_Printf("%s errno=%d\n", __func__, errno);
    return -1;
}

int OSAPP_OpenFileEx(char *pcFileName,unsigned char ucMode,unsigned char *pucAttr)
{
    reset_last_error();

    if ( strlen(pcFileName) >FILEFS_FILENAMELEN )
    {
        ucFileLastErr = FILESYSERR_FILENAME;
        return(-1);
    }

    ZY_TODO("not implement OSAPP_OpenFileEx\n");
}

int OSAPP_FileRead(int iHandle,unsigned char *pucOutData,unsigned int uiOutLen)
{
    int ret;

    reset_last_error();
	
re_read:
    ret = read(iHandle, pucOutData, uiOutLen);
	if (ret<0){
		if (errno == EINTR)
			goto re_read;
		else{
			ucFileLastErr = FILESYSERR_READ;
			return ret;
		}
	}
    return ret;
}

int OSAPP_FileWrite(int iHandle,unsigned char *pucInData,unsigned int uiInLen)
{
    int ret=0;

	reset_last_error();

    //if (OSAPP_GetUnusedFlashSize()<uiInLen)
    //{
    //    ucFileLastErr = FILESYSERR_NOSPACE;
    //    return -1;
    //}
	
re_write:
    ret = write(iHandle, pucInData, uiInLen);
	if (ret < 0){
		if (errno == EINTR)
			goto re_write;
		else{
			fprintf(stderr,"write:%s\n",strerror(errno));
			ucFileLastErr = FILESYSERR_WRITE_2;
			return ret;
		}
	}
    if (ret != uiInLen)
        ucFileLastErr = FILESYSERR_WRITE;
    return ret;
}


int OSAPP_FileClose(int iHandle)
{
    int ret;

    //reset_last_error();
    ret=close(iHandle);
    return ret;

}

int OSAPP_FileSeek(int iHandle,long lOffset,unsigned char ucLocation)
{
    int ret;
    struct stat st;

    /* 2010-04-29 ,liuguoming */
    reset_last_error();

    if (fstat(iHandle,&st)<0)
    {
        ucFileLastErr = FILESYSERR_GENERAL;// last error
        goto fail;//for fail
    }
    if (ucLocation == 0 && lOffset > st.st_size)
    {
        perror("over file length \n");
        ucFileLastErr = FILESYSERR_BADOFFSET;// last error
        goto fail;;//for fail
    }
    ret=lseek(iHandle, lOffset, ucLocation);

    if (ret == -1)
    {
        perror("OSAPP_FileSeek:\n");
        ucFileLastErr = FILESYSERR_BADOFFSET;// last error
        goto fail;;//for fail
    }
    return 0;// for success
fail:
    return 1;
}

/*
	when will return negative?
	when file isnot exist!
*/
long int OSAPP_FileSize(char *pcFileName)
{
    long int ret_len;
    int ret;
    struct stat tmp;
    char RealFileName[FILEFS_FILENAMELEN+strlen(OSFS_FLASHPREFIX)];
    char acFileName[FILEFS_FILENAMELEN+1];

    reset_last_error();


    if ( strlen(pcFileName) > FILEFS_FILENAMELEN )
    {
        ucFileLastErr = FILESYSERR_FILENAME;
        return(-1);
    }


    memset(RealFileName, 0x0, sizeof(RealFileName));
    sprintf(RealFileName, "%s/%s",OSFS_FLASHPREFIX, pcFileName);


    ret=CheckFileExist(RealFileName);
    if (ret==-1 )
    {
        ucFileLastErr = FILESYSERR_INVALIDPARAM;
        Uart_Printf("Warning: file not exist, api will create(%s, filename=%s)\n", __func__, RealFileName);
        return -1;	// file does not exist!
    }

    ret=stat(RealFileName, &tmp);
    if (S_ISREG(tmp.st_mode)) ret_len=tmp.st_size;
    dbg("%s size %d\n", RealFileName, tmp.st_size);


    return ret_len;
}

int OSAPP_FileDelete(char *pcFileName)
{
    int ret, len;
    char *fname;
    char RealFileName[FILEFS_FILENAMELEN*2];

    reset_last_error();

    if ( strlen(pcFileName) > FILEFS_FILENAMELEN )
    {
        ucFileLastErr = FILESYSERR_FILENAME;
        return(-1);
    }

    memset(RealFileName, 0x0, sizeof(RealFileName));
    /* 2010-04-27 */
    sprintf(RealFileName, "%s/", OSFS_FLASHPREFIX);
    //memcpy(RealFileName, OSFS_FLASHPREFIX, strlen(OSFS_FLASHPREFIX));
    /* end */
    strcat(RealFileName, pcFileName);
    /*
    len=sizeof(RealFileName);
    fname=malloc(len);
    memset(fname, 0x0, len);

    strcpy(fname, "rm -rf ");
    strcat(fname, RealFileName);
    ret=system(fname);
    */
    ret = unlink(RealFileName);
    system("sync");
    dbg("cmd: %s\n", fname);
    //free(fname);
    return ret;
}

#if 0
/*
输入路径 TODO
输出OSApp的路径及
*/
int OSAPP_FileList(char *path)
{
    ftw(path, fn, 2);
}
#endif

int OSAPP_FileTrunc(int iHandle,unsigned int uiLen)
{
    char *buf0, *buf1, cmd[300], filepath[FILEFS_FILENAMELEN];
    unsigned int RealLen=0, i=0;
    int c, ret=1;

    reset_last_error();

    memset(cmd, 0, sizeof(cmd));
    memset(filepath, 0, sizeof(filepath));

    sprintf(cmd, "%s%d", "/proc/self/fd/", iHandle);
    dbg("cmd=%s\n", cmd);
    readlink(cmd, filepath, FILEFS_FILENAMELEN);
    dbg("filepath=%s\n", filepath);

    truncate(filepath, uiLen);

    return 0;
}



long int filesize_sum=0, filenums=0;

int fn(const char *file, const struct stat *sb, int flag)
{
    struct stat tmp;

    if (flag==FTW_D) dbg("%s\n", file);
    else  if (flag == FTW_F)
    {
        dbg("%s\n", file);
        filenums++;
        stat(file, &tmp);
        if (S_ISREG(tmp.st_mode)) filesize_sum += tmp.st_size;
    }

    return 0;
}


/*
获取全部已用空间的大小，
新增函数
*/
unsigned long  get_used_filesize(void)
{
    unsigned long size;

    filenums=0;
    filesize_sum=0;
    ftw(OSFS_FLASHPREFIX, fn, 2);
    dbg("whole size=%ld, file num=%d\n", filesize_sum, filenums);

    return filesize_sum;
}

/**
*新增加函数
*获取全部剩余空间的大小
*/
unsigned long OSAPP_GetUnusedFlashSize(void)
{
    unsigned long remain;

    reset_last_error();
    remain = FILE_TOTALSIZE - get_used_filesize();
    dbg("remain FILE_TOTALSIZE=%ld size=%ld\n", FILE_TOTALSIZE, remain);
    return remain;
}

unsigned long  OSAPP_FreeSpace(void)
{
    return OSAPP_GetUnusedFlashSize();
}

/*
新增加函数
设置应用程序数据的空间大小
TODO:此参数应该为全局变量，保存到系统配置中
*/
char OSAPP_SetFlashSize(long int size)
{
    if (size > FILE_TOTALSIZE_MAX) size = FILE_TOTALSIZE_MAX;
    FILE_TOTALSIZE = size;
    ucFileLastErr=FILESYSERR_SUCCESS;
    return 0;
}

/*
新增加函数
获取应用程序数据的空间大小

*/
long int  OSAPP_GetFlashSize(void)
{
    return FILE_TOTALSIZE;
}


unsigned char OSAPP_FileGetLastError(void)
{
    unsigned char err = ucFileLastErr ;
    ucFileLastErr = 0;
    return err;
}

static void reset_last_error(void)
{
    ucFileLastErr =0;
}

static void fd2pathname(int fd,char *pathname)
{
    char cmd[100];

    sprintf(cmd, "%s%d", "/proc/self/fd/", fd);
    readlink(cmd, pathname, sizeof(pathname));
}


int fileops_main(char argc, char **argv)
{
    int ret, fd, uiDataSize, times=1;
    char *pData="1234567890";
    uiDataSize = strlen(pData);
#if 0
    OSAPP_GetUnusedFlashSize();
    OSAPP_SetFlashSize(FILE_TOTALSIZE*2-12345);
    OSAPP_GetUnusedFlashSize();
#endif
    if (argc>1)
        times= atoi(argv[1]);

    fd=OSAPP_OpenFile("mytest", O_CREATE);

    while (times--)
    {
again:
        fd=OSAPP_OpenFile("mytest", O_WRITE);
        if (fd<0)
        {
            Uart_Printf("open file failed!\n");
            return -1;
        }
        //fd=OSAPP_OpenFile("/mnt/pos/constfile", O_CREATE);
        ret = OSAPP_FileWrite(fd, pData, strlen(pData));
        if (ret<0)
        {
            Uart_Printf("write failed!\n");
            return -1;
        }
//        OSAPP_FileSeek(fd, 0, SEEK_SET);
//        OSAPP_FileWrite(fd, (unsigned char*)pData, uiDataSize);
        Uart_Printf("remain space: %ul\n", OSAPP_FreeSpace());

        //  OSAPP_FileDelete("mytest");
    }
    return 0;
}


#endif
