ool, GFP_KERNEL, d->prg_bus+i);

                if (d->prg_cpu[i] != NULL) {
                        memset(d->prg_cpu[i], 0, sizeof(struct at_dma_prg));
		} else {
			PRINT(KERN_ERR,
			      "Failed to allocate %s", "AT DMA prg");
			free_dma_trm_ctx(d);
			return -ENOMEM;
		}
	}

        spin_lock_init(&d->lock);

	/* initialize tasklet */
	d->ctrlSet = context_base + OHCI1394_ContextControlSet;
	d->ctrlClear = context_base + OHCI1394_ContextControlClear;
	d->cmdPtr = context_base + OHCI1394_ContextCommandPtr;
	tasklet_init(&d->task, dma_trm_tasklet, (unsigned long)d);
	return 0;
}

static void ohci_set_hw_config_rom(struct hpsb_host *host, __be32 *config_rom)
{
	struct ti_ohci *ohci = host->hostdata;

	reg_write(ohci, OHCI1394_ConfigROMhdr, be32_to_cpu(config_rom[0]));
	reg_write(ohci, OHCI1394_BusOptions, be32_to_cpu(config_rom[2]));

	memcpy(ohci->csr_config_rom_cpu, config_rom, OHCI_CONFIG_ROM_LEN);
}


static quadlet_t ohci_hw_csr_reg(struct hpsb_host *host, int reg,
                                 quadlet_t data, quadlet_t compare)
{
	struct ti_ohci *ohci = host->hostdata;
	int i;

	reg_write(ohci, OHCI1394_CSRData, data);
	reg_write(ohci, OHCI1394_CSRCompareData, compare);
	reg_write(ohci, OHCI1394_CSRControl, reg & 0x3);

	for (i = 0; i < OHCI_LOOP_COUNT; i++) {
		if (reg_read(ohci, OHCI1394_CSRControl) & 0x80000000)
			break;

		mdelay(1);
	}

	return reg_read(ohci, OHCI1394_CSRData);
}

static struct hpsb_host_driver ohci1394_driver = {
	.owner =		THIS_MODULE,
	.name =			OHCI1394_DRIVER_NAME,
	.set_hw_config_rom =	ohci_set_hw_config_rom,
	.transmit_packet =	ohci_transmit,
	.devctl =		ohci_devctl,
	.isoctl =               ohci_isoctl,
	.hw_csr_reg =		ohci_hw_csr_reg,
};

/***********************************
 * PCI Driver Interface functions  *
 ***********************************/

#ifdef CONFIG_PPC_PMAC
static void ohci1394_pmac_on(struct pci_dev *dev)
{
	if (machine_is(powermac)) {
		struct device_node *ofn = pci_device_to_OF_node(dev);

		if (ofn) {
			pmac_call_feature(PMAC_FTR_1394_CABLE_POWER, ofn, 0, 1);
			pmac_call_feature(PMAC_FTR_1394_ENABLE, ofn, 0, 1);
		}
	}
}

static void ohci1394_pmac_off(struct pci_dev *dev)
{
	if (machine_is(powermac)) {
		struct device_node *ofn = pci_device_to_OF_node(dev);

		if (ofn) {
			pmac_call_feature(PMAC_FTR_1394_ENABLE, ofn, 0, 0);
			pmac_call_feature(PMAC_FTR_1394_CABLE_POWER, ofn, 0, 0);
		}
	}
}
#else
#define ohci1394_pmac_on(dev)
#define ohci1394_pmac_off(dev)
#endif /* CONFIG_PPC_PMAC */

