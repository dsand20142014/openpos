ng zero. User can disable this feature in 
	 *  sym53c8xx.h. Residual support is enabled by default.
	 */
	if (!SYM_SETUP_RESIDUAL_SUPPORT)
		resid  = 0;
#ifdef DEBUG_2_0_X
if (resid)
	printf("XXXX RESID= %d - 0x%x\n", resid, resid);
#endif

	/*
	 *  Build result in CAM ccb.
	 */
	sym_set_cam_result_ok(cp, cmd, resid);

#ifdef SYM_OPT_HANDLE_DEVICE_QUEUEING
	/*
	 *  If max number of started ccbs had been reduced,
	 *  increase it if 200 good status received.
	 */
	if (lp && lp->started_max < lp->started_limit) {
		++lp->num_sgood;
		if (lp->num_sgood >= 200) {
			lp->num_sgood = 0;
			++lp->started_max;
			if (sym_verbose >= 2) {
				sym_print_addr(cmd, " queue depth is now %d\n",
				       lp->started_max);
			}
		}
	}
#endif

	/*
	 *  Free our CCB.
	 */
	sym_free_ccb (np, cp);

#ifdef SYM_OPT_HANDLE_DEVICE_QUEUEING
	/*
	 *  Requeue a couple of awaiting scsi commands.
	 */
	if (!sym_que_empty(&lp->waiting_ccbq))
		sym_start_next_ccbs(np, lp, 2);
#endif
	/*
	 *  Complete the command.
	 */
	sym_xpt_done(np, cmd);
}

/*
 *  Soft-attach the controller.
 */
int sym_hcb_attach(struct Scsi_Host *shost, struct sym_fw *fw, struct sym_nvram *nvram)
{
	struct sym_hcb *np = sym_get_hcb(shost);
	int i;

	/*
	 *  Get some info about the firmware.
	 */
	np->scripta_sz	 = fw->a_size;
	np->scriptb_sz	 = fw->b_size;
	np->scriptz_sz	 = fw->z_size;
	np->fw_setup	 = fw->setup;
	np->fw_patch	 = fw->patch;
	np->fw_name	 = fw->name;

	/*
	 *  Save setting of some IO registers, so we will 
	 *  be able to probe specific implementations.
	 */
	sym_save_initial_setting (np);

	/*
	 *  Reset the chip now, since it has been reported 
	 *  that SCSI clock calibration may not work properly 
	 *  if the chip is currently active.
	 */
	sym_chip_reset(np);

	/*
	 *  Prepare controller and devices settings, according 
	 *  to chip features, user set-up and driver set-up.
	 */
	sym_p