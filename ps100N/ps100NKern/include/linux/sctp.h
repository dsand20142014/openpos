r = 104635 * v;
		g = -25690 * u + -53294 * v;
		b = 132278 * u;
		break;
	default:
		y = *yuv++;
		u = *yuv++;
		y1 = *yuv++;
		v = *yuv;
		/* Just to avoid compiler warnings */
		r = 0;
		g = 0;
		b = 0;
		break;
	}
	switch(out_fmt) {
	case VIDEO_PALETTE_RGB555:
		*rgb++ = ((LIMIT(g+y) & 0xf8) << 2) | (LIMIT(b+y) >> 3);
		*rgb++ = ((LIMIT(r+y) & 0xf8) >> 1) | (LIMIT(g+y) >> 6);
		*rgb++ = ((LIMIT(g+y1) & 0xf8) << 2) | (LIMIT(b+y1) >> 3);
		*rgb = ((LIMIT(r+y1) & 0xf8) >> 1) | (LIMIT(g+y1) >> 6);
		return 4;
	case VIDEO_PALETTE_RGB565:
		*rgb++ = ((LIMIT(g+y) & 0xfc) << 3) | (LIMIT(b+y) >> 3);
		*rgb++ = (LIMIT(r+y) & 0xf8) | (LIMIT(g+y) >> 5);
		*rgb++ = ((LIMIT(g+y1) & 0xfc) << 3) | (LIMIT(b+y1) >> 3);
		*rgb = (LIMIT(r+y1) & 0xf8) | (LIMIT(g+y1) >> 5);
		return 4;
	case VIDEO_PALETTE_RGB24:
		if (mmap_kludge) {
			*rgb++ = LIMIT(b+y);
			*rgb++ = LIMIT(g+y);
			*rgb++ = LIMIT(r+y);
			*rgb++ = LIMIT(b+y1);
			*rgb++ = LIMIT(g+y1);
			*rgb = LIMIT(r+y1);
		} else {
			*rgb++ = LIMIT(r+y);
			*rgb++ = LIMIT(g+y);
			*rgb++ = LIMIT(b+y);
			*rgb++ = LIMIT(r+y1);
			*rgb++ = LIMIT(g+y1);
			*rgb = LIMIT(b+y1);
		}
		return 6;
	case VIDEO_PALETTE_RGB32:
		if (mmap_kludge) {
			*rgb++ = LIMIT(b+y);
			*rgb++ = LIMIT(g+y);
			*rgb++ = LIMIT(r+y);
			rgb++;
			*rgb++ = LIMIT(b+y1);
			*rgb++ = LIMIT(g+y1);
			*rgb = LIMIT(r+y1);
		} else {
			*rgb++ = LIMIT(r+y);
			*rgb++ = LIMIT(g+y);
			*rgb++ = LIMIT(b+y);
			rgb++;
			*rgb++ = LIMIT(r+y1);
			*rgb++ = LIMIT(g+y1);
			*rgb = LIMIT(b+y1);
		}
		return 8;
	case VIDEO_PALETTE_GREY:
		*rgb++ = y;
		*rgb = y1;
		return 2;
	case VIDEO_PALETTE_YUV422:
	case VIDEO_PALETTE_YUYV:
		*rgb++ = y;
		*rgb++ = u;
		*rgb++ = y1;
		*rgb = v;
		return 4;
	case VIDEO_PALETTE_UYVY:
		*rgb++ = u;
		*rgb++ = y;
		*rgb++ = v;
		*rgb = y1;
		return 4;
	default:
		DBG("Empty: %d\n", out_fmt);
		return 0;
	}
}

static int skipcount(int count, int fmt)
{
	switch(fmt) {
	case VIDEO_PALETTE_GREY:
		return count;
	case VIDEO_PALETTE_RGB555:
	case VIDEO_PALETTE_RGB565:
	case VIDEO_PALETTE_YUV422:
	case VIDEO_PALETTE_YUYV:
	case VIDEO_PALETTE_UYVY:
		return 2*count;
	case VIDEO_PALETTE_RGB24:
		return 3*count;
	case VIDEO_PALETTE_RGB32:
		return 4*count;
	default:
		return 0;
	}
}

