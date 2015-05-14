/*
 * dv1394-private.h - DV input/output over IEEE 1394 on OHCI chips
 *   Copyright (C)2001 Daniel Maas <dmaas@dcine.com>
 *     receive by Dan Dennedy <dan@dennedy.org>
 *
 * based on:
 *   video1394.h - driver for OHCI 1394 boards
 *   Copyright (C)1999,2000 Sebastien Rougeaux <sebastien.rougeaux@anu.edu.au>
 *                          Peter Schlaile <udbz@rz.uni-karlsruhe.de>
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _DV_1394_PRIVATE_H
#define _DV_1394_PRIVATE_H

#include "ieee1394.h"
#include "ohci1394.h"
#include "dma.h"

/* data structures private to the dv1394 driver */
/* none of this is exposed to user-space */


/*
   the 8-byte CIP (Common Isochronous Packet) header that precedes
   each packet of DV data.

   See the IEC 61883 standard.
*/

struct CIP_header { unsigned char b[8]; };

static inline void fill_cip_header(struct CIP_header *cip,
				   unsigned char source_node_id,
				   unsigned long counter,
				   enum pal_or_ntsc format,
				   unsigned long timestamp)
{
	cip->b[0] = source_node_id;
	cip->b[1] = 0x78; /* packet size in quadlets (480/4) - even for empty packets! */
	cip->b[2] = 0x00;
	cip->b[3] = counter;

	cip->b[4] = 0x80; /* const */

	switch(format) {
	case DV1394_PAL:
		cip->b[5] = 0x80;
		break;
	case DV1394_NTSC:
		cip->b[5] = 0x00;
		break;
	}

	cip->b[6] = timestamp >> 8;
	cip->b[7] = timestamp & 0xFF;
}



/*
   DMA commands used to program the OHCI's DMA engine

   See the Texas Instruments OHCI 1394 chipset documentation.
*/

struct output_more_immediate { __le32 q[8]; };
struct output_more { __le32 q[4]; };
struct output_last { __le32 q[4]; };
struct input_more { __le32 q[4]; };
struct input_last { __le32 q[4]; };

/* outputs */

static inline void fill_output_more_immediate(struct output_more_immediate *omi,
					      unsigned char tag,
					      unsigned char channel,
					      unsigned char sync_tag,
					      unsigned int  payload_size)
{
	omi->q[0] = cpu_to_le32(0x02000000 | 8); /* OUTPUT_MORE_IMMEDIATE; 8 is the size of the IT header */
	omi->q[1] = cpu_to_le32(0);
	omi->q[2] = cpu_to_le32(0);
	omi->q[3] = cpu_to_le32(0);

	/* IT packet header */
	omi->q[4] = cpu_to_le32(  (0x0 << 16)  /* IEEE1394_SPEED_100 */
				  | (tag << 14)
				  | (channel << 8)
				  | (TCODE_ISO_DATA << 4)
				  | (sync_tag) );

	/* reserved field; mimic behavior of my Sony DSR-40 */
	omi->q[5] = cpu_to_le32((payload_size << 16) | (0x7F << 8) | 0xA0);

	omi->q[6] = cpu_to_le32(0);
	omi->q[7] = cpu_to_le32(0);
}

static inline void fill_output_more(struct output_more *om,
				    unsigned int data_size,
				    unsigned long data_phys_addr)
{
	om->q[0] = cpu_to_le32(data_size);
	om->q[1] = cpu_to_le32(data_phys_addr);
	om->q[2] = cpu_to_le32(0);
	om->q[3] = cpu_to_le32(0);
}

static inline void fill_output_last(struct output_last *ol,
				    int want_timestamp,
				    int want_interrupt,
				    unsigned int data_size,
				    unsigned long data_phys_addr)
{
	u32 temp = 0;
	temp |= 1 << 28; /* OUTPUT_LAST */

	if (want_timestamp) /* controller will update timestamp at DMA time */
		temp |= 1 << 27;

	if (want_interrupt)
		temp |= 3 << 20;

	temp |= 3 << 18; /* must take branch */
	temp |= data_size;

	ol->q[0] = cpu_to_le32(temp);
	ol->q[1] = cpu_to_le32(data_phys_addr);
	ol->q[2] = cpu_to_le32(0);
	ol->q[3] = cpu_to_le32(0);
}

/* inputs */

static inline void fill_input_more(struct input_more *im,
				   int want_interrupt,
				   unsigned int data_size,
				   unsigned long data_phys_addr)
{
	u32 temp =  2 << 28; /* INPUT_MORE */
	temp |= 8 << 24; /* s = 1, update xferStatus and resCount */
	if (want_interrupt)
		temp |= 0 << 20; /* interrupts, i=0 in packet-per-buffer mode */
	temp |= 0x0 << 16; /* disable branch to address for packet-per-buffer mode */
	                       /* disable wait on sync field, not used in DV :-( */
       	temp |= data_size;

	im->q[0] = cpu_to_le32(temp);
	im->q[1] = cpu_to_le32(data_phys_addr);
	im->q[2] = cpu_to_le32(0); /* branchAddress and Z not use in packet-per-buffer mode */
	im->q[3] = cpu_to_le32(0); /* xferStatus & resCount, resCount must be initialize to data_size */
}
 
static inline void fill_input_last(struct input_last *il,
				    int want_interrupt,
				    unsigned int data_size,
				    unsigned long data_phys_addr)
{
	u32 temp =  3 << 28; /* INPUT_LAST */
	temp |= 8 << 24; /* s = 1, update xferStatus and resCount */
	if (want_interrupt)
		temp |= 3 << 20; /* enable interrupts */
	temp |= 0xC << 16; /* enable branch to address */
	                       /* disable wait on sync field, not used in DV :-( */
	temp |= data_size;

	il->q[0] = cpu_to_le32(temp);
	il->q[1] = cpu_to_le32(data_phys_addr);
	il->q[2] = cpu_to_le32(1); /* branchAddress (filled in later) and Z = 1 descriptor in next block */
	il->q[3] = cpu_to_le32(data_size); /* xferStatus & resCount, resCount must be initialize to data_size */
}



