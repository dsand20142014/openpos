
	 * hence subtract with 4.
	 */
	const u16 buff_offset = 4;

	/* Modem port dl configuration */
	dc->port[PORT_MDM].dl_addr[CH_A] = offset;
	dc->port[PORT_MDM].dl_addr[CH_B] =
				(offset += dc->config_table.dl_mdm_len1);
	dc->port[PORT_MDM].dl_size[CH_A] =
				dc->config_table.dl_mdm_len1 - buff_offset;
	dc->port[PORT_MDM].dl_size[CH_B] =
				dc->config_table.dl_mdm_len2 - buff_offset;

	/* Diag port dl configuration */
	dc->port[PORT_DIAG].dl_addr[CH_A] =
				(offset += dc->config_table.dl_mdm_len2);
	dc->port[PORT_DIAG].dl_size[CH_A] =
				dc->config_table.dl_diag_len1 - buff_offset;
	dc->port[PORT_DIAG].dl_addr[CH_B] =
				(offset += dc->config_table.dl_diag_len1);
	dc->port[PORT_DIAG].dl_size[CH_B] =
				dc->config_table.dl_diag_len2 - buff_offset;

	/* App1 port dl configuration */
	dc->port[PORT_APP1].dl_addr[CH_A] =
				(offset += dc->config_table.dl_diag_len2);
	dc->port[PORT_APP1].dl_size[CH_A] =
				dc->config_table.dl_app1_len - buff_offset;

	/* App2 port dl configuration */
	dc->port[PORT_APP2].dl_addr[CH_A] =
				(offset += dc->config_table.dl_app1_len);
	dc->port[PORT_APP2].dl_size[CH_A] =
				dc->config_table.dl_app2_len - buff_offset;

	/* Ctrl dl configuration */
	dc->port[PORT_CTRL].dl_addr[CH_A] =
				(offset += dc->config_table.dl_app2_len);
	dc->port[PORT_CTRL].dl_size[CH_A] =
				dc->config_table.dl_ctrl_len - buff_offset;

	offset = dc->base_addr + dc->config_table.ul_start;

	/* Modem Port ul configuration */
	dc->port[PORT_MDM].ul_addr[CH_A] = offset;
	dc->port[PORT_MDM].ul_size[CH_A] =
				dc->config_table.ul_mdm_len1 - buff_offset;
	dc->port[PORT_MDM].ul_addr[CH_B] =
				(offset += dc->config_table.ul_mdm_len1);
	dc->port[PORT_MDM].ul_size[CH_B] =
				dc->config_table.ul_mdm_len2 - buff_offset;

	/* Diag port ul configuration */
	dc->port[PORT_DIAG].ul_addr[CH_A] =
				(offset += dc->config_table.ul_mdm_len2);
	dc->port[PORT_DIAG].ul_size[CH_A] =
				dc->config_table.ul_diag_len - buff_offset;

	/* App1 port ul configuration */
	dc->port[PORT_APP1].ul_addr[CH_A] =
				(offset += dc->config_table.ul_diag_len);
	dc->port[PORT_APP1].ul_size[CH_A] =
				dc->config_table.ul_app1_len - buff_offset;

	/* App2 port ul configuration */
	dc->port[PORT_APP2].ul_addr[CH_A] =
				(offset += dc->config_table.ul_app1_len);
	dc->port[PORT_APP2].ul_size[CH_A] =
				dc->config_table.ul_app2_len - buff_offset;

	/* Ctrl ul configuration */
	dc->port[PORT_CTRL].ul_addr[CH_A] =
				(offset += dc->config_table.ul_app2_len);
	dc->port[PORT_CTRL].ul_size[CH_A] =
				dc->config_table.ul_ctrl_len - buff_offset;
}

