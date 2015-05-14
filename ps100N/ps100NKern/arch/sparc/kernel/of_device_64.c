meout;
		command[cmdlen-3] = qualifier;
		command[cmdlen-2] = LSB_of(len);
		command[cmdlen-1] = MSB_of(len);

		result = usbat_execute_command(us, command, cmdlen);

		if (result != USB_STOR_XFER_GOOD)
			return USB_STOR_TRANSPORT_ERROR;

		if (i==0) {

			for (j=0; j<num_registers; j++) {
				data[j<<1] = registers[j];
				data[1+(j<<1)] = data_out[j];
			}

			result = usbat_bulk_write(us, data, num_registers*2, 0);
			if (result != USB_STOR_XFER_GOOD)
				return USB_STOR_TRANSPORT_ERROR;

		}

		result = usb_stor_bulk_transfer_sg(us,
			pipe, buf, len, use_sg, NULL);

		/*
		 * If we get a stall on the bulk download, we'll retry
		 * the bulk download -- but not the SCSI command because
		 * in some sense the SCSI command is still 'active' and
		 * waiting for the data. Don't ask me why this should be;
		 * I'm only following what the Windoze driver did.
		 *
		 * Note that a stall for the test-and-read/write command means
		 * that the test failed. In this case we're testing to make
		 * sure that the device is error-free
		 * (i.e. bit 0 -- CHK -- of status is 0). The most likely
		 * hypothesis is that the USBAT chip somehow knows what
		 * the device will accept, but doesn't give the device any
		 * data until all data is received. Thus, the device would
		 * still be waiting for the first byte of data if a stall
		 * occurs, even if the stall implies that some data was
		 * transferred.
		 */

		if (result == USB_STOR_XFER_SHORT ||
				result == USB_STOR_XFER_STALLED) {

			/*
			 * If we're reading and we stalled, then clear
			 * the bulk output pipe only the first time.
			 */

			if (direction==DMA_FROM_DEVICE && i==0) {
				if (usb_stor_clear_halt(us,
						us->send_bulk_pipe) < 0)
					return USB_STOR_TRANSPORT_ERROR;
			}

			/*
			 * Read status: is the device angry, or just busy?
			 */

 			result = usbat_read(us, USBAT_ATA, 
				direction==DMA_TO_DEVICE ?
					USBAT_ATA_STATUS : USBAT_ATA_ALTSTATUS,
				status);

			if (result!=USB_STOR_XFER_GOOD)
				return USB_STOR_TRANSPORT_ERROR;
			if (*status & 0x01) /* check condition */
				return USB_STOR_TRANSPORT_FAILED;
			if (*status & 0x20) /* device fault */
				return USB_STOR_TRANSPORT_FAILED;

			US_DEBUGP("Redoing %s\n",
			  direction==DMA_TO_DEVICE ? "write" : "read");

		} else if (result != USB_STOR_XFER_GOOD)
			return USB_STOR_TRANSPORT_ERROR;
		else
			return usbat_wait_not_busy(us, minutes);

	}

	US_DEBUGP("Bummer! %s bulk data 20 times failed.\n",
		direction==DMA_TO_DEVICE ? "Writing" : "Reading");

	return USB_STOR_TRANSPORT_FAILED;
}

/*
 * Write to multiple registers:
 * Allows us to write specific data to any registers. The data to be written
 * gets packed in this sequence: reg0, data0, reg1, data1, ..., regN, dataN
 * which gets sent through bulk out.
 * Not designed for large transfers of data!
 */
