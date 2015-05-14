#ifndef _SAND_MP3PLAYER_
#define _SAND_MP3PLAYER_

#include <stdio.h>   
#include <stdlib.h>   
#include <unistd.h>   
#include <sys/stat.h>   
#include <sys/mman.h>   
#include <sys/soundcard.h>   
#include <sys/ioctl.h>   
#include <sys/fcntl.h>   
#include <sys/types.h>   
#include <mad.h>   

#define SND_DEV "/dev/dsp"
#define MIX_DEV "/dev/mixer"
struct buffer {   
	unsigned char const *start;   
	unsigned long length;   
}; 

int audio_volume(int volume);
int mp3_player(char * mp3file) ;


#endif