/* Dump config table under initalization phase */
#ifdef DEBUG
static void dump_table(const struct nozomi *dc)
{
	DBG3("signature: 0x%08X", dc->config_table.signature);
	DBG3("version: 0x%04X", dc->config_table.version);
	DBG3("product_information: 0x%04X", \
				dc->config_table.product_information);
	DBG3("toggle enabled: %d", dc->config_table.toggle.enabled);
	DBG3("toggle up_mdm: %d", dc->config_table.toggle.mdm_ul);
	DBG3("toggle dl_mdm: %d", dc->config_table.toggle.mdm_dl);
	DBG3("toggle dl_dbg: %d", dc->config_table.toggle.diag_dl);

	DBG3("dl_start: 0x%04X", dc->config_table.dl_start);
	DBG3("dl_mdm_len0: 0x%04X, %d", dc->config_table.dl_mdm_len1,
	   dc->config_table.dl_mdm_len1);
	DBG3("dl_mdm_len1: 0x%04X, %d", dc->config_table.dl_mdm_len2,
	   dc->config_table.dl_mdm_len2);
	DBG3("dl_diag_len0: 0x%04X, %d", dc->config_table.dl_diag_len1,
	   dc->config_table.dl_diag_len1);
	DBG3("dl_diag_len1: 0x%04X, %d", dc->config_table.dl_diag_len2,
	   dc->config_table.dl_diag_len2);
	DBG3("dl_app1_len: 0x%04X, %d", dc->config_table.dl_app1_len,
	   dc->config_table.dl_app1_len);
	DBG3("dl_app2_len: 0x%04X, %d", dc->config_table.dl_app2_len,
	   dc->config_table.dl_app2_len);
	DBG3("dl_ctrl_len: 0x%04X, %d", dc->config_table.dl_ctrl_len,
	   dc->config_table.dl_ctrl_len);
	DBG3("ul_start: 0x%04X, %d", dc->config_table.ul_start,
	   dc->config_table.ul_start);
	DBG3("ul_mdm_len[0]: 0x%04X, %d", dc->config_table.ul_mdm_len1,
	   dc->config_table.ul_mdm_len1);
	DBG3("ul_mdm_len[1]: 0x%04X, %d", dc->config_tablface = usb_find_interface(&piusb_driver, subminor);
	if (!interface) {
		printk(KERN_ERR "%s - error, can't find device for minor %d\n",
		       __func__, subminor);
		retval = -ENODEV;
		goto exit_no_device;
	}

	pdx = usb_get_intfdata(interface);
	if (!pdx) {
		retval = -ENODEV;
		goto exit_no_device;
	}
	dbg("Alternate Setting = %d", interface->num_altsetting);

	pdx->bulk_in_size_returned = 0;
	pdx->bulk_in_byte_trk = 0;
	pdx->PixelUrb = NULL;
	pdx->frameIdx = 0;
	pdx->urbIdx = 0;
	pdx->maplist_numPagesMapped = NULL;
	pdx->userBufMapped = 0;
	pdx->sgl = NULL;
	pdx->sgEntries = NULL;
	pdx->gotPixelData = 0;
	pdx->pendingWrite = 0;
	pdx->pendedPixelUrbs = NULL;
	pdx->num_frames = 0;
	pdx->active_frame = 0;
	pdx->frameSize = 0;

	/* increment our usage count for the device */
	kref_get(&pdx->kref);

	/* save our object in the file's private structure */
	file->private_data = pdx;

exit_no_device:
	return retval;
}

static int piusb_release(struct inode *inode, struct file *file)
{
	struct device_extension *pdx;
	int retval = 0;

	dbg("Piusb_Release()");
	pdx = (struct device_extension *)file->private_data;
	if (pdx == NULL) {
		dbg("%s - object is NULL", __func__);
		retval = -ENODEV;
		goto object_null;
	}
	/* decrement the count on our device */
	kref_put(&pdx->kref, piusb_delete);

object_null:
	return retval;
}

static int pixis_io(struct ioctl_struct *ctrl, struct device_extension *pdx,
		struct ioctl_struct *arg)
{
	unsigned int numToRead = 0;
	unsigned int totalRead = 0;
	unsigned char *uBuf;
	int numbytes;
	int i;

	uBuf = kmalloc(ctrl->numbytes, GFP_KERNEL);
	if (!uBuf) {
		dbg("Alloc for uBuf failed");
		return 0;
	}
	numbytes = (int) ctrl->numbytes;
	numToRead = (unsigned int) ctrl->numbytes;
	dbg("numbytes to read = %d", numbytes);
	dbg("endpoint # %d", ctrl->endpoint);

	if (copy_from_user(uBuf, ctrl->pData, numbytes)) {
		dbg("copying ctrl->pData to dummyBuf failed");
		return -EFAULT;
	}

	do {
		i = usb_bulk_msg(pdx->udev, pdx->hEP[ctrl->endpoint],
				(uBuf + totalRead),
				/* EP0 can only handle 64 bytes at a time */
				(numToRead > 64) ? 64 : numToRead,
				&numbytes, HZ * 10);
		if (i) {
			dbg("CMD = %s, Address = 0x%02X",
					((uBuf[3] == 0x02) ? "WRITE" : "READ"),
					uBuf[1]);
			dbg("Number of bytes Attempted to read = %d",
					(int)ctrl->numbytes);
			dbg("Blocking ReadI/O Failed with status %d", i);
			kfree(uBuf);
			return -1;
		}
		dbg("Pixis EP0 Read %d bytes", numbytes);
		totalRead += numbytes;
		numToRead -= numbytes;
	} while (numToRead);

	memcpy(ctrl->pData, uBuf, totalRead);
	dbg("Total Bytes Read from PIXIS EP0 = %d", totalRead);
	ctrl->numbytes = totalRead;

	if (copy_to_user(arg, ctrl, sizeof(struct ioctl_struct)))
		dbg("copy_to_user failed in IORB");

	kfree(uBuf);
	return ctrl->numbytes;
}

static int pixel_data(struct ioctl_struct *ctrl, struct device_extension *pdx)
{
	int i;

	if (!pdx->gotPixelData)
		return 0;

	pdx->gotPixelData = 0;
	ctrl->numbytes = pdx->bulk_in_size_returned;
	pdx->bulk_in_size_returned -= pdx->frameSize;

	for (i = 0; i < pdx->maplist_numPagesMapped[pdx->active_frame]; i++)
		SetPageDirty(sg_page(&pdx->sgl[pdx->active_frame][i]));

	pdx->active_frame = ((pdx->active_frame + 1) % pdx->num_frames);

	return ctrl->numbytes;
}

/**
 *	piusb_ioctl
 */
static int piusb_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		       unsigned long arg)
{
	struct device_extension *pdx;
	char dummyCtlBuf[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned long devRB = 0;
	int err = 0;
	int retval = 0;
	struct ioctl_struct ctrl;
	unsigned short controlData = 0;

	pdx = (struct device_extension *)file->private_data;
	/* verify that the device wasn't unplugged */
	if (!pdx->present) {
		dbg("No Device Present\n");
		return -ENODEV;
	}
	/* fill in your device specific stuff here */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg,
				_IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user *)arg,
			       _IOC_SIZE(cmd));
	if (err) {
		dev_err(&pdx->udev->dev, "return with error = %d\n", err);
		return -EFAULT;
	}
	switch (cmd) {
	case PIUSB_GETVNDCMD:
		if (__copy_from_user
		    (&ctrl, (void __user *)arg, sizeof(struct ioctl_struct))) {
			dev_err(&pdx->udev->dev, "copy_from_user failed\n");
			return -EFAULT;
		}
		dbg("%s %x\n", "Get Vendor Command = ", ctrl.cmd);
		retval =
		    usb_control_msg(pdx->udev, usb_rcvctrlpipe(pdx->udev, 0),
				    ctrl.cmd, USB_DIR_IN, 0, 0, &devRB,
				    ctrl.numbytes, HZ * 10);
		if (ctrl.cmd == 0xF1) {
			dbg("FW Version returned from HW = %ld.%ld",
			    (devRB >> 8), (devRB & 0xFF));
		}
		if (retval >= 0)
			retval = (int)devRB;
		return retval;

	case PIUSB_SETVNDCMD:
		if (__copy_from_user
		    (&ctrl, (void __user *)arg, sizeof(struct ioctl_struct))) {
			dev_err(&pdx->udev->dev, "copy_from_user failed\n");
			return -EFAULT;
		}
		/* dbg( "%s %x", "Set Vendor Command = ",ctrl.cmd ); */
		controlData = ctrl.pData[0];
		controlData |= (ctrl.pData[1] << 8);
		/* dbg( "%s %d", "Vendor Data =",controlData ); */
		retval = usb_control_msg(pdx->udev,
				usb_sndctrlpipe(pdx->udev, 0),
				ctrl.cmd,
				(USB_DIR_OUT | USB_TYPE_VENDOR
				 /* | USB_RECIP_ENDPOINT */),
				controlData, 0,
				&dummyCtlBuf, ctrl.numbytes, HZ * 10);
		return retval;

	case PIUSB_ISHIGHSPEED:
		return ((pdx->udev->speed == USB_SPEED_HIGH) ? 1 : 0);

	case PIUSB_WRITEPIPE:
		if (__copy_from_user(&ctrl, (void __user *)arg, _IOC_SIZE(cmd))) {
			dev_err(&pdx->udev->dev,
					"copy_from_user WRITE_DUMMY failed\n");
			return -EFAULT;
		}
		if (!access_ok(VERIFY_READ, ctrl.pData, ctrl.numbytes)) {
			dbg("can't access pData");
			return 0;
		}
		piusb_output(&ctrl, ctrl.pData /* uBuf */, ctrl.numbytes, pdx);
		return ctrl.numbytes;

	case PIUSB_USERBUFFER:
		if (__copy_from_user
		    (&ctrl, (void __user *)arg, sizeof(struct ioctl_struct))) {
			dev_err(&pdx->udev->dev, "copy_from_user failed\n");
			return -EFAULT;
		}
		return MapUserBuffer((struct ioctl_struct *) &ctrl, pdx);

	case PIUSB_UNMAP_USERBUFFER:
		retval = UnMapUserBuffer(pdx);
		return retval;

	case PIUSB_READPIPE:
		if (__copy_from_user(&ctrl, (void __user *)arg,
					sizeof(struct ioctl_struct))) {
			dev_err(&pdx->udev->dev, "copy_from_user failed\n");
			return -EFAULT;
		}
		if (((0 == ctrl.endpoint) && (PIXIS_PID == pdx->iama)) ||
				(1 == ctrl.endpoint) ||	/* ST133IO */
				(4 == ctrl.endpoint))	/* PIXIS IO */
			return pixis_io(&ctrl, pdx,
					(struct ioctl_struct *)arg);
		else if ((0 == ctrl.endpoint) || /* ST133 Pixel Data */
				(2 == ctrl.endpoint) || /* PIXIS Ping */
				(3 == ctrl.endpoint))	/* PIXIS Pong */
			return pixel_data(&ctrl, pdx);

		break;

	case PIUSB_WHATCAMERA:
		return pdx->iama;

	case PIUSB_SETFRAMESIZE:
		dbg("PIUSB_SETFRAMESIZE");
		if (__copy_from_user
		    (&ctrl, (void __user *)arg, sizeof(struct ioctl_struct))) {
			dev_err(&pdx->udev->dev, "copy_from_user failed\n");
			return -EFAULT;
		}
		pdx->frameSize = ctrl.numbytes;
		pdx->num_frames = ctrl.numFrames;
		if (!pdx->sgl)
			pdx->sgl =
			    kmalloc(sizeof(struct scatterlist *) *
				    pdx->num_frames, GFP_KERNEL);
		if (!pdx->sgEntries)
			pdx->sgEntries =
			    kmalloc(sizeof(unsigned int) * pdx->num_frames,
				    GFP_KERNEL);
		if (!pdx->PixelUrb)
			pdx->PixelUrb =
			    kmalloc(sizeof(struct urb **) * pdx->num_frames,
				    GFP_KERNEL);
		if (!pdx->maplist_numPagesMapped)
			pdx->maplist_numPagesMapped =
			    vmalloc(sizeof(unsigned int) * pdx->num_frames);
		if (!pdx->pendedPixelUrbs)
			pdx->pendedPixelUrbs =
			    kmalloc(sizeof(char *) * pdx->num_frames,
				    GFP_KERNEL);
		return 0;

	default:
		dbg("%s\n", "No IOCTL found");
		break;

	}
	/* return that we did not understand this ioctl call */
	dbg("Returning -ENOTTY");
	return -ENOTTY;
}