static int parse_picture(struct cam_data *cam, int size)
{
	u8 *obuf, *ibuf, *end_obuf;
	int ll, in_uyvy, compressed, decimation, even_line, origsize, out_fmt;
	int rows, cols, linesize, subsample_422;

	/* make sure params don't change while we are decoding */
	mutex_lock(&cam->param_lock);

	obuf = cam->decompressed_frame.data;
	end_obuf = obuf+CPIA_MAX_FRAME_SIZE;
	ibuf = cam->raw_image;
	origsize = size;
	out_fmt = cam->vp.palette;

	if ((ibuf[0] != MAGIC_0) || (ibuf[1] != MAGIC_1)) {
		LOG("header not found\n");
		mutex_unlock(&cam->param_lock);
		return -1;
	}

	if ((ibuf[16] != VIDEOSIZE_QCIF) && (ibuf[16] != VIDEOSIZE_CIF)) {
		LOG("wrong video size\n");
		mutex_unlock(&cam->param_lock);
		return -1;
	}

	if (ibuf[17] != SUBSAMPLE_420 && ibuf[17] != SUBSAMPLE_422) {
		LOG("illegal subtype %d\n",ibuf[17]);
		mutex_unlock(&cam->param_lock);
		return -1;
	}
	subsample_422 = ibuf[17] == SUBSAMPLE_422;

	if (ibuf[18] != YUVORDER_YUYV && ibuf[18] != YUVORDER_UYVY) {
		LOG("illegal yuvorder %d\n",ibuf[18]);
		mutex_unlock(&cam->param_lock);
		return -1;
	}
	in_uyvy = ibuf[18] == YUVORDER_UYVY;

	if ((ibuf[24] != cam->params.roi.colStart) ||
	    (ibuf[25] != cam->params.roi.colEnd) ||
	    (ibuf[26] != cam->params.roi.rowStart) ||
	    (ibuf[27] != cam->params.roi.rowEnd)) {
		LOG("ROI mismatch\n");
		mutex_unlock(&cam->param_lock);
		return -1;
	}
	cols = 8*(ibuf[25] - ibuf[24]);
	rows = 4*(ibuf[27] - ibuf[26]);


	if ((ibuf[28] != NOT_COMPRESSED) && (ibuf[28] != COMPRESSED)) {
		LOG("illegal compression %d\n",ibuf[28]);
		mutex_unlock(&cam->param_lock);
		return -1;
	}
	compressed = (ibuf[28] == COMPRESSED);

	if (ibuf[29] != NO_DECIMATION && ibuf[29] != DECIMATION_ENAB) {
		LOG("illegal decimation %d\n",ibuf[29]);
		mutex_unlock(&cam->param_lock);
		return -1;
	}
	decimation = (ibuf[29] == DECIMATION_ENAB);

	cam->params.yuvThreshold.yThreshold = ibuf[30];
	cam->params.yuvThreshold.uvThreshold = ibuf[31];
	cam->params.status.systemState = ibuf[32];
	cam->params.status.grabState = ibuf[33];
	cam->params.status.streamState = ibuf[34];
	cam->params.status.fatalError = ibuf[35];
	cam->params.status.cmdError = ibuf[36];
	cam->params.status.debugFlags = ibuf[37];
	cam->params.status.vpStatus = ibuf[38];
	cam->params.status.errorCode = ibuf[39];
	cam->fps = ibuf[41];
	mutex_unlock(&cam->param_lock);

	linesize = skipcount(cols, out_fmt);
	ibuf += FRAME_HEADER_SIZE;
	size -= FRAME_HEADER_SIZE;
	ll = ibuf[0] | (ibuf[1] << 8);
	ibuf += 2;
	even_line = 1;

	while (size > 0) {
		size -= (ll+2);
		if (size < 0) {
			LOG("Insufficient data in buffer\n");
			return -1;
		}

		while (ll > 1) {
			if (!compressed || (compressed && !(*ibuf & 1))) {
				if(subsample_422 || even_line) {
				obuf += yuvconvert(ibuf, obuf, out_fmt,
						   in_uyvy, cam->mmap_kludge);
				ibuf += 4;
				ll -= 4;
			} else {
					/* SUBSAMPLE_420 on an odd line */
					obuf += convert420(ibuf, obuf,
							   out_fmt, linesize,
							   cam->mmap_kludge);
					ibuf += 2;
					ll -= 2;
				}
			} else {
				/*skip compressed interval from previous frame*/
				obuf += skipcount(*ibuf >> 1, out_fmt);
				if (obuf > end_obuf) {
					LOG("Insufficient buffer size\n");
					return -1;
				}
				++ibuf;
				ll--;
			}
		}
		if (ll == 1) {
			if (*ibuf != EOL) {
				DBG("EOL not found giving up after %d/%d"
				    " bytes\n", origsize-size, origsize);
				return -1;
			}

			++ibuf; /* skip over EOL */

			if ((size > 3) && (ibuf[0] == EOI) && (ibuf[1] == EOI) &&
			   (ibuf[2] == EOI) && (ibuf[3] == EOI)) {
				size -= 4;
				break;
			}

			if(decimation) {
				/* skip the odd lines for now */
				obuf += linesize;
			}

			if (size > 1) {
				ll = ibuf[0] | (ibuf[1] << 8);
				ibuf += 2; /* skip over line length */
			}
			if(!decimation)
				even_line = !even_line;
		} else {
			LOG("line length was not 1 but %d after %d/%d bytes\n",
			    ll, origsize-size, origsize);
			return -1;
		}
	}

	if(decimation) {
		/* interpolate odd rows */
		int i, j;
		u8 *prev, *next;
		prev = cam->decompressed_frame.data;
		obuf = prev+linesize;
		next = obuf+linesize;
		for(i=1; i<rows-1; i+=2) {
			for(j=0; j<linesize; ++j) {
				*obuf++ = ((int)*prev++ + *next++) / 2;
			}
			prev += linesize;
			obuf += linesize;
			next += linesize;
		}
		/* last row is odd, just copy previous row */
		memcpy(obuf, prev, linesize);
	}

	cam->decompressed_frame.count = obuf-cam->decompressed_frame.data;

	return cam->decompressed_frame.count;
}

