#ifndef __MARVELL_CONFIG__H
#define __MARVELL_CONFIG__H

#include "iwlib.h"

#ifdef __cplusplus
extern "C" {
#endif

int get_info(char *	 ifname,
	    			 struct  wireless_info *	info);
int set_mode_info(char *		ifname,
				          char *		args[],		/* Command line args */
				          int		count);		/* Args count */
int set_essid_info( char *	ifname,
					          char *	args[],		/* Command line args */
					          int		count);		/* Args count */
int set_freq_info(char *		ifname,
				           char *		args[],		/* Command line args */
				           int		count);		/* Args count */					          				          	     									 
int set_bitrate_info(char *		ifname,
					          char *		args[],		/* Command line args */
					          int		count);		/* Args count */
int  set_enc_info(char *		ifname,
			            char *		args[],		/* Command line args */
			            int		count);		/* Args count */			
int  set_power_info(char *		ifname,
			              char *		args[],		/* Command line args */
			              int		count);		/* Args count */			 
			              
int  set_commit_info(char *		ifname,
		               	 char *		args[],		/* Command line args */
		              	 int		count);		/* Args count */		
		              	 
int set_nick_info(char *		ifname,
				          char *		args[],		/* Command line args */
				          int		count);		/* Args count */	
int set_frag_info (char *		ifname,
				           char *		args[],		/* Command line args */
				           int		count);		/* Args count */	
int set_txpower_info (char *		ifname,
				               char *		args[],		/* Command line args */
				               int		count);		/* Args count */				           
int set_modulation_info (char *		ifname,
				                 char *		args[],		/* Command line args */
				                 int		count);		/* Args count */				           
int set_rts_info  (char *		ifname,
	                 char *		args[],		/* Command line args */
	                 int		count);		/* Args count */				           
int  set_retry_info (char *		ifname,
	                 	char *		args[],		/* Command line args */
	                 	int		count);		/* Args count */				           
int  set_sens_info(char *		ifname,
	                 char *		args[],		/* Command line args */
	                 int		count);		/* Args count */			 	                              
int  set_wlaneth_info  (char * ifname,
	                 char *		args[],		/* Command line args */
	                 int		count);		/* Args count */	                           			          	              	 	              


#ifdef __cplusplus
}
#endif			                         		          
#endif