static void piusb_write_bulk_callback(struct urb *urb)
{
	struct device_extension *pdx = urb->context;
	int status = urb->status;

	/* sync/async unlink faults aren't errors */
	if (status && !(status == -ENOENT || status == -ECONNRESET))
		dev_dbg(&urb->dev->dev,
			"%s - nonzero write bulk status received: %d",
			__func__, status);

	pdx->pendingWrite = 0ak;
	case CTRL_MDM:
		port = PORT_MDM;
		enable_ier = MDM_DL;
		break;
	case CTRL_DIAG:
		port = PORT_DIAG;
		enable_ier = DIAG_DL;
		break;
	case CTRL_APP1:
		port = PORT_APP1;
		enable_ier = APP1_DL;
		break;
	case CTRL_APP2:
		port = PORT_APP2;
		enable_ier = APP2_DL;
		if (dc->state == NOZOMI_STATE_ALLOCATED) {
			/*
			 * After card initialization the flow control
			 * received for APP2 is always the last
			 */
			dc->state = NOZOMI_STATE_READY;
			dev_info(&dc->pdev->dev, "Device READY!\n");
		}
		break;
	default:
		dev_err(&dc->pdev->dev,
			"ERROR: flow control received for non-existing port\n");
		return 0;
	};

	DBG1("0x%04X->0x%04X", *((u16 *)&dc->port[port].ctrl_dl),
	   *((u16 *)&ctrl_dl));

	old_ctrl = dc->port[port].ctrl_dl;
	dc->port[port].ctrl_dl = ctrl_dl;

	if (old_ctrl.CTS == 1 && ctrl_dl.CTS == 0) {
		DBG1("Disable interrupt (0x%04X) on port: %d",
			enable_ier, port);
		disable_transmit_ul(port, dc);

	} else if (old_ctrl.CTS == 0 && ctrl_dl.CTS == 1) {

		if (__kfifo_len(dc->port[port].fifo_ul)) {
			DBG1("Enable interrupt (0x%04X) on port: %d",
				enable_ier, port);
			DBG1("Data in buffer [%d], enable transmit! ",
				__kfifo_len(dc->port[port].fifo_ul));
			enable_transmit_ul(port, dc);
		} else {
			DBG1("No data in buffer...");
		}
	}

	if (*(u16 *)&old_ctrl == *(u16 *)&ctrl_dl) {
		DBG1(" No change in mctrl");
		return 1;
	}
	/* Update statistics */
	if (old_ctrl.CTS != ctrl_dl.CTS)
		dc->port[port].tty_icount.cts++;
	if (old_ctrl.DSR != ctrl_dl.DSR)
		dc->port[port].tty_icount.dsr++;
	if (old_ctrl.RI != ctrl_dl.RI)
		dc->port[port].tty_icount.rng++;
	if (old_ctrl.DCD != ctrl_dl.DCD)
		dc->port[port].tty_icount.dcd++;

	wake_up_interruptible(&dc->port[port].tty_wait);

	DBG1("port: %d DCD(%d), CTS(%d), RI(%d), DSR(%d)",
	   port,
	   dc->port[port].tty_icount.dcd, dc->port[port].tty_icount.cts,
	   dc->port[port].tty_icount.rng, dc->port[port].tty_icount.dsr);

	return 1;
}