/* InitStreamCap wrapper to select correct start line */
static inline int init_stream_cap(struct cam_data *cam)
{
	return do_command(cam, CPIA_COMMAND_InitStreamCap,
			  0, cam->params.streamStartLine, 0, 0);
}


/*  find_over_exposure
 *    Finds a suitable value of OverExposure for use with SetFlickerCtrl
 *    Some calculation is required because this value changes with the brightness
 *    set with SetColourParameters
 *
 *  Parameters: Brightness  -  last brightness value set with SetColourParameters
 *
 *  Returns: OverExposure value to use with SetFlickerCtrl
 */
#define FLICKER_MAX_EXPOSURE                    250
#define FLICKER_ALLOWABLE_OVER_EXPOSURE         146
#define FLICKER_BRIGHTNESS_CONSTANT             59
static int find_over_exposure(int brightness)
{
	int MaxAllowableOverExposure, OverExposure;

	MaxAllowableOverExposure = FLICKER_MAX_EXPOSURE - brightness -
				   FLICKER_BRIGHTNESS_CONSTANT;

	if (MaxAllowableOverExposure < FLICKER_ALLOWABLE_OVER_EXPOSURE) {
		OverExposure = MaxAllowableOverExposure;
	} else {
		OverExposure = FLICKER_ALLOWABLE_OVER_EXPOSURE;
	}

	return OverExposure;
}
#undef FLICKER_MAX_EXPOSURE
#undef FLICKER_ALLOWABLE_OVER_EXPOSURE
#undef FLICKER_BRIGHTNESS_CONSTANT

