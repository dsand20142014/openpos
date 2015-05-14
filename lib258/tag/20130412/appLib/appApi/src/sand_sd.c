#include <stdio.h>

#include <unistd.h>

/*
0³É¹¦£¬·Ç0Ê§°Ü
*/
int Os__check_tf_ready(void)
{
	int iret=-1;
	iret=access("/mnt/mmcblk0p1",F_OK);
	return iret;
}