static enum ctrl_port_type port2ctrl(enum port_type port,
					const struct nozomi *dc)
{
	switch (port) {
	case PORT_MDM:
		return CTRL_MDM;
	case PORT_DIAG:
		return CTRL_DIAG;
	case PORT_APP1:
		return CTRL_APP1;
	case PORT_APP2:
		return CTRL_APP2;
	default:
		dev_err(&dc->pdev->dev,
			"ERROR: send flow control " \
			"received for non-existing port\n");
	};
	return CTRL_ERROR;
}

/*
 * Send flow control, can only update one channel at a time
 * Return 0 - If we have updated all flow control
 * Return 1 - If we need to update more flow control, ack current enable more
 */
static int send_flow_control(struct nozomi *dc)
{
	u32 i, more_flow_control_to_be_updated = 0;
	u16 *ctrl;

	for (i = PORT_MDM; i < MAX_PORT; i++) {
		if (dc->port[i].update_flow_control) {
			if (more_flow_control_to_be_updated) {
				/* We have more flow control to be updated */
				return 1;
			}
			dc->port[i].ctrl_ul.port = port2ctrl(i, dc);
			ctrl = (u16 *)&dc->port[i].ctrl_ul;
			write_mem32(dc->port[PORT_CTRL].ul_addr[0], \
				(u32 *) ctrl, 2);
			dc->port[i].update_flow_control = 0;
			more_flow_control_to_be_updated = 1;
		}
	}
	return 0;
}