/*
   A "DMA descriptor block" consists of several contiguous DMA commands.
   struct DMA_descriptor_block encapsulates all of the commands necessary
   to send one packet of DV data.

   There are three different types of these blocks:

        1) command to send an empty packet (CIP header only, no DV data):

	    OUTPUT_MORE-Immediate <-- contains the iso header in-line
	    OUTPUT_LAST           <-- points to the CIP header

	2) command to send a full packet when the DV data payload does NOT
	   cross a page boundary:

	    OUTPUT_MORE-Immediate <-- contains the iso header in-line
	    OUTPUT_MORE           <-- points to the CIP header
	    OUTPUT_LAST           <-- points to entire DV data payload

	3) command to send a full packet when the DV payload DOES cross
	   a page boundary:

	    OUTPUT_MORE-Immediate <-- contains the iso header in-line
	    OUTPUT_MORE           <-- points to the CIP header
	    OUTPUT_MORE           <-- points to first part of DV data payload
	    OUTPUT_LAST           <-- points to second part of DV data payload

   This struct describes all three block types using unions.

   !!! It is vital that an even number of these descriptor blocks fit on one
   page of memory, since a block cannot cross a page boundary !!!

 */

struct DMA_descriptor_block {

	union {
		struct {
			/*  iso header, common to all output block types */
			struct output_more_immediate omi;

			union {
				/* empty packet */
				struct {
					struct output_last ol;  /* CIP header */
				} empty;

				/* full packet */
				struct {
					struct output_more om;  /* CIP header */

					union {
				               /* payload does not cross page boundary */
						struct {
							struct output_last ol;  /* data payload */
						} nocross;

				               /* payload crosses page boundary */
						struct {
							struct output_more om;  /* data payload */
							struct output_last ol;  /* data payload */
						} cross;
					} u;

				} full;
			} u;
		} out;

		struct {
			struct input_last il;
		} in;

	} u;

	/* ensure that PAGE_SIZE % sizeof(struct DMA_descriptor_block) == 0
	   by padding out to 128 bytes */
	u32 __pad__[12];
};


/* struct frame contains all data associated with one frame in the
   ringbuffer these are allocated when the DMA context is initialized
   do_dv1394_init().  They are re-used after the card finishes
   transmitting the frame. */

struct video_card; /* forward declaration */

struct frame {

	/* points to the struct video_card that owns this frame */
	struct video_card *video;

	/* index of this frame in video_card->frames[] */
	unsigned int frame_num;

	/* FRAME_CLEAR - DMA program not set up, waiting for data
	   FRAME_READY - DMA program written, ready to transmit

	   Changes to these should be locked against the interrupt
	*/
	enum {
		FRAME_CLEAR = 0,
		FRAME_READY
	} state;

	/* whether this frame has been DMA'ed already; used only from
	   the IRQ handler to determine whether the frame can be reset */
	int done;


	/* kernel virtual pointer to the start of this frame's data in
	   the user ringbuffer. Use only for CPU access; to get the DMA
	   bus address you must go through the video->user_dma mapping */
	unsigned long data;

	/* Max # of packets per frame */
#define MAX_PACKETS 500


	/* a PAGE_SIZE memory pool for allocating CIP headers
	   !header_pool must be aligned to PAGE_SIZE! */
	struct CIP_header *header_pool;
	dma_addr_t         header_pool_dma;


	/* a physically contiguous memory pool for allocating DMA
	   descriptor blocks; usually around 64KB in size
	   !descriptor_pool must be aligned to PAGE_SIZE! */
	struct DMA_descriptor_block *descriptor_pool;
	dma_addr_t                   descriptor_pool_dma;
	unsigned long                descriptor_pool_size;


	/* # of packets allocated for this frame */
	unsigned int n_packets;


	/* below are several pointers (kernel virtual addresses, not
	   DMA bus addresses) to parts of the DMA program.  These are
	   set each time the DMA program is written in
	   frame_prepare(). They are used later on, e.g. from the
	   interrupt handler, to check the status of the frame */

	/* points to status/timestamp field of first DMA packet */
	/* (we'll check it later to monitor timestamp accuracy) */
	__le32 *frame_begin_timestamp;

	/* the timestamp we assigned to the first packet in the frame */
	u32 assigned_timestamp;

	/* pointer to the first packet's CIP header (where the timestamp goes) */
	struct CIP_header *cip_syt1;

	/* pointer to the second packet's CIP header
	   (only set if the first packet was empty) */
	struct CIP_header *cip_syt2;

	/* in order to figure out what caused an interrupt,
	   store pointers to the status fields of the two packets
	   that can cause interrupts. We'll check these from the
	   interrupt handler.
	*/
	__le32 *mid_frame_timestamp;
	__le32 *frame_end_timestamp;

	/* branch address field of final packet. This is effectively
	   the "tail" in the chain of DMA descriptor blocks.
	   We will fill it with the address of the first DMA descriptor
	   block in the subsequent frame, once it is ready.
	*/
	__le32 *frame_end_branch;

	/* the number of descriptors in the first descriptor block
	   of the frame. Needed to start DMA */
	int first_n_descriptors;
};


struct packet {
	__le16	timestamp;
	u16	invalid;
	u16	iso_header;
	__le16	data_length;
	u32	cip_h1;
	u32	cip_h2;
	unsigned char data[480];
	unsigned char padding[16]; /* force struct size =512 for page alignment */
};


/* allocate/free a frame */
static struct frame* frame_new(unsigned int frame_num, struct video_card *video);
static void frame_delete(struct frame *f);