static int __devinit ohci1394_pci_probe(struct pci_dev *dev,
					const struct pci_device_id *ent)
{
	struct hpsb_host *host;
	struct ti_ohci *ohci;	/* shortcut to currently handled device */
	resource_size_t ohci_base;
	int err = -ENOMEM;

	ohci1394_pmac_on(dev);
	if (pci_enable_device(dev)) {
		PRINT_G(KERN_ERR, "Failed to enable OHCI hardware");
		err = -ENXIO;
		goto err;
	}
        pci_set_master(dev);

	host = hpsb_alloc_host(&ohci1394_driver, sizeof(struct ti_ohci), &dev->dev);
	if (!host) {
		PRINT_G(KERN_ERR, "Failed to allocate %s", "host structure");
		goto err;
	}
	ohci = host->hostdata;
	ohci->dev = dev;
	ohci->host = host;
	ohci->init_state = OHCI_INIT_ALLOC_HOST;
	host->pdev = dev;
	pci_set_drvdata(dev, ohci);

	/* We don't want hardware swapping */
	pci_write_config_dword(dev, OHCI1394_PCI_HCI_Control, 0);

	/* Some oddball Apple controllers do not order the selfid
	 * properly, so we make up for it here.  */
#ifndef __LITTLE_ENDIAN
	/* XXX: Need a better way to check this. I'm wondering if we can
	 * read the values of the OHCI1394_PCI_HCI_Control and the
	 * noByteSwapData registers to see if they were not cleared to
	 * zero. Should this work? Obviously it's not defined what these
	 * registers will read when they aren't supported. Bleh! */
	if (dev->vendor == PCI_VENDOR_ID_APPLE &&
	    dev->device == PCI_DEVICE_ID_APPLE_UNI_N_FW) {
		ohci->no_swap_incoming = 1;
		ohci->selfid_swap = 0;
	} else
		ohci->selfid_swap = 1;
#endif


#ifndef PCI_DEVICE_ID_NVIDIA_NFORCE2_FW
#define PCI_DEVICE_ID_NVIDIA_NFORCE2_FW 0x006e
#endif

	/* These chipsets require a bEL:1;	/*!<  2D memory register set  bit             */
	volatile u32 MDIR:1;	/*!< Transfer direction, inversed or normal          */
	volatile u32 SMOD:2;	/*!<  Source mode, 2 bit in length            */
	volatile u32 DMOD:2;	/*!<  Dest mode, 2 bit in length              */
	volatile u32 ACRPT:1;	/*!<  Auto clear repeat bit           */
	volatile u32 Reserver:17;	/*!<  Reserved bits           */

} dma_regs_control;

#define DMA_CTL_CEN 0x1
#define DMA_CTL_FRC 0x2
#define DMA_CTL_RPT 0x4
#define DMA_CTL_REN 0x8

#define DMA_CTL_MSEL 0x100
#define DMA_CTL_MDIR 0x200
#define DMA_CTL_ACRPT 0x4000

#define DMA_CTL_GET_SSIZ(x) (((x)>>4)&0x3)
#define DMA_CTL_GET_DSIZ(x) (((x)>>6)&0x3)
#define DMA_CTL_GET_SMOD(x)  (((x)>>10)&0x3)
#define DMA_CTL_GET_DMOD(x)  (((x)>>12)&0x3)

#define DMA_CTL_SET_SSIZ(x,value) 	do{ \
									(x)&=~(0x3<<4); \
									(x)|=(value)<<4; 		\
								}while(0)

#define DMA_CTL_SET_DSIZ(x,value) 	do{ \
									(x)&=~(0x3<<6); \
									(x)|=(value)<<6; 		\
								}while(0)

#define DMA_CTL_SET_SMOD(x,value) 	do{ \
									(x)&=~(0x3<<10); \
									(x)|=(value)<<10; 		\
								}while(0)

#define DMA_CTL_SET_DMOD(x,value) 	do{ \
									(x)&=~(0x3<<12); \
									(x)|=(value)<<12; 		\
								}while(0)

typedef struct {
	volatile u32 SourceAddr;
	volatile u32 DestAddr;
	volatile u32 Count;
	volatile u32 Ctl;
	volatile u32 RequestSource;
	volatile u32 BurstLength;
	union {
		volatile u32 ReqTimeout;
		volatile u32 BusUtilt;
	};
	volatile u32 transferd;
} dma_regs_t;

#ifndef TRANSFER_32BIT
/*!
 * This defines DMA access data size
 */