/*
 * Handle downlink data, ports that are handled are modem and diagnostics
 * Return 1 - ok
 * Return 0 - toggle fields are out of sync
 */
static int handle_data_dl(struct nozomi *dc, enum port_type port, u8 *toggle,
			u16 read_iir, u16 mask1, u16 mask2)
{
	if (*toggle == 0 && read_iir & mask1) {
		if (receive_data(port, dc)) {
			writew(mask1, dc->reg_fcr);
			*toggle = !(*toggle);
		}

		if (read_iir & mask2) {
			if (receive_data(port, dc)) {
				writew(mask2, dc->reg_fcr);
				*toggle = !(*toggle);
			}
		}
	} else if (*toggle == 1 && read_iir & mask2) {
		if (receive_data(port, dc)) {
			writew(mask2, dc->reg_fcr);
			*toggle = !(*toggle);
		}

		if (read_iir & mask1) {
			if (receive_data(port, dc)) {
				writew(mask1, dc->reg_fcr);
				*toggle = !(*toggle);
			}
		}
	} else {
		dev_err(&dc->pdev->dev, "port out of sync!, toggle:%d\n",
			*toggle);
		return 0;
	}
	return 1;
}

/*
 * Handle uplink data, this is currently for the modem port
 * Return 1 - ok
 * Return 0 - toggle field are out of sync
 */
