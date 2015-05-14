#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "iwlib.h"
#include "wifi.h"
#include "param.h"
#include "sysparam.h"
#include "devSignal.h"


#define IW_SCAN_HACK		0x8000
#define MAX_BUF_SIZE		10240
#define WIFIINFO_FILE_NAME	"/etc/usbwifi/wpa_supplicant.conf"
#define WIFI_LIST_PATH		"/etc/usbwifi/wifilist.ini"

#define WIFI_SIGNAL_LEVEL1	35
#define WIFI_SIGNAL_LEVEL2	55
#define WIFI_SIGNAL_LEVEL3	75

/*
 * Scan state and meta-information, used to decode events...
 */
typedef struct iwscan_state
{
  /* State */
  int			ap_num;		/* Access Point number 1->N */
  int			val_index;	/* Value in table 0->(N-1) */
} iwscan_state;

/*
 * Bit to name mapping
 */
typedef struct iwmask_name
{
  unsigned int	mask;	/* bit mask for the value */
  const char *	name;	/* human readable name for the value */
} iwmask_name;

/*
 * Types of authentication parameters
 */
typedef struct iw_auth_descr
{
  int				value;		/* Type of auth value */
  const char *			label;		/* User readable version */
  const struct iwmask_name *	names;		/* Names for this value */
  const int			num_names;	/* Number of names */
} iw_auth_descr;



