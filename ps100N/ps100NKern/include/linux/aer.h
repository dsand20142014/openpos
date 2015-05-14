);

static void sym2_setup_params(void)
{
	char *p = excl_string;
	int xi = 0;

	while (p && (xi < 8)) {
		char *next_p;
		int val = (int) simple_strtoul(p, &next_p, 0);
		sym_driver_setup.excludes[xi++] = val;
		p = next_p;
	}

	if (safe_string) {
		if (*safe_string == 'y') {
			sym_driver_setup.max_tag = 0;
			sym_driver_setup.burst_order = 0;
			sym_driver_setup.scsi_led = 0;
			sym_driver_setup.scsi_diff = 1;
			sym_driver_setup.irq_mode = 0;
			sym_driver_setup.scsi_bus_check = 2;
			sym_driver_setup.host_id = 7;
			sym_driver_setup.verbose = 2;
			sym_driver_setup.settle_delay = 10;
			sym_driver_setup.use_nvram = 1;
		} else if (*safe_string != 'n') {
			printk(KERN_WARNING NAME53C8XX "Ignoring parameter %s"
					" passed to safe option", safe_string);
		}