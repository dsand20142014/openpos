gram a (non-VGA) video mode. */
int intelfbhw_program_mode(struct intelfb_info *dinfo,
			   const struct intelfb_hwstate *hw, int blank)
{
	int pipe = PIPE_A;
	u32 tmp;
	const u32 *dpll, *fp0, *fp1, *pipe_conf;
	const u32 *hs, *ht, *hb, *vs, *vt, *vb, *ss;
	u32 dpll_reg, fp0_reg, fp1_reg, pipe_conf_reg, pipe_stat_reg;
	u32 hsync_reg, htotal_reg, hblank_reg;
	u32 vsync_reg, vtotal_reg, vblank_reg;
	u32 src_size_reg;
	u32 count, tmp_val[3];

	/* Assume single pipe, display plane A, analog CRT. */

#if VERBOSE > 0
	DBG_MSG("intelfbhw_program_mode\n");
#endif

	/* Disable VGA */
	tmp = INREG(VGACNTRL);
	tmp |= VGA_DISABLE;
	OUTREG(VGACNTRL, tmp);

	/* Check whether pipe A or pipe B is enabled. */
	if (hw->pipe_a_conf & PIPECONF_ENABLE)
		pipe = PIPE_A;
	else if (hw->pipe_b_conf & PIPECONF_ENABLE)
		pipe = PIPE_B;

	dinfo->pipe = pipe;

	if (pipe == PIPE_B) {
		dpll = &hw->dpll_b;
		fp0 = &hw->fpb0;
		fp1 = &hw->fpb1;
		pipe_conf = &hw->pipe_b_conf;
		hs = &hw->hsync_b;
	