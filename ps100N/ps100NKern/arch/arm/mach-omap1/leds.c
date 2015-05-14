 *pic = arg;
		DBG("VIDIOCGPICT\n");
		*pic = cam->vp;
		break;
	}

	case VIDIOCSPICT:
	{
		struct video_picture *vp = arg;

		DBG("VIDIOCSPICT\n");

		/* check validity */
		DBG("palette: %d\n", vp->palette);
		DBG("depth: %d\n", vp->depth);
		if (!valid_mode(vp->palette, vp->depth)) {
			retval = -EINVAL;
			break;
		}

		mutex_lock(&cam->param_lock);
		/* brightness, colour, contrast need no check 0-65535 */
		cam->vp = *vp;
		/* update cam->params.colourParams */
		cam->params.colourParams.brightness = vp->brightness*100/65535;
		cam->params.colourParams.contrast = vp->contrast*100/65535;
		cam->params.colourParams.saturation = vp->colour*100/65535;
		/* contrast is in steps of 8, so round */
		cam->params.colourParams.contrast =
			((cam->params.colourParams.contrast + 3) / 8) * 8;
		if (cam->params.version.firmwareVersion == 1 &&
		    cam->params.version.firmwareRevision == 2 &&
		    cam->params.colourParams.contrast > 80) {
			/* 1-02 firmware limits contrast to 80 */
			cam->params.colourParams.contrast = 80;
		}

		/* Adjust flicker control if necessary */
		if(cam->params.flickerControl.allowableOverExposure < 0)
			cam->params.flickerControl.allowableOverExposure =
				-find_over_exposure(cam->params.colourParams.brightness);
		if(cam->params.flickerControl.flickerMode != 0)
			cam->cmd_queue |= COMMAND_SETFLICKERCTRL;


		/* queue command to update camera */
		cam->cmd_queue |= COMMAND_SETCOLOURPARAMS;
		mutex_unlock(&cam->