static int handle_data_ul(struct nozomi *dc, enum port_type port, u16 read_iir)
{
	u8 *toggle = &(dc->port[port].toggle_ul);

	if (*toggle == 0 && read_iir & MDM_UL1) {
		dc->last_ier &= ~MDM_UL;
		writew(dc->last_ier, dc->reg_ier);
		if (send_data(port, dc)) {
			writew(MDM_UL1, dc->reg_fcr);
			dc->last_ier = dc->last_ier | MDM_UL;
			writew(dc->last_ier, dc->reg_ier);
			*toggle = !*toggle;
		}

		if (read_iir & MDM_UL2) {
			dc->last_ier &= ~MDM_UL;
			writew(dc->last_ier, dc->reg_ier);
			if (send_data(port, dc)) {
				writew(MDM_UL2, dc->reg_fcr);
				dc->last_ier = dc->last_ier | MDM_UL;
				writew(dc->last_ier, dc->reg_ier);
				*toggle = !*toggle;
			}
		}

	} else if (*toggle == 1 && read_iir & MDM_UL2) {
		dc->last_ier &= ~MDM_UL;
		writew(dc->last_ier, dc->reg_ier);
		if (send_data(port, dc)) {
			writew(MDM_UL2, dc->reg_fcr);
			dc->last_ier = dc->last_ier | MDM_UL;
			writew(dc->last_ier, dc->reg_ier);
			*toggle = !*toggle;
		}

		if (read_iir & MDM_UL1) {
			dc->last_ier &= ~MDM_UL;
			writew(dc->last_ier, dc->reg_ier);
			if (send_data(port, dc)) {
				writew(MDM_UL1, dc->reg_fcr);
				dc->last_ier = dc->last_ier | MDM_UL;
				writew(dc->last_ier, dc->reg_ier);
				*toggle = !*toggle;
			}
		}
	} else {
		writew(read_iir & MDM_UL, dc->reg_fcr);
		dev_err(&dc->pdev->dev, "port out of sync!\n");
		return 0;
	}
	return 1;
}

static irqreturn_t interrupt_handler(int irq, void *dev_id)
{
	struct nozomi *dc = dev_id;
	unsigned int a;
	u16 read_iir;

	if (!dc)
		return IRQ_NONE;

	spin_lock(&dc->spin_mutex);
	read_iir = readw(dc->reg_iir);

	/* Card removed */
	if (read_iir == (u16)-1)
		goto none;
	/*
	 * Just handle interrupt enabled in IER
	 * (by masking with dc->last_ier)
	 */
	read_iir &= dc->last_ier;

	if (read_iir == 0)
		goto none;


	DBG4("%s irq:0x%04X, prev:0x%04X", interrupt2str(read_iir), read_iir,
		dc->last_ier);

	if (read_iir & RESET) {
		if (unlikely(!nozomi_read_config_table(dc))) {
			dc->last_ier = 0x0;
			writew(dc->last_ier, dc->reg_ier);
			dev_err(&dc->pdev->dev, "Could not read status from "
				"card, we should disable interface\n");
		} else {
			writew(RESET, dc->reg_fcr);
		}
		/* No more useful info if this was the reset interrupt. */
		goto exit_handler;
	}
	if (read_iir & CTRL_UL) {
		DBG1("CTRL_UL");
		dc->last_ier &= ~CTRL_UL;
		writew(dc->last_ier, dc->reg_ier);
		if (send_flow_control(dc)) {
			writew(CTRL_UL, dc->reg_fcr);
			dc->last_ier = dc->last_ier | CTRL_UL;
			writew(dc->last_ier, dc->reg_ier);
		}
	}
	if (read_iir & CTRL_DL) {
		receive_flow_control(dc);
		writew(CTRL_DL, dc->reg_fcr);
	}
	if (read_iir & MDM_DL) {
		if (!handle_data_dl(dc, PORT_MDM,
				&(dc->port[PORT_MDM].toggle_dl), read_iir,
				MDM_DL1, MDM_DL2)) {
			dev_err(&dc->pdev->dev, "MDM_DL out of sync!\n");
			goto exit_handler;
		}
	}
	if (read_iir & MDM_UL) {
		if (!handle_data_ul(dc, PORT_MDM, read_iir)) {
			dev_err(&dc->pdev->dev, "MDM_UL out of sync!\n");
			goto exit_handler;
		}
	}
	if (read_iir & DIAG_DL) {
		if (!handle_data_dl(dc, PORT_DIAG,
				&(dc->port[PORT_DIAG].toggle_dl), read_iir,
				DIAG_DL1, DIAG_DL2)) {
			dev_err(&dc->pdev->dev, "DIAG_DL out of sync!\n");
			goto exit_handler;
		}
	}
	if (read_iir & DIAG_UL) {
		dc->last_ier &= ~DIAG_UL;
		writew(dc->last_ier, dc->reg_ier);
		if (send_data(PORT_DIAG, dc)) {
			writew(DIAG_UL, dc->reg_fcr);
			dc->last_ier = dc->last_ier | DIAG_UL;
			writew(dc->last_ier, dc->reg_ier);
		}
	}
	if (read_iir & APP1_DL) {
		if (receive_data(PORT_APP1, dc))
			writew(APP1_DL, dc->reg_fcr);
	}
	if (read_iir & APP1_UL) {
		dc->last_ier &= ~APP1_UL;
		writew(dc->last_ier, dc->reg_ier);
		if (send_data(PORT_APP1, dc)) {
			writew(APP1_UL, dc->reg_fcr);
			dc->last_ier = dc->last_ier | APP1_UL;
			writew(dc->last_ier, dc->reg_ier);
		}
	}
	if (read_iir & APP2_DL) {
		if (receive_data(PORT_APP2, dc))
			writew(APP2_DL, dc->reg_fcr);
	}
	if (read_iir & APP2_UL) {
		dc->last_ier &= ~APP2_UL;
		writew(dc->last_ier, dc->reg_ier);
		if (send_data(PORT_APP2, dc)) {
			writew(APP2_UL, dc->reg_fcr);
			dc->last_ier = dc->last_ier | APP2_UL;
			writew(dc->last_ier, dc->reg_ier);
		}
	}

exit_handler:
	spin_unlock(&dc->spin_mutex);
	for (a = 0; a < NOZOMI_MAX_PORTS; a++) {
		struct tty_struct *tty;
		if (test_and_clear_bit(a, &dc->flip)) {
			tty = tty_port_tty_get(&dc->port[a].port);
			if (tty)
				tty_flip_buffer_push(tty);
			tty_kref_put(tty);
		}
	}
	return IRQ_HANDLED;
none:
	spin_unlock(&dc->spin_mutex);
	return IRQ_NONE;
}