/* update various camera modes and settings */
static void dispatch_commands(struct cam_data *cam)
{
	mutex_lock(&cam->param_lock);
	if (cam->cmd_queue==COMMAND_NONE) {
		mutex_unlock(&cam->param_lock);
		return;
	}
	DEB_BYTE(cam->cmd_queue);
	DEB_BYTE(cam->cmd_queue>>8);
	if (cam->cmd_queue & COMMAND_SETFORMAT) {
		do_command(cam, CPIA_COMMAND_SetFormat,
			   cam->params.format.videoSize,
			   cam->params.format.subSample,
			   cam->params.format.yuvOrder, 0);
		do_command(cam, CPIA_COMMAND_SetROI,
			   cam->params.roi.colStart, cam->params.roi.colEnd,
			   cam->params.roi.rowStart, cam->params.roi.rowEnd);
		cam->first_frame = 1;
	}

	if (cam->cmd_queue & COMMAND_SETCOLOURPARAMS)
		do_command(cam, CPIA_COMMAND_SetColourParams,
			   cam->params.colourParams.brightness,
			   cam->params.colourParams.contrast,
			   cam->params.colourParams.saturation, 0);

	if (cam->cmd_queue & COMMAND_SETAPCOR)
		do_command(cam, CPIA_COMMAND_SetApcor,
			   cam->params.apcor.gain1,
			   cam->params.apcor.gain2,
			   cam->params.apcor.gain4,
			   cam->params.apcor.gain8);

	if (cam->cmd_queue & COMMAND_SETVLOFFSET)
		do_command(cam, CPIA_COMMAND_SetVLOffset,
			   cam->params.vlOffset.gain1,
			   cam->params.vlOffset.gain2,
			   cam->params.vlOffset.gain4,
			   cam->params.vlOffset.gain8);

	if (cam->cmd_queue & COMMAND_SETEXPOSURE) {
		do_command_extended(cam, CPIA_COMMAND_SetExposure,
				    cam->params.exposure.gainMode,
				    1,
				    cam->params.exposure.compMode,
				    cam->params.exposure.centreWeight,
				    cam->params.exposure.gain,
				    cam->params.exposure.fineExp,
				    cam->params.exposure.coarseExpLo,
				    cam->params.exposure.coarseExpHi,
				    cam->params.exposure.redComp,
				    cam->params.exposure.green1Comp,
				    cam->params.exposure.green2Comp,
				    cam->params.exposure.blueComp);
		if(cam->params.exposure.expMode != 1) {
			do_command_extended(cam, CPIA_COMMAND_SetExposure,
					    0,
					    cam->params.exposure.expMode,
					    0, 0,
					    cam->params.exposure.gain,
					    cam->params.exposure.fineExp,
					    cam->params.exposure.coarseExpLo,
					    cam->params.exposure.coarseExpHi,
					    0, 0, 0, 0);
		}
	}

	if (cam->cmd_queue & COMMAND_SETCOLOURBALANCE) {
		if (cam->params.colourBalance.balanceMode == 1) {
			do_command(cam, CPIA_COMMAND_SetColourBalance,
				   1,
				   cam->params.colourBalance.redGain,
				   cam->params.colourBalance.greenGain,
				   cam->params.colourBalance.blueGain);
			do_command(cam, CPIA_COMMAND_SetColourBalance,
				   3, 0, 0, 0);
		}
		if (cam->params.colourBalance.balanceMode == 2) {
			do_command(cam, CPIA_COMMAND_SetColourBalance,
				   2, 0, 0, 0);
		}
		if (cam->params.colourBalance.balanceMode == 3) {
			do_command(cam, CPIA_COMMAND_SetColourBalance,
				   3, 0, 0, 0);
		}
	}

	if (cam->cmd_queue & COMMAND_SETCOMPRESSIONTARGET)
		do_command(cam, CPIA_COMMAND_SetCompressionTarget,
			   cam->params.compressionTarget.frTargeting,
			   cam->params.compressionTarget.targetFR,
			   cam->params.compressionTarget.targetQ, 0);

	if (cam->cmd_queue & COMMAND_SETYUVTHRESH)
		do_command(cam, CPIA_COMMAND_SetYUVThresh,
			   cam->params.yuvThreshold.yThreshold,
			   cam->params.yuvThreshold.uvThreshold, 0, 0);

	if (cam->cmd_queue & COMMAND_SETCOMPRESSIONPARAMS)
		do_command_extended(cam, CPIA_COMMAND_SetCompressionParams,
			    0, 0, 0, 0,
			    cam->params.compressionParams.hysteresis,
			    cam->params.compressionParams.threshMax,
			    cam->params.compressionParams.smallStep,
			    cam->params.compressionParams.largeStep,
			    cam->params.compressionParams.decimationHysteresis,
			    cam->params.compressionParams.frDiffStepThresh,
			    cam->params.compressionParams.qDiffStepThresh,
			    cam->params.compressionParams.decimationThreshMod);

	if (cam->cmd_queue & COMMAND_SETCOMPRESSION)
		do_command(cam, CPIA_COMMAND_SetCompression,
			   cam->params.compression.mode,
			   cam->params.compression.decimation, 0, 0);

	if (cam->cmd_queue & COMMAND_SETSENSORFPS)
		do_command(cam, CPIA_COMMAND_SetSensorFPS,
			   cam->params.sensorFps.divisor,
			   cam->params.sensorFps.baserate, 0, 0);

	if (cam->cmd_queue & COMMAND_SETFLICKERCTRL)
		do_command(cam, CPIA_COMMAND_SetFlickerCtrl,
			   cam->params.flickerControl.flickerMode,
			   cam->params.flickerControl.coarseJump,
			   abs(cam->params.flickerControl.allowableOverExposure),
			   0);

	if (cam->cmd_queue & COMMAND_SETECPTIMING)
		do_command(cam, CPIA_COMMAND_SetECPTiming,
			   cam->params.ecpTiming, 0, 0, 0);

	if (cam->cmd_queue & COMMAND_PAUSE)
		do_command(cam, CPIA_COMMAND_EndStreamCap, 0, 0, 0, 0);

	if (cam->cmd_queue & COMMAND_RESUME)
		init_stream_cap(cam);

	if (cam->cmd_queue & COMMAND_SETLIGHTS && cam->params.qx3.qx3_detected)
	  {
	    int p1 = (cam->params.qx3.bottomlight == 0) << 1;
	    int p2 = (cam->params.qx3.toplight == 0) << 3;
	    do_command(cam, CPIA_COMMAND_WriteVCReg,  0x90, 0x8F, 0x50, 0);
	    do_command(cam, CPIA_COMMAND_WriteMCPort, 2, 0, (p1|p2|0xE0), 0);
	  }

	cam->cmd_queue = COMMAND_NONE;
	mutex_unlock(&cam->param_lock);
	return;
}