static int usbat_multiple_write(struct us_data *us,
				unsigned char *registers,
				unsigned char *data_out,
				unsigned short num_registers)
{
	int i, result;
	unsigned char *data = us->iobuf;
	unsigned char *command = us->iobuf;

	BUG_ON(num_registers > US_IOBUF_SIZE/2);

	/* Write to multiple registers, ATA access */
	command[0] = 0x40;
	command[1] = USBAT_ATA | USBAT_CMD_WRITE_REGS;

	/* No relevance */
	command[2] = 0;
	command[3] = 0;
	command[4] = 0;
	command[5] = 0;

	/* Number of bytes to be transferred (incl. addresses and data) */
	command[6] = LSB_of(num_registers*2);
	command[7] = MSB_of(num_registers*2);

	/* The setup command */
	result = usbat_execute_command(us, command, 8);
	if (result != USB_STOR_XFER_GOOD)
		return USB_STOR_TRANSPORT_ERROR;

	/* Create the reg/data, reg/data sequence */
	for (i=0; i<num_registers; i++) {
		data[i<<1] = registers[i];
		data[1+(i<<1)] = data_out[i];
	}

	/* Send the data */
	result = usbat_bulk_write(us, data, num_registers*2, 0);
	if (result != USB_STOR_XFER_GOOD)
		return USB_STOR_TRANSPORT_ERROR;

	if (usbat_get_device_type(us) == USBAT_DEV_HP8200)
		return usbat_wait_not_busy(us, 0);
	else
		return USB_STOR_TRANSPORT_GOOD;
}

/*
 * Conditionally read blocks from device:
 * Allows us to read blocks from a specific data register, based upon the
 * condition that a status register can be successfully masked with a status
 * qualifier. If this condition is not initially met, the read will wait
 * up until a maximum amount of time has elapsed, as specified by timeout.
 * The read will start when the condition is met, otherwise the command aborts.
 *
 * The qualifier defined here is not the value that is masked, it defines
 * conditions for the write to take place. The actual masked qualifier (and
 * other related details) are defined beforehand with _set_shuttle_features().
 */
static int usbat_read_blocks(struct us_data *us,
			     void* buffer,
			     int len,
			     int use_sg)
{
	int result;
	unsigned char *command = us->iobuf;

	command[0] = 0xC0;
	command[1] = USBAT_ATA | USBAT_CMD_COND_READ_BLOCK;
	command[2] = USBAT_ATA_DATA;
	command[3] = USBAT_ATA_STATUS;
	command[4] = 0xFD; /* Timeout (ms); */
	command[5] = USBAT_QUAL_FCQ;
	command[6] = LSB_of(len);
	command[7] = MSB_of(len);

	/* Multiple block read setup command */
	result = usbat_execute_command(us, command, 8);
	if (result != USB_STOR_XFER_GOOD)
		return USB_STOR_TRANSPORT_FAILED;
	
	/* Read the blocks we just asked for */
	result = usbat_bulk_read(us, buffer, len, use_sg);
	if (result != USB_STOR_XFER_GOOD)
		return USB_STOR_TRANSPORT_FAILED;

	return USB_STOR_TRANSPORT_GOOD;
}

/*
 * Conditionally write blocks to device:
 * Allows us to write blocks to a specific data register, based upon the
 * condition that a status register can be successfully masked with a status
 * qualifier. If this condition is not initially met, the write will wait
 * up until a maximum amount of time has elapsed, as specified by timeout.
 * The read will start when the condition is met, otherwise the command aborts.
 *
 * The qualifier defined here is not the value that is masked, it defines
 * conditions for the write to take place. The actual masked qualifier (and
 * other related details) are defined beforehand with _set_shuttle_features().
 */
static int usbat_write_blocks(struct us_data *us,
			      void* buffer,
			      int len,
			      int use_sg)
{
	int result;
	unsigned char *command = us->iobuf;

	command[0] = 0x40;
	command[1] = USBAT_ATA | USBAT_CMD_COND_WRITE_BLOCK;
	command[2] = USBAT_ATA_DATA;
	command[3] = USBAT_ATA_STATUS;
	command[4] = 0xFD; /* Timeout (ms) */
	command[5] = USBAT_QUAL_FCQ;
	command[6] = LSB_of(len);
	command[7] = MSB_of(len);

	/* Multiple block write setup command */
	result = usbat_execute_command(us, command, 8);
	if (result != USB_STOR_XFER_GOOD)
		return USB_STOR_TRANSPORT_FAILED;
	
	/* Write the data */
	result = usbat_bulk_write(us, buffer, len, use_sg);
	if (result != USB_STOR_XFER_GOOD)
		return USB_STOR_TRANSPORT_FAILED;

	return USB_STOR_TRANSPORT_GOOD;
}