static void nozomi_get_card_type(struct nozomi *dc)
{
	int i;
	u32 size = 0;

	for (i = 0; i < 6; i++)
		size += pci_resource_len(dc->pdev, i);

	/* Assume card type F32_8 if no match */
	dc->card_type = size == 2048 ? F32_2 : F32_8;

	dev_info(&dc->pdev->dev, "Card type is: %d\n", dc->card_type);
}

static void nozomi_setup_private_data(struct nozomi *dc)
{
	void __iomem *offset = dc->base_addr + dc->card_type / 2;
	unsigned int i;

	dc->reg_fcr = (void __iomem *)(offset + R_FCR);
	dc->reg_iir = (void __iomem *)(offset + R_IIR);
	dc->reg_ier = (void __iomem *)(offset + R_IER);
	dc->last_ier = 0;
	dc->flip = 0;

	dc->port[PORT_MDM].token_dl = MDM_DL;
	dc->port[PORT_DIAG].token_dl = DIAG_DL;
	dc->port[PORT_APP1].token_dl = APP1_DL;
	dc->port[PORT_APP2].token_dl = APP2_DL;

	for (i = 0; i < MAX_PORT; i++)
		init_waitqueue_head(&dc->port[i].tty_wait);
}

static ssize_t card_type_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	const struct nozomi *dc = pci_get_drvdata(to_pci_dev(dev));

	return sprintf(buf, "%d\n", dc->card_type);
}
static DEVICE_ATTR(card_type, S_IRUGO, card_type_show, NULL);

static ssize_t open_ttys_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	const struct nozomi *dc = pci_get_drvdata(to_pci_dev(dev));

	return sprintf(buf, "%u\n", dc->open_ttys);
}
static DEVICE_ATTR(open_ttys, S_IRUGO, open_ttys_show, NULL);

static void make_sysfs_files(struct nozomi *dc)
{
	if (device_create_file(&dc->pdev->dev, &dev_attr_card_type))
		dev_err(&dc->pdev->dev,
			"Could not create sysfs file for card_type\n");
	if (device_create_file(&dc->pdev->dev, &dev_attr_open_ttys))
		dev_err(&dc->pdev->dev,
			"Could not create sysfs file for open_ttys\n");
}

static void remove_sysfs_files(struct nozomi *dc)
{
	device_remove_file(&dc->pdev->dev, &dev_attr_card_type);
	device_remove_file(&dc->pdev->dev, &dev_attr_open_ttys);
}

