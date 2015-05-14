BUG_TRACE, ("CmdThread : CMDTHREAD_SET_ASIC_WCID : WCID = %ld, SetTid  = %lx, DeleteTid = %lx.\n", SetAsicWcid.WCID, SetAsicWcid.SetTid, SetAsicWcid.DeleteTid));
						MACValue = (pAd->MacTab.Content[SetAsicWcid.WCID].Addr[3]<<24)+(pAd->MacTab.Content[SetAsicWcid.WCID].Addr[2]<<16)+(pAd->MacTab.Content[SetAsicWcid.WCID].Addr[1]<<8)+(pAd->MacTab.Content[SetAsicWcid.WCID].Addr[0]);
						DBGPRINT_RAW(RT_DEBUG_TRACE, ("1-MACValue= %x,\n", MACValue));
						RTUSBWriteMACRegister(pAd, offset, MACValue);
						// Read bitmask
						RTUSBReadMACRegister(pAd, offset+4, &MACRValue);
						if ( SetAsicWcid.DeleteTid != 0xffffffff)
							MACRValue &= (~SetAsicWcid.DeleteTid);
						if (SetAsicWcid.SetTid != 0xffffffff)
							MACRValue |= (SetAsicWcid.SetTid);
						MACRValue &= 0xffff0000;

						MACValue = (pAd->MacTab.Content[SetAsicWcid.WCID].Addr[5]<<8)+pAd->MacTab.Content[SetAsicWcid.WCID].Addr[4];
						MACValue |= MACRValue;
						RTUSBWriteMACRegister(pAd, offset+4, MACValue);

						DBGPRINT_RAW(RT_DEBUG_TRACE, ("2-MACValue= %x,\n", MACValue));
					}
					break;

				case CMDTHREAD_SET_ASIC_WCID_CIPHER:
					{
						RT_SET_ASIC_WCID_ATTRI	SetAsicWcidAttri;
						USHORT		offset;
						UINT32		MACRValue = 0;
						SHAREDKEY_MODE_STRUC csr1;
						SetAsicWcidAttri = *((PRT_SET_ASIC_WCID_ATTRI)(pData));

						if (SetAsicWcidAttri.WCID >= MAX_LEN_OF_MAC_TABLE)
							return;

						offset = MAC_WCID_ATTRIBUTE_BASE + ((UCHAR)SetAsicWcidAttri.WCID)*HW_WCID_ATTRI_SIZE;

						DBGPRINT_RAW(RT_DEBUG_TRACE, ("Cmd : CMDTHREAD_