ller is not required to hold any lock.
 **/
static lpfc_iocb_type
lpfc_sli_iocb_cmd_type(uint8_t iocb_cmnd)
{
	lpfc_iocb_type type = LPFC_UNKNOWN_IOCB;

	if (iocb_cmnd > CMD_MAX_IOCB_CMD)
		return 0;

	switch (iocb_cmnd) {
	case CMD_XMIT_SEQUENCE_CR:
	case CMD_XMIT_SEQUENCE_CX:
	case CMD_XMIT_BCAST_CN:
	case CMD_XMIT_BCAST_CX:
	case CMD_ELS_REQUEST_CR:
	case CMD_ELS_REQUEST_CX:
	case CMD_CREATE_XRI_CR:
	case CMD_CREATE_XRI_CX:
	case CMD_GET_RPI_CN:
	case CMD_XMIT_ELS_RSP_CX:
	case CMD_GET_RPI_CR:
	case CMD_FCP_IWRITE_CR:
	case CMD_FCP_IWRITE_CX:
	case CMD_FCP_IREAD_CR:
	case CMD_FCP_IREAD_CX:
	case CMD_FCP_ICMND_CR:
	case CMD_FCP_ICMND_CX:
	case CMD_FCP_TSEND_CX:
	case CMD_FCP_TRSP_CX:
	case CMD_FCP_TRECEIVE_CX:
	case CMD_FCP_AUTO_TRSP_CX:
	case CMD_ADAPTER_MSG:
	case CMD_ADAPTER_DUMP:
	case CMD_XMIT_SEQUENCE64_CR:
	case CMD_XMIT_SEQUENCE64_CX:
	case CMD_XMIT_BCAST64_C