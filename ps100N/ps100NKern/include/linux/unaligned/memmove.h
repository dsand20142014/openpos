
				continue;
			spin_unlock_irqrestore(&dd->ipath_uctxt_lock, flags);
			ipath_dbg("unit %u port %d is in use "
				  "(PID %u cmd %s), can't reset\n",
				  unit, i,
				  pid_nr(dd->ipath_pd[i]->port_pid),
				  dd->ipath_pd[i]->port_comm);
			ret = -EBUSY;
			goto bail;
		}
	spin_unlock_irqrestore(&dd->ipath_uctxt_lock, flags);

	if (dd->ipath_flags & IPATH_HAS_SEND_DMA)
		teardown_sdma(dd);

	dd->ipath_flags &= ~IPATH_INITTED;
	ipath_write_kreg(dd, dd->ipath_kregs->kr_intmask, 0ULL);
	ret = dd->ipath_f_reset(dd);
	if (ret == 1) {
		ipath_dbg("Reinitializing unit %u after reset attempt\n",
			  unit);
		ret = ipath_init_chip(dd, 1);
	} else
		ret = -EAGAIN;
	if (ret)
		ipath_dev_err(dd, "Reinitialize unit %u after "
			      "reset failed with %d\n", unit, ret);
	else
		dev_info(&dd->pcidev->dev, "Reinitialized u