/*
 * Read the User IO register
 */
static int usbat_read_user_io(struct us_data *us, unsigned char *data_flags)
{
	int result;

	result = usb_stor_ctrl_transfer(us,
		us->recv_ctrl_pipe,
		USBAT_CMD_UIO,
		0xC0,
		0,
		0,
		data_flags,
		USBAT_UIO_READ);

	US_DEBUGP("usbat_read_user_io: UIO register reads %02X\n", (unsigned short) (*data_flags));

	return result;
}

/*
 * Write to the User IO register
 */
static int usbat_write_user_io(struct us_data *us,
			       unsigned char enable_flags,
			       unsigned char data_flags)
{
	return usb_stor_ctrl_transfer(us,
		us->send_ctrl_pipe,
		USBAT_CMD_UIO,
		0x40,
		short_pack(enable_flags, data_flags),
		0,
		NULL,
		USBAT_UIO_WRITE);
}

/*
 * Reset the device
 * Often needed on media change.
 */
static int usbat_device_reset(struct us_data *us)
{
	int rc;

	/*
	 * Reset peripheral, enable peripheral control signals
	 * (bring reset signal up)
	 */
	rc = usbat_write_user_io(us,
							 USBAT_UIO_DRVRST | USBAT_UIO_OE1 | USBAT_UIO_OE0,
							 USBAT_UIO_EPAD | USBAT_UIO_1);
	if (rc != USB_STOR_XFER_GOOD)
		return USB_STOR_TRANSPORT_ERROR;
			
	/*
	 * Enable peripheral control signals
	 * (bring reset signal down)
	 */
	rc = usbat_write_user_io(us,
							 USBAT_UIO_OE1  | USBAT_UIO_OE0,
							 USBAT_UIO_EPAD | USBAT_UIO_1);
	if (rc != USB_STOR_XFER_GOOD)
		return USB_STOR_TRANSPORT_ERROR;

	return USB_STOR_TRANSPORT_GOOD;
}

/*
 * Enable card detect
 */
static int usbat_device_enable_cdt(struct us_data *us)
{
	int rc;

	/* Enable peripheral control signals and card detect */
	rc = usbat_write_user_io(us,
							 USBAT_UIO_ACKD | USBAT_UIO_OE1  | USBAT_UIO_OE0,
							 USBAT_UIO_EPAD | USBAT_UIO_1);
	if (rc != USB_STOR_XFER_GOOD)
		return USB_STOR_TRANSPORT_ERROR;

	return USB_STOR_TRANSPORT_GOOD;
}

/*
 * Determine if media is present.
 */
static int usbat_flash_check_media_present(unsigned char *uio)
{
	if (*uio & USBAT_UIO_UI0) {
		US_DEBUGP("usbat_flash_check_media_present: no media detected\n");
		return USBAT_FLASH_MEDIA_NONE;
	}

	return USBAT_FLASH_MEDIA_CF;
}

/*
 * Determine if media has changed since last operation
 */
static int usbat_flash_check_media_changed(unsigned char *uio)
{
	if (*uio & USBAT_UIO_0) {
		US_DEBUGP("usbat_flash_check_media_changed: media change detected\n");
		return USBAT_FLASH_MEDIA_CHANGED;
	}

	return USBAT_FLASH_MEDIA_SAME;
}

/*
 * Check for media change / no media and handle the situation appropriately
 */
static int usbat_flash_check_media(struct us_data *us,
				   struct usbat_info *info)
{
	int rc;
	unsigned char *uio = us->iobuf;

	rc = usbat_read_user_io(us, uio);
	if (rc != USB_STOR_XFER_GOOD)
		return USB_STOR_TRANSPORT_ERROR;

	/* Check for media existence */
	rc = usbat_flash_check_media_present(uio);
	if (rc == USBAT_FLASH_MEDIA_NONE) {
		info->sense_key = 0x02;
		info->sense_asc = 0x3A;
		info->sense_ascq = 0x00;
		return USB_STOR_TRANSPORT_FAILED;
	}