/* reset f so that it can be used again */
static void frame_reset(struct frame *f);

/* struct video_card contains all data associated with one instance
   of the dv1394 driver
*/
enum modes {
	MODE_RECEIVE,
	MODE_TRANSMIT
};

struct video_card {

	/* ohci card to which this instance corresponds */
	struct ti_ohci *ohci;

	/* OHCI card id; the link between the VFS inode and a specific video_card
	   (essentially the device minor number) */
	int id;

	/* entry in dv1394_cards */
	struct list_head list;

	/* OHCI card IT DMA context number, -1 if not in use */
	int ohci_it_ctx;
	struct ohci1394_iso_tasklet it_tasklet;

	/* register offsets for current IT DMA context, 0 if not in use */
	u32 ohci_IsoXmitContextControlSet;
	u32 ohci_IsoXmitContextControlClear;
	u32 ohci_IsoXmitCommandPtr;

	/* OHCI card IR DMA context number, -1 if not in use */
	struct ohci1394_iso_tasklet ir_tasklet;
	int ohci_ir_ctx;

	/* register offsets for current IR DMA context, 0 if not in use */
	u32 ohci_IsoRcvContextControlSet;
	u32 ohci_IsoRcvContextControlClear;
	u32 ohci_IsoRcvCommandPtr;
	u32 ohci_IsoRcvContextMatch;


	/* CONCURRENCY CONTROL */

	/* there are THREE levels of locking associated with video_card. */

	/*
	   1) the 'open' flag - this prevents more than one process from
	   opening the device. (the driver currently assumes only one opener).
	   This is a regular int, but use test_and_set_bit() (on bit zero) 
	   for atomicity.
	 */
	unsigned long open;

	/*
	   2) the spinlock - this provides mutual exclusion between the interrupt
	   handler and process-context operations. Generally you must take the
	   spinlock under the following conditions:
	     1) DMA (and hence the interrupt handler) may be running
	     AND
	     2) you need to operate on the video_card, especially active_frame

	     It is OK to play with video_card without taking the spinlock if
	     you are certain that DMA is not running. Even if DMA is running,
	     it is OK to *read* active_frame with the lock, then drop it
	     immediately. This is safe because the interrupt handler will never
	     advance active_frame onto a frame that is not READY (and the spinlock
	     must be held while marking a frame READY).

	     spinlock is also used to protect ohci_it_ctx and ohci_ir_ctx,
	     which can be accessed from both process and interrupt context
	 */
	spinlock_t spinlock;

	/* flag to prevent spurious interrupts (which OHCI seems to
	   generate a lot :) from accessing the struct */
	int dma_running;

	/*
	  3) the sleeping mutex 'mtx' - this is used from process context only,
	  to serialize various operations on the video_card. Even though only one
	  open() is allowed, we still need to prevent multiple threads of execution
	  from entering calls like read, write, ioctl, etc.

	  I honestly can't think of a good reason to use dv1394 from several threads
	  at once, but we need to serialize anyway to prevent oopses =).

	  NOTE: if you need both spinlock and mtx, take mtx first to avoid deadlock!
	 */
	struct mutex mtx;

	/* people waiting for buffer space, please form a line here... */
	wait_queue_head_t waitq;

	/* support asynchronous I/O signals (SIGIO) */
	struct fasync_struct *fasync;

	/* the large, non-contiguous (rvmalloc()) ringbuffer for DV
           data, exposed to user-space via mmap() */
	unsigned long      dv_buf_size;
	struct dma_region  dv_buf;

	/* next byte in the ringbuffer that a write() call will fill */
	size_t write_off;

	struct frame *frames[DV1394_MAX_FRAMES];

	/* n_frames also serves as an indicator that this struct video_card is
	   initialized and ready to run DMA buffers */

	int n_frames;

	/* this is the frame that is currently "owned" by the OHCI DMA controller
	   (set to -1 iff DMA is not running)

	   ! must lock against the interrupt handler when accessing it !

	   RULES:

	       Only the interrupt handler may change active_frame if DMA
	          is running; if not, process may change it

	       If the next frame is READY, the interrupt handler will advance
	       active_frame when the current frame is finished.

	       If the next frame is CLEAR, the interrupt handler will re-transmit
	       the current frame, and the dropped_frames counter will be  incremented.

	       The interrupt handler will NEVER advance active_frame to a
	       frame that is not READY.
	*/
	int active_frame;
	int first_run;

	/* the same locking rules apply to these three fields also: */

	/* altered ONLY from process context. Must check first_clear_frame->state;
	   if it's READY, that means the ringbuffer is full with READY frames;
	   if it's CLEAR, that means one or more ringbuffer frames are CLEAR */
	unsigned int first_clear_frame;

	/* altered both by process and interrupt */
	unsigned int n_clear_frames;

	/* only altered by the interrupt */
	unsigned int dropped_frames;



	/* the CIP accumulator and continuity counter are properties
	   of the DMA stream as a whole (not a single frame), so they
	   are stored here in the video_card */

	unsigned long cip_accum;
	unsigned long cip_n, cip_d;
	unsigned int syt_offset;
	unsigned int continuity_counter;

	enum pal_or_ntsc pal_or_ntsc;

	/* redundant, but simplifies the code somewhat */
	unsigned int frame_size; /* in bytes */

	/* the isochronous channel to use, -1 if video card is inactive */
	int channel;


	/* physically contiguous packet ringbuffer for receive */
	struct dma_region packet_buf;
	unsigned long  packet_buf_size;

	unsigned int current_packet;
	int first_frame; 	/* received first start frame marker? */
	enum modes mode;
};

/*
   if the video_card is not initialized, then the ONLY fields that are valid are:
   ohci
   open
   n_frames
*/

static inline int video_card_initialized(struct video_card *v)
{
	return v->n_frames > 0;
}

