/* systbls.S: System call entry point tables for OS compatibility.
 *            The native Linux system call table lives here also.
 *
 * Copyright (C) 1995, 2007 David S. Miller (davem@davemloft.net)
 *
 * Based upon preliminary work which is:
 *
 * Copyright (C) 1995 Adrian M. Rodriguez (adrian@remus.rutgers.edu)
 */


	.data
	.align 4

	/* First, the Linux native syscall table. */

	.globl sys_call_table
sys_call_table:
/*0*/	.long sys_restart_syscall, sys_exit, sys_fork, sys_read, sys_write
/*5*/	.long sys_open, sys_close, sys_wait4, sys_creat, sys_link
/*10*/  .long sys_unlink, sunos_execv, sys_chdir, sys_chown16, sys_mknod
/*15*/	.long sys_chmod, sys_lchown16, sparc_brk, sys_nis_syscall, sys_lseek
/*20*/	.long sys_getpid, sys_capget, sys_capset, sys_setuid16, sys_getuid16
/*25*/	.long sys_vmsplice, sys_ptrace, sys_alarm, sys_sigaltstack, sys_pause
/*30*/	.long sys_utime, sys_lchown, sys_fchown, sys_access, sys_nice
/*35*/	.long sys_chown, sys_sync, sys_kill, sys_newstat, sys_sendfile
/*40*/	.long sys_newlstat, sys_dup, sys_sparc_pipe, sys_times, sys_getuid
/*45*/	.long sys_umount, sys_setgid16, sys_getgid16, sys_signal, sys_geteuid16
/*50*/	.long sys_getegid16, sys_acct, sys_nis_syscall, sys_getgid, sys_ioctl
/*55*/	.long sys_reboot, sys_mmap2, sys_symlink, sys_readlink, sys_execve
/*60*/	.long sys_umask, sys_chroot, sys_newfstat, sys_fstat64, sys_getpagesize
/*65*/	.long sys_msync, sys_vfork, sys_pread64, sys_pwrite64, sys_geteuid
/*70*/	.long sys_getegid, sys_mmap, sys_setreuid, sys_munmap, sys_mprotect
/*75*/	.long sys_madvise, sys_vhangup, sys_truncate64, sys_mincore, sys_getgroups16
/*80*/	.long sys_setgroups16, sys_getpgrp, sys_setgroups, sys_setitimer, sys_ftruncate64
/*85*/	.long sys_swapon, sys_getitimer, sys_setuid, sys_sethostname, sys_setgid
/*90*/	.long sys_dup2, sys_setfsuid, sys_fcntl, sys_select, sys_setfsgid
/*95*/	.long sys_fsync, sys_setpriority, sys_nis_syscall, sys_nis_syscall, sys_nis_syscall
/*100*/	.long sys_getpriority, sys_rt_sigreturn, sys_rt_sigaction, sys_rt_sigprocmask, sys_rt_sigpending
/*105*/	.long sys_rt_sigtimedwait, sys_rt_sigqueueinfo, sys_rt_sigsuspend, sys_setresuid, sys_getresuid
/*110*/	.long sys_setresgid, sys_getresgid, sys_setregid, sys_nis_syscall, sys_nis_syscall
/*115*/	.long sys_getgroups, sys_gettimeofday, sys_getrusage, sys_nis_syscall, sys_getcwd
/*120*/	.long sys_readv, sys_writev, sys_settimeofday, sys_fchown16, sys_fchmod
/*125*/	.long sys_nis_syscall, sys_setreuid16, sys_setregid16, sys_rename, sys_truncate
/*130*/	.long sys_ftruncate, sys_flock, sys_lstat64, sys_nis_syscall, sys_nis_syscall
/*135*/	.long sys_nis_syscall, sys_mkdir, sys_rmdir, sys_utimes, sys_stat64
/*140*/	.long sys_sendfile64, sys_nis_syscall, sys_futex, sys_gettid, sys_getrlimit
/*145*/	.long sys_setrlimit, sys_pivot_root, sys_prctl, sys_pciconfig_read, sys_pciconfig_write
/*150*/	.long sys_nis_syscall, sys_inotify_init, sys_inotify_add_watch, sys_poll, sys_getdents64
/*155*/	.long sys_fcntl64, sys_inotify_rm_watch, sys_statfs, sys_fstatfs, sys_oldumount
/*160*/	.long sys_sched_setaffinity, sys_sched_getaffinity, sys_getdomainname, sys_setdomainname, sys_nis_syscall
/*165*/	.long sys_quotactl, sys_set_tid_address, sys_mount, sys_ustat, sys_setxattr
/*170*/	.long sys_lsetxattr, sys_fsetxattr, sys_getxattr, sys_lgetxattr, sys_getdents
/*175*/	.long sys_setsid, sys_fchdir, sys_fgetxattr, sys_listxattr, sys_llistxattr
/*180*/	.long sys_flistxattr, sys_removexattr, sys_lremovexattr, sys_sigpending, sys_ni_syscall
/*185*/	.long sys_setpgid, sys_fremovexattr, sys_tkill, sys_exit_group, sys_newuname
/*190*/	.long sys_init_module, sys_personality, sparc_remap_file_pages, sys_epoll_create, sys_epoll_ctl
/*195*/	.long sys_epoll_wait, sys_ioprio_set, sys_getppid, sparc_sigaction, sys_sgetmask
/*200*/	.long sys_ssetmask, sys_sigsuspend, sys_newlstat, sys_uselib, sys_old_readdir
/*205*/	.long sys_readahead, sys_socketcall, sys_syslog, sys_lookup_dcookie, sys_fadvise64
/*210*/	.long sys_fadvise64_64, sys_tgkill, sys_waitpid, sys_swapoff, sys_sysinfo
/*215*/	.long sys_ipc, sys_sigreturn, sys_clone, sys_iop/
	if ((ahc->flags & AHC_DISABLE_PCI_PERR) != 0)
		ahc->seqctl |= FAILDIS;

	ahc->bus_intr = ahc_pci_intr;
	ahc->bus_chip_init = ahc_pci_chip_init;

	/* Remeber how the card was setup in case there is no SEEPROM */
	if ((ahc_inb(ahc, HCNTRL) & POWRDN) == 0) {
		ahc_pause(ahc);
		if ((ahc->features & AHC_ULTRA2) != 0)
			our_id = ahc_inb(ahc, SCSIID_ULTRA2) & OID;
		else
			our_id = ahc_inb(ahc, SCSIID) & OID;
		sxfrctl1 = ahc_inb(ahc, SXFRCTL1) & STPWEN;
		scsiseq = ahc_inb(ahc, SCSISEQ);
	} else {
		sxfrctl1 = STPWEN;
		our_id = 7;
		scsiseq = 0;
	}

	error = ahc_reset(ahc, /*reinit*/FALSE);
	if (error != 0)
		return (ENXIO);

	if ((ahc->features & AHC_DT) != 0) {
		u_int sfunct;

		/* Perform ALT-Mode Setup */
		sfunct = ahc_inb(ahc, SFUNCT) & ~ALT_MODE;
		ahc_outb(ahc, SFUNCT, sfunct | ALT_MODE);
		ahc_outb(ahc, OPTIONMODE,
			 OPTIONMODE_DEFAULTS|AUTOACKEN|BUSFREEREV|EXPPHASEDIS);
		ahc_outb(ahc, SFUNCT, sfunct);

		/* Normal mode setup */
		ahc_outb(ahc, CRCCONTROL1, CRCVALCHKEN|CRCENDCHKEN|CRCREQCHKEN
					  |TARGCRCENDEN);
	}

	dscommand0 = ahc_inb(ahc, DSCOMMAND0);
	dscommand0 |= MPARCKEN|CACHETHEN;
	if ((ahc->features & AHC_ULTRA2) != 0) {

		/*
		 * DPARCKEN doesn't work correctly on
		 * some MBs so don't use it.
		 */
		dscommand0 &= ~DPARCKEN;
	}

	/*
	 * Handle chips that must have cache line
	 * streaming (dis/en)abled.
	 */
	if ((ahc->bugs & AHC_CACHETHEN_DIS_BUG) != 0)
		dscommand0 |= CACHETHEN;

	if ((ahc->bugs & AHC_CACHETHEN_BUG) != 0)
		dscommand0 &= ~CACHETHEN;

	ahc_outb(ahc, DSCOMMAND0, dscommand0);

	ahc->pci_cachesize =
	    ahc_pci_read_config(ahc->dev_softc, CSIZE_LATTIME,
				/*bytes*/1) & CACHESIZE;
	ahc->pci_cachesize *= 4;

	if ((ahc->bugs & AHC_PCI_2_1_RETRY_BUG) != 0
	 && ahc->pci_cachesize == 4) {

		ahc_pci_write_config(ahc->dev_softc, CSIZE_LATTIME,
				     0, /*bytes*/1);
		ahc->pci_cachesize = 0;
	}

	/*
	 * We cannot perform ULTRA speeds without the presense
	 * of the external precision resistor.
	 */
	if ((ahc->features & AHC_ULTRA) != 0) {
		uint32_t devconfig;

		devconfig = ahc_pci_read_config(ahc->dev_softc