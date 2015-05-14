val;
}

/**
 *	initio_se2_wr		-	read E2PROM word
 *	@base: Base of InitIO controller
 *	@addr: Address of word in E2PROM
 *	@val: Value to write
 *
 *	Write a word to the NV E2PROM device. Used when recovering from
 *	a problem with the NV.
 */
static void initio_se2_wr(unsigned long base, u8 addr, u16 val)
{
	u8 rb;
	u8 instr;
	int i;

	instr = (u8) (addr | 0x40);
	initio_se2_instr(base, instr);	/* WRITE INSTR */
	for (i = 15; i >= 0; i--) {
		if (val & 0x8000)
			outb(SE2CS | SE2DO, base + TUL_NVRAM);	/* -CLK+dataBit 1 */
		else
			outb(SE2CS, base + TUL_NVRAM);		/* -CLK+dataBit 0 */
		udelay(30);
		outb(SE2CS | SE2CLK, base + TUL_NVRAM);		/* +CLK */
		udelay(30);
		val <<= 1;
	}
	outb(SE2CS, base + TUL_NVRAM);				/* -CLK */
	udelay(30);
	outb(0, base + TUL_NVRAM);				/* -CS  */
	udelay(30);

	outb(SE2CS, base + TUL_NVRAM);				/* +CS  */
	udelay(30);

	for (;;) {
		outb(SE2CS | SE2CLK, base + TU