static int do_dv1394_init(struct video_card *video, struct dv1394_init *init);
static int do_dv1394_init_default(struct video_card *video);
static void do_dv1394_shutdown(struct video_card *video, int free_user_buf);


/* NTSC empty packet rate accurate to within 0.01%,
   calibrated against a Sony DSR-40 DVCAM deck */

#define CIP_N_NTSC   68000000
#define CIP_D_NTSC 1068000000

#define CIP_N_PAL  1
#define CIP_D_PAL 16

#endif /* _DV_1394_PRIVATE_H */

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 config VIDEO_BT848
	tristate "BT848 Video For Linux"
	depends on VIDEO_DEV && PCI && I2C && VIDEO_V4L2 && INPUT
	select I2C_ALGOBIT
	select VIDEO_BTCX
	select VIDEOBUF_DMA_SG
	select VIDEO_IR
	select VIDEO_TUNER
	select VIDEO_TVEEPROM
	select VIDEO_MSP3400 if VIDEO_HELPER_CHIPS_AUTO
	select VIDEO_TVAUDIO if VIDEO_HELPER_CHIPS_AUTO
	select VIDEO_TDA7432 if VIDEO_HELPER_CHIPS_AUTO
	select VIDEO_SAA6588 if VIDEO_HELPER_CHIPS_AUTO
	---help---
	  Support for BT848 based frame grabber/overlay boards. This includes
	  the Miro, Hauppauge and STB boards. Please read the material in
	  <file:Documentation/video4linux/bttv/> for more information.

	  To compile this driver as a module, choose M here: the
	  module will be called bttv.

config VIDEO_BT848_DVB
	bool "DVB/ATSC Support for bt878 based TV cards"
	depends on VIDEO_BT848 && DVB_CORE
	select DVB_BT8XX
	---help---
	  This adds support for DVB/ATSC cards based on the BT878 chip.
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    /* OmniVision OV7610/OV7110 Camera Chip Support Code
 *
 * Copyright (c) 1999-2004 Mark McClelland <mark@alpha.dyndns.org>
 * http://alpha.dyndns.org/ov511/
 *
 * Color fixes by by Orion Sky Lawlor <olawlor@acm.org> (2/26/2000)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. NO WARRANTY OF ANY KIND is expressed or implied.
 */

#define DEBUG

#include <linux/slab.h>
#include "ovcamchip_priv.h"

/* Registers */
#define REG_GAIN		0x00	/* gain [5:0] */
#define REG_BLUE		0x01	/* blue channel balance */
#define REG_RED			0x02	/* red channel balance */
#define REG_SAT			0x03	/* saturation */
#define REG_CNT			0x05	/* Y contrast */
#define REG_BRT			0x06	/* Y brightness */
#define REG_BLUE_BIAS		0x0C	/* blue channel bias [5:0] */
#define REG_RED_BIAS		0x0D	/* red channel bias [5:0] */
#define REG_GAMMA_COEFF		0x0E	/* gamma settings */
#define REG_WB_RANGE		0x0F	/* AEC/ALC/S-AWB settings */
#define REG_EXP			0x10	/* manual exposure setting */
#define REG_CLOCK		0x11	/* polarity/clock prescaler */
#define REG_FIELD_DIVIDE	0x16	/* field interval/mode settings */
#define REG_HWIN_START		0x17	/* horizontal window start */
#define REG_HWIN_END		0x18	/* horizontal window end */
#define REG_VWIN_START		0x19	/* vertical window start */
#define REG_VWIN_END		0x1A	/* vertical window end */
#define REG_PIXEL_SHIFT   	0x1B	/* pixel shift */
#define REG_YOFFSET		0x21	/* Y channel offset */
#define REG_UOFFSET		0x22	/* U channel offset */
#define REG_ECW			0x24	/* exposure white level for AEC */
#define REG_ECB			0x25	/* exposure black level for AEC */
#define REG_FRAMERATE_H		0x2A	/* frame rate MSB + misc */
#define REG_FRAMERATE_L		0x2B	/* frame rate LSB */
#define REG_ALC			0x2C	/* Auto Level Control settings */
#define REG_VOFFSET		0x2E	/* V channel offset adjustment */
#define REG_ARRAY_BIAS		0x2F	/* array bias -- don't change */
#define REG_YGAMMA		0x33	/* misc gamma settings [7:6] */
#define REG_BIAS_ADJUST		0x34	/* misc bias settings */

/* Window parameters */
#define HWSBASE 0x38
#define HWEBASE 0x3a
#define VWSBASE 0x05
#define VWEBASE 0x05

struct ov7x10 {
	int auto_brt;
	int auto_exp;
	int bandfilt;
	int mirror;
};

/* Lawrence Glaister <lg@jfm.bc.ca> reports:
 *
 * Register 0x0f in the 7610 has the following effects:
 *
 * 0x85 (AEC method 1): Best overall, good contrast range
 * 0x45 (AEC method 2): Very overexposed
 * 0xa5 (spec sheet default): Ok, but the black level is
 *	shifted resulting in loss of contrast
 * 0x05 (old driver setting): very overexposed, too much
 *	contrast
 */
static struct ovcamchip_regvals regvals_init_7x10[] = {
	{ 0x10, 0xff },
	{ 0x16, 0x03 },
	{ 0x28, 0x24 },
	{ 0x2b, 0xac },
	{ 0x12, 0x00 },
	{ 0x38, 0x81 },
	{ 0x28, 0x24 },	/* 0c */
	{ 0x0f, 0x85 },	/* lg's setting */
	{ 0x15, 0x01 },
	{ 0x20, 0x1c },
	{ 0x23, 0x2a },
	{ 0x24, 0x10 },
	{ 0x25, 0x8a },
	{ 0x26, 0xa2 },
	{ 0x27, 0xc2 },
	{ 0x2a, 0x04 },
	{ 0x2c, 0xfe },
	{ 0x2d, 0x93 },
	{ 0x30, 0x71 },
	{ 0x31, 0x60 },
	{ 0x32, 0x26 },
	{ 0x33, 0x20 },
	{ 0x34, 0x48 },
	{ 0x12, 0x24 },
	{ 0x11, 0x01 },
	{ 0x0c, 0x24 },
	{ 0x0d, 0x24 },
	{ 0xff, 0xff },	/* END MARKER */
};