static void set_flicker(struct cam_params *params, volatile u32 *command_flags,
			int on)
{
	/* Everything in here is from the Windows driver */
#define FIRMWARE_VERSION(x,y) (params->version.firmwareVersion == (x) && \
			       params->version.firmwareRevision == (y))
/* define for compgain calculation */
#if 0
#define COMPGAIN(base, curexp, newexp) \
    (u8) ((((float) base - 128.0) * ((float) curexp / (float) newexp)) + 128.5)
#define EXP_FROM_COMP(basecomp, curcomp, curexp) \
    (u16)((float)curexp * (float)(u8)(curcomp + 128) / (float)(u8)(basecomp - 128))
#else
  /* equivalent functions without floating point math */
#define COMPGAIN(base, curexp, newexp) \
    (u8)(128 + (((u32)(2*(base-128)*curexp + newexp)) / (2* newexp)) )
#define EXP_FROM_COMP(basecomp, curcomp, curexp) \
     (u16)(((u32)(curexp * (u8)(curcomp + 128)) / (u8)(basecomp - 128)))
#endif


	int currentexp = params->exposure.coarseExpLo +
			 params->exposure.coarseExpHi*256;
	int startexp;
	if (on) {
		int cj = params->flickerControl.coarseJump;
		params->flickerControl.flickerMode = 1;
		params->flickerControl.disabled = 0;
		if(params->exposure.expMode != 2)
			*command_flags |= COMMAND_SETEXPOSURE;
		params->exposure.expMode = 2;
		currentexp = currentexp << params->exposure.gain;
		params->exposure.gain = 0;
		/* round down current exposure to nearest value */
		startexp = (currentexp + ROUND_UP_EXP_FOR_FLICKER) / cj;
		if(startexp < 1)
			startexp = 1;
		startexp = (startexp * cj) - 1;
		if(FIRMWARE_VERSION(1,2))
			while(startexp > MAX_EXP_102)
				startexp -= cj;
		else
			while(startexp > MAX_EXP)
				startexp -= cj;
		params->exposure.coarseExpLo = startexp & 0xff;
		params->exposure.coarseExpHi = startexp >> 8;
		if (currentexp > startexp) {
			if (currentexp > (2 * startexp))
				currentexp = 2 * startexp;
			params->exposure.redComp = COMPGAIN (COMP_RED, currentexp, startexp);
			params->exposure.green1Comp = COMPGAIN (COMP_GREEN1, currentexp, startexp);
			params->exposure.green2Comp = COMPGAIN (COMP_GREEN2, currentexp, startexp);
			params->exposure.blueComp = COMPGAIN (COMP_BLUE, currentexp, startexp);
		} else {
			params->exposure.redComp = COMP_RED;
			params->exposure.green1Comp = COMP_GREEN1;
			params->exposure.green2Comp = COMP_GREEN2;
			params->exposure.blueComp = COMP_BLUE;
		}
		if(FIRMWARE_VERSION(1,2))
			params->exposure.compMode = 0;
		else
			params->exposure.compMode = 1;

		params->apcor.gain1 = 0x18;
		params->apcor.gain2 = 0x18;
		params->apcor.gain4 = 0x16;
		params->apcor.gain8 = 0x14;
		*command_flags |= COMMAND_SETAPCOR;
	} else {
		params->flickerControl.flickerMode = 0;
		params->flickerControl.disabled = 1;
		/* Coarse = average of equivalent coarse for each comp channel */
		startexp = EXP_FROM_COMP(COMP_RED, params->exposure.redComp, currentexp);
		startexp += EXP_FROM_COMP(COMP_GREEN1, params->exposure.green1Comp, currentexp);
		startexp += EXP_FROM_COMP(COMP_GREEN2, params->exposure.green2Comp, currentexp);
		startexp += EXP_FROM_COMP(COMP_BLUE, params->exposure.blueComp, currentexp);
		startexp = startexp >> 2;
		while(startexp > MAX_EXP &&
		      params->exposure.gain < params->exposure.gainMode-1) {
			startexp = startexp >> 1;
			++params->exposure.gain;
		}
		if(FIRMWARE_VERSION(1,2) && startexp > MAX_EXP_102)
			startexp = MAX_EXP_102;
		if(startexp > MAX_EXP)
			startexp = MAX_EXP;
		params->exposure.coarseExpLo = startexp&0xff;
		params->exposure.coarseExpHi = startexp >> 8;
		params->exposure.redComp = COMP_RED;
		params->exposure.green1Comp = COMP_GREEN1;
		params->exposure.green2Comp = COMP_GREEN2;
		params->exposure.blueComp = COMP_BLUE;
		params->exposure.compMode = 1;
		*command_flags |= COMMAND_SETEXPOSURE;
		params->apcor.gain1 = 0x18;
		params->apcor.gain2 = 0x16;
		params->apcor.gain4 = 0x24;
		params->apcor.gain8 = 0x34;
		*command_flags |= COMMAND_SETAPCOR;
	}
	params->vlOffset.gain1 = 20;
	params->vlOffset.gain2 = 24;
	params->vlOffset.gain4 = 26;
	params->vlOffset.gain8 = 26;
	*command_flags |= COMMAND_SETVLOFFSET;
