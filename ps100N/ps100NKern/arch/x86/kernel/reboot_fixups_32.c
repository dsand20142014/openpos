, 20080724 <--
#endif

				case CMDTHREAD_SET_CLIENT_MAC_ENTRY:
					{
						MAC_TABLE_ENTRY *pEntry;
						pEntry = (MAC_TABLE_ENTRY *)pData;

						{
							AsicRemovePairwiseKeyEntry(pAd, pEntry->apidx, (UCHAR)pEntry->Aid);
							if ((pEntry->AuthMode <= Ndis802_11AuthModeAutoSwitch) && (pEntry->WepStatus == Ndis802_11Encryption1Enabled))
							{
								UINT32 uIV = 0;
								PUCHAR  ptr;

								ptr = (PUCHAR) &uIV;
								*(ptr + 3) = (pAd->StaCfg.DefaultKeyId << 6);
								AsicUpdateWCIDIVEIV(pAd, pEntry->Aid, uIV, 0);
								AsicUpdateWCIDAttribute(pAd, pEntry->Aid, BSS0, pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].CipherAlg, FALSE);
							}
							else if (pEntry->AuthMode == Ndis802_11AuthModeWPANone)
							{
								UINT32 uIV = 0;
								PUCHAR  ptr;

								ptr = (PUCHAR) &uIV;
								*(ptr + 3) = (pAd->StaCfg.DefaultKeyId << 6);
								AsicUpdateWCIDIVEIV(pAd, pEntry->Aid, uIV, 0);
								AsicUpdateWCIDAttribute(pAd, pEntry->Aid, BSS0, pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].CipherAlg, FALSE);
							}
							else
							{
								//
								// Other case, disable engine.
								// Don't worry WPA key, we will add WPA Key after 4-Way handshaking.
								//
								USHORT   offset;
								offset = MAC_WCID_ATTRIBUTE_BASE + (pEntry->Aid * HW_WCID_ATTRI_SIZE);
								// RX_PKEY_MODE:0 for no security; RX_KEY_TAB:0 for shared key table; BSS_IDX:0
								RTUSBWriteMACRegister(pAd, offset, 0);
							}
						}

						AsicUpdateRxWCIDTable(pAd, pEntry->Aid, pEntry->Addr);
						printk("UpdateRxWCIDTable(): Aid=%d, Addr=%02x:%02x:%02x:%02x:%02x:%02x!\n", pEntry->Aid,
								pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2], pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);
					}
					break;

#ifdef RT30xx
// add by johnli, fix "in_interrupt" error when call "MacTableDeleteEntry" in Rx tasklet
				case CMDTHREAD_UPDATE_PROTECT:
					{
						AsicUpdateProtect(pAd, 0, (ALLN_SETPROTECT), TRUE, 0);
					}
					break;
// end johnli
#endif

				case OID_802_11_ADD_WEP:
					{
						UINT	i;
						UINT32	KeyIdx;
						PNDIS_802_11_WEP	pWepKey;

						DBGPRINT(RT_DEBUG_TRACE, ("CmdThread::OID_802_11_ADD_WEP  \n"));

						pWepKey = (PNDIS_802_11_WEP)pData;
						KeyIdx = pWepKey->KeyIndex & 0