/* This initializes the OV7x10 camera chip and relevant variables. */
static int ov7x10_init(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov7x10 *s;
	int rc;

	DDEBUG(4, &c->dev, "entered");

	rc = ov_write_regvals(c, regvals_init_7x10);
	if (rc < 0)
		return rc;

	ov->spriv = s = kzalloc(sizeof *s, GFP_KERNEL);
	if (!s)
		return -ENOMEM;

	s->auto_brt = 1;
	s->auto_exp = 1;

	return rc;
}

static int ov7x10_free(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);

	kfree(ov->spriv);
	return 0;
}

static int ov7x10_set_control(struct i2c_client *c,
			      struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov7x10 *s = ov->spriv;
	int rc;
	int v = ctl->value;

	switch (ctl->id) {
	case OVCAMCHIP_CID_CONT:
		rc = ov_write(c, REG_CNT, v >> 8);
		break;
	case OVCAMCHIP_CID_BRIGHT:
		rc = ov_write(c, REG_BRT, v >> 8);
		break;
	case OVCAMCHIP_CID_SAT:
		rc = ov_write(c, REG_SAT, v >> 8);
		break;
	case OVCAMCHIP_CID_HUE:
		rc = ov_write(c, REG_RED, 0xFF - (v >> 8));
		if (rc < 0)
			goto out;

		rc = ov_write(c, REG_BLUE, v >> 8);
		break;
	case OVCAMCHIP_CID_EXP:
		rc = ov_write(c, REG_EXP, v);
		break;
	case OVCAMCHIP_CID_FREQ:
	{
		int sixty = (v == 60);

		rc = ov_write_mask(c, 0x2a, sixty?0x00:0x80, 0x80);
		if (rc < 0)
			goto out;

		rc = ov_write(c, 0x2b, sixty?0x00:0xac);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x13, 0x10, 0x10);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x13, 0x00, 0x10);
		break;
	}
	case OVCAMCHIP_CID_BANDFILT:
		rc = ov_write_mask(c, 0x2d, v?0x04:0x00, 0x04);
		s->bandfilt = v;
		break;
	case OVCAMCHIP_CID_AUTOBRIGHT:
		rc = ov_write_mask(c, 0x2d, v?0x10:0x00, 0x10);
		s->auto_brt = v;
		break;
	case OVCAMCHIP_CID_AUTOEXP:
		rc = ov_write_mask(c, 0x29, v?0x00:0x80, 0x80);
		s->auto_exp = v;
		break;
	case OVCAMCHIP_CID_MIRROR:
		rc = ov_write_mask(c, 0x12, v?0x40:0x00, 0x40);
		s->mirror = v;
		break;
	default:
		DDEBUG(2, &c->dev, "control not supported: %d", ctl->id);
		return -EPERM;
	}

out:
	DDEBUG(3, &c->dev, "id=%d, arg=%d, rc=%d", ctl->id, v, rc);
	return rc;
}

