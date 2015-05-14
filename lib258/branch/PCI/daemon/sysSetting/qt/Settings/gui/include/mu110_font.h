#ifndef MU110_FONT_H
#define MU110_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_FONT   96
void rd_dat_5x7en(unsigned char Code, unsigned char *DotDataBuf);
void rd_dat_7x9en(unsigned char Code, unsigned char *DotDataBuf);
void rd_dat_7x9gb(unsigned char Code, unsigned char *DotDataBuf);

#ifdef __cplusplus
}
#endif

#endif
