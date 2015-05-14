
#include "iwlib.h"		/* Header */
#include "marvell_config.h"

#include	"oswifi.h"
#include	"appdef.h"

/**************************** CONSTANTS ****************************/

/*
 * Error codes defined for setting args
 */
#define MARVELL 1 
 

#define IWERR_ARG_NUM				-2
#define IWERR_ARG_TYPE			-3
#define IWERR_ARG_SIZE			-4
#define IWERR_ARG_CONFLICT	-5
#define IWERR_SET_EXT				-6
#define IWERR_GET_EXT				-7

static int	errarg;
static int	errmax;
typedef struct iwconfig_modifier {
  const char *		cmd;		/* Command line shorthand */
  __u16			flag;		/* Flags to add */
  __u16			exclude;	/* Modifiers to exclude */
} iwconfig_modifier;

static const struct iwconfig_modifier	iwmod_retry[] = {
  { "min",	IW_RETRY_MIN,		IW_RETRY_MAX },
  { "max",	IW_RETRY_MAX,		IW_RETRY_MIN },
  { "short",	IW_RETRY_SHORT,		IW_RETRY_LONG },
  { "long",	IW_RETRY_LONG,		IW_RETRY_SHORT },
  { "limit",	IW_RETRY_LIMIT,		IW_RETRY_LIFETIME },
  { "lifetime",	IW_RETRY_LIFETIME,	IW_RETRY_LIMIT },
};
#define IWMOD_RETRY_NUM	(sizeof(iwmod_retry)/sizeof(iwmod_retry[0]))

static const struct iwconfig_modifier	iwmod_power[] = {
  { "min",	IW_POWER_MIN,		IW_POWER_MAX },
  { "max",	IW_POWER_MAX,		IW_POWER_MIN },
  { "period",	IW_POWER_PERIOD,	IW_POWER_TIMEOUT | IW_POWER_SAVING },
  { "timeout",	IW_POWER_TIMEOUT,	IW_POWER_PERIOD | IW_POWER_SAVING },
  { "saving",	IW_POWER_SAVING,	IW_POWER_TIMEOUT | IW_POWER_PERIOD },
};
#define IWMOD_POWER_NUM	(sizeof(iwmod_power)/sizeof(iwmod_power[0]))
/*------------------------------------------------------------------*/
/*
 * Parse command line modifiers.
 * Return error or number arg parsed.
 * Modifiers must be at the beggining of command line.
 */
static int parse_modifiers(char *		args[],		/* Command line args */
														int		count,		/* Args count */
														__u16 *		pout,		/* Flags to write */
														const struct iwconfig_modifier	modifier[],
														int		modnum)
{
  int		i = 0;
  int		k = 0;
  __u16		result = 0;	/* Default : no flag set */

  /* Get all modifiers and value types on the command line */
  do
    {
      for(k = 0; k < modnum; k++)
			{
			  /* Check if matches */
			  if(!strcasecmp(args[i], modifier[k].cmd))
			    {
			      /* Check for conflicting flags */
			      if(result & modifier[k].exclude)
				{
				  errarg = i;
				  return(IWERR_ARG_CONFLICT);
				}
			      /* Just add it */
			      result |= modifier[k].flag;
			      ++i;
			      break;
			    }
			}
    }
  /* For as long as current arg matched and not out of args */
  while((i < count) && (k < modnum));

  /* Check there remains one arg for value */
  if(i >= count)
    return(IWERR_ARG_NUM);

  /* Return result */
  *pout = result;
  return(i);
}	

/*------------------------------------------------------------------*/
/*
 * Print on the screen in a neat fashion all the info we have collected
 * on a device.
 */
