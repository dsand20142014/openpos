present */
	if (IS_FWI2_CAPABLE(ha))
		loop_id = NPH_F_PORT;
	else
		loop_id = SNS_FL_PORT;
	rval = qla2x00_get_port_name(vha, loop_id, vha->fabric_node_name, 1);
	if (rval != QLA_SUCCESS) {
		DEBUG2(printk("scsi(%ld): MBC_GET_PORT_NAME Failed, No FL "
		    "Port\n", vha->host_no));

		vha->device_flags &= ~SWITCH_FOUND;
		return (QLA_SUCCESS);
	}
	vha->device_flags |= SWITCH_FOUND;

	/* Mark devices that need re-synchronization. */
	rval2 = qla2x00_device_resync(vha);
	if (rval2 == QLA_RSCNS_HANDLED) {
		/* No point doing the scan, just continue. */
		return (QLA_SUCCESS);
	}
	do {
		/* FDMI support. */
		if (ql2xfdmienable &&
		    test_and_clear_bit(REGISTER_FDMI_NEEDED, &vha->dpc_flags))
			qla2x00_fdmi_register(vha);

		/* Ensure we are logged into the SNS. */
		if (IS_FWI2_CAPABLE(ha))
			loop_id = NPH_SNS;
		else
			loop_id = SIMPLE_NAME_SERVER;
		ha->isp_ops->fabric_login(vha, loop_id, 0xff, 0xff,
		    0xfc, mb, BIT_1 | BIT_0);
		if (mb[0] != MBS_COMMAND_COMPLETE) {
			DEBUG2(qla_printk(KERN_INFO, ha,
			    "Failed SNS login: loop_id=%x mb[0]=%x mb[1]=%x "
			    "mb[2]=%x mb[6]=%x mb[7]=%x\n", loop_id,
			    mb[0], mb[1], mb[2], mb[6], mb[7]));
			return (QLA_SUCCESS);
		}

		if (test_and_clear_bit(REGISTER_FC4_NEEDED, &vha->dpc_flags)) {
			if (qla2x00_rft_id(vha)) {
				/* EMPTY */
				DEBUG2(printk("scsi(%ld): Register FC-4 "
				    "TYPE failed.\n", vha->host_no));
			}
			if (qla2x00_rff_id(vha)) {
				/* EMPTY */
				DEBUG2(printk("scsi(%ld): Register FC-4 "
				    "Features failed.\n", vha->host_no));
			}
			if (qla2x00_rnn_id(vha)) {
				/* EMPTY */
				DEBUG2(printk("scsi(%ld): Register Node Name "
				    "failed.\n", vha->host_no));
			} else if (qla2x00_rsnn_nn(vha)) {
				/* EMPTY */
				DEBUG2(printk("scsi(%ld): Register Symbolic "
				    "Node Name failed.\n", vha->host_no));
			}
		}

		rval = qla2x00_find_all_fabric_devs(vha, &new_fcports);
		if (rval != QLA_SUCCESS)
			break;

		/*
		 * Logout all previous fabric devices marked lost, except
		 * tape devices.
		 */
		list_for_each_entry(fcport, &vha->vp_fcports, list) {
			if (test_bit(LOOP_RESYNC_NEEDED, &vha->dpc_flags))
				break;

			if ((fcport->flags & FCF_FABRIC_DEVICE) == 0)
				continue;

			if (atomic_read(&fcport->state) == FCS_DEVICE_LOST) {
				qla2x00_mark_device_lost(vha, fcport,
				    ql2xplogiabsentdevice, 0);
				if (fcport->loop_id != FC_NO_LOOP_ID &&
				    (fcport->flags & FCF_TAPE_PRESENT) == 0 &&
				    fcport->port_type != FCT_INITIATOR &&
				    fcport->port_type != FCT_BROADCAST) {
					ha->isp_ops->fabric_logout(vha,
					    fcport->loop_id,
					    fcport->d_id.b.domain,
					    fcport->d_id.b.area,
					    fcport->d_id.b.al_pa);
					fcport->loop_id = FC_NO_LOOP_ID;
				}
			}
		}

		/* Starting free loop ID. */
		next_loopid = ha->min_external_loopid;

		/*
		 * Scan through our port list and login entries that need to be
		 * logged in.
		 */
		list_for_each_entry(fcport, &vha->vp_fcports, list) {
			if (atomic_read(&vha->loop_down_timer) ||
			    test_bit(LOOP_RESYNC_NEEDED, &vha->dpc_flags))
				break;

			if ((fcport->flags & FCF_FABRIC_DEVICE) == 0 ||
			    (fcport->flags & FCF_LOGIN_NEEDED) == 0)
				continue;

			if (fcport->loop_id == FC_NO_LOOP_ID) {
				fcport->loop_id = next_loopid;
				rval = qla2x00_find_new_loop_id(
				    base_vha, fcport);
				if (rval != QLA_SUCCESS) {
					/* Ran out of IDs to use */
					break;
				}
			}
			/* Login and update database */
			qla2x00_fabric_dev_login(vha, fcport, &next_loopid);
		}

		/* Exit if out of loop IDs. */
		if (rval != QLA_SUCCESS) {
			break;
		}

		/*
		 * Login and add the new devices to our port list.
		 */
		list_for_each_entry_safe(fcport, fcptemp, &new_fcports, list) {
			if (atomic_read(&vha->loop_down_timer) ||
			    test_bit(LOOP_RESYNC_NEEDED, &vha->dpc_flags))
				break;

			/* Find a new loop ID to use. */
			fcport->loop_id = next_loopid;
			rval = qla2x00_find_new_loop_id(base_vha, fcport);
			if (rval != QLA_SUCCESS) {
				/* Ran out of IDs to use */
				break;
			}

			/* Login and update database */
			qla2x00_fabric_dev_login(vha, fcport, &next_loopid);

			if (vha->vp_idx) {
				fcport->vha = vha;
				fcport->vp_idx = vha->vp_idx;
			}
			list_move_tail(&fcport->list, &vha->vp_fcports);
		}
	} while (0);

	/* Free all new device structures not processed. */
	list_for_each_entry_safe(fcport, fcptemp, &new_fcports, list) {
		list_del(&fcport->list);
		kfree(fcport);
	}

	if (rval) {
		DEBUG2(printk("scsi(%ld): Configure fabric error exit: "
		    "rval=%d\n", vha->host_no, rval));
	}

	return (rval);
}


