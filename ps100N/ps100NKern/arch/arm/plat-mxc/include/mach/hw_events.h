_STOR_TRANSPORT_GOOD;
}

/*
 * Read data from a specific sector address
 */
static int alauda_read_data(struct us_data *us, unsigned long address,
		unsigned int sectors)
{
	unsigned char *buffer;
	u16 lba, max_lba;
	unsigned int page, len, offset;
	unsigned int blockshift = MEDIA_INFO(us).blockshift;
	unsigned int pageshift = MEDIA_INFO(us).pageshift;
	unsigned int blocksize = MEDIA_INFO(us).blocksize;
	unsigned int pagesize = MEDIA_INFO(us).pagesize;
	unsigned int uzonesize = MEDIA_INFO(us).uzonesize;
	struct scatterlist *sg;
	int result;

	/*
	 * Since we only read in one block at a time, we have to create
	 * a bounce buffer and move the data a piece at a time between the
	 * bounce buffer and the actual transfer buffer.
	 * We make this buffer big enough to hold temporary redundancy data,
	 * which we use when reading the data blocks.
	 */

	len = min(sectors, blocksize) * (pagesize + 64);
	buffer = kmalloc(len, GFP_NOIO);
	if (buffer == NULL) {
		printk(KERN_WARNING "alauda_read_data: Out of memory\n");
		return USB_STOR_TRANSPORT_ERROR;
	}

	/* Figure out the initial LBA and page */
	lba = address >> blockshift;