	/* Check for media change */
	rc = usbat_flash_check_media_changed(uio);
	if (rc == USBAT_FLASH_MEDIA_CHANGED) {

		/* Reset and re-enable card detect */
		rc = usbat_device_reset(us);
		if (rc != USB_STOR_TRANSPORT_GOOD)
			return rc;
		rc = usbat_device_enable_cdt(us);
		if (rc != USB_STOR_TRANSPORT_GOOD)
			return rc;

		msleep(50);

		rc = usbat_read_user_io(us, uio);
		if (rc != USB_STOR_XFER_GOOD)
			return USB_STOR_TRANSPORT_ERROR;
		
		info->sense_key = UNIT_ATTENTION;
		info->sense_asc = 0x28;
		info->sense_ascq = 0x00;
		return USB_STOR_TRANSPORT_FAILED;
	}

	return USB_STOR_TRANSPORT_GOOD;
}

/*
 * Determine whether we are controlling a flash-based reader/writer,
 * or a HP8200-based CD drive.
 * Sets transport functions as appropriate.
 */
static int usbat_identify_device(struct us_data *us,
				 struct usbat_info *info)
{
	int rc;
	unsigned char status;

	if (!us || !info)
		return USB_STOR_TRANSPORT_ERROR;

	rc = usbat_device_reset(us);
	if (rc != USB_STOR_TRANSPORT_GOOD)
		return rc;
	msleep(500);

	/*
	 * In attempt to distinguish between HP CDRW's and Flash readers, we now
	 * execute the IDENTIFY PACKET DEVICE command. On ATA devices (i.e. flash
	 * readers), this command should fail with error. On ATAPI devices (i.e.
	 * CDROM drives), it should succeed.
	 */
	rc = usbat_write(us, USBAT_ATA, USBAT_ATA_CMD, 0xA1);
 	if (rc != USB_STOR_XFER_GOOD)
 		return USB_STOR_TRANSPORT_ERROR;

	rc = usbat_get_status(us, &status);
 	if (rc != USB_STOR_XFER_GOOD)
 		return USB_STOR_TRANSPORT_ERROR;

	/* Check for error bit, or if the command 'fell through' */
	if (status == 0xA1 || !(status & 0x01)) {
		/* Device is HP 8200 */
		US_DEBUGP("usbat_identify_device: Detected HP8200 CDRW\n");
		info->devicetype = USBAT_DEV_HP8200;
	} else {
		/* Device is a CompactFlash reader/writer */
		US_DEBUGP("usbat_identify_device: Detected Flash reader/writer\n");
		info->devicetype = USBAT_DEV_FLASH;
	}

	return USB_STOR_TRANSPORT_GOOD;
}

/*
 * Set the transport function based on the device type
 */
static int usbat_set_transport(struct us_data *us,
			       struct usbat_info *info,
			       int devicetype)
{

	if (!info->devicetype)
		info->devicetype = devicetype;

	if (!info->devicetype)
		usbat_identify_device(us, info);

	switch (info->devicetype) {
	default:
		return USB_STOR_TRANSPORT_ERROR;

	case  USBAT_DEV_HP8200:
		us->transport = usbat_hp8200e_transport;
		break;

	case USBAT_DEV_FLASH:
		us->transport = usbat_flash_transport;
		break;
	}

	return 0;
}

/*
 * Read the media capacity
 */
static int usbat_flash_get_sector_count(struct us_data *us,
					struct usbat_info *info)
{
	unsigned char registers[3] = {
		USBAT_ATA_SECCNT,
		USBAT_ATA_DEVICE,
		USBAT_ATA_CMD,
	};
	unsigned char  command[3] = { 0x01, 0xA0, 0xEC };
	unsigned char *reply;
	unsigned char status;
	int rc;

	if (!us || !info)
		return USB_STOR_TRANSPORT_ERROR;

	reply = kmalloc(512, GFP_NOIO);
	if (!reply)
		return USB_STOR_TRANSPORT_ERROR;

	/* ATA command : IDENTIFY DEVICE */
	rc = usbat_multiple_write(us, registers, command, 3);
	if (rc != USB_STOR_XFER_GOOD) {
		US_DEBUGP("usbat_flash_get_sector_count: Gah! identify_device failed\n");
		rc = USB_STOR_TRANSPORT_ERROR;
		goto leave;
	}