/*
 * qla2x00_find_all_fabric_devs
 *
 * Input:
 *	ha = adapter block pointer.
 *	dev = database device entry pointer.
 *
 * Returns:
 *	0 = success.
 *
 * Context:
 *	Kernel context.
 */
static int
qla2x00_find_all_fabric_devs(scsi_qla_host_t *vha,
	struct list_head *new_fcports)
{
	int		rval;
	uint16_t	loop_id;
	fc_port_t	*fcport, *new_fcport, *fcptemp;
	int		found;

	sw_info_t	*swl;
	int		swl_idx;
	int		first_dev, last_dev;
	port_id_t	wrap, nxt_d_id;
	struct qla_hw_data *ha = vha->hw;
	struct scsi_qla_host *vp, *base_vha = pci_get_drvdata(ha->pdev);
	struct scsi_qla_host *tvp;

	rval = QLA_SUCCESS;

	/* Try GID_PT to get device list, else GAN. */
	swl = kcalloc(MAX_FIBRE_DEVICES, sizeof(sw_info_t), GFP_KERNEL);
	if (!swl) {
		/*EMPTY*/
		DEBUG2(printk("scsi(%ld): GID_PT allocations failed, fallback "
		    "on GA_NXT\n", vha->host_no));
	} else {
		if (qla2x00_gid_pt(vha, swl) != QLA_SUCCESS) {
			kfree(swl);
			swl = NULL;
		} else if (qla2x00_gpn_id(vha, swl) != QLA_SUCCESS) {
			kfree(swl);
			swl = NULL;
		} else if (qla2x00_gnn_id(vha, swl) != QLA_SUCCESS) {
			kfree(swl);
			swl = NULL;
		} else if (ql2xiidmaenable &&
		    qla2x00_gfpn_id(vha, swl) == QLA_SUCCESS) {
			qla2x00_gpsc(vha, swl);
		}
	}
	swl_idx = 0;

	/* Allocate temporary fcport for any new fcports discovered. */
	new_fcport = qla2x00_alloc_fcport(vha, GFP_KERNEL);
	if (new_fcport == NULL) {
		kfree(swl);
		return (QLA_MEMORY_ALLOC_FAILED);
	}
	new_fcport->flags |= (FCF_FABRIC_DEVICE | FCF_LOGIN_NEEDED);
	/* Set start port ID scan at adapter ID. */
	first_dev = 1;
	last_dev = 0;

	/* Starting free loop ID. */
	loop_id = ha->min_external_loopid;
	for (; loop_id <= ha->max_loop_id; loop_id++) {
		if (qla2x00_is_reserved_id(vha, loop_id))
			continue;

		if (atomic_read(&vha->loop_down_timer) || LOOP_TRANSITION(vha))
			break;

		if (swl != NULL) {
			if (last_dev) {
				wrap.b24 = new_fcport->d_id.b24;
			} else {
				new_fcport->d_id.b24 = swl[swl_idx].d_id.