#ifndef __SAND_TOOLS_H__
#define __SAND_TOOLS_H__

#ifdef __cplusplus 
extern "C" { 
#endif

#ifndef __SAND_TOOLS__
#define TAXCOMMON_EXTERN extern
#else
#define TAXCOMMON_EXTERN
#endif

TAXCOMMON_EXTERN	void long_to_0_asc(unsigned char *str, unsigned int num, int L);

#ifdef __cplusplus 
}
#endif

#endif