static void display_info(struct wireless_info *	info,
	     									 char *			ifname)
{
  char		buffer[128];	/* Temporary buffer */

  /* One token is more of less 5 characters, 14 tokens per line */
  int	tokens = 3;	/* For name */

  /* Display device name and wireless name (name of the protocol used) */
  Uart_Printf("%-8.16s  %s  ", ifname, info->b.name);

  /* Display ESSID (extended network), if any */
  if(info->b.has_essid)
    {
      if(info->b.essid_on)
			{
			  /* Does it have an ESSID index ? */
			  if((info->b.essid_on & IW_ENCODE_INDEX) > 1)
			    Uart_Printf("ESSID:\"%s\" [%d]  ", info->b.essid,
				   (info->b.essid_on & IW_ENCODE_INDEX));
			  else
			    Uart_Printf("ESSID:\"%s\"  ", info->b.essid);
			}
      else
				Uart_Printf("ESSID:off/any  ");
    }

#ifndef WE_ESSENTIAL
  /* Display NickName (station name), if any */
  if(info->has_nickname)
    Uart_Printf("Nickname:\"%s\"", info->nickname);
#endif	/* WE_ESSENTIAL */

  /* Formatting */
  if(info->b.has_essid || info->has_nickname)
    {
      Uart_Printf("\n          ");
      tokens = 0;
    }

#ifndef WE_ESSENTIAL
  /* Display Network ID */
  if(info->b.has_nwid)
    {
      /* Note : should display proper number of digits according to info
       * in range structure */
      if(info->b.nwid.disabled)
				Uart_Printf("NWID:off/any  ");
      else
				Uart_Printf("NWID:%X  ", info->b.nwid.value);
      	tokens +=2;
    }
#endif	/* WE_ESSENTIAL */

  /* Display the current mode of operation */
  if(info->b.has_mode)
    {
      Uart_Printf("Mode:%s  ", iw_operation_mode[info->b.mode]);
      tokens +=3;
    }

  /* Display frequency / channel */
  if(info->b.has_freq)
    {
      double		freq = info->b.freq;	/* Frequency/channel */
      int		channel = -1;		/* Converted to channel */
      /* Some drivers insist of returning channel instead of frequency.
       * This fixes them up. Note that, driver should still return
       * frequency, because other tools depend on it. */
      if(info->has_range && (freq < KILO))
				channel = iw_channel_to_freq((int) freq, &freq, &info->range);
      /* Display */
      iw_print_freq(buffer, sizeof(buffer), freq, -1, info->b.freq_flags);
      Uart_Printf("%s  ", buffer);
      tokens +=4;
    }

  /* Display the address of the current Access Point */
  if(info->has_ap_addr)
    {
      /* A bit of clever formatting */
      if(tokens > 8)
			{
			  Uart_Printf("\n          ");
			  tokens = 0;
			}
      tokens +=6;

      /* Oups ! No Access Point in Ad-Hoc mode */
      if((info->b.has_mode) && (info->b.mode == IW_MODE_ADHOC))
				Uart_Printf("Cell:");
      else
				Uart_Printf("Access Point:");
     	  Uart_Printf(" %s   ", iw_sawap_ntop(&info->ap_addr, buffer));
    }

  /* Display the currently used/set bit-rate */
  if(info->has_bitrate)
    {
      /* A bit of clever formatting */
      if(tokens > 11)
			{
			  Uart_Printf("\n          ");
			  tokens = 0;
			}
      tokens +=3;

      /* Display it */
      iw_print_bitrate(buffer, sizeof(buffer), info->bitrate.value);
      Uart_Printf("Bit Rate%c%s   ", (info->bitrate.fixed ? '=' : ':'), buffer);
    }

#ifndef WE_ESSENTIAL
  /* Display the Transmit Power */
  if(info->has_txpower)
    {
      /* A bit of clever formatting */
      if(tokens > 11)
			{
			  Uart_Printf("\n          ");
			  tokens = 0;
			}
      tokens +=3;

      /* Display it */
      iw_print_txpower(buffer, sizeof(buffer), &info->txpower);
      Uart_Printf("Tx-Power%c%s   ", (info->txpower.fixed ? '=' : ':'), buffer);
    }

  /* Display sensitivity */
  if(info->has_sens)
    {
      /* A bit of clever formatting */
      if(tokens > 10)
			{
			  Uart_Printf("\n          ");
			  tokens = 0;
			}
      tokens +=4;

      /* Fixed ? */
      Uart_Printf("Sensitivity%c", info->sens.fixed ? '=' : ':');

      if(info->has_range)
				/* Display in dBm ? */
				if(info->sens.value < 0)
	  			Uart_Printf("%d dBm  ", info->sens.value);
				else
	  			Uart_Printf("%d/%d  ", info->sens.value, info->range.sensitivity);
      else
				Uart_Printf("%d  ", info->sens.value);
    }
#endif	/* WE_ESSENTIAL */

  Uart_Printf("\n          ");
  tokens = 0;

#ifndef WE_ESSENTIAL
  /* Display retry limit/lifetime information */
  if(info->has_retry)
    { 
      Uart_Printf("Retry");
      /* Disabled ? */
      if(info->retry.disabled)
				Uart_Printf(":off");
      else
			{
			  /* Let's check the value and its type */
			  if(info->retry.flags & IW_RETRY_TYPE)
			    {
			      iw_print_retry_value(buffer, sizeof(buffer),
						   info->retry.value, info->retry.flags,
						   info->range.we_version_compiled);
			      Uart_Printf("%s", buffer);
			    }
		
			  /* Let's check if nothing (simply on) */
			  if(info->retry.flags == IW_RETRY_ON)
			    Uart_Printf(":on");
		 	}
      Uart_Printf("   ");
      tokens += 5;	/* Between 3 and 5, depend on flags */
    }

  /* Display the RTS threshold */
  if(info->has_rts)
    {
      /* Disabled ? */
      if(info->rts.disabled)
				Uart_Printf("RTS thr:off   ");
      else
			{
			  /* Fixed ? */
			  Uart_Printf("RTS thr%c%d B   ",
				 info->rts.fixed ? '=' : ':',
				 info->rts.value);
			}
      tokens += 3;
    }

  /* Display the fragmentation threshold */
  if(info->has_frag)
    {
      /* A bit of clever formatting */
      if(tokens > 10)
			{
			  Uart_Printf("\n          ");
			  tokens = 0;
			}
      tokens +=4;

      /* Disabled ? */
      if(info->frag.disabled)
				Uart_Printf("Fragment thr:off");
      else
			{
			  /* Fixed ? */
			  Uart_Printf("Fragment thr%c%d B   ",
				 info->frag.fixed ? '=' : ':',
				 info->frag.value);
			}
    }

  /* Formating */
  if(tokens > 0)
    Uart_Printf("\n          ");
#endif	/* WE_ESSENTIAL */

  /* Display encryption information */
  /* Note : we display only the "current" key, use iwlist to list all keys */
  if(info->b.has_key)
    {
      Uart_Printf("Encryption key:");
      if((info->b.key_flags & IW_ENCODE_DISABLED) || (info->b.key_size == 0))
				Uart_Printf("off");
      else
			{
			  /* Display the key */
			  iw_print_key(buffer, sizeof(buffer),
				       info->b.key, info->b.key_size, info->b.key_flags);
			  Uart_Printf("%s", buffer);
		
			  /* Other info... */
			  if((info->b.key_flags & IW_ENCODE_INDEX) > 1)
			    Uart_Printf(" [%d]", info->b.key_flags & IW_ENCODE_INDEX);
			  if(info->b.key_flags & IW_ENCODE_RESTRICTED)
			    Uart_Printf("   Security mode:restricted");
			  if(info->b.key_flags & IW_ENCODE_OPEN)
			    Uart_Printf("   Security mode:open");
		 	}
      Uart_Printf("\n          ");
    }

  /* Display Power Management information */
  /* Note : we display only one parameter, period or timeout. If a device
   * (such as HiperLan) has both, the user need to use iwlist... */
  if(info->has_power)	/* I hope the device has power ;-) */
    { 
      Uart_Printf("Power Management");
      /* Disabled ? */
      if(info->power.disabled)
				Uart_Printf(":off");
      else
				{
				  /* Let's check the value and its type */
				  if(info->power.flags & IW_POWER_TYPE)
				    {
				      iw_print_pm_value(buffer, sizeof(buffer),
							info->power.value, info->power.flags,
							info->range.we_version_compiled);
				      Uart_Printf("%s  ", buffer);
				    }
			
				  /* Let's check the mode */
				  iw_print_pm_mode(buffer, sizeof(buffer), info->power.flags);
				  Uart_Printf("%s", buffer);
			
				  /* Let's check if nothing (simply on) */
				  if(info->power.flags == IW_POWER_ON)
				    Uart_Printf(":on");
			 	}
      Uart_Printf("\n          ");
    }

  /* Display statistics */
  if(info->has_stats)
    {
      iw_print_stats(buffer, sizeof(buffer),
		     &info->stats.qual, &info->range, info->has_range);
      Uart_Printf("Link %s\n", buffer);

      if(info->range.we_version_compiled > 11){
		  Uart_Printf("Rx invalid nwid:%d ", info->stats.discard.nwid);
		  //Uart_Printf("Rx invalid crypt:%d", info->stats.discard.code);
		  Uart_Printf("Rx invalid frag:%d\n",info->stats.discard.fragment);
		  Uart_Printf("Tx excessive retries:%d",  info->stats.discard.retries);
		  Uart_Printf("Invalid misc:%d",  info->stats.discard.misc);
		  Uart_Printf("Missed beacon:%d\n",info->stats.miss.beacon);
	  }
      else{
	  		Uart_Printf("Rx invalid nwid:%d ", info->stats.discard.nwid);
	        //Uart_Printf("Rx invalid crypt:%d", info->stats.discard.code);
	        Uart_Printf("Invalid misc:%d",  info->stats.discard.misc);  
	  }
    }

  Uart_Printf("\n");
}
	