static inline void
_set_scanning_token(struct stream_descr *	stream,	/* Stream of events */
		     struct iw_event *		event,	/* Extracted token */
		     struct iwscan_state *	state,
		     struct iw_range *	iw_range,	/* Range info */
		     int has_range, WIFISCAN *wifiscan)
{
	char buffer[128];	/* Temporary buffer */

  /* Now, let's decode the event */
	switch(event->cmd)
	{
		//获取地址
		case SIOCGIWAP:
    		strncpy(wifiscan->addressMAC,iw_saether_ntop(&event->u.ap_addr, buffer),sizeof(wifiscan->addressMAC)-1);
			state->ap_num++;
      		break;
		//获取频率
		case SIOCGIWFREQ:
		{
			double	freq;			/* Frequency/channel */
			int	 channel = -1;		/* Converted to channel */
			freq = iw_freq2float(&(event->u.freq));
			/* Convert to channel if possible */
			if(has_range)
				channel = iw_freq_to_channel(freq, iw_range);
			iw_print_freq(buffer, sizeof(buffer), freq, channel, event->u.freq.flags);
			strncpy(wifiscan->frequency,buffer,sizeof(wifiscan->frequency)-1);
			break;
		}
		//模式
		case SIOCGIWMODE:
			/* Note : event->u.mode is unsigned, no need to check <= 0 */
			if(event->u.mode >= IW_NUM_OPER_MODE)
				event->u.mode = IW_NUM_OPER_MODE;
      		break;
      	//协议
		case SIOCGIWNAME:
      		//printf("Protocol:%-1.16s\n", event->u.name);
      		break;
      	//essid
		case SIOCGIWESSID:
		{
			char essid[IW_ESSID_MAX_SIZE+1];
			memset(essid, '\0', sizeof(essid));
			if((event->u.essid.pointer) && (event->u.essid.length))
				memcpy(essid, event->u.essid.pointer, event->u.essid.length);
			if(event->u.essid.flags)
	  			strncpy(wifiscan->essid,essid,sizeof(wifiscan->essid)-1);
      		break;
      	}
      	//是否加密
      	case SIOCGIWENCODE:
		{
			unsigned char	key[IW_ENCODING_TOKEN_MAX];
				if(event->u.data.pointer)
					memcpy(key, event->u.data.pointer, event->u.data.length);
				else
					event->u.data.flags |= IW_ENCODE_NOKEY;
				if(event->u.data.flags & IW_ENCODE_DISABLED)
					wifiscan->encryption[0] = 0x30;
				else
					wifiscan->encryption[0] = 0x31;
				wifiscan->encryption[1] = 0x0;
		}
      		break;
		//获取波特率
		case SIOCGIWRATE:
			iw_print_bitrate(buffer, sizeof(buffer), event->u.bitrate.value);
			strncpy(wifiscan->bitRates,buffer,sizeof(wifiscan->bitRates)-1);
			if(stream->value == NULL)
				state->val_index = 0;
			else
				state->val_index++;
			break;
		//获取信号质量
		case IWEVQUAL:
			//iw_print_stats(buffer, sizeof(buffer), &event->u.qual, iw_range, has_range);
		{
			if(has_range && ((event->u.qual.level != 0)
					|| (event->u.qual.updated & (IW_QUAL_DBM | IW_QUAL_RCPI))))
			{
				/* Deal with quality : always a relative value */
				if(!(event->u.qual.updated & IW_QUAL_QUAL_INVALID))
				{
					snprintf(wifiscan->quality,sizeof(wifiscan->quality)-1,"%d/%d",event->u.qual.qual, iw_range->max_qual.qual);
				}

				/* Check if the statistics are in RCPI (IEEE 802.11k) */
				if(event->u.qual.updated & IW_QUAL_RCPI)
				{
					/* Deal with signal level in RCPI */
					/* RCPI = int{(Power in dBm +110)*2} for 0dbm > Power > -110dBm */
					if(!(event->u.qual.updated & IW_QUAL_LEVEL_INVALID))
					{
						double rcpilevel = (event->u.qual.level / 2.0) - 110.0;
						
						if(rcpilevel <= WIFI_SIGNAL_LEVEL1)
							wifiscan->sigLevNum = 4;
						else if(rcpilevel <= WIFI_SIGNAL_LEVEL2)
							wifiscan->sigLevNum = 3;
						else if(rcpilevel <= WIFI_SIGNAL_LEVEL3)
							wifiscan->sigLevNum = 2;
						else
							wifiscan->sigLevNum = 1;
						
						snprintf(wifiscan->sigLev,sizeof(wifiscan->sigLev)-1,"%g dBm",rcpilevel);
	
					}
	
					/* Deal with noise level in dBm (absolute power measurement) */
					if(!(event->u.qual.updated & IW_QUAL_NOISE_INVALID))
					{
						//噪音单位dBm
						double	rcpinoise = (event->u.qual.noise / 2.0) - 110.0;
					}
				}
				else
				{
					/* Check if the statistics are in dBm */
					if((event->u.qual.updated & IW_QUAL_DBM)
						|| (event->u.qual.level > iw_range->max_qual.level))
					{
						/* Deal with signal level in dBm  (absolute power measurement) */
						if(!(event->u.qual.updated & IW_QUAL_LEVEL_INVALID))
						{
							int	dblevel = event->u.qual.level;
							/* Implement a range for dBm [-192; 63] */
							if(event->u.qual.level >= 64)
								dblevel -= 0x100;
								
							if(dblevel <= WIFI_SIGNAL_LEVEL1)
								wifiscan->sigLevNum = 4;
							else if(dblevel <= WIFI_SIGNAL_LEVEL2)
								wifiscan->sigLevNum = 3;
							else if(dblevel <= WIFI_SIGNAL_LEVEL3)
								wifiscan->sigLevNum = 2;
							else
								wifiscan->sigLevNum = 1;
						
							snprintf(wifiscan->sigLev,sizeof(wifiscan->sigLev)-1,"%g dBm",dblevel);
						}
	
						/* Deal with noise level in dBm (absolute power measurement) */
						if(!(event->u.qual.updated & IW_QUAL_NOISE_INVALID))
						{
							int	dbnoise = event->u.qual.noise;
							/* Implement a range for dBm [-192; 63] */
							if(event->u.qual.noise >= 64)
			    				dbnoise -= 0x100;
						}
					}
					else
					{
						/* Deal with signal level as relative value (0 -> max) */
						if(!(event->u.qual.updated & IW_QUAL_LEVEL_INVALID))
						{
							if(event->u.qual.level >= 65 )
								wifiscan->sigLevNum = 4;
							else if(event->u.qual.level >= 45)
								wifiscan->sigLevNum = 3;
							else if(event->u.qual.level >= 2)
								wifiscan->sigLevNum = 2;
							else
								wifiscan->sigLevNum = 1;
							snprintf(wifiscan->sigLev,sizeof(wifiscan->sigLev)-1,"%d/%d",event->u.qual.level, iw_range->max_qual.level);
						}
					}
				}
			}
      		break;
      	}
		default:
    		break;
		//event->cmd);
   }
}