static int ov7x10_get_control(struct i2c_client *c,
			      struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov7x10 *s = ov->spriv;
	int rc = 0;
	unsigned char val = 0;

	switch (ctl->id) {
	case OVCAMCHIP_CID_CONT:
		rc = ov_read(c, REG_CNT, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_BRIGHT:
		rc = ov_read(c, REG_BRT, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_SAT:
		rc = ov_read(c, REG_SAT, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_HUE:
		rc = ov_read(c, REG_BLUE, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_EXP:
		rc = ov_read(c, REG_EXP, &val);
		ctl->value = val;
		break;
	case OVCAMCHIP_CID_BANDFILT:
		ctl->value = s->bandfilt;
		break;
	case OVCAMCHIP_CID_AUTOBRIGHT:
		ctl->value = s->auto_brt;
		break;
	case OVCAMCHIP_CID_AUTOEXP:
		ctl->value = s->auto_exp;
		break;
	case OVCAMCHIP_CID_MIRROR:
		ctl->value = s->mirror;
		break;
	default:
		DDEBUG(2, &c->dev, "control not supported: %d", ctl->id);
		return -EPERM;
	}

	DDEBUG(3, &c->dev, "id=%d, arg=%d, rc=%d", ctl->id, ctl->value, rc);
	return rc;
}

static int ov7x10_mode_init(struct i2c_client *c, struct ovcamchip_window *win)
{
	int qvga = win->quarter;

	/******** QVGA-specific regs ********/

	ov_write(c, 0x14, qvga?0x24:0x04);

	/******** Palette-specific regs ********/

	if (win->format == VIDEO_PALETTE_GREY) {
		ov_write_mask(c, 0x0e, 0x40, 0x40);
		ov_write_mask(c, 0x13, 0x20, 0x20);
	} else {
		ov_write_mask(c, 0x0e, 0x00, 0x40);
		ov_write_mask(c, 0x13, 0x00, 0x20);
	}

	/******** Clock programming ********/

	ov_write(c, 0x11, win->clockdiv);

	/******** Resolution-specific ********/

	if (win->width == 640 && win->height == 480)
		ov_write(c, 0x35, 0x9e);
	else
		ov_write(c, 0x35, 0x1e);

	return 0;
}

static int ov7x10_set_window(struct i2c_client *c, struct ovcamchip_window *win)
{
	int ret, hwscale, vwscale;

	ret = ov7x10_mode_init(c, win);
	if (ret < 0)
		return ret;

	if (win->quarter) {
		hwscale = 1;
		vwscale = 0;
	} else {
		hwscale = 2;
		vwscale = 1;
	}

	ov_write(c, 0x17, HWSBASE + (win->x >> hwscale));
	ov_write(c, 0x18, HWEBASE + ((win->x + win->width) >> hwscale));
	ov_write(c, 0x19, VWSBASE + (win->y >> vwscale));
	ov_write(c, 0x1a, VWEBASE + ((win->y + win->height) >> vwscale));

	return 0;
}

static int ov7x10_command(struct i2c_client *c, unsigned int cmd, void *arg)
{
	switch (cmd) {
	case OVCAMCHIP_CMD_S_CTRL:
		return ov7x10_set_control(c, arg);
	case OVCAMCHIP_CMD_G_CTRL:
		return ov7x10_get_control(c, arg);
	case OVCAMCHIP_CMD_S_MODE:
		return ov7x10_set_window(c, arg);
	default:
		DDEBUG(2, &c->dev, "command not supported: %d", cmd);
		return -ENOIOCTLCMD;
	}
}

struct ovcamchip_ops ov7x10_ops = {
	.init    =	ov7x10_init,
	.free    =	ov7x10_free,
	.command =	ov7x10_command,
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 INDX( 	                 (       è       Î                    ¤?     h X     ’?      ¢Ê_<ŽÐ—í:%
£ÎÐÊ_<ŽÐ¥WÓÀ„Ð        ¯               v e r m i l i o n . h               ’?      ¢Ê_<ŽÐ—í:%
£ÎÐÊ_<ŽÐ¥WÓÀ„Ð        ¯               v e r m i l i o n . h               ’?      ¢Ê_<ŽÐ—í:%
£ÎÐÊ_<ŽÐ¥WÓÀ„Ð        ¯               v e r m i l i o n . h               ’?      ¢Ê_<ŽÐ—í:%
£ÎÐÊ_<ŽÐ¥WÓÀ„Ð        ¯               v e r m i l i o n . h                ¢Ê_<ŽÐ—í:%
£ ÐÊ_<ŽÐ¥WÓÀ„Ð        ¯               v e r m i l i o n . h                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        /*
 * linux/drivers/video/riva/fbdev.c - nVidia RIVA 128/TNT/TNT2 fb driver
 *
 * Maintained by Ani Joshi <ajoshi@shell.unixbox.com>
 *
 * Copyright 1999-2000 Jeff Garzik
 *
 * Contributors:
 *
 *	Ani Joshi:  Lots of debugging and cleanup work, really helped
 *	get the driver going
 *
 *	Ferenc Bakonyi:  Bug fixes, cleanup, modularization
 *
 *	Jindrich Makovicka:  Accel code help, hw cursor, mtrr
 *
 *	Paul Richards:  Bug fixes, updates
 *
 * Initial template from skeletonfb.c, created 28 Dec 1997 by Geert Uytterhoeven
 * Includes riva_hw.c from nVidia, see copyright below.
 * KGI code provided the basis for state storage, init, and mode switching.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * Known bugs and issues:
 *	restoring text mode fails
 *	doublescan modes are broken
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/backlight.h>
#include <linux/bitrev.h>
#ifdef CONFIG_MTRR
#include <asm/mtrr.h>
#endif
#ifdef CONFIG_PPC_OF
#include <asm/prom.h>
#include <asm/pci-bridge.h>
#endif
#ifdef CONFIG_PMAC_BACKLIGHT
#include <asm/machdep.h>
#include <asm/backlight.h>
#endif

#include "rivafb.h"
#include "nvreg.h"

/* version number of this driver */
#define RIVAFB_VERSION "0.9.5b"

/* ------------------------------------------------------------------------- *
 *
 * various helpful macros and constants
 *
 * ------------------------------------------------------------------------- */
#ifdef CONFIG_FB_RIVA_DEBUG
#define NVTRACE          printk
#else
#define NVTRACE          if(0) printk
#endif

#define NVTRACE_ENTER(...)  NVTRACE("%s START\n", __func__)
#define NVTRACE_LEAVE(...)  NVTRACE("%s END\n", __func__)

#ifdef CONFIG_FB_RIVA_DEBUG
#define assert(expr) \
	if(!(expr)) { \
	printk( "Assertion failed! %s,%s,%s,line=%d\n",\
	#expr,__FILE__,__func__,__LINE__); \
	BUG(); \
	}
#else
#define assert(expr)
#endif

#define PFX "rivafb: "

/* macro that allows you to set overflow bits */
#define SetBitField(value,from,to) SetBF(to,GetBF(value,from))
#define SetBit(n)		(1<<(n))
#define Set8Bits(value)		((value)&0xff)

/* HW cursor parameters */
#define MAX_CURS		32

/* ------------------------------------------------------------------------- *
 *
 * prototypes
 *
 * ------------------------------------------------------------------------- */

static int rivafb_blank(int blank, struct fb_info *info);

/* ------------------------------------------------------------------------- *
 *
 * card identification
 *
 * ------------------------------------------------------------------------- */

static struct pci_device_id rivafb_pci_tbl[] = {
	{ PCI_VENDOR_ID_NVIDIA_SGS, PCI_DEVICE_ID_NVIDIA_SGS_RIVA128,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_TNT,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_TNT2,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_UTNT2,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_VTNT2,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_UVTNT2,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_ITNT2,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE_SDR,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE_DDR,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_QUADRO,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE2_MX,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE2_MX2,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE2_GO,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_QUADRO2_MXR,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE2_GTS,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE2_GTS2,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE2_ULTRA,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_QUADRO2_PRO,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE4_MX_460,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE4_MX_440,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	// NF2/IGP version, GeForce 4 MX, NV18
	{ PCI_VENDOR_ID_NVIDIA, 0x01f0,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE4_MX_420,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE4_440_GO,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE4_420_GO,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE4_420_GO_M32,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_QUADRO4_500XGL,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE4_440_GO_M64,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_QUADRO4_200,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_QUADRO4_550XGL,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_QUADRO4_500_GOGL,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_IGEFORCE2,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE3,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE3_1,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE3_2,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_QUADRO_DDC,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE4_TI_4600,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE4_TI_4400,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE4_TI_4200,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
 	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_QUADRO4_900XGL,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_QUADRO4_750XGL,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_QUADRO4_700XGL,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_GEFORCE_FX_GO_5200,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ 0, } /* terminate list */
};
MODULE_DEVICE_TABLE(pci, rivafb_pci_tbl);

/* ------------------------------------------------------------------------- *
 *
 * global variables
 *
 * ------------------------------------------------------------------------- */

/* command line data, set in rivafb_setup() */
static int flatpanel __devinitdata = -1; /* Autodetect later */
static int forceCRTC __devinitdata = -1;
static int noaccel   __devinitdata = 0;
#ifdef CONFIG_MTRR
static int nomtrr __devinitdata = 0;
#endif
#ifdef CONFIG_PMAC_BACKLIGHT
static int backlight __devinitdata = 1;
#else
static int backlight __devinitdata = 0;
#endif

static char *mode_option __devinitdata = NULL;
static int  strictmode       = 0;

static struct fb_fix_screeninfo __devinitdata rivafb_fix = {
	.type		= FB_TYPE_PACKED_PIXELS,
	.xpanstep	= 1,
	.ypanstep	= 1,
};

static struct fb_var_screeninfo __devinitdata rivafb_default_var = {
	.xres		= 640,
	.yres		= 480,
	.xres_virtual	= 640,
	.yres_virtual	= 480,
	.bits_per_pixel	= 8,
	.red		= {0, 8, 0},
	.green		= {0, 8, 0},
	.blue		= {0, 8, 0},
	.transp		= {0, 0, 0},
	.activate	= FB_ACTIVATE_NOW,
	.height		= -1,
	.width		= -1,
	.pixclock	= 39721,
	.left_margin	= 40,
	.right_margin	= 24,
	.upper_margin	= 32,
	.lower_margin	= 11,
	.hsync_len	= 96,
	.vsync_len	= 2,
	.vmode		= FB_VMODE_NONINTERLACED
};

/* from GGI */
static const struct riva_regs reg_template = {
	{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	/* ATTR */
	 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	 0x41, 0x01, 0x0F, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* CRT  */
	 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE3,	/* 0x10 */
	 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* 0x20 */
	 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* 0x30 */
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00,							/* 0x40 */
	 },
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,	/* GRA  */
	 0xFF},
	{0x03, 0x01, 0x0F, 0x00, 0x0E},				/* SEQ  */
	0xEB							/* MISC */
};

/*
 * Backlight control
 */
#ifdef CONFIG_FB_RIVA_BACKLIGHT
/* We do not have any information about which values are allowed, thus
 * we used safe values.
 */
#define MIN_LEVEL 0x158
#define MAX_LEVEL 0x534
#define LEVEL_STEP ((MAX_LEVEL - MIN_LEVEL) / FB_BACKLIGHT_MAX)

static int riva_bl_get_level_brightness(struct riva_par *par,
		int level)
{
	struct fb_info *info = pci_get_drvdata(par->pdev);
	int nlevel;

	/* Get and convert the value */
	/* No locking on bl_curve since accessing a single value */
	nlevel = MIN_LEVEL + info->bl_curve[level] * LEVEL_STEP;

	if (nlevel < 0)
		nlevel = 0;
	else if (nlevel < MIN_LEVEL)
		nlevel = MIN_LEVEL;
	else if (nlevel > MAX_LEVEL)
		nlevel = MAX_LEVEL;

	return nlevel;
}

static int riva_bl_update_status(struct backlight_device *bd)
{
	struct riva_par *par = bl_get_data(bd);
	U032 tmp_pcrt, tmp_pmc;
	int level;

	if (bd->props.power != FB_BLANK_UNBLANK ||
	    bd->props.fb_blank != FB_BLANK_UNBLANK)
		level = 0;
	else
		level = bd->props.brightness;

	tmp_pmc = NV_RD32(par->riva.PMC, 0x10F0) & 0x0000FFFF;
	tmp_pcrt = NV_RD32(par->riva.PCRTC0, 0x081C) & 0xFFFFFFFC;
	if(level > 0) {
		tmp_pcrt |= 0x1;
		tmp_pmc |= (1 << 31); /* backlight bit */
		tmp_pmc |= riva_bl_get_level_brightness(par, level) << 16; /* level */
	}
	NV_WR32(par->riva.PCRTC0, 0x081C, tmp_pcrt);
	NV_WR32(par->riva.PMC, 0x10F0, tmp_pmc);

	return 0;
}

static int riva_bl_get_brightness(struct backlight_device *bd)
{
	return bd->props.brightness;
}

static struct backlight_ops riva_bl_ops = {
	.get_brightness = riva_bl_get_brightness,
	.update_status	= riva_bl_update_status,
};

static void riva_bl_init(struct riva_par *par)
{
	struct fb_info *info = pci_get_drvdata(par->pdev);
	struct backlight_device *bd;
	char name[12];

	if (!par->FlatPanel)
		return;

#ifdef CONFIG_PMAC_BACKLIGHT
	if (!machine_is(powermac) ||
	    !pmac_has_backlight_type("mnca"))
		return;
#endif

	snprintf(name, sizeof(name), "rivabl%d", info->node);

	bd = backlight_device_register(name, info->dev, par, &riva_bl_ops);
	if (IS_ERR(bd)) {
		info->bl_dev = NULL;
		printk(KERN_WARNING "riva: Backlight registration failed\n");
		goto error;
	}

	info->bl_dev = bd;
	fb_bl_default_curve(info, 0,
		MIN_LEVEL * FB_BACKLIGHT_MAX / MAX_LEVEL,
		FB_BACKLIGHT_MAX);

	bd->props.max_brightness = FB_BACKLIGHT_LEVELS - 1;
	bd->props.brightness = bd->props.max_brightness;
	bd->props.power = FB_BLANK_UNBLANK;
	backlight_update_status(bd);

	printk("riva: Backlight initialized (%s)\n", name);

	return;

error:
	return;
}

static void riva_bl_exit(struct fb_info *info)
{
	struct backlight_device *bd = info->bl_dev;

	backlight_device_unregister(bd);
	printk("riva: Backlight unloaded\n");
}
#else
static inline void riva_bl_init(struct riva_par *par) {}
static inline void riva_bl_exit(struct fb_info *info) {}
#endif /* CONFIG_FB_RIVA_BACKLIGHT */

/* ------------------------------------------------------------------------- *
 *
 * MMIO access macros
 *
 * ------------------------------------------------------------------------- */

static inline void CRTCout(struct riva_par *par, unsigned char index,
			   unsigned char val)
{
	VGA_WR08(par->riva.PCIO, 0x3d4, index);
	VGA_WR08(par->riva.PCIO, 0x3d5, val);
}

static inline unsigned char CRTCin(struct riva_par *par,
				   unsigned char index)
{
	VGA_WR08(par->riva.PCIO, 0x3d4, index);
	return (VGA_RD08(par->riva.PCIO, 0x3d5));
}

static inline void GRAout(struct riva_par *par, unsigned char index,
			  unsigned char val)
{
	VGA_WR08(par->riva.PVIO, 0x3ce, index);
	VGA_WR08(par->riva.PVIO, 0x3cf, val);
}

static inline unsigned char GRAin(struct riva_par *par,
				  unsigned char index)
{
	VGA_WR08(par->riva.PVIO, 0x3ce, index);
	return (VGA_RD08(par->riva.PVIO, 0x3cf));
}

static inline void SEQout(struct riva_par *par, unsigned char index,
			  unsigned char val)
{
	VGA_WR08(par->riva.PVIO, 0x3c4, index);
	VGA_WR08(par->riva.PVIO, 0x3c5, val);
}

static inline unsigned char SEQin(struct riva_par *par,
				  unsigned char index)
{
	VGA_WR08(par->riva.PVIO, 0x3c4, index);
	return (VGA_RD08(par->riva.PVIO, 0x3c5));
}

static inline void ATTRout(struct riva_par *par, unsigned char index,
			   unsigned char val)
{
	VGA_WR08(par->riva.PCIO, 0x3c0, index);
	VGA_WR08(par->riva.PCIO, 0x3c0, val);
}

static inline unsigned char ATTRin(struct riva_par *par,
				   unsigned char index)
{
	VGA_WR08(par->riva.PCIO, 0x3c0, index);
	return (VGA_RD08(par->riva.PCIO, 0x3c1));
}

static inline void MISCout(struct riva_par *par, unsigned char val)
{
	VGA_WR08(par->riva.PVIO, 0x3c2, val);
}

static inline unsigned char MISCin(struct riva_par *par)
{
	return (VGA_RD08(par->riva.PVIO, 0x3cc));
}

static inline void reverse_order(u32 *l)
{
	u8 *a = (u8 *)l;
	a[0] = bitrev8(a[0]);
	a[1] = bitrev8(a[1]);
	a[2] = bitrev8(a[2]);
	a[3] = bitrev8(a[3]);
}

/* ------------------------------------------------------------------------- *
 *
 * cursor stuff
 *
 * ------------------------------------------------------------------------- */

/**
 * rivafb_load_cursor_image - load cursor image to hardware
 * @data: address to monochrome bitmap (1 = foreground color, 0 = background)
 * @par:  pointer to private data
 * @w:    width of cursor image in pixels
 * @h:    height of cursor image in scanlines
 * @bg:   background color (ARGB1555) - alpha bit determines opacity
 * @fg:   foreground color (ARGB1555)
 *
 * DESCRIPTiON:
 * Loads cursor image based on a monochrome source and mask bitmap.  The
 * image bits determines the color of the pixel, 0 for background, 1 for
 * foreground.  Only the affected region (as determined by @w and @h 
 * parameters) will be updated.
 *
 * CALLED FROM:
 * rivafb_cursor()
 */
static void rivafb_load_cursor_image(struct riva_par *par, u8 *data8,
				     u16 bg, u16 fg, u32 w, u32 h)
{
	int i, j, k = 0;
	u32 b, tmp;
	u32 *data = (u32 *)data8;
	bg = le16_to_cpu(bg);
	fg = le16_to_cpu(fg);

	w = (w + 1) & ~1;

	for (i = 0; i < h; i++) {
		b = *data++;
		reverse_order(&b);
		
		for (j = 0; j < w/2; j++) {
			tmp = 0;
#if defined (__BIG_ENDIAN)
			tmp = (b & (1 << 31)) ? fg << 16 : bg << 16;
			b <<= 1;
			tmp |= (b & (1 << 31)) ? fg : bg;
			b <<= 1;
#else
			tmp = (b & 1) ? fg : bg;
			b >>= 1;
			tmp |= (b & 1) ? fg << 16 : bg << 16;
			b >>= 1;
#endif
			writel(tmp, &par->riva.CURSOR[k++]);
		}
		k += (MAX_CURS - w)/2;
	}
}

/* ------------------------------------------------------------------------- *
 *
 * general utility functions
 *
 * ------------------------------------------------------------------------- */

/**
 * riva_wclut - set CLUT entry
 * @chip: pointer to RIVA_HW_INST object
 * @regnum: register number
 * @red: red component
 * @green: green component
