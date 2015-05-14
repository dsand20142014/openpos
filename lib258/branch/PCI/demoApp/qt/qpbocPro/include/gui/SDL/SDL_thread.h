/* Structures and definitions for magnetic tape I/O control commands.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* Written by H. Bergman <hennus@cybercomm.nl>.  */

#ifndef _SYS_MTIO_H
#define _SYS_MTIO_H	1

/* Get necessary definitions from system and kernel headers.  */
#include <sys/types.h>
#include <sys/ioctl.h>


/* Structure for MTIOCTOP - magnetic tape operation command.  */
struct mtop
  {
    short int mt_op;		/* Operations defined below.  */
    int mt_count;		/* How many of them.  */
  };
#define _IOT_mtop /* Hurd ioctl type field.  */ \
  _IOT (_IOTS (short), 1, _IOTS (int), 1, 0, 0)

/* Magnetic Tape operations [Not all operations supported by all drivers].  */
#define MTRESET 0	/* +reset drive in case of problems.  */
#define MTFSF	1	/* Forward space over FileMark,
			 * position at first record of next file.  */
#define MTBSF	2	/* Backward space FileMark (position before FM).  */
#define MTFSR	3	/* Forward space record.  */
#define MTBSR	4	/* Backward space record.  */
#define MTWEOF	5	/* Write an end-of-file record (mark).  */
#define MTREW	6	/* Rewind.  */
#define MTOFFL	7	/* Rewind and put the drive offline (eject?).  */
#define MTNOP	8	/* No op, set status only (read with MTIOCGET).  */
#define MTRETEN 9	/* Retension tape.  */
#define MTBSFM	10	/* +backward space FileMark, position at FM.  */
#define MTFSFM  11	/* +forward space FileMark, position at FM.  */
#define MTEOM	12	/* Goto end of recorded media (for appending files).
			   MTEOM positions after the last FM, ready for
			   appending another file.  */
#define MTERASE 13	/* Erase tape -- be careful!  */

#define MTRAS1  14	/* Run self test 1 (nondestructive).  */
#define MTRAS2	15	/* Run self test 2 (destructive).  */
#define MTRAS3  16	/* Reserved for self test 3.  */

#define MTSETBLK 20	/* Set block length (SCSI).  */
#define MTSETDENSITY 21	/* Set tape density (SCSI).  */
#define MTSEEK	22	/* Seek to block (Tandberg, etc.).  */
#define MTTELL	23	/* Tell block (Tandberg, etc.).  */
#define MTSETDRVBUFFER 24 /* Set the drive buffering according to SCSI-2.
			     Ordinary buffered operation with code 1.  */
#define MTFSS	25	/* Space forward over setmarks.  */
#define MTBSS	26	/* Space backward over setmarks.  */
#define MTWSM	27	/* Write setmarks.  */

#define MTLOCK  28	/* Lock the drive door.  */
#define MTUNLOCK 29	/* Unlock the drive door.  */
#define MTLOAD  30	/* Execute the SCSI load command.  */
#define MTUNLOAD 31	/* Execute the SCSI unload command.  */
#define MTCOMPRESSION 32/* Control compression with SCSI mode page 15.  */
#define MTSETPART 33	/* Change the active tape partition.  */
#define MTMKPART  34	/* Format the tape with one or two partitions.  */

/* structure for MTIOCGET - mag tape get status command */

struct mtget
  {
    long int mt_type;		/* Type of magtape device.  */
    long int mt_resid;		/* Residual count: (not sure)
				   number of bytes ignored, or
				   number of files not skipped, or
				   number of records not skipped.  */
    /* The following registers are device dependent.  */
    long int mt_dsreg;		/* Status register.  */
    long int mt_gstat;		/* Generic (device independent) status.  */
    long int mt_erreg;		/* Error register.  */
    /* The next two fields are not always used.  */
    __daddr_t mt_fileno;	/* Number of current file on tape.  */
    __daddr_t mt_blkno;		/* Current block number.  */
  };
#define _IOT_mtget /* Hurd ioctl type field.  */ \
  _IOT (_IOTS (long), 7, 0, 0, 0, 0)


/* Constants for mt_type. Not all of these are supported, and
   these are not all of the ones that are supported.  */
#define MT_ISUNKNOWN