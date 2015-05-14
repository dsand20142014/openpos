/*
 * QLogic Fibre Channel HBA Driver
 * Copyright (c)  2003-2008 QLogic Corporation
 *
 * See LICENSE.qla2xxx for copyright and licensing details.
 */
#include "qla_def.h"
#include "qla_gbl.h"

#include <linux/delay.h>
#include <linux/vmalloc.h>

#include "qla_devtbl.h"

#ifdef CONFIG_SPARC
#include <asm/prom.h>
#endif

/*
*  QLogic ISP2x00 Hardware Support Function Prototypes.
*/
static int qla2x00_isp_firmware(scsi_qla_host_t *);
static int qla2x00_setup_chip(scsi_qla_host_t *);
static int qla2x00_init_rings(scsi_qla_host_t *);
static int qla2x00_fw_ready(scsi_qla_host_t *);
static int qla2x00_configure_hba(scsi_qla_host_t *);
static int qla2x00_configure_loop(scsi_qla_host_t *);
static int qla2x00_configure_local_loop(scsi_qla_host_t *);
static int qla2x00_configure_fabric(scsi_qla_host_t *);
static int qla2x00_find_all_fabric_devs(scsi_qla_host_t *, struct list_head *);
static int qla2x00_device_resync(scsi_qla_host_t *);
static int qla2x00_fabric_dev_login(scsi_qla_host_t *, fc_port_t *,
    uint16_t *);

static int qla2x00_restart_isp(scsi_qla_host_t *);

static int qla2x00_find_new_loop_id(scsi_qla_host_t *, fc_port_t *);

static struct qla_chip_state_84xx *qla84xx_get_chip(struct scsi_qla_host *);
static int qla84xx_init_chip(scsi_qla_host_t *);
static int qla25xx_init_queues(struct qla_hw_data *);

/****************************************************************************/
/*                QLogic ISP2x00 Hardware Support Functions.                */
/****************************************************************************/

/*
* qla2x00_initialize_adapter
*      Initialize board.
*
* Input:
*      ha = adapter block pointer.
*
* Returns:
*      0 = success
*/
int
qla2x00_initialize_adapter(scsi_qla_host_t *vha)
{
	int	rval;
	struct qla_hw_data *ha = vha->hw;
	struct req_que *req = ha->req_q_map[0];

	/* Clear adapter flags. */
	vha->flags.online = 0;
	ha->flags.chip_reset_done = 0;
	vha->flags.reset_active = 0;
	atomic_set(&vha->loop_down_timer, LOOP_DOWN_TIME);
	atomic_set(&vha->loop_state, LOOP_DOWN);
	vha->device_flags = DFLG_NO_CABLE;
	vha->dpc_flags = 0;
	vha->flags.management_server_logged_in = 0;
	vha->marker_needed = 0;
	ha->isp_abort_cnt = 0;
	ha->beacon_blink_led = 0;
	set_bit(REGISTER_FDMI_NEEDED, &vha->dpc_flags);

	set_bit(0, ha->req_qid_map);
	set_bit(0, ha->rsp_qid_map);

	qla_printk(KERN_INFO, ha, "Configuring PCI space...\n");
	rval = ha->isp_ops->pci_config(vha);
	if (rval) {
		DEBUG2(printk("scsi(%ld): Unable to configure PCI space.\n",
		    vha->host_no));
		return (rval);
	}

	ha->isp_ops->reset_chip(vha);

	rval = qla2xxx_get_flash_info(vha);
	if (rval) {
		DEBUG2(printk("scsi(%ld): Unable to validate FLASH data.\n",
		    vha->host_no));
		return (rval);
	}

	ha->isp_ops->get_flash_version(vha, req->ring);

	qla_printk(KERN_INFO, ha, "Configure NVRAM parameters...\n");

	ha->isp_ops->nvram_config(vha);

	if (ha->flags.disable_serdes) {
		/* Mask HBA via NVRAM settings? */
		qla_printk(KERN_INFO, ha, "Masking HBA WWPN "
		    "%02x%02x%02x%02x%02x%02x%02x%02x (via NVRAM).\n",
		    vha->port_name[0], vha->port_name[1],
		    vha->port_name[2], vha->port_name[3],
		    vha->port_name[4], vha->port_name[5],
		    vha->port_name[6], vha->port_name[7]);
		return QLA_FUNCTION_FAILED;
	}

	qla_printk(KERN_INFO, ha, "Verifying loaded RISC code...\n");

	if (qla2x00_isp_firmware(vha) != QLA_SUCCESS) {
		rval = ha->isp_ops->chip_diag(vha);
		if (rval)
			return (rval);
		rval = qla2x00_setup_chip(vha);
		if (rval)
			return (rval);
	}
	if (IS_QLA84XX(ha)) {
		ha->cs84xx = qla84xx_get_chip(vha);
		if (!ha->cs84xx) {
			qla_printk(KERN_ERR, ha,
			    "Unable to configure ISP84XX.\n");
			return QLA_FUNCTION_FAILED;
		}
	}
	rval = qla