int WIFI_scan(char *ifname, WIFISCAN wifiscan[], int maxSize)
{
	int skfd;
	struct iwreq		wrq;
	struct iw_scan_req    scanopt;		/* Options for 'set' */
	int			scanflags = 0;		/* Flags for scan */
	unsigned char *	buffer = NULL;		/* Results */
	int			buflen = IW_SCAN_MAX_DATA; /* Min for compat WE<17 */
	struct iw_range	range;
	int			has_range;
	struct timeval	tv;				/* Select timeout */
	int			timeout = 15000000;		/* 15s */
	int cn = -1, i = 0, n = 0;
	
	
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
	{
		perror("socket");
		return -1;
	}
	
	/* Get range stuff */
	has_range = (iw_get_range_info(skfd, ifname, &range) >= 0);
	
	/* Check if the interface could support scanning. */
	if((!has_range) || (range.we_version_compiled < 14))
	{
		fprintf(stderr, "%-8.16s  Interface doesn't support scanning.\n\n",ifname);
		return(-1);
	}
	
	/* Init timeout value -> 250ms between set and first get */
	tv.tv_sec = 0;
	tv.tv_usec = 250000;
	
	/* Clean up set args */
	memset(&scanopt, 0, sizeof(scanopt));
	
	/* Check if we have scan options */
	if(scanflags)
	{
		wrq.u.data.pointer = (caddr_t) &scanopt;
		wrq.u.data.length = sizeof(scanopt);
		wrq.u.data.flags = scanflags;
	}
	else
	{
		wrq.u.data.pointer = NULL;
		wrq.u.data.flags = 0;
		wrq.u.data.length = 0;
	}
	
	/* If only 'last' was specified on command line, don't trigger a scan */
	if(scanflags == IW_SCAN_HACK)
	{
		/* Skip waiting */
		tv.tv_usec = 0;
	}
	else
	{
		/* Initiate Scanning */
		if(iw_set_ext(skfd, ifname, SIOCSIWSCAN, &wrq) < 0)
		{
			if((errno != EPERM) || (scanflags != 0))
			{
				fprintf(stderr, "%-8.16s  Interface doesn't support scanning : %s\n\n",
				ifname, strerror(errno));
				return(-1);
			}
			/* If we don't have the permission to initiate the scan, we may
			* still have permission to read left-over results.
			* But, don't wait !!! */
			tv.tv_usec = 0;
		}
	}
	timeout -= tv.tv_usec;

	/* Forever */
	while(1)
	{
		fd_set rfds;/* File descriptors for select */
		int last_fd;/* Last fd */
		int ret;
		
		/* Guess what ? We must re-generate rfds each time */
		FD_ZERO(&rfds);
		last_fd = -1;
		
		/* In here, add the rtnetlink fd in the list */
		
		/* Wait until something happens */
		ret = select(last_fd + 1, &rfds, NULL, NULL, &tv);
		
		/* Check if there was an error */
		if(ret < 0)
		{
			if(errno == EAGAIN || errno == EINTR)
				continue;
			fprintf(stderr, "Unhandled signal - exiting...\n");
			return(-1);
		}
		
		/* Check if there was a timeout */
		if(ret == 0)
		{
			unsigned char *	newbuf;
		
			realloc:
			/* (Re)allocate the buffer - realloc(NULL, len) == malloc(len) */
			newbuf = realloc(buffer, buflen);
			if(newbuf == NULL)
			{
				if(buffer)
				free(buffer);
				fprintf(stderr, "%s: Allocation failed\n", __FUNCTION__);
				return(-1);
			}
			buffer = newbuf;
		
			/* Try to read the results */
			wrq.u.data.pointer = buffer;
			wrq.u.data.flags = 0;
			wrq.u.data.length = buflen;
			if(iw_get_ext(skfd, ifname, SIOCGIWSCAN, &wrq) < 0)
			{
				/* Check if buffer was too small (WE-17 only) */
				if((errno == E2BIG) && (range.we_version_compiled > 16))
				{
					/* Some driver may return very large scan results, either
					* because there are many cells, or because they have many
					* large elements in cells (like IWEVCUSTOM). Most will
					* only need the regular sized buffer. We now use a dynamic
					* allocation of the buffer to satisfy everybody. Of course,
					* as we don't know in advance the size of the array, we try
					* various increasing sizes. Jean II */
					
					/* Check if the driver gave us any hints. */
					if(wrq.u.data.length > buflen)
						buflen = wrq.u.data.length;
					else
						buflen *= 2;
					
					/* Try again */
					goto realloc;
				}
		
				/* Check if results not available yet */
				if(errno == EAGAIN)
				{
					/* Restart timer for only 100ms*/
					tv.tv_sec = 0;
					tv.tv_usec = 100000;
					timeout -= tv.tv_usec;
					if(timeout > 0)
					continue;	/* Try again later */
				}
				
				/* Bad error */
				free(buffer);
				fprintf(stderr, "%-8.16s  Failed to read scan data : %s\n\n",
				ifname, strerror(errno));
				return(-2);
			}
			else
				/* We have the results, go to process them */
				break;
		}
		
		/* In here, check if event and event type
		* if scan event, read results. All errors bad & no reset timeout */
	}

	if(wrq.u.data.length)
	{
		struct iw_event		iwe;
		struct stream_descr	stream;
		struct iwscan_state	state = { .ap_num = 1, .val_index = 0 };
		int ret;

		//printf("%-8.16s  Scan completed :\n", ifname);
		iw_init_event_stream(&stream, (char *) buffer, wrq.u.data.length);
		do
		{
			/* Extract an event and print it */
			ret = iw_extract_event_stream(&stream, &iwe, range.we_version_compiled);
			if(ret > 0)
			{
				if(iwe.cmd == SIOCGIWAP)
					cn++;
				if(cn >= maxSize)
					return maxSize;
				_set_scanning_token(&stream, &iwe, &state, &range, has_range, &wifiscan[cn]);
				
			}
		}
		while(ret > 0);
	}

	free(buffer);

	for(i=0;i<=cn-n;i++)
	{
		if(wifiscan[i].essid[0] == '\0' || wifiscan[i].sigLevNum == 0)
		{
			if(i == cn-n)
			{
				n++;
				break;
			}
			memset(&wifiscan[i],0,sizeof(wifiscan[i]));
			strncpy(wifiscan[i].addressMAC,wifiscan[cn-n].addressMAC,sizeof(wifiscan[i].addressMAC));
			strncpy(wifiscan[i].essid,wifiscan[cn-n].essid,sizeof(wifiscan[i].essid));
			strncpy(wifiscan[i].encryption,wifiscan[cn-n].encryption,sizeof(wifiscan[i].encryption));
			strncpy(wifiscan[i].frequency,wifiscan[cn-n].frequency,sizeof(wifiscan[i].frequency));
			strncpy(wifiscan[i].bitRates,wifiscan[cn-n].bitRates,sizeof(wifiscan[i].bitRates));
			strncpy(wifiscan[i].quality,wifiscan[cn-n].quality,sizeof(wifiscan[i].quality));
			strncpy(wifiscan[i].sigLev,wifiscan[cn-n].sigLev,sizeof(wifiscan[i].sigLev));
			wifiscan[i].sigLevNum = wifiscan[cn-n].sigLevNum;
			i--;
			n++;
		}
	}
	if(n > 0)
	{
		for(i=cn-n+1;i<=cn;i++)
			memset(&wifiscan[i],0,sizeof(wifiscan[i]));
	}
	
	return (cn+1-n);
}


