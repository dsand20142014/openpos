/*
 *   ALSA modem driver for Intel ICH (i8x0) chipsets
 *
 *	Copyright (c) 2000 Jaroslav Kysela <perex@perex.cz>
 *
 *   This is modified (by Sasha Khapyorsky <sashak@alsa-project.org>) version
 *   of ALSA ICH sound driver intel8x0.c .
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */      

#include <asm/io.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/ac97_codec.h>
#include <sound/info.h>
#include <sound/initval.h>

MODULE_AUTHOR("Jaroslav Kysela <perex@perex.cz>");
MODULE_DESCRIPTION("Intel 82801AA,82901AB,i810,i820,i830,i840,i845,MX440; "
		   "SiS 7013; NVidia MCP/2/2S/3 modems");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{Intel,82801AA-ICH},"
		"{Intel,82901AB-ICH0},"
		"{Intel,82801BA-ICH2},"
		"{Intel,82801CA-ICH3},"
		"{Intel,82801DB-ICH4},"
		"{Intel,ICH5},"
		"{Intel,ICH6},"
		"{Intel,ICH7},"
	        "{Intel,MX440},"
		"{SiS,7013},"
		"{NVidia,NForce Modem},"
		"{NVidia,NForce2 Modem},"
		"{NVidia,NForce2s Modem},"
		"{NVidia,NForce3 Modem},"
		"{AMD,AMD768}}");

static int index = -2; /* Exclude the first card */
static char *id = SNDRV_DEFAULT_STR1;	/* ID for this card */
static int ac97_clock;

module_param(index, int, 0444);
MODULE_PARM_DESC(index, "Index value for Intel i8x0 modemcard.");
module_param(id, charp, 0444);
MODULE_PARM_DESC(id, "ID string for Intel i8x0 modemcard.");
module_param(ac97_clock, int, 0444);
MODULE_PARM_DESC(ac97_clock, "AC'97 codec clock (0 = auto-detect).");

/* just for backward compatibility */
static int enable;
module_param(enable, bool, 0444);

/*
 *  Direct registers
 */
enum { DEVICE_INTEL, DEVICE_SIS, DEVICE_ALI, DEVICE_NFORCE };

#define ICHREG(x) ICH_REG_##x

#define DEFINE_REGSET(name,base) \
enum { \
	ICH_REG_##name##_BDBAR	= base + 0x0,	/* dword - buffer descriptor list base address */ \
	ICH_REG_##name##_CIV	= base + 0x04,	/* byte - current index value */ \
	ICH_REG_##name##_LVI	= base + 0x05,	/* byte - last valid index */ \
	ICH_REG_##name##_SR	= base + 0x06,	/* byte - status register */ \
	ICH_REG_##name##_PICB	= base + 0x08,	/* word - position in current buffer */ \
	ICH_REG_##name##_PIV	= base + 0x0a,	/* byte - prefetched index value */ \
	ICH_REG_##name##_CR	= base + 0x0b,	/* byte - control register */ \
};

/* busmaster blocks */
DEFINE_REGSET(OFF, 0);		/* offset */

/* values for each busmaster block */

/* LVI */
#define ICH_REG_LVI_MASK		0x1f

/* SR */
#define ICH_FIFOE			0x10	/* FIFO error */
#define ICH_BCIS			0x08	/* buffer completion interrupt status */
#define ICH_LVBCI			0x04	/* last valid buffer completion interrupt */
#define ICH_CELV			0x02	/* current equals last valid */
#define ICH_DCH				0x01	/* DMA controller halted */

/* PIV */
#define ICH_REG_PIV_MASK		0x1f	/* mask */

/* CR */
#define ICH_IOCE			0x10	/* interrupt on completion enable */
#define ICH_FEIE			0x08	/* fifo error interrupt enable */
#define ICH_LVBIE			0x04	/* last valid buffer interrupt enable */
#define ICH_RESETREGS			0x02	/* reset busmaster registers */
#define ICH_STARTBM			0x01	/* start busmaster operation */


/* global block */
#define ICH_REG_GLOB_CNT		0x3c	/* dword - global control */
#define   ICH_TRIE		0x00000040	/* tertiary resume interrupt enable */
#define   ICH_SRIE		0x00000020	/* secondary resume cpu_to_be32(host->node_count << 16
                                                | host->selfid_count);
        host->csr.topology_map[0] =
                cpu_to_be32((host->selfid_count + 2) << 16
                            | csr_crc16(host->csr.topology_map + 1,
                                        host->selfid_count + 2));

	be32_add_cpu(&host->csr.speed_map[1], 1);
        host->csr.speed_map[0] = cpu_to_be32(0x3f1 << 16
                                             | csr_crc16(host->csr.speed_map+1,
                                                         0x3f1));
}

/*
 * HI == seconds (bits 0:2)
 * LO == fractions of a second in units of 125usec (bits 19:31)
 *
 * Convert SPLIT_TIMEOUT to jiffies.
 * The default and minimum as per 1394a-2000 clause 8.3.2.2.6 is 100ms.
 */
static inline void calculate_expire(struct csr_control *csr)
{
	unsigned int usecs = (csr->split_timeout_hi & 7) * 1000000 +
			     (csr->split_timeout_lo >> 19) * 125;

	csr->expire = usecs_to_jiffies(usecs > 100000 ? usecs : 100000);
	HPSB_VERBOSE("CSR: setting expire to %lu, HZ=%u", csr->expire, HZ);
}


