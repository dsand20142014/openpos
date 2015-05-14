t(intf, timed_out_ipmb_broadcasts));
	out += sprintf(out, "sent_ipmb_responses:         %u\n",
		       ipmi_get_stat(intf, sent_ipmb_responses));
	out += sprintf(out, "handled_ipmb_responses:      %u\n",
		       ipmi_get_stat(intf, handled_ipmb_responses));
	out += sprintf(out, "invalid_ipmb_responses:      %u\n",
		       ipmi_get_stat(intf, invalid_ipmb_responses));
	out += sprintf(out, "unhandled_ipmb_responses:    %u\n",
		       ipmi_get_stat(intf, unhandled_ipmb_responses));
	out += sprintf(out, "sent_lan_commands:           %u\n",
		       ipmi_get_stat(intf, sent_lan_commands));
	out += sprintf(out, "sent_lan_command_errs:       %u\n",
		       ipmi_get_stat(intf, sent_lan_command_errs));
	out += sprintf(out, "retransmitted_lan_commands:  %u\n",
		       ipmi_get_stat(intf, retransmitted_lan_commands));
	out += sprintf(out, "timed_out_lan_commands:      %u\n",
		       ipmi_get_stat(intf, timed_out_lan_commands));
	out += sprintf(out, "sent_lan_responses:          %u\n",
		       ipmi_get_stat(intf, sent_lan_responses));
	out += sprintf(out, "handled_lan_responses:       %u\n",
		       ipmi_get_stat(intf, handled_lan_responses));
	out += sprintf(out, "invalid_lan_responses:       %u\n",
		       ipmi_get_stat(intf, invalid_lan_responses));
	out += sprintf(out, "unhandled_lan_responses:     %u\n",
		       ipmi_get_stat(intf, unhandled_lan_responses));
	out += sprintf(out, "handled_commands:            %u\n",
		       ipmi_get_stat(intf, handled_commands));
	out += sprintf(out, "invalid_commands:            %u\n",
		       ipmi_get_stat(intf, invalid_commands));
	out += sprintf(out, "unhandled_commands:          %u\n",
		       ipmi_get_stat(intf, unhandled_commands))