int WIFI_check_list(WIFIINFO *wifiinfo)
{
	int ret = -1;
	int etcFile, etcSection;

	unsigned char ucParm[50];
	if(strlen(wifiinfo->addressMAC) < 17 || wifiinfo->essid[0] == '\0')
		return -2;
	
	//查找列表中是否有此wifi信息
	if(access(WIFI_LIST_PATH,F_OK) != 0)
	{
		//创建文件
		int fd = creat(WIFI_LIST_PATH,O_CREAT|O_RDWR);
		if(fd > 0)
			close(fd);
		memset(ucParm,0,sizeof(ucParm));
		sprintf(ucParm,"chmod 777 %s",WIFI_LIST_PATH);
		system(ucParm);
	}
	
	etcFile = Os__param_loadFullPath(WIFI_LIST_PATH);
	if(etcFile > 0)
	{
		etcSection = Os__param_findSection(etcFile,wifiinfo->addressMAC,0);
		if(etcSection > 0)
		{
			memset(ucParm,0,sizeof(ucParm));
  			Os__param_get(etcSection,"ESSID",ucParm,sizeof(ucParm));
  			if(ucParm[0] != '\0' && strcmp(wifiinfo->essid,ucParm) == 0)
  			{
  				Os__param_get(etcSection,"ENCRYPTION",wifiinfo->encryption,1);
  				Os__param_get(etcSection,"PASSWORD",wifiinfo->psk,sizeof(wifiinfo->psk));
  				Os__param_get(etcSection,"CURLINK",wifiinfo->curLink,1);
				ret = 0;
			}
		}
		Os__param_unload(etcFile);
	}
	return ret;
}