static void add_host(struct hpsb_host *host)
{
	struct csr1212_keyval *root;
	quadlet_t bus_info[CSR_BUS_INFO_SIZE];

	hpsb_register_addrspace(&csr_highlevel, host, &reg_ops,
				CSR_REGISTER_BASE,
				CSR_REGISTER_BASE + CSR_CONFIG_ROM);
	hpsb_register_addrspace(&csr_highlevel, host, &config_rom_ops,
				CSR_REGISTER_BASE + CSR_CONFIG_ROM,
				CSR_REGISTER_BASE + CSR_CONFIG_ROM_END);
	if (fcp) {
		hpsb_register_addrspace(&csr_highlevel, host, &fcp_ops,
					CSR_REGISTER_BASE + CSR_FCP_COMMAND,
					CSR_REGISTER_BASE + CSR_FCP_END);
	}
	hpsb_register_addrspace(&csr_highlevel, host, &map_ops,
				CSR_REGISTER_BASE + CSR_TOPOLOGY_MAP,
				CSR_REGISTER_BASE + CSR_TOPOLOGY_MAP_END);
	hpsb_register_addrspace(&csr_highlevel, host, &map_ops,
				CSR_REGISTER_BASE + CSR_SPEED_MAP,
				CSR_REGISTER_BASE + CSR_SPEED_MAP_END);

        spin_lock_init(&host->csr.lock);

        host->csr.state                 = 0;
        host->csr.node_ids              = 0;
        host->csr.split_timeout_hi      = 0;
        host->csr.split_timeout_lo      = 800 << 19;
	calculate_expire(&host->csr);
        host->csr.cycle_time            = 0;
        host->csr.bus_time              = 0;
        host->csr.bus_manager_id        = 0x3f;
        host->csr.bandwidth_available   = 4915;
	host->csr.channels_available_hi = 0xfffffffe;	/* pre-alloc ch 31 per 1394a-2000 */
        host->csr.channels_available_lo = ~0;
	host->csr.broadcast_channel = 0x80000000 | 31;

	if (host->is_irm) {
		if (host->driver->hw_csr_reg) {
			host->driver->hw_csr_reg(host, 2, 0xfffffffe, ~0);
		}
	}

	if (host->csr.max_rec >= 9)
		host->csr.max_rom = 2;
	else if (host->csr.max_rec >= 5)
		host->csr.max_rom = 1;
	else
		host->csr.max_rom = 0;

	host->csr.generation = 2;

	bus_info[1] = IEEE1394_BUSID_MAGIC;
	bus_info[2] = cpu_to_be32((hpsb_disable_irm ? 0 : 1 << CSR_IRMC_SHIFT) |
				  (1 << CSR_CMC_SHIFT) |
				  (1 << CSR_ISC_SHIFT) |
				  (0 << CSR_BMC_SHIFT) |
				  (0 << CSR_PMC_SHIFT) |
				  (host->csr.cyc_clk_acc << CSR_CYC_CLK_ACC_SHIFT) |
				  (host->csr.max_rec << CSR_MAX_REC_SHIFT) |
				  (host->csr.max_rom << CSR_MAX_ROM_SHIFT) |
				  (host->csr.generation << CSR_GENERATION_SHIFT) |
				  host->csr.lnk_spd);

	bus_info[3] = cpu_to_be32(host->csr.guid_hi);
	bus_info[4] = cpu_to_be32(host->csr.guid_lo);

	/* The hardware copy of the bus info block will be set later when a
	 * bus reset is issued. */

	csr1212_init_local_csr(host->csr.rom, bus_info, host->csr.max_rom);

	root = host->csr.rom->root_kv;

	if(csr1212_attach_keyval_to_directory(root, node_cap) != CSR1212_SUCCESS) {
		HPSB_ERR("Failed to attach Node Capabilities to root directory");
	}

	host->update_config_rom = 1;
}

