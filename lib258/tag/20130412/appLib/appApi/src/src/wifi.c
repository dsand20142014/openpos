#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iwlib.h"
#include "wifi.h"


#define IW_SCAN_HACK		0x8000


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

int wifi_scan(char *ifname, WIFISCAN wifiscan[], int maxSize)
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
	int cn = -1;
	
	
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
	return (cn+1);
}
