RT_SYMBOL_GPL(smscore_onresponse);

/**
 * return pointer to next free buffer descriptor from core pool
 *
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 *
 * @return pointer to descriptor on success, NULL on error.
 */
struct smscore_buffer_t *smscore_getbuffer(struct smscore_device_t *coredev)
{
	struct smscore_buffer_t *cb = NULL;
	unsigned long flags;

	DEFINE_WAIT(wait);

	spin_lock_irqsave(&coredev->bufferslock, flags);

	/* This function must return a valid buffer, since the buffer list is
	 * finite, we check that there is an available buffer, if not, we wait
	 * until such buffer become available.
	 */

	prepare_to_wait(&coredev->buffer_mng_waitq, &wait, TASK_INTERRUPTIBLE);

	if (list_empty(&coredev->buffers))
		schedule();

	finish_wait(&coredev->buffer_mng_waitq, &wait);

	cb = (struct smscore_buffer_t *) coredev->buffers.next;
	list_del(&cb->entry);

	spin_unlock_irqrestore(&coredev->bufferslock, flags);

	return cb;
}
EXPORT_SYMBOL_GPL(smscore_getbuffer);

/**
 * return buffer descriptor to a pool
 *
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 * @param cb pointer buffer descriptor
 *
 */
void smscore_putbuffer(struct smscore_device_t *coredev,
		struct smscore_buffer_t *cb) {
	wake_up_interruptible(&coredev->buffer_mng_waitq);
	list_add_locked(&cb->entry, &coredev->buffers, &coredev->bufferslock);
}
EXPORT_SYMBOL_GPL(smscore_putbuffer);

static int smscore_validate_client(struct smscore_device_t *coredev,
				   struct smscore_client_t *client,
				   int data_type, int id)
{
	struct smscore_idlist_t *listentry;
	struct smscore_client_t *registered_client;

	if (!client) {
		sms_err("bad parameter.");
		return -EFAULT;
	}
	registered_client = smscore_find_client(coredev, data_type, id);
	if (registered_client == client)
		return 0;

	if (registered_client) {
		sms_err("The msg ID already registered to another client.");
		return -EEXIST;
	}
	listentry = kzalloc(sizeof(struct smscore_idlist_t), GFP_KERNEL);
	if (!listentry) {
		sms_err("Can't allocate memory for client id.");
		return -ENOMEM;
	}
	listentry->id = id;
	listentry->data_type = data_type;
	list_add_locked(&listentry->entry, &client->idlist,
			&coredev->clientslock);
	return 0;
}

/**
 * creates smsclient object, check that id is taken by another client
 *
 * @param coredev pointer to a coredev object from clients hotplug
 * @param initial_id all messages with this id would be sent to this client
 * @param data_type all messages of this type would be sent to this client
 * @param onresponse_handler client handler that is called to
 *                           process incoming messages
 * @param onremove_handler client handler that is called when device is removed
 * @param context client-specific context
 * @param client pointer to a value that receives created smsclient object
 *
 * @return 0 on success, <0 on error.
 */
int smscore_register_client(struct smscore_device_t *coredev,
			    struct smsclient_params_t *params,
			    struct smscore_client_t **client)
{
	struct smscore_client_t *newclient;
	/* check that no other channel with same parameters exists */
	if (smscore_find_client(coredev, params->data_type,
				params->initial_id)) {
		sms_err("Client already exist.");
		return -EEXIST;
	}

	newclient = kzalloc(sizeof(struct smscore_client_t), GFP_KERNEL);
	if (!newclient) {
		sms_err("Failed to allocate memory for client.");
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&newclient->idlist);
	newclient->coredev = coredev;
	newclient->onresponse_handler = params->onresponse_handler;
	newclient->onremove_handler = params->onremove_handler;
	newclient->context = params->context;
	list_add_locked(&newclient->entry, &coredev->clients,
			&coredev->clientslock);
	smscore_validate_client(coredev, newclient, params->data_type,
				params->initial_id);
	*client = newclient;
	sms_debug("%p %d %d", params->context, params->data_type,
		  params->initial_id);

	return 0;
}
EXPORT_SYMBOL_GPL(smscore_register_client);

/**
 * frees smsclient object and all subclients associated with it
 *
 * @param client pointer to smsclient object returned by
 *               smscore_register_client
 *
 */
