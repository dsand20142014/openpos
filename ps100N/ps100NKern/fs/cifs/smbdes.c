= val;
				else
					retval = -EINVAL;
			}
			command_flags |= COMMAND_SETCOLOURPARAMS;
		} else if (MATCH("sensor_fps")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				/* find values so that sensorFPS is minimized,
				 * but >= val */
				if (val > 30)
					retval = -EINVAL;
				else if (val > 25) {
					new_params.sensorFps.divisor = 0;
					new_params.sensorFps.baserate = 1;
				} else if (val > 15) {
					new_params.sensorFps.divisor = 0;
					new_params.sensorFps.baserate = 0;
				} else if (val > 12) {
					new_params.sensorFps.divisor = 1;
					new_params.sensorFps.baserate = 1;
				} else if (val > 7) {
					new_params.sensorFps.divisor = 1;
					new_params.sensorFps.baserate = 0;
				} else if (val > 6) {
					new_params.sensorFps.divisor = 2;
					new_params.sensorFps.baserate = 1;
				} else if (val > 3) {
					new_params.sensorFps.divisor = 2;
					new_params.sensorFps.baserate = 0;
				} else {
					new_params.sensorFps.divisor = 3;
					/* Either base rate would work here */
					new_params.sensorFps.baserate = 1;
				}
				new_params.flickerControl.coarseJump =
					flicker_jumps[new_mains]
					[new_params.sensorFps.baserate]
					[new_params.sensorFps.divisor];
				if (new_params.flickerControl.flickerMode)
					command_flags |= COMMAND_SETFLICKERCTRL;
			}
			command_flags |= COMMAND_SETSENSORFPS;
			cam->exposure_status = EXPOSURE_NORMAL;
		} else if (MATCH("stream_start_line")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				int max_line = 288;

				if (new_params.format.videoSize == VIDEOSIZE_QCIF)
					max_line = 144;
				if (val <= max_line)
					new_params.streamStartLine = val/2;
				else
					retval = -EINVAL;
			}
		} else if (MATCH("sub_sample")) {
			if (!retval && MATCH("420"))
				new_params.format.subSample = SUBSAMPLE_420;
			else if (!retval && MATCH("422"))
				new_params.format.subSample = SUBSAMPLE_422;
			else
				retval = -EINVAL;

			command_flags |= COMMAND_SETFORMAT;
		} else if (MATCH("yuv_order")) {
			if (!retval && MATCH("YUYV"))
				new_params.format.yuvOrder = YUVORDER_YUYV;
			else if (!retval && MATCH("UYVY"))
				new_params.format.yuvOrder = YUVORDER_UYVY;
			else
				retval = -EINVAL;

			command_flags |= COMMAND_SETFORMAT;
		} else if (MATCH("ecp_timing")) {
			if (!retval && MATCH("normal"))
				new_params.ecpTiming = 0;
			else if (!retval && MATCH("slow"))
				new_params.ecpTiming = 1;
			else
				retval = -EINVAL;

			command_flags |= COMMAND_SETECPTIMING;
		} else if (MATCH("color_balance_mode")) {
			if (!retval && MATCH("manual"))
				new_params.colourBalance.balanceMode = 3;
			else if (!retval && MATCH("auto"))
				new_params.colourBalance.balanceMode = 2;
			else
				retval = -EINVAL;

			command_flags |= COMMAND_SETCOLOURBALANCE;
		} else if (MATCH("red_gain")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				if (val <= 212) {
					new_params.colourBalance.redGain = val;
					new_params.colourBalance.balanceMode = 1;
				} else
					retval = -EINVAL;
			}
			command_flags |= COMMAND_SETCOLOURBALANCE;
		} else if (MATCH("green_gain")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				if (val <= 212) {
					new_params.colourBalance.greenGain = val;
					new_params.colourBalance.balanceMode = 1;
				} else
					retval = -EINVAL;
			}
			command_flags |= COMMAND_SETCOLOURBALANCE;
		} else if (MATCH("blue_gain")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				if (val <= 212) {
					new_params.colourBalance.blueGain = val;
					new_params.colourBalance.balanceMode = 1;
				} else
					retval = -EINVAL;
			}
			command_flags |= COMMAND_SETCOLOURBALANCE;
		} else if (MATCH("max_gain")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				/* 1-02 firmware limits gain to 2 */
				if (FIRMWARE_VERSION(1,2) && val > 2)
					val = 2;
				switch(val) {
				case 1:
					new_params.exposure.gainMode = 1;
					break;
				case 2:
					new_params.exposure.gainMode = 2;
					break;
				case 4:
					new_params.exposure.gainMode = 3;
					break;
				case 8:
					new_params.exposure.gainMode = 4;
					break;
				default:
					retval = -EINVAL;
					break;
				}
			}
			command_flags |= COMMAND_SETEXPOSURE;
		} else if (MATCH("exposure_mode")) {
			if (!retval && MATCH("auto"))
				new_params.exposure.expMode = 2;
			else if (!retval && MATCH("manual")) {
				if (new_params.exposure.expMode == 2)
					new_params.exposure.expMode = 3;
				if(new_params.flickerControl.flickerMode != 0)
					command_flags |= COMMAND_SETFLICKERCTRL;
				new_params.flickerControl.flickerMode = 0;
			} else
				retval = -EINVAL;

			command_flags |= COMMAND_SETEXPOSURE;
		} else if (MATCH("centre_weight")) {
			if (!retval && MATCH("on"))
				new_params.exposure.centreWeight = 1;
			else if (!retval && MATCH("off"))
				new_params.exposure.centreWeight = 2;
			else
				retval = -EINVAL;

			command_flags |= COMMAND_SETEXPOSURE;
		} else if (MATCH("gain")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				switch(val) {
				case 1:
					new_params.exposure.gain = 0;
					break;
				case 2:
					new_params.exposure.gain = 1;
					break;
				case 4:
					new_params.exposure.gain = 2;
					break;
				case 8:
					new_params.exposure.gain = 3;
					break;
				default:
					retval = -EINVAL;
					break;
				}
				new_params.exposure.expMode = 1;
				if(new_params.flickerControl.flickerMode != 0)
					command_flags |= COMMAND_SETFLICKERCTRL;
				new_params.flickerControl.flickerMode = 0;
				command_flags |= COMMAND_SETEXPOSURE;
				if (new_params.exposure.gain >
				    new_params.exposure.gainMode-1)
					retval = -EINVAL;
			}
		} else if (MATCH("fine_exp")) {
			if (!retval)
				val = VALUE/2;

			if (!retval) {
				if (val < 256) {
					/* 1-02 firmware limits fineExp/2 to 127*/
					if (FIRMWARE_VERSION(1,2) && val > 127)
						val = 127;
					new_params.exposure.fineExp = val;
					new_params.exposure.expMode = 1;
					command_flags |= COMMAND_SETEXPOSURE;
					if(new_params.flickerControl.flickerMode != 0)
						command_flags |= COMMAND_SETFLICKERCTRL;
					new_params.flickerControl.flickerMode = 0;
					command_flags |= COMMAND_SETFLICKERCTRL;
				} else
					retval = -EINVAL;
			}
		} else if (MATCH("coarse_exp")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				if (val <= MAX_EXP) {
					if (FIRMWARE_VERSION(1,2) &&
					    val > MAX_EXP_102)
						val = MAX_EXP_102;
					new_params.exposure.coarseExpLo =
						val & 0xff;
					new_params.exposure.coarseExpHi =
						val >> 8;
					new_params.exposure.expMode = 1;
					command_flags |= COMMAND_SETEXPOSURE;
					if(new_params.flickerControl.flickerMode != 0)
						command_flags |= COMMAND_SETFLICKERCTRL;
					new_params.flickerControl.flickerMode = 0;
					command_flags |= COMMAND_SETFLICKERCTRL;
				} else
					retval = -EINVAL;
			}
		} else if (MATCH("red_comp")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				if (val >= COMP_RED && val <= 255) {
					new_params.exposure.redComp = val;
					new_params.exposure.compMode = 1;
					command_flags |= COMMAND_SETEXPOSURE;
				} else
					retval = -EINVAL;
			}
		} else if (MATCH("green1_comp")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				if (val >= COMP_GREEN1 && val <= 255) {
					new_params.exposure.green1Comp = val;
					new_params.exposure.compMode = 1;
					command_flags |= COMMAND_SETEXPOSURE;
				} else
					retval = -EINVAL;
			}
		} else if (MATCH("green2_comp")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				if (val >= COMP_GREEN2 && val <= 255) {
					new_params.exposure.green2Comp = val;
					new_params.exposure.compMode = 1;
					command_flags |= COMMAND_SETEXPOSURE;
				} else
					retval = -EINVAL;
			}
		} else if (MATCH("blue_comp")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				if (val >= COMP_BLUE && val <= 255) {
					new_params.exposure.blueComp = val;
					new_params.exposure.compMode = 1;
					command_flags |= COMMAND_SETEXPOSURE;
				} else
					retval = -EINVAL;
			}
		} else if (MATCH("apcor_gain1")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				command_flags |= COMMAND_SETAPCOR;
				if (val <= 0xff)
					new_params.apcor.gain1 = val;
				else
					retval = -EINVAL;
			}
		} else if (MATCH("apcor_gain2")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				command_flags |= COMMAND_SETAPCOR;
				if (val <= 0xff)
					new_params.apcor.gain2 = val;
				else
					retval = -EINVAL;
			}
		} else if (MATCH("apcor_gain4")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				command_flags |= COMMAND_SETAPCOR;
				if (val <= 0xff)
					new_params.apcor.gain4 = val;
				else
					retval = -EINVAL;
			}
		} else if (MATCH("apcor_gain8")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				command_flags |= COMMAND_SETAPCOR;
				if (val <= 0xff)
					new_params.apcor.gain8 = val;
				else
					retval = -EINVAL;
			}
		} else if (MATCH("vl_offset_gain1")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				if (val <= 0xff)
					new_params.vlOffset.gain1 = val;
				else
					retval = -EINVAL;
			}
			command_flags |= COMMAND_SETVLOFFSET;
		} else if (MATCH("vl_offset_gain2")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				if (val <= 0xff)
					new_params.vlOffset.gain2 = val;
				else
					retval = -EINVAL;
			}
			command_flags |= COMMAND_SETVLOFFSET;
		} else if (MATCH("vl_offset_gain4")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				if (val <= 0xff)
					new_params.vlOffset.gain4 = val;
				else
					retval = -EINVAL;
			}
			command_flags |= COMMAND_SETVLOFFSET;
		} else if (MATCH("vl_offset_gain8")) {
			if (!retval)
				val = VALUE;

			if (!retval) {
				if (val <= 0xff)
					new_params.vlOffset.gain8 = val;
				else
					retval = -EINVAL;
			}
			command_flags |= COMMAND_SETVLOFFSET;
		} else if (MATCH("flicker_control")) {
			if (!retval && MATCH("on")) {
				set_flicker(&new_params, &command_flags, 1);
			} else if (!retval && MATCH("off")) {
				set_flicker(&new_params, &command_flags, 0);
			} else
				retval = -EINVAL;

			command_flags |= COMMAND_SETFLICKERCTRL;
		} else if (MATCH("mains_frequency")) {
			if (!retval && MATCH("50")) {
				new_mains = 0;
				new_params.flickerControl.coarseJump =
					flicker_jumps[new_mains]
					[new_params.sensorFps.baserate]
					[new_params.sensorFps.divisor];
				i