/* Allocate memory for one device */
static int __devinit nozomi_card_init(struct pci_dev *pdev,
				      const struct pci_device_id *ent)
{
	resource_size_t start;
	int ret;
	struct nozomi *dc = NULL;
	int ndev_idx;
	int i;

	dev_dbg(&pdev->dev, "Init, new card found\n");

	for (ndev_idx = 0; ndev_idx < ARRAY_SIZE(ndevs); ndev_idx++)
		if (!ndevs[ndev_idx])
			break;

	if (ndev_idx >= ARRAY_SIZE(ndevs)) {
		dev_err(&pdev->dev, "no free tty range for this card left\n");
		ret = -EIO;
		goto err;
	}

	dc = kzalloc(sizeof(struct nozomi), GFP_KERNEL);
	if (unlikely(!dc)) {
		dev_err(&pdev->dev, "Could not allocate memory\n");
		ret = -ENOMEM;
		goto err_free;
	}

	dc->pdev = pdev;

	ret = pci_enable_device(dc->pdev);
	if (ret) {
		dev_err(&pdev->dev, "Failed to enable PCI Device\n");
		goto err_free;
	}

	ret = pci_request_regions(dc->pdev, NOZOMI_NAME);
	if (ret) {
		dev_err(&pdev->dev, "I/O address 0x%04x already in use\n",
			(int) /* nozomi_private.io_addr */ 0);
		goto err_disable_device;
	}

	start = pci_resource_start(dc->pdev, 0);
	if (start == 0) {
		dev_err(&pdev->dev, "No I/O address for card detected\n");
		ret = -ENODEV;
		goto err_rel_regs;
	}

	/* Find out what card type it is */
	nozomi_get_card_type(dc);

	dc->base_addr = ioremap_nocache(start, dc->card_type);
	if (!dc->base_addr) {
		dev_err(&pdev->dev, "Unable to map card MMIO\n");
		ret = -ENODEV;
		goto err_rel_regs;
	}

	dc->send_buf = kmalloc(SEND_BUF_MAX, GFP_KERNEL);
	if (!dc->send_buf) {
		dev_err(&pdev->dev, "Could not allocate send buffer?\n");
		ret = -ENOMEM;
		goto err_free_sbuf;
	}

	spin_lock_init(&dc->spin_mutex);

	nozomi_setup_private_data(dc);

	/* Disable all interrupts */
	dc->last_ier = 0;
	writew(dc->last_ier, dc->reg_ier);

	ret = request_irq(pdev->irq, &interrupt_handler, IRQF_SHARED,
			NOZOMI_NAME, dc);
	if (unlikely(ret)) {
		dev_err(&pdev->dev, "can't request irq %d\n", pdev->irq);
		goto err_free_sbuf;
	}

	DBG1("base_addr: %p", dc->base_addr);

	make_sysfs_files(dc);

	dc->index_start = ndev_idx * MAX_PORT;
	ndevs[ndev_idx] = dc;

	pci_set_drvdata(pdev, dc);

	/* Enable RESET interrupt */
	dc->last_ier = RESET;
	iowrite16(dc->last_ier, dc->reg_ier);

	dc->state = NOZOMI_STATE_ENABLED;

	for (i = 0; i < MAX_PORT; i++) {
		mutex_init(&dc->port[i].tty_sem);
		tty_port_init(&dc->port[i].port);
		tty_register_device(ntty_driver, dc->index_start + i,
							&pdev->dev);
	}
	return 0;

err_free_sbuf:
	kfree(dc->send_buf);
	iounmap(dc->base_addr);
err_rel_regs:
	pci_release_regions(pdev);
err_disable_device:
	pci_disable_device(pdev);
err_free:
	kfree(dc);
err:
	return ret;
}

static void __devexit tty_exit(struct nozomi *dc)
{
	unsigned int i;

	DBG1(" ");

	flush_scheduled_work();

	for (i = 0; i < MAX_PORT; ++i) {
		struct tty_struct *tty = tty_port_tty_get(&dc->port[i].port);
		if (tty && list_empty(&tty->hangup_work.entry))
			tty_hangup(tty);
		tty_kref_put(tty);
	}
	/* Racy below - surely should wait for scheduled work to be done or
	   complete off a hangup method ? */
	while (dc->open_ttys)
		msleep(1);
	for (i = dc->index_start; i < dc->index_start + MAX_PORT; ++i)
		tty_unregister_device(ntty_driver, i);
}

/* Deallocate memory for one device */
static void __devexit nozomi_card_exit(struct pci_dev *pdev)
{
	int i;
	struct ctrl_ul ctrl;
	struct nozo