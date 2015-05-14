em28xx, cx2388x and cx23418 */
	if (eeprom_data[0] == 0x1a &&
	    eeprom_data[1] == 0xeb &&
	    eeprom_data[2] == 0x67 &&
	    eeprom_data[3] == 0x95)
		start = 0xa0; /* Generic em28xx offset */
	else if ((eeprom_data[0] & 0xe1) == 0x01 &&
		 eeprom_data[1] == 0x00 &&
		 eeprom_data[2] == 0x00 &&
		 eeprom_data[8] == 0x84)
		start = 8; /* Generic cx2388x offset */
	else if (eeprom_data[1] == 0x70 &&
		 eeprom_data[2] == 0x00 &&
		 eeprom_data[4] == 0x74 &&
		 eeprom_data[8] == 0x84)
		start = 8; /* Generic cx23418 offset (models 74xxx) */
	else
		start = 0;

	for (i = start; !done && i < 256; i += len) {
		if (eeprom_data[i] == 0x84) {
			len = eeprom_data[i + 1] + (eeprom_data[i + 2] << 8);
			i += 3;
		} else if ((eeprom_data[i] & 0xf0) == 0x70) {
			if (eeprom_data[i] & 0x08) {
				/* verify checksum! */
				done = 1;
				break;
			}
			len = eeprom_data[i] & 0x07;
			++i;
		} else {
			tveeprom_warn("Encountered bad packet header [%02x]. "
				"Corrupt or not a Hauppauge eeprom.\n",
				eeprom_data[i]);
			return;
		}

		if (debug) {
			tveeprom_info("Tag [%02x] + %d bytes:",
					eeprom_data[i], len - 1);
			for (j = 1; j < len; j++)
				printk(KERN_CONT " %02x", eeprom_data[i + j]);
			printk(KERN_CONT "\n");
		}

		/* process by tag */
		tag = eeprom_data[i];
		switch (tag) {
		case 0x00:
			/* tag: 'Comprehensive' */
			tuner1 = eeprom_data[i+6];
			t_format1 = eeprom_data[i+5];
			tvee->has_radio = eeprom_data[i+len-1];
			/* old style tag, don't know how to detect
			IR presence, mark as unknown. */
			tvee->has_ir = 0;
			tvee->model =
				eeprom_data[i+8] +
				(eeprom_data[i+9] << 8);
			tvee->revision = eeprom_data[i+10] +
				(eeprom_data[i+11] << 8) +
				(eeprom_data[i+12] << 16);
			break;

		case 0x01:
			/* tag: 'SerialID' */
			tvee->serial_number =
				eeprom_data[i+6] +
				(eeprom_data[i+7] << 8) +
				(eeprom_data[i+8] << 16);
			break;

		case 0x02:
			/* tag 'AudioInfo'
			Note mask with 0x7F, high bit used on some older models
			to indicate 4052 mux was removed in favor of using MSP
			inputs directly. */
			audioic = eeprom_data[i+2] & 0x7f;
			if (audioic < ARRAY_SIZE(audioIC))
				tvee->audio_processor = audioIC[audioic].id;
			else
				tvee->audio_processor = V4L2_IDENT_UNKNOWN;
			break;

		/* case 0x03: tag 'EEInfo' */

		case 0x04:
			/* tag 'SerialID2' */
			tvee->serial_number =
				eeprom_data[i+5] +
				(eeprom_data[i+6] << 8) +
				(eeprom_data[i+7] << 16);

			if ((eeprom_data[i + 8] & 0xf0) &&
					(tvee->serial_number < 0xffffff)) {
				tvee->MAC_address[0] = 0x00;
				tvee->MAC_address[1] = 0x0D;
				tvee->MAC_address[2] = 0xFE;
				tvee->MAC_address[3] = eeprom_data[i + 7];
				tvee->MAC_address[4] = eeprom_data[i + 6];
				tvee->MAC_address[5] = eeprom_data[i + 5];
				tvee->has_MAC_address = 1;
			}
			break;

		case 0x05:
			/* tag 'Audio2'
			Note mask with 0x7F, high bit used on some older models
			to indicate 4052 mux was removed in favor of using MSP
			inputs directly. */
			audioic = eeprom_data[i+1] & 0x7f;
			if (audioic < ARRAY_SIZE(audioIC))
				tvee->audio_processor = audioIC[audioic].id;
			else
				tvee->audio_processor = V4L2_IDENT_UNKNOWN;

			break;

		case 0x06:
			/* tag 'ModelRev' */
			tvee->model =
				eeprom_data[i + 1] +
				(eeprom_data[i + 2] << 8) +
				(eeprom_data[i + 3] << 16) +
				(eeprom_data[i + 4] << 24);
			tvee->revision =
				eeprom_data[i + 5] +
				(eeprom_data[i + 6] << 8) +
				(eeprom_data[i + 7] << 16);
			break;

		case 0x07:
			/* tag 'Details': according to Hauppauge not interesting
			on any PCI-era or later boards. */
			break;

		/* there is no tag 0x08 defined */

		case 0x09:
			/* tag 'Video' */
			tvee->decoder_processor = eeprom_data[i + 1];
			break;

		case 0x0a:
			/* tag 'Tuner' */
			if (beenhere == 0) {
				tuner1 = eeprom_data[i + 2];
				t_format1 = eeprom_data[i + 1];
				beenhere = 1;
			} else {
				/* a second (radio) tuner may be present */
				tuner2 = eeprom_data[i + 2];
				t_format2 = eeprom_data[i + 1];
				/* not a TV tuner? */
				if (t_format2 == 0)
					tvee->has_radio = 1; /* must be radio */
	tnum;
		intrfc = ipx_primary_net;
	} else {
		rt = ipxrtr_lookup(usipx->sipx_network);
		rc = -ENETUNREACH;
		if (!rt)
			goto out;
		intrfc = rt->ir_intrfc;
	}

	ipxitf_hold(intrfc);
	ipx_offset = intrfc->if_ipx_offset;
	size = sizeof(struct ipxhdr) + len + ipx_offset;

	skb = sock_alloc_send_skb(sk, size, noblock, &rc);
	if (!skb)
		goto out_put;

	skb_reserve(skb, ipx_offset);
	skb->sk = sk;

	/* Fill in IPX header */
	skb_reset_network_header(skb);
	skb_reset_transport_header(skb);
	skb_put(skb, sizeof(struct ipxhdr));
	ipx = ipx_hdr(skb);
	ipx->ipx_pktsize = htons(len + sizeof(struct ipxhdr));
	IPX_SKB_CB(skb)->ipx_tctrl = 0;
	ipx->ipx_type 	 = usipx->sipx_type;

	IPX_SKB_CB(skb)->last_hop.index = -1;
#ifdef CONFIG_IPX_INTERN
	IPX_SKB_CB(skb)->ipx_source_net = ipxs->intrfc->if_netnum;
	memcpy(ipx->ipx_source.node, ipxs->node, IPX_NODE_LEN);
#else
	rc = ntohs(ipxs->port);
	if (rc == 0x453 || rc == 0x452) {
		/* RIP/SAP special handling for mars_nwe */
		IPX_SKB_CB(skb)->ipx_source_net = intrfc->if_netnum;
		memcpy(ipx->ipx_source.node, intrfc->if_node, IPX_NODE_LEN);
	} else {
		IPX_SKB_CB(skb)->ipx_source_net = ipxs->intrfc->if_netnum;
		memcpy(ipx->ipx_source.node, ipxs->intrfc->if_node,
			IPX_NODE_LEN);
	}
#endif	/* CONFIG_IPX_INTERN */
	ipx->ipx_source.sock		= ipxs->port;
	IPX_SKB_CB(skb)->ipx_dest_net	= usipx->sipx_network;
	memcpy(ipx->ipx_dest.node, usipx->sipx_node, IPX_NODE_LEN);
	ipx->ipx_dest.sock		= usipx->sipx_port;

	rc = memcpy_fromiovec(skb_put(skb, len), iov, len);
	if (rc) {
		kfree_skb(skb);
		goto out_put;
	}

	/* Apply checksum. Not allowed on 802.3 links. */
	if (sk->sk_no_check || intrfc->if_dlink_type == htons(IPX_FRAME_8023))
		ipx->ipx_checksum = htons(0xFFFF);
	else
		ipx->ipx_checksum = ipx_cksum(ipx, len + sizeof(struct ipxhdr));

	rc = ipxitf_send(intrfc, skb, (rt && rt->ir_routed) ?
			 rt->ir_router_node : ipx->ipx_dest.node);
out_put:
	ipxitf_put(intrfc);
	if (rt)
		ipxrtr_put(rt);
out:
	return rc;
}

/*
 * We use a normal struct rtentry for route handling
 */
int ipxrtr_ioctl(unsigned int cmd, void __user *arg)
{
	struct rtentry rt;	/* Use these to behave like 'other' stacks */
	struct sockaddr_ipx *sg, *st;
	int rc = -EFAULT;

	if (copy_from_user(&rt, arg, sizeof(rt)))
		goto out;

	sg = (struct sockaddr_ipx *)&rt.rt_gateway;
	st = (struct sockaddr_ipx *)&rt.rt_dst;

	rc = -EINVAL;
	if (!(rt.rt_flags & RTF_GATEWAY) || /* Direct routes are fixed */
	    sg->sipx_family != AF_IPX ||
	    st->sipx_family != AF_IPX)
		goto out;

	switch (cmd) {
	case SIOCDELRT:
		rc = ipxrtr_delete(st->sipx_network);
		break;
	case SIOCADDRT: {
		struct ipx_route_definition f;
		f.ipx_network		= st->sipx_network;
		f.ipx_router_network	= sg->sipx_network;
		memcpy(f.ipx_router_node, sg->sipx_node, IPX_NODE_LEN);
		rc = ipxrtr_create(&f);
		break;
	}
	}

out:
	return rc;
}
