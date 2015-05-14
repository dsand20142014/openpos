#include "sand_mp3player.h"

static int dsp_fd;        

static enum mad_flow input(void *data, struct mad_stream *stream)   
{  
	struct buffer *buffer = data;   
	if (!buffer->length)   
		return MAD_FLOW_STOP;   
	mad_stream_buffer(stream, buffer->start, buffer->length);   
	buffer->length = 0;  
	return MAD_FLOW_CONTINUE;   
}   

static inline signed int scale(mad_fixed_t sample)   
{   
	sample += (1L << (MAD_F_FRACBITS - 16));   
	if (sample >= MAD_F_ONE)   
		sample = MAD_F_ONE - 1;   
	else if (sample < -MAD_F_ONE)   
		sample = -MAD_F_ONE;   
		
	return sample >> (MAD_F_FRACBITS + 1 - 16);   
}  

static enum mad_flow output(void *data,
				struct mad_header const *header, struct mad_pcm *pcm)   
{   
	unsigned int nchannels, nsamples, n;   
	mad_fixed_t const *left_ch, *right_ch;   
	unsigned char Output[4608], *OutputPtr;   
	int fmt, wrote, speed;   

	nchannels = pcm->channels;   
	n = nsamples = pcm->length;   
	left_ch = pcm->samples[0];   
	right_ch = pcm->samples[1];   
	 
	fmt = AFMT_S16_LE;   
	speed = pcm->samplerate; 
	ioctl(dsp_fd, SNDCTL_DSP_SPEED, &(speed));   
	ioctl(dsp_fd, SNDCTL_DSP_SETFMT, &fmt);   
	ioctl(dsp_fd, SNDCTL_DSP_CHANNELS, &(pcm->channels));   
	OutputPtr = Output;  
		
	while (nsamples--) {   
		signed int sample;   
		sample = scale(*left_ch++);   
		*(OutputPtr++) = sample >> 0;   
		*(OutputPtr++) = sample >> 8;   
		if (nchannels == 2) {   
			sample = scale(*right_ch++);   
			*(OutputPtr++) = sample >> 0;   
			*(OutputPtr++) = sample >> 8;   
		}   
	}  
	if(nchannels == 2)
		n *= 4;
	else
		n *= 2;
	OutputPtr = Output;   
	while (n) {   
		wrote = write(dsp_fd, OutputPtr, n);   
		OutputPtr += wrote;   
		n -= wrote;   
	}   
	OutputPtr = Output; 
		
	return MAD_FLOW_CONTINUE;   
}   
  
static enum mad_flow error(void *data,   
            struct mad_stream *stream, struct mad_frame *frame)   
{   
    return MAD_FLOW_CONTINUE;   
}   
  
static int decode(unsigned char const *start, unsigned long length)   
{   
	struct buffer buffer;   
	struct mad_decoder decoder;   
	int result;  

	buffer.start = start;   
	buffer.length = length;   
	mad_decoder_init(&decoder, &buffer, input, 0, 0, output, error, 0);   
	mad_decoder_options(&decoder, 0);   
	result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);   
	mad_decoder_finish(&decoder); 
		
	return result;   
}  

int audio_volume(int volume)
{
	int mix_fd,ret = -1;

	mix_fd = open(MIX_DEV,O_RDWR);
	if(mix_fd < 0)
		return ret;
	if(volume > 100)
		volume = 100;
	else if(volume < 0)
		volume = 0;
	ret = ioctl(mix_fd,SOUND_MIXER_WRITE_VOLUME,&volume);
	close(mix_fd);
	
	return ret;
	
}

int mp3_player(char * mp3file)   
{   
	struct stat stat;   
	void *fdm = NULL;   
	int fd;   
	 
	fd = open(mp3file, O_RDONLY);   
	if ((dsp_fd = open(SND_DEV, O_WRONLY)) < 0) {   
		printf("can not open device!!!\n");   
		return -1;   
	}   
	ioctl(dsp_fd, SNDCTL_DSP_SYNC, 0); 
	if (fstat(fd, &stat) == -1 || stat.st_size == 0){   
		close(dsp_fd);
		return -2;   
	}
	fdm = mmap(0, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);   
	if (fdm == MAP_FAILED){   
		close(dsp_fd);
		return -3;   
	}
	decode(fdm, stat.st_size);   
	if (munmap(fdm, stat.st_size) == -1){   
		close(dsp_fd);
		return -4;   
	}
	
	ioctl(dsp_fd, SNDCTL_DSP_RESET, 0);   
	close(dsp_fd); 
	
	return 0;   
} 

#if 0
int main(int argc,char *argv[])
{
	int ret,volume;
	char * file;
	if(argc !=3)
		return -1;
	volume = atoi(argv[2]);
	file = argv[1];
	ret = audio_volume(volume);
	if(ret < 0)
		return ret;
	ret = mp3_player(file);
	
	return ret;

}
#endif