// print_info(skfd, argv[1], NULL, 0);
/**返回值: -1 -ENODEV -ENOTSUP 
	*					0
**/
int  get_info(char *	 ifname,
	    				struct  wireless_info *	info)
{
	struct iwreq		wrq;

		int skfd;		/* generic raw socket desc.	*/
  	/* Create a channel to the NET kernel. */
  	if((skfd = iw_sockets_open()) < 0)
    {
      perror("socket");
      exit(-1);
    }
	  
	  memset((char *) info, 0, sizeof(struct wireless_info));
	  /* Get basic information */
	  if(iw_get_basic_config(skfd, ifname, &(info->b)) < 0)
		{
		  /* If no wireless name : no wireless extensions */
		  /* But let's check if the interface exists at all */
		  struct ifreq ifr;
	
		  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
		  if(ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
				return(-ENODEV);
		  else
				return(-ENOTSUP);
		}
	
	  /* Get ranges */
	  if(iw_get_range_info(skfd, ifname, &(info->range)) >= 0)
		info->has_range = 1;
	
	  /* Get AP address */
	  if(iw_get_ext(skfd, ifname, SIOCGIWAP, &wrq) >= 0)
		{
		  info->has_ap_addr = 1;
		  memcpy(&(info->ap_addr), &(wrq.u.ap_addr), sizeof (sockaddr));
		}
	
	  /* Get bit rate */
	  if(iw_get_ext(skfd, ifname, SIOCGIWRATE, &wrq) >= 0)
		{
		  info->has_bitrate = 1;
		  memcpy(&(info->bitrate), &(wrq.u.bitrate), sizeof(iwparam));
		}
	
	  /* Get Power Management settings */
	  wrq.u.power.flags = 0;
	  if(iw_get_ext(skfd, ifname, SIOCGIWPOWER, &wrq) >= 0)
		{
		  info->has_power = 1;
		  memcpy(&(info->power), &(wrq.u.power), sizeof(iwparam));
		}
	
	  /* Get stats */
	  if(iw_get_stats(skfd, ifname, &(info->stats),
			  &info->range, info->has_range) >= 0)
		{
		  info->has_stats = 1;
		}
	
#ifndef WE_ESSENTIAL
	  /* Get NickName */
	  wrq.u.essid.pointer = (caddr_t) info->nickname;
	  wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
	  wrq.u.essid.flags = 0;
	  if(iw_get_ext(skfd, ifname, SIOCGIWNICKN, &wrq) >= 0)
		if(wrq.u.data.length > 1)
		  info->has_nickname = 1;
	
	  if((info->has_range) && (info->range.we_version_compiled > 9))
		{
		  /* Get Transmit Power */
		  if(iw_get_ext(skfd, ifname, SIOCGIWTXPOW, &wrq) >= 0)
			{
			  info->has_txpower = 1;
			  memcpy(&(info->txpower), &(wrq.u.txpower), sizeof(iwparam));
			}
		}
	
	  /* Get sensitivity */
	  if(iw_get_ext(skfd, ifname, SIOCGIWSENS, &wrq) >= 0)
		{
		  info->has_sens = 1;
		  memcpy(&(info->sens), &(wrq.u.sens), sizeof(iwparam));
		}
	
	  if((info->has_range) && (info->range.we_version_compiled > 10))
		{
		  /* Get retry limit/lifetime */
		  if(iw_get_ext(skfd, ifname, SIOCGIWRETRY, &wrq) >= 0)
			{
			  info->has_retry = 1;
			  memcpy(&(info->retry), &(wrq.u.retry), sizeof(iwparam));
			}
		}
	
	  /* Get RTS threshold */
	  if(iw_get_ext(skfd, ifname, SIOCGIWRTS, &wrq) >= 0)
		{
		  info->has_rts = 1;
		  memcpy(&(info->rts), &(wrq.u.rts), sizeof(iwparam));
		}
	
	  /* Get fragmentation threshold */
	  if(iw_get_ext(skfd, ifname, SIOCGIWFRAG, &wrq) >= 0)
		{
		  info->has_frag = 1;
		  memcpy(&(info->frag), &(wrq.u.frag), sizeof(iwparam));
		}
#endif	/* WE_ESSENTIAL */
	
		//使用 display_info来显示info,使用的时候可以注释掉
		display_info(info, ifname);
	iw_sockets_close(skfd);
	return(0);
}



int set_mode_info(char *		ifname,
				          char *		args[],		/* Command line args */
				          int		count)		/* Args count */
{
	struct iwreq		wrq;
  unsigned int		k;		/* Must be unsigned */
	int skfd;		/* generic raw socket desc.	*/
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
  {
    perror("socket");
    exit(-1);
  }

  /* Avoid "Unused parameter" warning */
  count = count;
  /* Check if it is a uint, otherwise get is as a string */
  if(sscanf(args[0], "%i", &k) != 1)
    {
      k = 0;
      while((k < IW_NUM_OPER_MODE) &&
	    strncasecmp(args[0], iw_operation_mode[k], 3))
			k++;
    }
  if(k >= IW_NUM_OPER_MODE)
    {
      errarg = 0;
      return(IWERR_ARG_TYPE);
    }
  wrq.u.mode = k;
  if(iw_set_ext(skfd, ifname, SIOCSIWMODE, &wrq) < 0)
    return(IWERR_SET_EXT);

  /* 1 arg */
	iw_sockets_close(skfd);
	return(0);	
}

int set_essid_info( char *	ifname,
					          char *	args[],		/* Command line args */
					          int		count)		/* Args count */
{
  struct iwreq		wrq;
  int			i = 1;
  char			essid[IW_ESSID_MAX_SIZE + 1];
  int			we_kernel_version;
	int skfd;		/* generic raw socket desc.	*/
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
  {
    perror("socket");
    exit(-1);
  }	
  if((!strcasecmp(args[0], "off")) ||
     (!strcasecmp(args[0], "any")))
    {
      wrq.u.essid.flags = 0;
      essid[0] = '\0';
    }
  else
    if(!strcasecmp(args[0], "on"))
      {
				/* Get old essid */
				memset(essid, '\0', sizeof(essid));
				wrq.u.essid.pointer = (caddr_t) essid;
				wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
				wrq.u.essid.flags = 0;
				if(iw_get_ext(skfd, ifname, SIOCGIWESSID, &wrq) < 0)
				  return(IWERR_GET_EXT);
				wrq.u.essid.flags = 1;
      }
    else
      {
				i = 0;
			
				/* '-' or '--' allow to escape the ESSID string, allowing
				 * to set it to the string "any" or "off".
				 * This is a big ugly, but it will do for now 
				 */
				if((!strcmp(args[0], "-")) || (!strcmp(args[0], "--")))
				  {
				    if(++i >= count)
				      return(IWERR_ARG_NUM);
				  }
			
				/* Check the size of what the user passed us to avoid
				 * buffer overflows */
				if(strlen(args[i]) > IW_ESSID_MAX_SIZE)
				  {
				    errmax = IW_ESSID_MAX_SIZE;
				    return(IWERR_ARG_SIZE);
				  }
				else
				  {
				    int		temp;
			
				    wrq.u.essid.flags = 1;
				    strcpy(essid, args[i]);	/* Size checked, all clear */
				    i++;
			
				    /* Check for ESSID index */
				    if((i < count) &&
				       (sscanf(args[i], "[%i]", &temp) == 1) &&
				       (temp > 0) && (temp < IW_ENCODE_INDEX))
				      {
								wrq.u.essid.flags = temp;
								++i;
				      }
				  }
      }

  /* Get version from kernel, device may not have range... */
  we_kernel_version = iw_get_kernel_we_version();
  /* Finally set the ESSID value */
  wrq.u.essid.pointer = (caddr_t) essid;
  wrq.u.essid.length = strlen(essid);
  if(we_kernel_version < 21)
    wrq.u.essid.length++;

  if(iw_set_ext(skfd, ifname, SIOCSIWESSID, &wrq) < 0)
    return(IWERR_SET_EXT);
  
  
  
	iw_sockets_close(skfd);
	return(0);	
}


int set_freq_info(char *		ifname,
				           char *		args[],		/* Command line args */
				           int		count)		/* Args count */
{
	struct iwreq		wrq;
  int			i = 1;
	int skfd;		/* generic raw socket desc.	*/
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
  {
    perror("socket");
    exit(-1);
  }	
  


  if(!strcasecmp(args[0], "auto"))
    {
      wrq.u.freq.m = -1;
      wrq.u.freq.e = 0;
      wrq.u.freq.flags = 0;
    }
  else
    {
      if(!strcasecmp(args[0], "fixed"))
			{
			  /* Get old frequency */
			  if(iw_get_ext(skfd, ifname, SIOCGIWFREQ, &wrq) < 0)
			    return(IWERR_GET_EXT);
			  wrq.u.freq.flags = IW_FREQ_FIXED;
			}
      else			/* Should be a numeric value */
			{
			  double		freq;
			  char *		unit;
		
			  freq = strtod(args[0], &unit);
			  if(unit == args[0])
			    {
			      errarg = 0;
			      return(IWERR_ARG_TYPE);
			    }
			  if(unit != NULL)
			    {
			      if(unit[0] == 'G') freq *= GIGA;
			      if(unit[0] == 'M') freq *= MEGA;
			      if(unit[0] == 'k') freq *= KILO;
			    }
		
			  iw_float2freq(freq, &(wrq.u.freq));
		
			  wrq.u.freq.flags = IW_FREQ_FIXED;
		
			  /* Check for an additional argument */
			  if((i < count) && (!strcasecmp(args[i], "auto")))
			    {
			      wrq.u.freq.flags = 0;
			      ++i;
			    }
			  if((i < count) && (!strcasecmp(args[i], "fixed")))
			    {
			      wrq.u.freq.flags = IW_FREQ_FIXED;
			      ++i;
			    }
			}
    }

  if(iw_set_ext(skfd, ifname, SIOCSIWFREQ, &wrq) < 0)
    return(IWERR_SET_EXT);
  	
	
	iw_sockets_close(skfd);
	return(0);	
}

int set_bitrate_info(char *		ifname,
					          char *		args[],		/* Command line args */
					          int		count)		/* Args count */
{
  struct iwreq		wrq;
  int			i = 1;	
	int skfd;		/* generic raw socket desc.	*/
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
  {
    perror("socket");
    exit(-1);
  }	
	
  wrq.u.bitrate.flags = 0;
  if(!strcasecmp(args[0], "auto"))
    {
      wrq.u.bitrate.value = -1;
      wrq.u.bitrate.fixed = 0;
    }
  else
    {
      if(!strcasecmp(args[0], "fixed"))
			{
			  /* Get old bitrate */
			  if(iw_get_ext(skfd, ifname, SIOCGIWRATE, &wrq) < 0)
			    return(IWERR_GET_EXT);
			  wrq.u.bitrate.fixed = 1;
			}
      else			/* Should be a numeric value */
			{
			  double		brate;
			  char *		unit;
		
			  brate = strtod(args[0], &unit);
			  if(unit == args[0])
			    {
			      errarg = 0;
			      return(IWERR_ARG_TYPE);
			    }
			  if(unit != NULL)
			    {
			      if(unit[0] == 'G') brate *= GIGA;
			      if(unit[0] == 'M') brate *= MEGA;
			      if(unit[0] == 'k') brate *= KILO;
			    }
			  wrq.u.bitrate.value = (long) brate;
			  wrq.u.bitrate.fixed = 1;
		
			  /* Check for an additional argument */
			  if((i < count) && (!strcasecmp(args[i], "auto")))
			    {
			      wrq.u.bitrate.fixed = 0;
			      ++i;
			    }
			  if((i < count) && (!strcasecmp(args[i], "fixed")))
			    {
			      wrq.u.bitrate.fixed = 1;
			      ++i;
			    }
			  if((i < count) && (!strcasecmp(args[i], "unicast")))
			    {
			      wrq.u.bitrate.flags |= IW_BITRATE_UNICAST;
			      ++i;
			    }
			  if((i < count) && (!strcasecmp(args[i], "broadcast")))
			    {
			      wrq.u.bitrate.flags |= IW_BITRATE_BROADCAST;
			      ++i;
			    }
			}
    }

  if(iw_set_ext(skfd, ifname, SIOCSIWRATE, &wrq) < 0)
    return(IWERR_SET_EXT);	
	
	iw_sockets_close(skfd);
	return(0);		
}

int  set_enc_info(char *		ifname,
			            char *		args[],		/* Command line args */
			            int		count)		/* Args count */
{
	int skfd;		/* generic raw socket desc.	*/
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
  {
    perror("socket");
    exit(-1);
  }	
  
  struct iwreq		wrq;
  int			i = 1;
  unsigned char		key[IW_ENCODING_TOKEN_MAX];

  if(!strcasecmp(args[0], "on"))
    {
      /* Get old encryption information */
      wrq.u.data.pointer = (caddr_t) key;
      wrq.u.data.length = IW_ENCODING_TOKEN_MAX;
      wrq.u.data.flags = 0;
      if(iw_get_ext(skfd, ifname, SIOCGIWENCODE, &wrq) < 0)
				return(IWERR_GET_EXT);
      wrq.u.data.flags &= ~IW_ENCODE_DISABLED;	/* Enable */
    }
  else
    {
      int	gotone = 0;
      int	oldone;
      int	keylen;
      int	temp;

      wrq.u.data.pointer = (caddr_t) NULL;
      wrq.u.data.flags = 0;
      wrq.u.data.length = 0;
      i = 0;

      /* Allow arguments in any order (it's safe) */
      do
			{
			  oldone = gotone;
		
			  /* -- Check for the key -- */
			  if(i < count)
			    {
			      keylen = iw_in_key_full(skfd, ifname,
						      args[i], key, &wrq.u.data.flags);
			      if(keylen > 0)
				{
				  wrq.u.data.length = keylen;
				  wrq.u.data.pointer = (caddr_t) key;
				  ++i;
				  gotone++;
				}
			    }
		
			  /* -- Check for token index -- */
			  if((i < count) &&
			     (sscanf(args[i], "[%i]", &temp) == 1) &&
			     (temp > 0) && (temp < IW_ENCODE_INDEX))
			    {
			      wrq.u.encoding.flags |= temp;
			      ++i;
			      gotone++;
			    }
		
			  /* -- Check the various flags -- */
			  if((i < count) && (!strcasecmp(args[i], "off")))
			    {
			      wrq.u.data.flags |= IW_ENCODE_DISABLED;
			      ++i;
			      gotone++;
			    }
			  if((i < count) && (!strcasecmp(args[i], "open")))
			    {
			      wrq.u.data.flags |= IW_ENCODE_OPEN;
			      ++i;
			      gotone++;
			    }
			  if((i < count) && (!strncasecmp(args[i], "restricted", 5)))
			    {
			      wrq.u.data.flags |= IW_ENCODE_RESTRICTED;
			      ++i;
			      gotone++;
			    }
			  if((i < count) && (!strncasecmp(args[i], "temporary", 4)))
			    {
			      wrq.u.data.flags |= IW_ENCODE_TEMP;
			      ++i;
			      gotone++;
			    }
			}
      while(gotone != oldone);

      /* Pointer is absent in new API */
      if(wrq.u.data.pointer == NULL)
				wrq.u.data.flags |= IW_ENCODE_NOKEY;

      /* Check if we have any invalid argument */
      if(!gotone)
			{
			  errarg = 0;
			  return(IWERR_ARG_TYPE);
			}
    }

  if(iw_set_ext(skfd, ifname, SIOCSIWENCODE, &wrq) < 0)
    return(IWERR_SET_EXT);

	iw_sockets_close(skfd);
	return(0);		
}

int  set_power_info(char *		ifname,
			              char *		args[],		/* Command line args */
			              int		count)		/* Args count */
{
	int skfd;		/* generic raw socket desc.	*/
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
  {
    perror("socket");
    exit(-1);
  }	
  struct iwreq		wrq;
  int			i = 1;

  if(!strcasecmp(args[0], "off"))
    wrq.u.power.disabled = 1;	/* i.e. max size */
  else
    if(!strcasecmp(args[0], "on"))
      {
				/* Get old Power info */
				wrq.u.power.flags = 0;
				if(iw_get_ext(skfd, ifname, SIOCGIWPOWER, &wrq) < 0)
				  return(IWERR_GET_EXT);
				wrq.u.power.disabled = 0;
      }
    else
      {
				double		value;
				char *		unit;
				int		gotone = 0;
			
				/* Parse modifiers */
				i = parse_modifiers(args, count, &wrq.u.power.flags,
						    iwmod_power, IWMOD_POWER_NUM);
				if(i < 0)
				  return(i);
			
				wrq.u.power.disabled = 0;
			
				/* Is there any value to grab ? */
				value = strtod(args[0], &unit);
				if(unit != args[0])
				  {
				    struct iw_range	range;
				    int			flags;
				    /* Extract range info to handle properly 'relative' */
				    if(iw_get_range_info(skfd, ifname, &range) < 0)
				      memset(&range, 0, sizeof(range));
			
				    /* Get the flags to be able to do the proper conversion */
				    switch(wrq.u.power.flags & IW_POWER_TYPE)
				      {
					      case IW_POWER_SAVING:
											flags = range.pms_flags;
											break;
					      case IW_POWER_TIMEOUT:
											flags = range.pmt_flags;
											break;
					      default:
											flags = range.pmp_flags;
											break;
				      }
				    /* Check if time or relative */
				    if(flags & IW_POWER_RELATIVE)
				      {
								if(range.we_version_compiled < 21)
								  value *= MEGA;
								else
								  wrq.u.power.flags |= IW_POWER_RELATIVE;
				      }
				    else
				      {
								value *= MEGA;	/* default = s */
								if(unit[0] == 'u') value /= MEGA;
								if(unit[0] == 'm') value /= KILO;
				      }
				    wrq.u.power.value = (long) value;
				    /* Set some default type if none */
				    if((wrq.u.power.flags & IW_POWER_TYPE) == 0)
				      wrq.u.power.flags |= IW_POWER_PERIOD;
				    ++i;
				    gotone = 1;
				  }
			
				/* Now, check the mode */
				if(i < count)
				  {
				    if(!strcasecmp(args[i], "all"))
				      wrq.u.power.flags |= IW_POWER_ALL_R;
				    if(!strncasecmp(args[i], "unicast", 4))
				      wrq.u.power.flags |= IW_POWER_UNICAST_R;
				    if(!strncasecmp(args[i], "multicast", 5))
				      wrq.u.power.flags |= IW_POWER_MULTICAST_R;
				    if(!strncasecmp(args[i], "force", 5))
				      wrq.u.power.flags |= IW_POWER_FORCE_S;
				    if(!strcasecmp(args[i], "repeat"))
				      wrq.u.power.flags |= IW_POWER_REPEATER;
				    if(wrq.u.power.flags & IW_POWER_MODE)
				      {
								++i;
								gotone = 1;
				      }
				  }
				if(!gotone)
				  {
				    errarg = i;
				    return(IWERR_ARG_TYPE);
				  }
      }

  if(iw_set_ext(skfd, ifname, SIOCSIWPOWER, &wrq) < 0)
    return(IWERR_SET_EXT);

  
  
  	
	iw_sockets_close(skfd);
	return(0);		
}


int  set_commit_info(char *		ifname,
		               	 char *		args[],		/* Command line args */
		              	 int		count)		/* Args count */
{
	int skfd;		/* generic raw socket desc.	*/
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
  {
    perror("socket");
    exit(-1);
  }	

  struct iwreq		wrq;

  /* Avoid "Unused parameter" warning */
  args = args; count = count;

  if(iw_set_ext(skfd, ifname, SIOCSIWCOMMIT, &wrq) < 0)
    return(IWERR_SET_EXT);  
  
  	
	iw_sockets_close(skfd);
	return(0);		
}

int set_nick_info(char *		ifname,
				          char *		args[],		/* Command line args */
				          int		count)		/* Args count */
{
	int skfd;		/* generic raw socket desc.	*/
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
  {
    perror("socket");
    exit(-1);
  }	
  
  struct iwreq		wrq;
  int			we_kernel_version;

  /* Avoid "Unused parameter" warning */
  count = count;

  if(strlen(args[0]) > IW_ESSID_MAX_SIZE)
    {
      errmax = IW_ESSID_MAX_SIZE;
      return(IWERR_ARG_SIZE);
    }

  we_kernel_version = iw_get_kernel_we_version();

  wrq.u.essid.pointer = (caddr_t) args[0];
  wrq.u.essid.length = strlen(args[0]);
  if(we_kernel_version < 21)
    wrq.u.essid.length++;

  if(iw_set_ext(skfd, ifname, SIOCSIWNICKN, &wrq) < 0)
    return(IWERR_SET_EXT);
  	
	iw_sockets_close(skfd);
	return(0);		
}

int set_frag_info (char *		ifname,
				           char *		args[],		/* Command line args */
				           int		count)		/* Args count */
{
	int skfd;		/* generic raw socket desc.	*/
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
  {
    perror("socket");
    exit(-1);
  }	
  
  struct iwreq		wrq;

  /* Avoid "Unused parameter" warning */
  count = count;

  wrq.u.frag.value = -1;
  wrq.u.frag.fixed = 1;
  wrq.u.frag.disabled = 0;

  if(!strcasecmp(args[0], "off"))
    wrq.u.frag.disabled = 1;	/* i.e. max size */
  else
    if(!strcasecmp(args[0], "auto"))
      wrq.u.frag.fixed = 0;
    else
      {
				if(!strcasecmp(args[0], "fixed"))
				  {
				    /* Get old fragmentation threshold */
				    if(iw_get_ext(skfd, ifname, SIOCGIWFRAG, &wrq) < 0)
				      return(IWERR_GET_EXT);
				    wrq.u.frag.fixed = 1;
				  }
				else
				  {	/* Should be a numeric value */
				    long	temp;
				    if(sscanf(args[0], "%li", &(temp)) != 1)
				      {
								errarg = 0;
								return(IWERR_ARG_TYPE);
				      }
				    wrq.u.frag.value = temp;
				  }
      }

  if(iw_set_ext(skfd, ifname, SIOCSIWFRAG, &wrq) < 0)
    return(IWERR_SET_EXT);
  
  	
	iw_sockets_close(skfd);
	return(0);		
}

int set_txpower_info (char *		ifname,
				               char *		args[],		/* Command line args */
				               int		count)		/* Args count */

{
	int skfd;		/* generic raw socket desc.	*/
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
  {
    perror("socket");
    exit(-1);
  }	


  struct iwreq		wrq;
  int			i = 1;

  /* Avoid "Unused parameter" warning */
  args = args; count = count;

  /* Prepare the request */
  wrq.u.txpower.value = -1;
  wrq.u.txpower.fixed = 1;
  wrq.u.txpower.disabled = 0;
  wrq.u.txpower.flags = IW_TXPOW_DBM;

  if(!strcasecmp(args[0], "off"))
    wrq.u.txpower.disabled = 1;	/* i.e. turn radio off */
  else
    if(!strcasecmp(args[0], "auto"))
      wrq.u.txpower.fixed = 0;	/* i.e. use power control */
    else
      {
				if(!strcasecmp(args[0], "on"))
				  {
				    /* Get old tx-power */
				    if(iw_get_ext(skfd, ifname, SIOCGIWTXPOW, &wrq) < 0)
				      return(IWERR_GET_EXT);
				    wrq.u.txpower.disabled = 0;
				  }
				else
				  {
				    if(!strcasecmp(args[0], "fixed"))
				      {
								/* Get old tx-power */
								if(iw_get_ext(skfd, ifname, SIOCGIWTXPOW, &wrq) < 0)
								  return(IWERR_GET_EXT);
								wrq.u.txpower.fixed = 1;
								wrq.u.txpower.disabled = 0;
				      }
				    else			/* Should be a numeric value */
				      {
								int		power;
								int		ismwatt = 0;
								struct iw_range	range;
						
								/* Extract range info to do proper conversion */
								if(iw_get_range_info(skfd, ifname, &range) < 0)
								  memset(&range, 0, sizeof(range));
						
								/* Get the value */
								if(sscanf(args[0], "%i", &(power)) != 1)
								  {
								    errarg = 0;
								    return(IWERR_ARG_TYPE);
								  }
						
								/* Check if milliWatt
								 * We authorise a single 'm' as a shorthand for 'mW',
								 * on the other hand a 'd' probably means 'dBm'... */
								ismwatt = ((strchr(args[0], 'm') != NULL)
									   && (strchr(args[0], 'd') == NULL));
						
								/* We could check 'W' alone... Another time... */
						
								/* Convert */
								if(range.txpower_capa & IW_TXPOW_RELATIVE)
								  {
								    /* Can't convert */
								    if(ismwatt)
								      {
													errarg = 0;
													return(IWERR_ARG_TYPE);
								      }
								    wrq.u.txpower.flags = IW_TXPOW_RELATIVE;
								  }
								else
								  if(range.txpower_capa & IW_TXPOW_MWATT)
								    {
								      if(!ismwatt)
												power = iw_dbm2mwatt(power);
								      wrq.u.txpower.flags = IW_TXPOW_MWATT;
								    }
								  else
								    {
								      if(ismwatt)
												power = iw_mwatt2dbm(power);
								      wrq.u.txpower.flags = IW_TXPOW_DBM;
								    }
								wrq.u.txpower.value = power;
						
								/* Check for an additional argument */
								if((i < count) && (!strcasecmp(args[i], "auto")))
								  {
								    wrq.u.txpower.fixed = 0;
								    ++i;
								  }
								if((i < count) && (!strcasecmp(args[i], "fixed")))
								  {
								    wrq.u.txpower.fixed = 1;
								    ++i;
								  }
				      }
				  }
      }

  if(iw_set_ext(skfd, ifname, SIOCSIWTXPOW, &wrq) < 0)
    return(IWERR_SET_EXT);

	iw_sockets_close(skfd);
	return(0);		
}


int set_modulation_info (char *		ifname,
				                 char *		args[],		/* Command line args */
				                 int		count)		/* Args count */
{
	int skfd;		/* generic raw socket desc.	*/
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
  {
    perror("socket");
    exit(-1);
  }	
  

  struct iwreq		wrq;
  int			i = 1;

  /* Avoid "Unused parameter" warning */
  args = args; count = count;

  if(!strcasecmp(args[0], "auto"))
    wrq.u.param.fixed = 0;	/* i.e. use any modulation */
  else
    {
      if(!strcasecmp(args[0], "fixed"))
			{
			  /* Get old modulation */
			  if(iw_get_ext(skfd, ifname, SIOCGIWMODUL, &wrq) < 0)
			    return(IWERR_GET_EXT);
			  wrq.u.param.fixed = 1;
			}
      else
			{
			  int		k;
		
			  /* Allow multiple modulations, combine them together */
			  wrq.u.param.value = 0x0;
			  i = 0;
		  	do
		    {
		      for(k = 0; k < IW_SIZE_MODUL_LIST; k++)
					{
					  if(!strcasecmp(args[i], iw_modul_list[k].cmd))
					    {
					      wrq.u.param.value |= iw_modul_list[k].mask;
					      ++i;
					      break;
					    }
					}
		    }
			  /* For as long as current arg matched and not out of args */
			  while((i < count) && (k < IW_SIZE_MODUL_LIST));
		
			  /* Check we got something */
			  if(i == 0)
			    {
			      errarg = 0;
			      return(IWERR_ARG_TYPE);
			    }
		
			  /* Check for an additional argument */
			  if((i < count) && (!strcasecmp(args[i], "auto")))
			    {
			      wrq.u.param.fixed = 0;
			      ++i;
			    }
			  if((i < count) && (!strcasecmp(args[i], "fixed")))
			    {
			      wrq.u.param.fixed = 1;
			      ++i;
			    }
			}
    }

  if(iw_set_ext(skfd, ifname, SIOCSIWMODUL, &wrq) < 0)
    return(IWERR_SET_EXT);

  	
	iw_sockets_close(skfd);
	return(0);		
}

int set_rts_info  (char *		ifname,
	                 char *		args[],		/* Command line args */
	                 int		count)		/* Args count */
{
	int skfd;		/* generic raw socket desc.	*/
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
  {
    perror("socket");
    exit(-1);
  }	
  
  struct iwreq		wrq;

  /* Avoid "Unused parameter" warning */
  count = count;

  wrq.u.rts.value = -1;
  wrq.u.rts.fixed = 1;
  wrq.u.rts.disabled = 0;

  if(!strcasecmp(args[0], "off"))
    wrq.u.rts.disabled = 1;	/* i.e. max size */
  else
    if(!strcasecmp(args[0], "auto"))
      wrq.u.rts.fixed = 0;
    else
      {
				if(!strcasecmp(args[0], "fixed"))
				  {
				    /* Get old RTS threshold */
				    if(iw_get_ext(skfd, ifname, SIOCGIWRTS, &wrq) < 0)
				      return(IWERR_GET_EXT);
				    wrq.u.rts.fixed = 1;
				  }
				else
				  {	/* Should be a numeric value */
				    long	temp;
				    if(sscanf(args[0], "%li", (unsigned long *) &(temp)) != 1)
				      {
					errarg = 0;
					return(IWERR_ARG_TYPE);
				      }
				    wrq.u.rts.value = temp;
				  }
      }

  if(iw_set_ext(skfd, ifname, SIOCSIWRTS, &wrq) < 0)
    return(IWERR_SET_EXT);
	iw_sockets_close(skfd);
	return(0);		
}


int  set_retry_info (char *		ifname,
	                 	char *		args[],		/* Command line args */
	                 	int		count)		/* Args count */
{
	int skfd;		/* generic raw socket desc.	*/
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
  {
    perror("socket");
    exit(-1);
  }	
  
  struct iwreq		wrq;
  int			i = 0;
  double		value;
  char *		unit;

  /* Parse modifiers */
  i = parse_modifiers(args, count, &wrq.u.retry.flags,
		      						iwmod_retry, IWMOD_RETRY_NUM);
  if(i < 0)
    return(i);

  /* Add default type if none */
  if((wrq.u.retry.flags & IW_RETRY_TYPE) == 0)
    wrq.u.retry.flags |= IW_RETRY_LIMIT;

  wrq.u.retry.disabled = 0;

  /* Is there any value to grab ? */
  value = strtod(args[0], &unit);
  if(unit == args[0])
    {
      errarg = i;
      return(IWERR_ARG_TYPE);
    }

  /* Limit is absolute, on the other hand lifetime is seconds */
  if(wrq.u.retry.flags & IW_RETRY_LIFETIME)
    {
      struct iw_range	range;
      /* Extract range info to handle properly 'relative' */
      if(iw_get_range_info(skfd, ifname, &range) < 0)
				memset(&range, 0, sizeof(range));

      if(range.r_time_flags & IW_RETRY_RELATIVE)
				{
				  if(range.we_version_compiled < 21)
				    value *= MEGA;
				  else
				    wrq.u.retry.flags |= IW_RETRY_RELATIVE;
				}
      else
				{
				  /* Normalise lifetime */
				  value *= MEGA;	/* default = s */
				  if(unit[0] == 'u') value /= MEGA;
				  if(unit[0] == 'm') value /= KILO;
				}
    }
  wrq.u.retry.value = (long) value;
  ++i;

  if(iw_set_ext(skfd, ifname, SIOCSIWRETRY, &wrq) < 0)
    return(IWERR_SET_EXT);
	
	iw_sockets_close(skfd);
	return(0);		
}

int  set_sens_info(char *		ifname,
	                 char *		args[],		/* Command line args */
	                 int		count)		/* Args count */
{
	int skfd;		/* generic raw socket desc.	*/
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
  {
    perror("socket");
    exit(-1);
  }	

  struct iwreq		wrq;
  int			temp;

  /* Avoid "Unused parameter" warning */
  count = count;

  if(sscanf(args[0], "%i", &(temp)) != 1)
    {
      errarg = 0;
      return(IWERR_ARG_TYPE);
    }
  wrq.u.sens.value = temp;

  if(iw_set_ext(skfd, ifname, SIOCSIWSENS, &wrq) < 0)
    return(IWERR_SET_EXT);
	
	iw_sockets_close(skfd);
	return(0);		
}


int  set_wlaneth_info  (char * ifname,
	                 char *		args[],		/* Command line args */
	                 int		count)		/* Args count */
{
	char temp [40];
        unsigned char i=0;
	sprintf (temp , "ifconfig ");
	strcat (temp ,ifname);
	strcat (temp ," down");
	system (temp);
	
	for(i=0;i<count ;i++)
	{
		system (args[i]);	
		sleep(1);
	}
	
	sprintf (temp , "ifconfig ");
	strcat (temp ,ifname);
	strcat (temp ," up");
	system (temp);
	
	return 0 ;
}

unsigned char	OSWIFI_ParamConfig(PWIFIPARAMCFG	pWIFIParamCfg)
{
	
	char kettmp[64];
	unsigned char ret;
	char *		ifname;
	char * wifi_args[3];
	wifi_args[0]= NULL;
	wifi_args[1]= NULL;
	wifi_args[2]= NULL;
	wifi_args[0]= "Managed" ;

#ifdef MARVELL
	ifname = "eth0";
#endif
	//配置mode 
	ret = set_mode_info(ifname, wifi_args,	1);	
	if (ret != 0)
		return ret;
	//配置key
	memset(kettmp,0,sizeof(kettmp));
	switch (pWIFIParamCfg-> ucKeyLength)
	{
		unsigned char j,i=0; 
		case 0:
			break;
		case 10:
			for (j=0,i=0;i<10;i++,j++)
			{
					if(((i%4)==0) && (j!=0))
					{
							kettmp[j]='-';
							j++ ;
					}
					kettmp[j]= pWIFIParamCfg ->aucEncryKey [i];										
			}
			break;
		case 26:
			for (j=0,i=0;i<26;i++,j++)
			{
					if(((i%4)==0) && (j!=0))
					{
							kettmp[j]='-';
							j++ ;
					}
					kettmp[j]= pWIFIParamCfg ->aucEncryKey [i];										
			}			
			break;
		default:
			return 1;
	}
	 wifi_args[0]= kettmp;
	 sleep(1);
	 ret = set_enc_info(ifname, wifi_args, 1);
	 if (ret != 0)
		return ret;
	 sleep(1);
	 wifi_args[0]= "open" ;
	 ret =  set_enc_info(ifname, wifi_args, 1);
	 if (ret != 0)
		 return ret;
	 sleep(1);
	 wifi_args[0] = pWIFIParamCfg -> aucSSID;
	 ret = set_essid_info( ifname, wifi_args,1);
	sleep(1);
	 set_commit_info( ifname, wifi_args,1);
	 
	 	
	 return ret;
}

unsigned char	OSWIFI_NetConfig(PWIFINETCFG  pWIFIConfig)
{
		char * wifi_args[3];
		char temp[3][35];
		wifi_args[0]= NULL;
		wifi_args[1]= NULL;
		wifi_args[2]= NULL;
		sprintf(temp[0], "ifconfig eth0 %d.%d.%d.%d",
						pWIFIConfig ->aucOwnIP[0],
						pWIFIConfig ->aucOwnIP[1],
						pWIFIConfig ->aucOwnIP[2],
						pWIFIConfig ->aucOwnIP[3]) ;
		sprintf(temp[1], "ifconfig eth0 netmask %d.%d.%d.%d",
		 				pWIFIConfig ->aucMask[0],
		 				pWIFIConfig ->aucMask[1],
		 				pWIFIConfig ->aucMask[2],
		 				pWIFIConfig ->aucMask[3]);
		sprintf(temp[2], "route add default gw %d.%d.%d.%d",
						pWIFIConfig ->aucGetewayIP[0],
						pWIFIConfig ->aucGetewayIP[1],
						pWIFIConfig ->aucGetewayIP[2],
						pWIFIConfig ->aucGetewayIP[3]);
		
		unsigned char i =0;
		for(i=0;i<3;i++)
		{
				wifi_args[i] =  temp [i];
		}
		set_wlaneth_info  ("eth0", wifi_args,	3);	
		return 0;
}


unsigned char OSWIFI_Connect(int* piTCPHandle,
                             unsigned long ulPeerAddr,
                             unsigned short uiPeerPort,
                             unsigned int uiTimeout)
{
  unsigned char ucResult;
  ucResult=TCP_Connect(piTCPHandle,ulPeerAddr,uiPeerPort);
  
  printf("\n**print from api connect %02x handle %02x**\n",ucResult,piTCPHandle);
  
  return ucResult;
}


unsigned char OSWIFI_Send(int  iTCPHandle,
                          unsigned char *pucInBuf,
                          unsigned short uiInLen)
{

  
  return (TCP_Send(iTCPHandle,pucInBuf,uiInLen));
}


unsigned char OSWIFI_Recv(int  iTCPHandle,
                          unsigned char *pucInBuf,
                          unsigned short *puiInLen,
                          unsigned int uiTimeout)
{
    unsigned char ucResult;
    ucResult =TCP_Recv(iTCPHandle,pucInBuf,puiInLen,uiTimeout);
  
    return ucResult;
}


unsigned char OSWIFI_Close(int  iTCPHandle)
{
    return (TCP_Close(iTCPHandle));
}


unsigned char OSWIFI_Init(POSWIFICFG pOSWIFICfg)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}

unsigned char OSWIFI_Stop(void)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}
/* Temp Added */
unsigned char	DNS_StdQuery(unsigned char* pQName,unsigned int *pauiHostIP,unsigned char* pucHostIPNum)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}

CONFIG_PARAM OSCFG_ReadParam(void)
{
	CONFIG_PARAM a;
	Uart_Printf("%s:not implement!!!\n",__func__);
	return a;
}


/////////////////////////////END///////////////////////////////////