void smscore_unregister_client(struct smscore_client_t *client)
{
	struct smscore_device_t *coredev = client->coredev;
	unsigned long flags;

	spin_lock_irqsave(&coredev->clientslock, flags);


	while (!list_empty(&client->idlist)) {
		struct smscore_idlist_t *identry =
			(struct smscore_idlist_t *) client->idlist.next;
		list_del(&identry->entry);
		kfree(identry);
	}

	sms_info("%p", client->context);

	list_del(&client->entry);
	kfree(client);

	spin_unlock_irqrestore(&coredev->clientslock, flags);
}
EXPORT_SYMBOL_GPL(smscore_unregister_client);

/**
 * verifies that source id is not taken by another client,
 * calls device handler to send requests to the device
 *
 * @param client pointer to smsclient object returned by
 *               smscore_register_client
 * @param buffer pointer to a request buffer
 * @param size size (in bytes) of request buffer
 *
 * @return 0 on success, <0 on error.
 */
int smsclient_sendrequest(struct smscore_client_t *client,
			  void *buffer, size_t size)
{
	struct smscore_device_t *coredev;
	struct SmsMsgHdr_ST *phdr = (struct SmsMsgHdr_ST *) buffer;
	int rc;

	if (client == NULL) {
		sms_err("Got NULL client");
		return -EINVAL;
	}

	coredev = client->coredev;

	/* check that no other channel with same id exists */
	if (coredev == NULL) {
		sms_err("Got NULL coredev");
		return -EINVAL;
	}

	rc = smscore_validate_client(client->coredev, client, 0,
				     phdr->msgSrcId);
	if (rc < 0)
		return rc;

	return coredev->sendrequest_handler(coredev->context, buffer, size);
}
EXPORT_SYMBOL_GPL(smsclient_sendrequest);


/* old GPIO managments implementation */
int smscore_configure_gpio(struct smscore_device_t *coredev, u32 pin,
			   struct smscore_config_gpio *pinconfig)
{
	struct {
		struct SmsMsgHdr_ST hdr;
		u32 data[6];
	} msg;

	if (coredev->device_flags & SMS_DEVICE_FAMILY2) {
		msg.hdr.msgSrcId = DVBT_BDA_CONTROL_MSG_ID;
		msg.hdr.msgDstId = HIF_TASK;
		msg.hdr.msgFlags = 0;
		msg.hdr.msgType  = MSG_SMS_GPIO_CONFIG_EX_REQ;
		msg.hdr.msgLength = sizeof(msg);

		msg.data[0] = pin;
		msg.data[1] = pinconfig->pullupdown;

		/* Convert slew rate for Nova: Fast(0) = 3 / Slow(1) = 0; */
		msg.data[2] = pinconfig->outputslewrate == 0 ? 3 : 0;

		switch (pinconfig->outputdriving) {
		case SMS_GPIO_OUTPUTDRIVING_16mA:
			msg.data[3] = 7; /* Nova - 16mA */
			break;
		case SMS_GPIO_OUTPUTDRIVING_12mA:
			msg.data[3] = 5; /* Nova - 11mA */
			break;
		case SMS_GPIO_OUTPUTDRIVING_8mA:
			msg.data[3] = 3; /* Nova - 7mA */
			break;
		case SMS_GPIO_OUTPUTDRIVING_4mA:
		default:
			msg.data[3] = 2; /* Nova - 4mA */
			break;
		}

		msg.data[4] = pinconfig->direction;
		msg.data[5] = 0;
	} else /* TODO: SMS_DEVICE_FAMILY1 */
		return -EINVAL;

	return coredev->sendrequest_handler(coredev->context,
					    &msg, sizeof(msg));
}

int smscore_set_gpio(struct smscore_device_t *coredev, u32 pin, int level)
{
	struct {
		struct SmsMsgHdr_ST hdr;
		u32 data[3];
	} msg;

	if (pin > MAX_GPIO_PIN_NUMBER)
		return -EINVAL;

	msg.hdr.msgSrcId = DVBT_BDA_CONTROL_MSG_ID;
	msg.hdr.msgDstId = HIF_TASK;
	msg.hdr.msgFlags = 0;
	msg.hdr.msgType  = MSG_SMS_GPIO_SET_LEVEL_REQ;
	msg.hdr.msgLength = sizeof(msg);

	msg.data[0] = pin;
	msg.data[1] = level ? 1 : 0;
	msg.data[2] = 0;

	return coredev->sendrequest_handler(coredev->context,
					    &msg, sizeof(msg));
}

