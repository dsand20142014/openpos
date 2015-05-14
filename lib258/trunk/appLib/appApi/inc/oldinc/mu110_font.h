#ifndef font_h

#define MAX_FONT   96

void rd_dat_5x7en(unsigned char Code, unsigned char *DotDataBuf);
void rd_dat_7x9en(unsigned char Code, unsigned char *DotDataBuf);
//void EN_FONT_ReadDotData_GB5x7(uint8 Code, uint8 *DotDataBuf);
void rd_dat_7x9gb(unsigned char Code, unsigned char *DotDataBuf);

#endif