	/* Read device status */
	if (usbat_get_status(us, &status) != USB_STOR_XFER_GOOD) {
		rc = USB_STOR_TRANSPORT_ERROR;
		goto leave;
	}

	msleep(100);

	/* Read the device identification data */
	rc = usbat_read_block(us, reply, 512, 0);
	if (rc != USB_STOR_TRANSPORT_GOOD)
		goto leave;

	info->sectors = ((u32)(reply[117]) << 24) |
		((u32)(reply[116]) << 16) |
		((u32)(reply[115]) <<  8) |
		((u32)(reply[114])      );

	rc = USB_STOR_TRANSPORT_GOOD;

 leave:
	kfree(reply);
	return rc;
}

/*
 * Read data from device
 */
static int usbat_flash_read_data(struct us_data *us,
								 struct usbat_info *info,
								 u32 sector,
								 u32 sectors)
{
	unsigned char registers[7] = {
		USBAT_ATA_FEATURES,
		USBAT_ATA_SECCNT,
		USBAT_ATA_SECNUM,
		USBAT_ATA_LBA_ME,
		USBAT_ATA_LBA_HI,
		USBAT_ATA_DEVICE,
		USBAT_ATA_STATUS,
	};
	unsigned char command[7];
	unsigned char *buffer;
	unsigned char  thistime;
	unsigned int totallen, alloclen;
	int len, result;
	unsigned int sg_offset = 0;
	struct scatterlist *sg = NULL;

	result = usbat_flash_check_media(us, info);
	if (result != USB_STOR_TRANSPORT_GOOD)
		return result;

	/*
	 * we're working in LBA mode.  according to the ATA spec,
	 * we can support up to 28-bit addressing.  I don't know if Jumpshot
	 * supports beyond 24-bit addressing.  It's kind of hard to test
	 * since it requires > 8GB CF card.
	 */

	if (sector > 0x0FFFFFFF)
		return USB_STOR_TRANSPORT_ERROR;

	totallen = sectors * info->ssize;

	/*
	 * Since we don't read more than 64 KB at a time, we have to create
	 * a bounce buffer and move the data a piece at a time between the
	 * bounce buffer and the actual transfer buffer.
	 */

	alloclen = min(totallen, 65536u);
	buffer = kmalloc(alloclen, GFP_NOIO);
	if (buffer == NULL)
		return USB_STOR_TRANSPORT_ERROR;

	do {
		/*
		 * loop, never allocate or transfer more than 64k at once
		 * (min(128k, 255*info->ssize) is the real limit)
		 */
		len = min(totallen, alloclen);
		thistime = (len / info->ssize) & 0xff;
 
		/* ATA command 0x20 (READ SECTORS) */
		usbat_pack_ata_sector_cmd(command, thistime, sector, 0x20);

		/* Write/execute ATA read command */
		result = usbat_multiple_write(us, registers, command, 7);
		if (result != USB_STOR_TRANSPORT_GOOD)
			goto leave;

		/* Read the data we just requested */
		result = usbat_read_blocks(us, buffer, len, 0);
		if (result != USB_STOR_TRANSPORT_GOOD)
			goto leave;
  	 
		US_DEBUGP("usbat_flash_read_data:  %d bytes\n", len);
	
		/* Store the data in the transfer buffer */
		usb_stor_access_xfer_buf(buffer, len, us->srb,
					 &sg, &sg_offset, TO_XFER_BUF);

		sector += thistime;
		totallen -= len;
	} while (totallen > 0);

	kfree(buffer);
	return USB_STOR_TRANSPORT_GOOD;

leave:
	kfree(buffer);
	return USB_STOR_TRANSPORT_ERROR;
}

/*
 * Write data to device
 */
static int usbat_flash_write_data(struct us_data *us,
								  struct usbat_info *info,
								  u32 sector,
								  u32 sectors)
{
	unsigned char registers[7] = {
		USBAT_ATA_FEATURES,
		USBAT_ATA_SECCNT,
		USBAT_ATA_SE