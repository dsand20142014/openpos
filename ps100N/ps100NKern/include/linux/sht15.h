o RIVA_HW_INST object
 * @regnum: register number
 * @red: red component
 * @green: green component
 * @blue: blue component
 *
 * DESCRIPTION:
 * Reads red, green, and blue from color register @regnum.
 *
 * CALLED FROM:
 * rivafb_setcolreg()
 */
static void riva_rclut(RIVA_HW_INST *chip,
		       unsigned char regnum, unsigned char *red,
		       unsigned char *green, unsigned char *blue)
{
	
	VGA_WR08(chip->PDIO, 0x3c7, regnum);
	*red = VGA_RD08(chip->PDIO, 0x3c9);
	*green = VGA_RD08(chip->PDIO, 0x3c9);
	*blue = VGA_RD08(chip->PDIO, 0x3c9);
}

/**
 * riva_save_state - saves current chip state
 * @par: pointer to riva_par object containing info for current riva board
 * @regs: pointer to riv