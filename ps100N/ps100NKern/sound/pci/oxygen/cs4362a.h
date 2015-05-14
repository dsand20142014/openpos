{ &dibusb_dib3000mb_table[3], NULL },
		},
		{	"DiBcom USB1.1 DVB-T reference design (MOD3000)",
			{ &dibusb_dib3000mb_table[5],  NULL },
			{ &dibusb_dib3000mb_table[6],  NULL },
		},
		{	"KWorld V-Stream XPERT DTV - DVB-T USB1.1",
			{ &dibusb_dib3000mb_table[7], NULL },
			{ &dibusb_dib3000mb_table[8], NULL },
		},
		{	"Grandtec USB1.1 DVB-T",
			{ &dibusb_dib3000mb_table[9],  &dibusb_dib3000mb_table[11], NULL },
			{ &dibusb_dib3000mb_table[10], &dibusb_dib3000mb_table[12], NULL },
		},
		{	"Unkown USB1.1 DVB-T device ???? please report the name to the author",
			{ &dibusb_dib3000mb_table[13], NULL },
			{ &dibusb_dib3000mb_table[14], NULL },
		},
		{	"TwinhanDTV USB-Ter USB1.1 / Magic Box I / HAMA USB1.1 DVB-T device",
			{ &dibusb_dib3000mb_table[15], &dibusb_dib3000mb_table[17], NULL},
			{ &dibusb_dib3000mb_table[16], &dibusb_dib3000mb_table[18], NULL},
		},
		{	"Artec T1 USB1.1 TVBOX with AN2135",
			{ &dibusb_dib3000mb_table[19], NULL },
			{ &dibusb_dib3000mb_table[20], NULL },
		},
		{	"VideoWalker DVB-T USB",
			{ &dibusb_dib3000mb_table[25], NULL },
			{ &dibusb_dib3000mb_table[26], NULL },
		},
	}
};

static struct dvb_usb_device_properties dibusb1_1_an2235_properties = {
	.caps = DVB_USB_IS_AN_I2C_ADAPTER,
	.usb_ctrl = CYPRESS_AN2235,

	.firmware = "dvb-usb-dibusb-an2235-01.fw",

	.num_adapters = 1,
	.adapter = {
		{
			.caps = DVB_USB_ADAP_PID_FILTER_CAN_BE_TURNED_OFF | DVB_USB_ADAP_HAS_PID_FILTER,
			.pid_filter_count = 16,

			.streaming_ctrl   = dibusb_streaming_ctrl,
			.pid_filter       = dibusb_pid_filter,
			.pid_filter_ctrl  = dibusb_pid_filter_ctrl,
			.frontend_attach  = dibusb_dib3000mb_frontend_attach,
			.tuner_attach     = dibusb_tuner_probe_and_attach,

			/* parameter for the MPEG2-data transfer */
			.stream = {
				.type = USB_BULK,
				.count = 7,
				.endpoint = 0x02,
				.u = {
					.bulk = {
						.buffersize = 4096,
					}
				}
			},
			.size_of_priv     = sizeof(struct dibusb_state),
		},
	},
	.power_ctrl       = dibusb_power_ctrl,

	.rc_interval      = DEFAULT