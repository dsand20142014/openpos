EINTR;

	save_camera_state(cam);

	return 0;
}

static void get_version_information(struct cam_data *cam)
{
	/* GetCPIAVersion */
	do_command(cam, CPIA_COMMAND_GetCPIAVersion, 0, 0, 0, 0);

	/* GetPnPID */
	do_command(cam, CPIA_COMMAND_GetPnPID, 0, 0, 0, 0);
}

/* initialize camera */
static int reset_camera(struct cam_data *cam)
{
	int err;
	/* Start the camera in low power mode */
	if (goto_low_power(cam)) {
		if (cam->params.status.systemState != WARM_BOOT_STATE)
			return -ENODEV;

		/* FIXME: this is just dirty trial and error */
		err = goto_high_power(cam);
		if(err)
			return err;
		do_command(cam, CPIA_COMMAND_DiscardFrame, 0, 0, 0, 0);
		if (goto_low_power(cam))
			return -ENODEV;
	}

	/* procedure described i