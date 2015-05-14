_size)>0) {
                                /* FIXME: do something about it */
                                PRINT(KERN_ERR,
                                      "%s: packet data addr: %p size %Zd bytes "
				      "cross page boundary", __func__,
                                      packet->data, packet->data_size);
                        }
#endif
                        d->prg_cpu[idx]->end.address = cpu_to_le32(
                                pci_map_single(ohci->dev, packet->data,
                                               packet->data_size,
                                               PCI_DMA_TODEVICE));

                        d->prg_cpu[idx]->end.branchAddress = 0;
                        d->prg_cpu[idx]->end.status = 0;
                        if (d->branchAddrPtr)
                                *(d->branchAddrPtr) =
					cpu_to_le32(d->prg_bus[idx] | 0x3);
                        d->branchAddrPtr =
                                &(d->prg_cpu[idx]->end.branchAddress);
                } else { /* quadlet transmit */
                        if (packet->type == hpsb_raw)
                                d->prg_cpu[idx]->begin.control =
					cpu_to_le32(DMA_CTL_OUTPUT_LAST |
						    DMA_CTL_IMMEDIATE |
						    DMA_CTL_IRQ |
						    DMA_CTL_BRANCH |
						    (packet->header_size + 4));
                        else
                                d->prg_cpu[idx]->begin.control =
					cpu_to_le32(DMA_CTL_OUTPUT_LAST |
						    DMA_CTL_IMMEDIATE |
						    DMA_CTL_IRQ |
						    DMA_CTL_BRANCH |
						    packet->header_size);

                        if (d->branchAddrPtr)
                                *(d->branchAddrPtr) =
					cpu_to_le32(d->prg_bus[idx] | 0x2);
                        d->branchAddrPtr =
                                &(d->prg_cpu[idx]->begin.branchAddress);
                }

        } else { /* iso packet */
                d->prg_cpu[idx]->data[0] = packet->speed_code<<16 |
                        (packet->header[0] & 0xFFFF);
                d->prg_cpu[idx]->data[1] = packet->header[0] & 0xFFFF0000;
		header_le32_to_cpu(d->prg_cpu[idx]->data, packet->tcode);

                d->prg_cpu[idx]->begin.control =
			cpu_to_le32(DMA_CTL_OUTPUT_MORE |
				    DMA_CTL_IMMEDIATE | 0x8);
                d->prg_cpu[idx]->end.control =
			cpu_to_le32(DMA_CTL_OUTPUT_LAST |
				    DMA_CTL_UPDATE |
				    DMA_CTL_IRQ |
				    DMA_CTL_BRANCH |
				    packet->data_size);
                d->prg_cpu[idx]->end.address = cpu_to_le32(
				pci_map_single(ohci->dev, packet->data,
				packet->data_size, PCI_DMA_TODEVICE));

                d->prg_cpu[idx]->end.branchAddress = 0;
                d->prg_cpu[idx]->end.status = 0;
                DBGMSG("Iso xmit context info: header[%08x %08x]\n"
                       "        