static void remove_host(struct hpsb_host *host)
{
	quadlet_t bus_info[CSR_BUS_INFO_SIZE];

	bus_info[1] = IEEE1394_BUSID_MAGIC;
	bus_info[2] = cpu_to_be32((0 << CSR_IRMC_SHIFT) |
				  (0 << CSR_CMC_SHIFT) |
				  (0 << CSR_ISC_SHIFT) |
				  (0 << CSR_BMC_SHIFT) |
				  (0 << CSR_PMC_SHIFT) |
				  (hEL },	/* ICH7 */
	{ PCI_VDEVICE(INTEL, 0x7196), DEVICE_INTEL },	/* 440MX */
	{ PCI_VDEVICE(AMD, 0x7446), DEVICE_INTEL },	/* AMD768 */
	{ PCI_VDEVICE(SI, 0x7013), DEVICE_SIS },	/* SI7013 */
	{ PCI_VDEVICE(NVIDIA, 0x01c1), DEVICE_NFORCE }, /* NFORCE */
	{ PCI_VDEVICE(NVIDIA, 0x0069), DEVICE_NFORCE }, /* NFORCE2 */
	{ PCI_VDEVICE(NVIDIA, 0x0089), DEVICE_NFORCE }, /* NFORCE2s */
	{ PCI_VDEVICE(NVIDIA, 0x00d9), DEVICE_NFORCE }, /* NFORCE3 */
#if 0
	{ PCI_VDEVICE(AMD, 0x746d), DEVICE_INTEL },	/* AMD8111 */
	{ PCI_VDEVICE(AL, 0x5455), DEVICE_ALI },   /* Ali5455 */
#endif
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, snd_intel8x0m_ids);

/*
 *  Lowlevel I/O - busmaster
 */

static inline u8 igetbyte(struct intel8x0m *chip, u32 offset)
{
	return ioread8(chip->bmaddr + offset);
}

static inline u16 igetword(struct intel8x0m *chip, u32 offset)
{
	return ioread16(chip->bmaddr + offset);
}

static inline u32 igetdword(struct intel8x0m *chip, u32 offset)
{
	return ioread32(chip->bmaddr + offset);
}

static inline void iputbyte(struct intel8x0m *chip, u32 offset, u8 val)
{
	iowrite8(val, chip->bmaddr + offset);
}

static inline void iputword(struct intel8x0m *chip, u32 offset, u16 val)
{
	iowrite16(val, chip->bmaddr + offset);
}

static inline void iputdword(struct intel8x0m *chip, u32 offset, u32 val)
{
	iowrite32(val, chip->bmaddr + offset);
}

/*
 *  Lowlevel I/O - AC'97 registers
 */

static inline u16 iagetword(struct intel8x0m *chip, u32 offset)
{
	return ioread16(chip->addr + offset);
}

static inline void iaputword(struct intel8x0m *chip, u32 offset, u16 val)
{
	iowrite16(val, chip->addr + offset);
}

/*
 *  Basic I/O
 */

/*
 * access to AC97 codec via normal i/o (for ICH and SIS7013)
 */

/* return the GLOB_STA bit for the corresponding codec */
static unsigned int get_ich_codec_bit(struct intel8x0m *chip, unsigned int codec)
{
	static unsigned int codec_bit[3] = {
		ICH_PCR, ICH_SCR, ICH_TCR
	};
	if (snd_BUG_ON(codec >= 3))
		return ICH_PCR;
	return codec_bit[codec];
}

static int snd_intel8x0m_codec_semaphore(struct intel8x0m *chip, unsigned int codec)
{
	int time;
	
	if (codec > 1)
		return -EIO;
	codec = get_ich_codec_bit(chip, codec);

	/* codec ready ? */
	if ((igetdword(chip, ICHREG(GLOB_STA)) & codec) == 0)
		return -EIO;

	/* Anyone holding a semaphore for 1 msec should be shot... */
	time = 100;
      	do {
      		if (!(igetbyte(chip, ICHREG(ACC_SEMA)) & ICH_CAS))
      			return 0;
		udelay(10);
	} while (time--);

	/* access to some forbidden (non existant) ac97 registers will not
	 * reset the semaphore. So even if you don't get the semaphore, still
	 * continue the access. We don't need the semaphore anyway. */
	snd_printk(KERN_ERR "codec_semaphore: semaphore is not ready [0x%x][0x%x]\n",
			igetbyte(chip, ICHREG(ACC_SEMA)), igetdword(chip, ICHREG(GLOB_STA)));
	iagetword(chip, 0);	/* clear semaphore flag */
	/* I don't care about the semaphore */
	return -EBUSY;
}
 
static void snd_intel8x0_codec_write(struct snd_ac97 *ac97,
				     unsigned short reg,
				     unsigned short val)
{
	struct intel8x0m *chip = ac97->private_data;
	
	if (snd_intel8x0m_codec_semaphore(chip, ac97->num) < 0) {
		if (! chip->in_ac97_init)
			snd_printk(KERN_ERR "codec_write %d: semaphore is not ready for register 0x%x\n", ac97->num, reg);
	}
	iaputword(chip, reg + ac97->num * 0x80, val);
}

static unsigned short snd_intel8x0_codec_read(struct snd_ac97 *ac97,
					      unsigned short reg)
{
	struct intel8x0m *chip = ac97->private_data;
	unsigned short res;
	unsigned int tmp;

	if (snd_intel8x0m_codec_semaphore(chip, ac97->num) < 0) {
		if (! chip->in_ac97_init)
			snd_printk(KERN_ERR "codec_read %d: semaphore is not ready for register 0x%x\n", ac97->num, reg);
		res = 0xffff;
	} else {
		res = iagetword(chip, reg + ac97->num * 0x80);
		if ((tmp = igetdword(chip, ICHREG(GLOB_STA))) & ICH_RCS) {
			/* reset RCS and preserve other R/WC bits */
			iputdword(chip, ICHREG(GLOB_STA),
				  tmp & ~(ICH_SRI|ICH_PRI|ICH_TRI|ICH_GSCI));
			if (! chip->in_ac97_init)
				snd_printk(KERN_ERR "codec_read %d: read timeout for register 0x%x\n", ac97->num, reg);
			res = 0xffff;
		}
	}
	if (reg == AC97_GPIO_STATUS)
		iagetword(chip, 0); /* clear semaphore */
	return res;
}


/*
 * DMA I/O
 */
static void snd_intel8x0_setup_periods(struct intel8x0m *chip, struct ichdev *ichdev) 
{
	int idx;
	u32 *bdbar = ichdev->bdbar;
	unsigned long port = ichdev->reg_offset;

	iputdword(chip, port + ICH_REG_OFF_BDBAR, ichdev->bdbar_addr);
	if (ichdev->size == ichdev->fragsize) {
		ichdev->ack_reload = ichdev->ack = 2;
		ichdev->fragsize1 = ichdev->fragsize >> 1;
		for (idx = 0; idx < (ICH_REG_LVI_MASK + 1) * 2; idx += 4) {
			bdbar[idx + 0] = cpu_to_le32(ichdev->physbuf);
			bdbar[idx + 1] = cpu_to_le32(0x80000000 | /* interrupt on completion */
						     ichdev->fragsize1 >> chip->pcm_pos_shift);
			bdbar[idx + 2] = cpu_to_le32(ichdev->physbuf + (ichdev->size >> 1));
			bdbar[idx + 3] = cpu_to_le32(0x80000000 | /* interrupt on completion */
						     ichdev->fragsize1 >> chip->pcm_pos_shift);
		}
		ichdev->frags = 2;
	} else {
		ichdev->ack_reload = ichdev->ack = 1;
		ichdev->fragsize1 = ichdev->fragsize;
		for (idx = 0; idx < (ICH_REG_LVI_MASK + 1) * 2; idx += 2) {
			bdbar[idx + 0] = cpu_to_le32(ichdev->physbuf + (((idx >> 1) * ichdev->fragsize) % ichdev->size));
			bdbar[idx + 1] = cpu_to_le32(0x80000000 | /* interrupt on completion */
						     ichdev->fragsize >> chip->pcm_pos_shift);
			/*
			printk(KERN_DEBUG "bdbar[%i] = 0x%x [0x%x]\n",
			       idx + 0, bdbar[idx + 0], bdbar[idx + 1]);
			*/
		}
		ichdev->frags = ichdev->size / ichdev->fragsize;
	}
	iputbyte(chip, port + ICH_REG_OFF_LVI, ichdev->lvi = ICH_REG_LVI_MASK);
	ichdev->civ = 0;
	iputbyte(chip, port + ICH_REG_OFF_CIV, 0);
	ichdev->lvi_frag = ICH_REG_LVI_MASK % ichdev->frags;
	ichdev->position = 0;
#if 0
	printk(KERN_DEBUG "lvi_frag = %i, frags = %i, period_size = 0x%x, "
	       "period_size1 = 0x%x\n",
	       ichdev->lvi_frag, ichdev->frags, ichdev->fragsize,
	       ichdev->fragsize1);
#endif
	/* clear interrupts */
	iputbyte(chip, port + ichdev->roff_sr, ICH_FIFOE | ICH_BCIS | ICH_LVBCI);
}

/*
 *  Interrupt handler
 */

static inline void snd_intel8x0_update(struct intel8x0m *chip, struct ichdev *ichdev)
{
	unsigned long port = ichdev->reg_offset;
	int civ, i, step;
	int ack = 0;

	civ = igetbyte(chip, port + ICH_REG_OFF_CIV);
	if (civ == ichdev->civ) {
		// snd_printd("civ same %d\n", civ);
		step = 1;
		ichdev->civ++;
		ichdev->civ &= ICH_REG_LVI_MASK;
	} else {
		step = civ - ichdev->civ;
		if (step < 0)
			step += ICH_REG_LVI_MASK + 1;
		// if (step != 1)
		//	snd_printd("step = %d, %d -> %d\n", step, ichdev->civ, civ);
		ichdev->civ = civ;
	}

	ichdev->position += step * ichdev->fragsize1;
	ichdev->position %= ichdev->size;
	ichdev->lvi += step;
	ichdev->lvi &= ICH_REG_LVI_MASK;
	iputbyte(chip, port + ICH_REG_OFF_LVI, ichdev->lvi);
	for (i = 0; i < step; i++) {
		ichdev->lvi_frag++;
		ichdev->lvi_frag %= ichdev->frags;
		ichdev->bdbar[ichdev->lvi * 2] = cpu_to_le32(ichdev->physbuf +
							     ichdev->lvi_frag *
							     ichdev->fragsize1);
#if 0
		printk(KERN_DEBUG "new: bdbar[%i] = 0x%x [0x%x], "
		       "prefetch = %i, all = 0x%x, 0x%x\n",
		       ichdev->lvi * 2, ichdev->bdbar[ichdev->lvi * 2],
		       ichdev->bdbar[ichdev->lvi * 2 + 1], inb(ICH_REG_OFF_PIV + port),
		       inl(port + 4), inb(port + ICH_REG_OFF_CR));
#endif
		if (--ichdev->ack == 0) {
			ichdev->ack = ichdev->ack_reload;
			ack = 1;
		}
	}
	if (ack && ichdev->substream) {
		spin_unlock(&chip->reg_lock);
		snd_pcm_period_elapsed(ichdev->substream);
		spin_lock(&chip->reg_lock);
	}
	iputbyte(chip, port + ichdev->roff_sr, ICH_FIFOE | ICH_BCIS | ICH_LVBCI);
}

static irqreturn_t snd_intel8x0_interrupt(int irq, void *dev_id)
{
	struct intel8x0m *chip = dev_id;
	struct ichdev *ichdev;
	unsigned int status;
	unsigned int i;

	spin_lock(&chip->reg_lock);
	status = igetdword(chip, chip->int_sta_reg);
	if (status == 0xffffffff) { /* we are not yet resumed */
		spin_unlock(&chip->reg_lock);
		return IRQ_NONE;
	}
	if ((status & chip->int_sta_mask) == 0) {
		if (status)
			iputdword(chip, chip->int_sta_reg, status);
		spin_unlock(&chip->reg_lo             /* not yet implemented */
                return RCODE_ADDRESS_ERROR;

        case CSR_BUS_MANAGER_ID:
        case CSR_BANDWIDTH_AVAILABLE:
        case CSR_CHANNELS_AVAILABLE_HI:
        case CSR_CHANNELS_AVAILABLE_LO:
                /* these are not writable, only lockable */
                return RCODE_TYPE_ERROR;

	case CSR_BROADCAST_CHANNEL:
		/* only the valid bit can be written */
		host->csr.broadcast_channel = (host->csr.broadcast_channel & ~0x40000000)
                        | (be32_to_cpu(*data) & 0x40000000);
		out;

                /* address gap to end - fall through */
        default:
                return RCODE_ADDRESS_ERROR;
        }

        return RCODE_COMPLETE;
}

#undef out


static int lock_regs(struct hpsb_host *host, int nodeid, quadlet_t *store,
                     u64 addr, quadlet_t data, quadlet_t arg, int extcode, u16 fl)
{
        int csraddr = addr - CSR_REGISTER_BASE;
        unsigned long flags;
        quadlet_t *regptr = NULL;

        if (csraddr & 0x3)
		return RCODE_TYPE_ERROR;

        if (csraddr < CSR_BUS_MANAGER_ID || csraddr > CSR_CHANNELS_AVAILABLE_LO
            || extcode != EXTCODE_COMPARE_SWAP)
                goto unsupported_lockreq;

        data = be32_to_cpu(data);
        arg = be32_to_cpu(arg);

	/* Is somebody releasing the broadcast_channel on us? */
	if (csraddr == CSR_CHANNELS_AVAILABLE_HI && (data & 0x1)) {
		/* Note: this is may not be the right way to handle
		 * the problem, so we should look into the proper way
		 * eventually. */
		HPSB_WARN("Node [" NODE_BUS_FMT "] wants to release "
			  "broadcast channel 31.  Ignoring.",
			  NODE_BUS_ARGS(host, nodeid));

		data &= ~0x1;	/* keep broadcast channel allocated */
	}

        if (host->driver->hw_csr_reg) {
                quadlet_t old;

                old = host->driver->
                        hw_csr_reg(host, (csraddr - CSR_BUS_MANAGER_ID) >> 2,
                                   data, arg);

                *store = cpu_to_be32(old);
                return RCODE_COMPLETE;
        }

        spin_lock_irqsave(&host->csr.lock, flags);

        switch (csraddr) {
        case CSR_BUS_MANAGER_ID:
                regptr = &host->csr.bus_manager_id;
		*store = cpu_to_be32(*regptr);
		if (*regptr == arg)
			*regptr = data;
                break;

        case CSR_BANDWIDTH_AVAILABLE:
        {
                quadlet_t bandwidth;
                quadlet_t old;
                quadlet_t new;

                regptr = &host->csr.bandwidth_available;
                old = *regptr;

                /* bandwidth available algorithm adapted from IEEE 1394a-2000 spec */
                if (arg > 0x1fff) {
                        *store = cpu_to_be32(old);	/* change nothing */
			break;
                }
                data &= 0x1fff;
                if (arg >= data) {
                        /* allocate bandwidth */
                        bandwidth = arg - data;
                        if (old >= bandwidth) {
                                new = old - bandwidth;
                                *store = cpu_to_be32(arg);
                                *regptr = new;
                        } else {
                                *store = cpu_to_be32(old);
                        }
                } else {
                        /* deallocate bandwidth */
                        bandwidth = data - arg;
                        if (old + bandwidth < 0x2000) {
                                new = old + bandwidth;
                                *store = cpu_to_be32(arg);
                                *regptr = new;
                        } else {
                                *store = cpu_to_be32(old);
                        }
                }
                break;
        }

        case CSR_CHANNELS_AVAILABLE_HI:
        {
                /* Lock algorithm for CHANNELS_AVAILABLE as recommended by 1394a-2000 */
                quadlet_t affected_channels = arg ^ data;

                regptr = &host->csr.channels_available_hi;

                if ((arg & affected_channels) == (*regptr & affected_channels)) {
                        *regptr ^= affected_channels;
                        *store = cpu_to_be32(arg);
                } else {
                        *store = cpu_to_be32(*regptr);
                }

                break;
        }

        case CSR_CHANNELS_AVAILABLE_LO:
        {
                /* Lock algorithm for CHANNELS_AVAILABLE as recommended by 1394a-2000 */
                quadlet_t affected_channels = arg ^ data;

                regptr = &host->csr.channels_available_lo;

                if ((arg & affected_channels) == (*regptr & affected_channels)) {
                        *regptr ^= affected_channels;
                        *store = cpu_to_be32(arg);
                } else {
                        *store = cpu_to_be32(*regptr);
                }
                break;
        }
        }

        spin_unlock_irqrestore(&host->csr.lock, flags);

        return RCODE_COMPLETE;

 unsupported_lockreq:
        switch (csraddr) {
        case CSR_STATE_CLEAR:
        case CSR_STATE_SET:
        case CSR_RESET_START:
        case CSR_NODE_IDS:
        case CSR_SPLIT_TIMEOUT_HI:
        case CSR_SPLIT_TIMEOUT_LO:
        case CSR_CYCLE_TIME:
        case CSR_BUS_TIME:
	case CSR_BROADCAST_CHANNEL:
                return RCODE_TYPE_ERROR;

        case CSR_BUSY_TIMEOUT:
                /* not yet implemented - fall through */
        default:
                return RCODE_ADDRESS_ERROR;
        }
}

static int lock64_regs(struct hpsb_host *host, int nodeid, octlet_t * store,
		       u64 addr, octlet_t data, octlet_t arg, int extcode, u16 fl)
{
	int csraddr = addr - CSR_REGISTER_BASE;
	unsigned long flags;

	data = be64_to_cpu(data);
	arg = be64_to_cpu(arg);

	if (csraddr & 0x3)
		return RCODE_TYPE_ERROR;

	if (csraddr != CSR_CHANNELS_AVAILABLE
	    || extcode != EXTCODE_COMPARE_SWAP)
		goto unsupported_lock64req;

	/* Is somebody releasing the broadcast_channel on us? */
	if (csraddr == CSR_CHANNELS_AVAILABLE_HI && (data & 0x100000000ULL)) {
		/* Note: this is may not be the right way to handle
		 * the problem, so we should look into the proper way
                 * eventually. */
		HPSB_WARN("Node [" NODE_BUS_FMT "] wants to release "
			  "broadcast channel 31.  Ignoring.",
			  NODE_BUS_ARGS(host, nodeid));

		data &= ~0x100000000ULL;	/* keep broadcast channel allocated */
	}

	if (host->driver->hw_csr_reg) {
		quadlet_t data_hi, data_lo;
		quadlet_t arg_hi, arg_lo;
		quadlet_t old_hi, old_lo;

		data_hi = data >> 32;
		data_lo = data & 0xFFFFFFFF;
		arg_hi = arg >> 32;
		arg_lo = arg & 0xFFFFFFFF;

		old_hi = host->driver->hw_csr_reg(host, (csraddr - CSR_BUS_MANAGER_ID) >> 2,
                                                  data_hi, arg_hi);

		old_lo = host->driver->hw_csr_reg(host, ((csraddr + 4) - CSR_BUS_MANAGER_ID) >> 2,
                                                  data_lo, arg_lo);

		*store = cpu_to_be64(((octlet_t)old_hi << 32) | old_lo);
	} else {
		octlet_t old;
		octlet_t affected_channels = arg ^ data;

		spin_lock_irqsave(&host->csr.lock, flags);

		old = ((octlet_t)host->csr.channels_available_hi << 32) | host->csr.channels_available_lo;

		if ((arg & affected_channels) == (old & affected_channels)) {
			host->csr.channels_available_hi ^= (affected_channels >> 32);
			host->csr.channels_available_lo ^= (affected_channels & 0xffffffff);
			*store = cpu_to_be64(arg);
		} else {
			*store = cpu_to_be64(old);
		}

		spin_unlock_irqrestore(&host->csr.lock, flags);
	}

	/* Is somebody erroneously releasing the broadcast_channel on us? */
	if (host->csr.channels_available_hi & 0x1)
		host->csr.channels_available_hi &= ~0x1;

	return RCODE_COMPLETE;

 unsupported_lock64req:
	switch (csraddr) {
	case CSR_STATE_CLEAR:
	case CSR_STATE_SET:
	case CSR_RESET_START:
	case CSR_NODE_IDS:
	case CSR_SPLIT_TIMEOUT_HI:
	case CSR_SPLIT_TIMEOUT_LO:
	case CSR_CYCLE_TIME:
	case CSR_BUS_TIME:
	case CSR_BUS_MANAGER_ID:
	case CSR_BROADCAST_CHANNEL:
	case CSR_BUSY_TIMEOUT:
	case CSR_BANDWIDTH_AVAILABLE:
		return RCODE_TYPE_ERROR;

	default:
		return RCODE_ADDRESS_ERROR;
	}
}

static int write_fcp(struct hpsb_host *host, int nodeid, int dest,
		     quadlet_t *data, u64 addr, size_t length, u16 flags)
{
        int csraddr = addr - CSR_REGISTER_BASE;

        if (length > 512)
                return RCODE_TYPE_ERROR;

        switch (csraddr) {
        case CSR_FCP_COMMAND:
                highlevel_fcp_request(host, nodeid, 0, (u8 *)data, length);
                break;
        case CSR_FCP_RESPONSE:
                highlevel_fcp_request(host, nodeid, 1, (u8 *)data, length);
                break;
        default:
                return RCODE_TYPE_ERROR;
        }

        return RCODE_COMPLETE;
}

static int read_config_rom(struct hpsb_host *host, int nodeid, quadlet_t *buffer,
			   u64 addr, size_t length, u16 fl)
{
	u32 offset = addr - CSR1212_REGISTER_SPACE_BASE;

	if (csr1212_read(host->csr.rom, offset, buffer, length) == CSR1212_SUCCESS)
		return RCODE_COMPLETE;
	else
		return RCODE_ADDRESS_ERROR;
}

static u64 allocate_addr_range(u64 size, u32 alignment, void *__host)
{
 	struct hpsb_host *host = (struct hpsb_host*)__host;

	return hpsb_allocate_and_register_addrspace(&csr_highlevel,
						    host,
						    &config_rom_ops,
						    size, alignment,
						    CSR1212_UNITS_SPACE_BASE,
						    CSR1212_UNITS_SPACE_END);
}

static void release_addr_range(u64 addr, void *__host)
{
 	struct hpsb_host *host = (struct hpsb_host*)__host;
	hpsb_unregister_addrspace(&csr_highlevel, host, addr);
}


int init_csr(void)
{
	node_cap = csr1212_new_immediate(CSR1212_KV_ID_NODE_CAPABILITIES, 0x0083c0);
	if (!node_cap) {
		HPSB_ERR("Failed to allocate memory for Node Capabilties ConfigROM entry!");
		return -ENOMEM;
	}

	hpsb_register_highlevel(&csr_highlevel);

	return 0;
}

void cleanup_csr(void)
{
	if (node_cap)
		csr1212_release_keyval(node_cap);
        hpsb_unregister_highlevel(&csr_highlevel);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  /* OmniVision OV6630/OV6130 Camera Chip Support Code
 *
 * Copyright (c) 1999-2004 Mark McClelland <mark@alpha.dyndns.org>
 * http://alpha.dyndns.org/ov511/
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
#define REG_BLUE		0x01	/* blue gain */
#define REG_RED			0x02	/* red gain */
#define REG_SAT			0x03	/* saturation [7:3] */
#define REG_CNT			0x05	/* Y contrast [3:0] */
#define REG_BRT			0x06	/* Y brightness */
#define REG_SHARP		0x07	/* sharpness */
#define REG_WB_BLUE		0x0C	/* WB blue ratio [5:0] */
#define REG_WB_RED		0x0D	/* WB red ratio [5:0] */
#define REG_EXP			0x10	/* exposure */

/* Window parameters */
#define HWSBASE 0x38
#define HWEBASE 0x3A
#define VWSBASE 0x05
#define VWEBASE 0x06

struct ov6x30 {
	int auto_brt;
	int auto_exp;
	int backlight;
	int bandfilt;
	int mirror;
};

static struct ovcamchip_regvals regvals_init_6x30[] = {
	{ 0x12, 0x80 }, /* reset */
	{ 0x00, 0x1f }, /* Gain */
	{ 0x01, 0x99 }, /* Blue gain */
	{ 0x02, 0x7c }, /* Red gain */
	{ 0x03, 0xc0 }, /* Saturation */
	{ 0x05, 0x0a }, /* Contrast */
	{ 0x06, 0x95 }, /* Brightness */
	{ 0x07, 0x2d }, /* Sharpness */
	{ 0x0c, 0x20 },
	{ 0x0d, 0x20 },
	{ 0x0e, 0x20 },
	{ 0x0f, 0x05 },
	{ 0x10, 0x9a }, /* "exposure check" */
	{ 0x11, 0x00 }, /* Pixel clock = fastest */
	{ 0x12, 0x24 }, /* Enable AGC and AWB */
	{ 0x13, 0x21 },
	{ 0x14, 0x80 },
	{ 0x15, 0x01 },
	{ 0x16, 0x03 },
	{ 0x17, 0x38 },
	{ 0x18, 0xea },
	{ 0x19, 0x04 },
	{ 0x1a, 0x93 },
	{ 0x1b, 0x00 },
	{ 0x1e, 0xc4 },
	{ 0x1f, 0x04 },
	{ 0x20, 0x20 },
	{ 0x21, 0x10 },
	{ 0x22, 0x88 },
	{ 0x23, 0xc0 }, /* Crystal circuit power level */
	{ 0x25, 0x9a }, /* Increase AEC black pixel ratio */
	{ 0x26, 0xb2 }, /* BLC enable */
	{ 0x27, 0xa2 },
	{ 0x28, 0x00 },
	{ 0x29, 0x00 },
	{ 0x2a, 0x84 }, /* (keep) */
	{ 0x2b, 0xa8 }, /* (keep) */
	{ 0x2c, 0xa0 },
	{ 0x2d, 0x95 },	/* Enable auto-brightness */
	{ 0x2e, 0x88 },
	{ 0x33, 0x26 },
	{ 0x34, 0x03 },
	{ 0x36, 0x8f },
	{ 0x37, 0x80 },
	{ 0x38, 0x83 },
	{ 0x39, 0x80 },
	{ 0x3a, 0x0f },
	{ 0x3b, 0x3c },
	{ 0x3c, 0x1a },
	{ 0x3d, 0x80 },
	{ 0x3e, 0x80 },
	{ 0x3f, 0x0e },
	{ 0x40, 0x00 }, /* White bal */
	{ 0x41, 0x00 }, /* White bal */
	{ 0x42, 0x80 },
	{ 0x43, 0x3f }, /* White bal */
	{ 0x44, 0x80 },
	{ 0x45, 0x20 },
	{ 0x46, 0x20 },
	{ 0x47, 0x80 },
	{ 0x48, 0x7f },
	{ 0x49, 0x00 },
	{ 0x4a, 0x00 },
	{ 0x4b, 0x80 },
	{ 0x4c, 0xd0 },
	{ 0x4d, 0x10 }, /* U = 0.563u, V = 0.714v */
	{ 0x4e, 0x40 },
	{ 0x4f, 0x07 }, /* UV average mode, color killer: strongest */
	{ 0x50, 0xff },
	{ 0x54, 0x23 }, /* Max AGC gain: 18dB */
	{ 0x55, 0xff },
	{ 0x56, 0x12 },
	{ 0x57, 0x81 }, /* (default) */
	{ 0x58, 0x75 },
	{ 0x59, 0x01 }, /* AGC dark current compensation: +1 */
	{ 0x5a, 0x2c },
	{ 0x5b, 0x0f }, /* AWB chrominance levels */
	{ 0x5c, 0x10 },
	{ 0x3d, 0x80 },
	{ 0x27, 0xa6 },
	/* Toggle AWB off and on */
	{ 0x12, 0x20 },
	{ 0x12, 0x24 },

	{ 0xff, 0xff },	/* END MARKER */
};

/* This initializes the OV6x30 camera chip and relevant variables. */
static int ov6x30_init(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov6x30 *s;
	int rc;

	DDEBUG(4, &c->dev, "entered");

	rc = ov_write_regvals(c, regvals_init_6x30);
	if (rc < 0)
		return rc;

	ov->spriv = s = kzalloc(sizeof *s, GFP_KERNEL);
	if (!s)
		return -ENOMEM;

	s->auto_brt = 1;
	s->auto_exp = 1;

	return rc;
}

static int ov6x30_free(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);

	kfree(ov->spriv);
	return 0;
}

static int ov6x30_set_control(struct i2c_client *c,
			      struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov6x30 *s = ov->spriv;
	int rc;
	int v = ctl->value;

	switch (ctl->id) {
	case OVCAMCHIP_CID_CONT:
		rc = ov_write_mask(c, REG_CNT, v >> 12, 0x0f);
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

		rc = ov_write(c, 0x2b, sixty?0xa8:0x28);
		if (rc < 0)
			goto out;

		rc = ov_write(c, 0x2a, sixty?0x84:0xa4);
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
		rc = ov_write_mask(c, 0x28, v?0x00:0x10, 0x10);
		s->auto_exp = v;
		break;
	case OVCAMCHIP_CID_BACKLIGHT:
	{
		rc = ov_write_mask(c, 0x4e, v?0x80:0x60, 0xe0);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x29, v?0x08:0x00, 0x08);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x28, v?0x02:0x00, 0x02);
		s->backlight = v;
		break;
	}
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

static int ov6x30_get_control(struct i2c_client *c,
			      struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov6x30 *s = ov->spriv;
	int rc = 0;
	unsigned char val = 0;

	switch (ctl->id) {
	case OVCAMCHIP_CID_CONT:
		rc = ov_read(c, REG_CNT, &val);
		ctl->value = (val & 0x0f) << 12;
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
	case OVCAMCHIP_CID_BACKLIGHT:
		ctl->value = s->backlight;
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

static int ov6x30_mode_init(struct i2c_client *c, struct ovcamchip_window *win)
{
	/******** QCIF-specific regs ********/

	ov_write_mask(c, 0x14, win->quarter?0x20:0x00, 0x20);

	/******** Palette-specific regs ********/

	if (win->format == VIDEO_PALETTE_GREY) {
		if (c->adapter->id == I2C_HW_SMBUS_OV518) {
			/* Do nothing - we're already in 8-bit mode */
		} else {
			ov_write_mask(c, 0x13, 0x20, 0x20);
		}
	} else {
		/* The OV518 needs special treatment. Although both the OV518
		 * and the OV6630 support a 16-bit video bus, only the 8 bit Y
		 * bus is actually used. The UV bus is tied to ground.
		 * Therefore, the OV6630 needs to be in 8-bit multiplexed
		 * output mode */

		if (c->adapter->id == I2C_HW_SMBUS_OV518) {
			/* Do nothing - we want to stay in 8-bit mode */
			/* Warning: Messing with reg 0x13 breaks OV518 color */
		} else {
			ov_write_mask(c, 0x13, 0x00, 0x20);
		}
	}

	/******** Clock programming ********/

	ov_write(c, 0x11, win->clockdiv);

	return 0;
}

static int ov6x30_set_window(struct i2c_client *c, struct ovcamchip_window *win)
{
	int ret, hwscale, vwscale;

	ret = ov6x30_mode_init(c, win);
	if (ret < 0)
		return ret;

	if (win->quarter) {
		hwscale = 0;
		vwscale = 0;
	} else {
		hwscale = 1;
		vwscale = 1;	/* The datasheet says 0; it's wrong */
	}

	ov_write(c, 0x17, HWSBASE + (win->x >> hwscale));
	ov_write(c, 0x18, HWEBASE + ((win->x + win->width) >> hwscale));
	ov_write(c, 0x19, VWSBASE + (win->y >> vwscale));
	ov_write(c, 0x1a, VWEBASE + ((win->y + win->height) >> vwscale));

	return 0;
}

static int ov6x30_command(struct i2c_client *c, unsigned int cmd, void *arg)
{
	switch (cmd) {
	case OVCAMCHIP_CMD_S_CTRL:
		return ov6x30_set_control(c, arg);
	case OVCAMCHIP_CMD_G_CTRL:
		return ov6x30_get_control(c, arg);
	case OVCAMCHIP_CMD_S_MODE:
		return ov6x30_set_window(c, arg);
	default:
		DDEBUG(2, &c->dev, "command not supported: %d", cmd);
		return -ENOIOCTLCMD;
	}
}

struct ovcamchip_ops ov6x30_ops = {
	.init    =	ov6x30_init,
	.free    =	ov6x30_free,
	.command =	ov6x30_command,
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  