int _WIFI_set_wpa(WIFIINFO *wifiinfo)
{

	unsigned char tmpBuf[MAX_BUF_SIZE];
	FILE *fd = NULL, *wfd = NULL;
	
	//打开要修改的文件
	fd = fopen(WIFIINFO_FILE_NAME,"w+");
	if(fd == NULL)
	{
		return -1;
	}
	memset(tmpBuf,0,sizeof(tmpBuf));
	sprintf(tmpBuf,"ctrl_interface=/var/run/wpa_supplicant\n");
	sprintf(tmpBuf,"%seapol_version=1\n",tmpBuf);
	sprintf(tmpBuf,"%sap_scan=1\n",tmpBuf);
	sprintf(tmpBuf,"%sfast_reauth=1\n",tmpBuf);
	sprintf(tmpBuf,"%sctrl_interface=/var/run/\n",tmpBuf);
	sprintf(tmpBuf,"%sctrl_interface_group=0\n",tmpBuf);
	sprintf(tmpBuf,"%snetwork={\n",tmpBuf);
	sprintf(tmpBuf,"%sssid=\"%s\"\n",tmpBuf,wifiinfo->essid);
	sprintf(tmpBuf,"%sscan_ssid=1\n",tmpBuf);

	fwrite(tmpBuf,1,strlen(tmpBuf),fd);
	
	memset(tmpBuf,0,sizeof(tmpBuf));
	if(wifiinfo->encryption[0] == '0')
	{
		sprintf(tmpBuf,"key_mgmt=NONE\n");
		sprintf(tmpBuf,"%spsk=\"nopassword\"",tmpBuf);
	}
	else
	{
		sprintf(tmpBuf,"key_mgmt=WPA-EAP WPA-PSK IEEE8021X NONE\n");
		sprintf(tmpBuf,"%spairwise=TKIP CCMP\n",tmpBuf);
		sprintf(tmpBuf,"%sgroup=CCMP TKIP WEP104 WEP40 \n",tmpBuf);
		sprintf(tmpBuf,"%spsk=\"%s\"",tmpBuf,wifiinfo->psk);
	}
	sprintf(tmpBuf,"%s\n}",tmpBuf);
	fwrite(tmpBuf,1,strlen(tmpBuf),fd);
	fclose(fd);
	return 0;
}



int WIFI_set_info(WIFIINFO *wifiinfo)
{
	int ret = 0;
	int etcFile, etcSection;
	WIFIINFO tmpWifi;
	
	memset(&tmpWifi,0,sizeof(tmpWifi));
	strncpy(tmpWifi.addressMAC,wifiinfo->addressMAC,sizeof(tmpWifi.addressMAC));
	strncpy(tmpWifi.essid,wifiinfo->essid,sizeof(tmpWifi.essid));
	ret = WIFI_check_list(&tmpWifi);

	if(ret == -2 || (wifiinfo->encryption[0] == 0x31 && strlen(wifiinfo->psk) < 8))
		return -1;
	else
	{
		etcFile = Os__param_loadFullPath(WIFI_LIST_PATH);
		if(etcFile > 0)
		{
			//设置前先清除curlink标志
			char addressMAC[50];
			memset(addressMAC,0,sizeof(addressMAC));
			if(WIFI_get_curMAC(addressMAC) == 0)
			{
				etcSection = Os__param_findSection(etcFile,addressMAC,0);
				if(etcSection > 0)
				{
					Os__param_set(etcSection,"CURLINK","0");
				}
			}
			
			etcSection = Os__param_findSection(etcFile,wifiinfo->addressMAC,1);
			if(etcSection > 0)
			{
	  			Os__param_set(etcSection,"ESSID",wifiinfo->essid);
	  			Os__param_set(etcSection,"ENCRYPTION",wifiinfo->encryption);
	  			Os__param_set(etcSection,"PASSWORD",wifiinfo->psk);
	  			Os__param_set(etcSection,"CURLINK","1");
			}

			Os__param_saveFullPath(etcFile,WIFI_LIST_PATH);
			Os__param_unload(etcFile);
		}
		else
			return -1;
	}
	_WIFI_set_wpa(wifiinfo);
	return 0;
}