/* new GPIO managment implementation */
static int GetGpioPinParams(u32 PinNum, u32 *pTranslatedPinNum,
		u32 *pGroupNum, u32 *pGroupCfg) {

	*pGroupCfg = 1;

	if (PinNum >= 0 && PinNum <= 1)	{
		*pTranslatedPinNum = 0;
		*pGroupNum = 9;
		*pGroupCfg = 2;
	} else if (PinNum >= 2 && PinNum <= 6) {
		*pTranslatedPinNum = 2;
		*pGroupNum = 0;
		*pGroupCfg = 2;
	} else if (PinNum >= 7 && PinNum <= 11) {
		*pTranslatedPinNum = 7;
		*pGroupNum = 1;
	} else if (PinNum >= 12 && PinNum <= 15) {
		*pTranslatedPinNum = 12;
		*pGroupNum = 2;
		*pGroupCfg = 3;
	} else if (PinNum == 16) {
		*pTranslatedPinNum = 16;
		*pGroupNum = 23;
	} else if (PinNum >= 17 && PinNum <= 24) {
		*pTranslatedPinNum = 17;
		*pGroupNum = 3;
	} else if (PinNum == 25) {
		*pTranslatedPinNum = 25;
		*pGroupNum = 6;
	} else if (PinNum >= 26 && PinNum <= 28) {
		*pTranslatedPinNum = 26;
		*pGroupNum = 4;
	} else if (PinNum == 29) {
		*pTranslatedPinNum = 29;
		*pGroupNum = 5;
		*pGroupCfg = 2;
	} else if (PinNum == 30) {
		*pTranslatedPinNum = 30;
		*pGroupNum = 8;
	} else if (PinNum == 31) {
		*pTranslatedPinNum = 31;
		*pGroupNum = 17;
	} else
		return -1;

	*pGroupCfg <<= 24;

	return 0;
}

int smscore_gpio_configure(struct smscore_device_t *coredev, u8 PinNum,
		struct smscore_gpio_config *pGpioConfig) {

	u32 totalLen;
	u32 TranslatedPinNum = 0;
	u32 GroupNum = 0;
	u32 ElectricChar;
	u32 groupCfg;
	void *buffer;
	int rc;

	struct SetGpioMsg {
		struct SmsMsgHdr_ST xMsgHeader;
		u32 msgData[6];
	} *pMsg;


	if (PinNum > MAX_GPIO_PIN_NUMBER)
		return -EINVAL;

	if (pGpioConfig == NULL)
		return -EINVAL;

	totalLen = sizeof(struct SmsMsgHdr_ST) + (sizeof(u32) * 6);

	buffer = kmalloc(totalLen + SMS_DMA_ALIGNMENT,
			GFP_KERNEL | GFP_DMA);
	if (!buffer)
		return -ENOMEM;

	pMsg = (struct SetGpioMsg *) SMS_ALIGN_ADDRESS(buffer);

	pMsg->xMsgHeader.msgSrcId = DVBT_BDA_CONTROL_MSG_ID;
	pMsg->xMsgHeader.msgDstId = HIF_TASK;
	pMsg->xMsgHeader.msgFlags = 0;
	pMsg->xMsgHeader.msgLength = (u16) totalLen;
	pMsg->msgData[0] = PinNum;

	if (!(coredev->device_flags & SMS_DEVICE_FAMILY2)) {
		pMsg->xMsgHeader.msgType = MSG_SMS_GPIO_CONFIG_REQ;
		if (GetGpioPinParams(PinNum, &TranslatedPinNum, &GroupNum,
				&groupCfg) != 0)
			return -EINVAL;

		pMsg->msgData[1] = TranslatedPinNum;
		pMsg->msgData[2] = GroupNum;
		ElectricChar = (pGpioConfig->PullUpDown)
				| (pGpioConfig->InputCharacteristics << 2)
				| (pGpioConfig->OutputSlewRate << 3)
				| (pGpioConfig->OutputDriving << 4);
		pMsg->msgData[3] = ElectricChar;
		pMsg->msgData[4] = pGpioConfig->Direction;
		pMsg->msgData[5] = groupCfg;
	} else {
		pMsg->xMsgHeader.msgType = MSG_SMS_GPIO_CONFIG_EX_REQ;
		pMsg->msgData[1] = pGpioConfig->PullUpDown;
		pMsg->msgData[2] = pGpioConfig->OutputSlewRate;
		pMsg->msgData[3] = pGpioConfig->OutputDriving;
		pMsg->msgData[4] = pGpioConfig->Direction;
		pMsg->msgData[5] = 0;
	}

	smsendian_handle_tx_