#undef FIRMWARE_VERSION
#undef EXP_FROM_COMP
#undef COMPGAIN
}

#define FIRMWARE_VERSION(x,y) (cam->params.version.firmwareVersion == (x) && \
			       cam->params.version.firmwareRevision == (y))
/* monitor the exposure and adjust the sensor frame rate if needed */
static void monitor_exposure(struct cam_data *cam)
{
	u8 exp_acc, bcomp, gain, coarseL, cmd[8], data[8];
	int retval, light_exp, dark_exp, very_dark_exp;
	int old_exposure, new_exposure, framerate;

	/* get necessary stats and register settings from camera */
	/* do_command can't handle this, so do it ourselves */
	cmd[0] = CPIA_COMMAND_ReadVPRegs>>8;
	cmd[1] = CPIA_COMMAND_ReadVPRegs&0xff;
	cmd[2] = 30;
	cmd[3] = 4;
	cmd[4] = 9;
	cmd[5] = 8;
	cmd[6] = 8;
	cmd[7] = 0;
	retval = cam->ops->transferCmd(cam->lowlevel_data, cmd, data);
	if (retval) {
		LOG("ReadVPRegs(30,4,9,8) - failed, retval=%d\n",
		    retval);
		return;
	}
	exp_acc = data[0];
	bcomp = data[1];
	gain = data[2];
	coarseL = data[3];

	mutex_lock(&cam->param_lock);
	light_exp = cam->params.colourParams.brightness +
		    TC - 50 + EXP_ACC_LIGHT;
	if(light_exp > 255)
		light_exp = 255;
	dark_exp = cam->params.colourParams.brightness +
		   TC - 50 - EXP_ACC_DARK;
	if(dark_exp < 0)
		dark_exp = 0;
	very_dark_exp = dark_exp/2;

	old_exposure = cam->params.exposure.coarseExpHi * 256 +
		       cam->params.exposure.coarseExpLo;

	if(!cam->params.flickerControl.disabled) {
		/* Flicker control on */
		int max_comp = FIRMWARE_VERSION(1,2) ? MAX_COMP : HIGH_COMP_102;
		bcomp += 128;	/* decode */
		if(bcomp >= max_comp && exp_acc < dark_exp) {
			/* dark */
			if(exp_acc < very_dark_exp) {
				/* very dark */
				if(cam->exposure_status == EXPOSURE_VERY_DARK)
					++cam->exposure_count;
				else {
					cam->exposure_status = EXPOSURE_VERY_DARK;
					cam->exposure_count = 1;
				}
			} else {
				/* just dark */
				if(cam->exposure_status == EXPOSURE_DARK)
					++cam->exposure_count;
				else {
					cam->exposure_status = EXPOSURE_DARK;
					cam->exposure_count = 1;
				}
			}
		} else if(old_exposure <= LOW_EXP || exp_acc > light_exp) {
			/* light */
			if(old_exposure <= VERY_LOW_EXP) {
				/* very light */
				if(cam->exposure_status == EXPOSURE_VERY_LIGHT)
					++cam->exposure_count;
				else {
					cam->exposure_status = EXPOSURE_VERY_LIGHT;
					cam->exposure_count = 1;
				}
			} else {
				/* just light */
				if(cam->exposure_status == EXPOSURE_LIGHT)
					++cam->exposure_count;
				else {
					cam->exposure_status = EXPOSURE_LIGHT;
					cam->exposure_count = 1;
				}
			}
		} else {
			/* not dark or light */
			cam->exposure_status = EXPOSURE_NORMAL;
		}
	} else {
		/* Flicker control off */
		if(old_exposure >= MAX_EXP && exp_acc < dark_exp) {
			/* dark */
			if(exp_acc < very_dark_exp) {
				/* very dark */
				if(cam->exposure_status == EXPOSURE_VERY_DARK)
					++cam->exposure_count;
				else {
					cam->exposure_status = EXPOSURE_VERY_DARK;
					cam->exposure_count = 1;
				}
			} else {
				/* just dark */
				if(cam->exposure_status == EXPOSURE_DARK)
					++cam->exposure_count;
				else {
					cam->exposure_status = EXPOSURE_DARK;
					cam->exposure_count = 1;
				}
			}
		} else if(old_exposure <= LOW_EXP || exp_acc > light_exp) {
			/* light */
			if(old_exposure <= VERY_LOW_EXP) {
				/* very light */
				if(cam->exposure_status == EXPOSURE_VERY_LIGHT)
					++cam->exposure_count;
				else {
					cam->exposure_status = EXPOSURE_VERY_LIGHT;
					cam->exposure_count = 1;
				}
			} else {
				/* just light */
				if(cam->exposure_status == EXPOSURE_LIGHT)
					++cam->exposure_count;
				else {
					cam->exposure_status = EXPOSURE_LIGHT;
					cam->exposure_count = 1;
				}
			}
		} else {
			/* not dark or light */
			cam->exposure_status = EXPOSURE_NORMAL;
		}
	}

	framerate = cam->fps;
	if(framerate > 30 || framerate < 1)
		framerate = 1;

	if(!cam->params.flickerControl.disabled) {
		/* Flicker control on */
		if((cam->exposure_status == EXPOSURE_VERY_DARK ||
		    cam->exposure_status == EXPOSURE_DARK) &&
		   cam->exposure_count >= DARK_TIME*framerate &&
		   cam->params.sensorFps.divisor < 3) {

			/* dark for too long */
			++cam->params.sensorFps.divisor;
			cam->cmd_queue |= COMMAND_SETSENSORFPS;

			cam->params.flickerControl.coarseJump =
				flicker_jumps[cam->mainsFreq]
					     [cam->params.sensorFps.baserate]
					     [cam->params.sensorFps.divisor];
			cam->cmd_queue |= COMMAND_SETFLICKERCTRL;

			new_exposure = cam->params.flickerControl.coarseJump-1;
			while(new_exposure < old_exposure/2)
				new_exposure += cam->params.flickerControl.coarseJump;
			cam->params.exposure.coarseExpLo = new_exposure & 0xff;
			cam->params.exposure.coarseExpHi = new_exposure >> 8;
			cam->cmd_queue |= COMMAND_SETEXPOSURE;
			cam->exposure_status = EXPOSURE_NORMAL;
			LOG("Automatically decreasing sensor_fps\n");

		} else if((cam->exposure_status == EXPOSURE_VERY_LIGHT ||
		    cam->exposure_status == EXPOSURE_LIGHT) &&
		   cam->exposure_count >= LIGHT_TIME*framerate &&
		   cam->params.sensorFps.divisor > 0) {

			/* light for too long */
			int max_exp = FIRMWARE_VERSION(1,2) ? MAX_EXP_102 : MAX_EXP ;

			--cam->params.sensorFps.divisor;
			cam->cmd_queue |= COMMAND_SETSENSORFPS;

			cam->params.flickerControl.coarseJump =
				flicker_jumps[cam->mainsFreq]
					     [cam->params.sensorFps.baserate]
					     [cam->params.sensorFps.divisor];
			cam->cmd_queue |= COMMAND_SETFLICKERCTRL;

			new_exposure = cam->params.flickerControl.coarseJump-1;
			while(new_exposure < 2*old_exposure &&
			      new_exposure+
			      cam->params.flickerControl.coarseJump < max_exp)
				new_exposure += cam->params.flickerControl.coarseJump;
			cam->params.exposure.coarseExpLo = new_exposure & 0xff;
			cam->params.exposure.coarseExpHi = new_exposure >> 8;
			cam->cmd_queue |= COMMAND_SETEXPOSURE;
			cam->exposure_status = EXPOSURE_NORMAL;
			LOG("Automatically increasing sensor_fps\n");
		}
	} else {
		/* Flicker control off */
		if((cam->exposure_status == EXPOSURE_VERY_DARK 