int WIFI_remove_info(char *addressMAC)
{
	int etcFile, etcSection, ret = -1;
	
	if(strlen(addressMAC) < 17)
		return ret;
	etcFile = Os__param_loadFullPath(WIFI_LIST_PATH);
	if(etcFile > 0)
	{
		etcSection = Os__param_findSection(etcFile,addressMAC,0);
		if(etcSection > 0)
		{
			Os__param_rmSection(etcFile,addressMAC);
		}
		Os__param_saveFullPath(etcFile,WIFI_LIST_PATH);
		Os__param_unload(etcFile);
		ret = 0;
	}
		
	return ret;
}

int WIFI_get_curMAC(char *addressMAC)
{
	char macList[20][50], curLink[2];
	int etcFile, etcSection , cn = 0, i;
	
	memset(macList,0,sizeof(macList));
	
	etcFile = Os__param_loadFullPath(WIFI_LIST_PATH);
	if(etcFile > 0)
	{
		cn = Os__param_getAllSection(etcFile,macList,20);

		for(i=0;i<cn;i++)
		{
			etcSection = Os__param_findSection(etcFile,macList[i],0);
			if(etcSection > 0)
			{
				memset(curLink,0,sizeof(curLink));
				Os__param_get(etcSection,"CURLINK",curLink,1);
				if(curLink[0] == 0x31)
				{
					strncpy(addressMAC,macList[i],sizeof(macList[i]));
					return 0;
				}
			}
		}
		Os__param_unload(etcFile);
	}
	return -1;
}

void WIFI_set_suspendModel(int model)
{
	SYS_SETTING sysSetting;
	//设置setting.ini中内容
	memset(&sysSetting,0,sizeof(SYS_SETTING));
	strcpy(sysSetting.section,"WIFI_SETTING");
	strcpy(sysSetting.key,"WIFISUSPENDTYPE");
	if(model == 1)
	{
		sysSetting.value[0] = 0x31;
	}
	else
	{
		sysSetting.value[0] = 0x30;
	}
	PARAM_setting_set(&sysSetting);
}

int WIFI_get_suspendModel(void)
{
	SYS_SETTING sysSetting;
	//设置setting.ini中内容
	memset(&sysSetting,0,sizeof(SYS_SETTING));
	strcpy(sysSetting.section,"WIFI_SETTING");
	strcpy(sysSetting.key,"WIFISUSPENDTYPE");
	PARAM_setting_get(&sysSetting);
	
	return atoi(sysSetting.value);
}

void Os__suspend_wifi(void)
{
	int wifi_open = 0, ret = 0, wifi_suspend_model = 0;
	SYS_SETTING sysSetting;
	
	//获取wifi是否开启
	ret = Os__get_device_signal(DEV_WIFI);
	
	if(ret == 0)
		wifi_open = 0;
	else
		wifi_open = 1;
	
	//如果wifi没有开启直接返回
	if(wifi_open == 0)
		return;
	
	//获取wifi休眠模式
	memset(&sysSetting,0,sizeof(SYS_SETTING));
	strcpy(sysSetting.section,"WIFI_SETTING");
	strcpy(sysSetting.key,"WIFISUSPENDTYPE");
	PARAM_setting_get(&sysSetting);
	
	wifi_suspend_model = atoi(sysSetting.value);
	
	//断电方式休眠直接关闭wifi
	if(wifi_suspend_model == 1)
		modules_wifi_active(0);
}

void Os__awake_wifi(void)
{
	int wifi_open = 0, ret = 0, wifi_suspend_model = 0;
	SYS_SETTING sysSetting;
	
	//获取wifi是否开启
	ret = Os__get_device_signal(DEV_WIFI);
	
	if(ret == 0)
		wifi_open = 0;
	else
		wifi_open = 1;
		
	//获取wifi休眠模式
	memset(&sysSetting,0,sizeof(SYS_SETTING));
	strcpy(sysSetting.section,"WIFI_SETTING");
	strcpy(sysSetting.key,"WIFISUSPENDTYPE");
	PARAM_setting_get(&sysSetting);
	
	wifi_suspend_model = atoi(sysSetting.value);
	
	//如果wifi已经开启直接返回
	if(wifi_open == 1)
	{
		if(wifi_suspend_model == 0)
			system("/etc/usbwifi/awake.sh");
		return;
	}
	modules_wifi_active(1);
}

void Os__set_wifi(void)
{
	system("/app/00001/00001 wifi -geometry 240x295+0+25");
}
