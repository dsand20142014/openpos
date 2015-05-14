;
	data[2] = g;
	data[3] = h;
	data[4] = i;
	data[5] = j;
	data[6] = k;
	data[7] = l;

	retval = cam->ops->transferCmd(cam->lowlevel_data, cmd, data);
	if (retval)
		DBG("%x - failed\n", command);

	return retval;
}

/**********************************************************************
 *
 * Colorspace conversion
 *
 **********************************************************************/
#define LIMIT(x) ((((x)>0xffffff)?0xff0000:(((x)<=0xffff)?0:(x)&0xff0000))>>16)

static int convert420(unsigned char *yuv, unsigned char *rgb, int out_fmt,
		      int linesize, int mmap_kludge)
{
	int y, u, v, r, g, b, y1;

	/* Odd lines use the same u and v as the previous line.
	 