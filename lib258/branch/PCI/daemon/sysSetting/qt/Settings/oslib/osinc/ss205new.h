/* linux/drivers/char/ss205new.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef   _LINUX_SS205_H
#define   _LINUX_SS205_H
#include <linux/ioctl.h>

typedef struct {
  unsigned int row;/*24*/
  unsigned int col;/*24*/
}fmode_t;


typedef struct
{
  unsigned short uiVoltage; /*50(5v)*/
  //unsigned short uiStepSpeed;
  //unsigned short uiStrobeNB;
  //unsigned char  ucSyncAdjust;
  //unsigned char  ucCheckPaper;
  fmode_t fmode;
} OSPRNCFG;


#define SS205_IOC_MAGIC   'S'

#define SS205_FEED_PAPER				_IOW(SS205_IOC_MAGIC,  1, int)
#define SS205_PRINT_VCC 				_IOW(SS205_IOC_MAGIC,  2, int)
#define SS205_PRINT_FONT 				_IOW(SS205_IOC_MAGIC,  3, int)

/*test */
#define SS205_PRINT_TEST      			_IOW(SS205_IOC_MAGIC,  4, int)

#define SS205_PRINT_CHECKPAPER      		_IOWR(SS205_IOC_MAGIC, 5, int)
#define SS205_PRINT_CFGPARAM      		_IOWR(SS205_IOC_MAGIC, 6, int)
#define SS205_PRINT_CLEANBUF      		_IOWR(SS205_IOC_MAGIC, 7, int)
#define SS205_PRINT_CHKSTATUS      		_IOWR(SS205_IOC_MAGIC, 8, int)
#define SS205_PRINT_SETVF      		_IOWR(SS205_IOC_MAGIC, 9, int)


#define SS205_IOC_MAXNR     9

#endif