#define TRANSFER_8BIT       DMA_MEM_SIZE_8
#define TRANSFER_16BIT      DMA_MEM_SIZE_16
#define TRANSFER_32BIT      DMA_MEM_SIZE_32

#endif

/*!
 * This defines maximum device name length passed during mxc_request_dma().
 */
#define MAX_DEVNAME_LENGTH 32
#define MAX_BD_SIZE	32

/*!
 * Structure containing dma channel parameters.
 */
typedef struct {
	unsigned long dma_chan;	/*!< the dma channel information: dynamic or channel number */
	u32 mode:1;		/*!< the initialized dma mode, 0 for dma read, 1 for dma write */
	u32 rto_en:1;		/*!< enable request-timeout. It is valid when REN=1 */
	u32 dir:1;		/*!< Transfer direction, 0 for increment, 1 for decrement */
	u32 dma_chaining:1;	/*!< Autoclear bit for chainbuffer */
	u32 ren:1;		/*!< enable transfer based request signal */
	u32 M2D_Valid:1;	/*!< enable 2D address module. 0 for disable it. 1 for enabled it */
	u32 msel:1;		/*!<2D memory selection, 0 for set A, 1 for set B */
	u32 burstLength;	/*!<    Channel burst length    */
	u32 request;		/*!<   Request source. */
	u32 busuntils;		/*!<   when REN=0, Bus utilization, otherwise it it request timeout */
	u32 sourceType;		/*!<    Source type, see DMA_TYPE_* */
	u32 sourcePort;		/*!<    Source port size, see DMA_MEM_SIZE_*  */
	u32 destType;		/*!<    Destination type, see DMA_TYPE_*    */
	u32 destPort;		/*!<    Destination port size, see DMA_MEM_SIZE_*       */
	__u32 per_address;	/*< Peripheral source/destination
				 *   physical address
				 */
	u32 W;			/*!<    2D Wide-size            */
	u32 X;			/*!<    2D X-size               */
	u32 Y;			/*!<    2D Y-size               */
} mx2_dma_info_t;

/*!
 * Structure of dma buffer descriptor
 */
typedef struct {
	unsigned long state;	/*!< dma bd state */
	int mode;		/*!< the dma mode of this bd */
	unsigned long count;	/*!< the length of the dma transfer */
	unsigned long src_addr;	/*!< the source address of the dma transfer */
	unsigned long dst_addr;	/*!< the destination address of the dma transfer */
} mx2_dma_bd_t;

/*!
 * the states of dma buffer descriptor
 */
#define DMA_BD_ST_BUSY	0x20000000	/*!< dma bd is transfering or has be configured into controller */
#define DMA_BD_ST_PEND	0x10000000	/*!< dma bd is waiting to be configured into controller */
#define DMA_BD_ST_LAST	0x08000000	/*!< dma bd is the last dma bd which is built in one dma transfer request
					 *          When completed this bd, the callback function must be called.
					 */

/*!
 *  This structure containing the private information for MX2
 */
typedef struct mx2_dma_priv_s {
	unsigned int dma_chaining:1;	/* 1: using headware dma chaining feature */
	unsigned int ren:1;	/* 1: dma start besed on request signal */
	unsigned long trans_bytes;	/* To store the transfered data bytes in this transfer */
	mx2_dma_info_t *dma_info;	/* To store the pointer for dma parameter for reading and wirting */
	int bd_rd;		/* the read index of bd ring */
	int bd_wr;		/* the write index of bd ring */
	atomic_t bd_used;	/* the valid bd number in bd ring */
	mx2_dma_bd_t *bd_ring;	/* the pointer of bd ring */
	unsigned long dma_base;	/* register base address of this channel */
	int dma_irq;		/* irq number of this channel */
} mx2_dma_priv_t;

/*!
 * @brief get the dma info by channel_id
 */
extern mx2_dma_info_t *mxc_dma_get_info(mxc_dma_device_t channel_id);

/*!
 * @brief: scan dma parameter list . And collect information about which channels are dynamic .
 */
extern void mxc_dma_load_info(mxc_dma_channel_t * dma);

#endif
