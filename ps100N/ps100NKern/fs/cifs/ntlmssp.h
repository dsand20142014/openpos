ruct pxa27x_keypad_platform_data *pdata = keypad->pdata;
	uint32_t kprec;

	/* read and reset to default count value */
	kprec = keypad_readl(KPREC);
	keypad_writel(KPREC, DEFAULT_KPREC);

	if (pdata->enable_rotary0)
		report_rotary_event(keypad, 0, rotary_delta(kprec));

	if (pdata->enable_rotary1)
		report_rotary_event(keypad, 1, rotary_delta(kprec >> 16));
}

static void pxa27x_keypad_scan_direct(struct pxa27x_keypad *keypad)
{
	struct pxa27x_keypad_platform_data *pdata = keypad->pdata;
	unsigned int new_state;
	uint32_t kpdk, bits_changed;
	int i;

	kpdk = keypad_readl(KPDK);

	if (pdata->enable_rotary0 || pdata->enable_rotary1)
		pxa27x_keypad_scan_rotary(keypad);

	if (pdata->direct_key_map == NULL)
		return;

	new_state = KPDK_DK(kpdk) & keypad->direct_key_mask;
	bits_changed = keypad->direct_key_state ^ new_state;

	if (bits_changed == 0)
		return;

	for (i = 0; i < pdata->direct_key_num; i++) {
		if (bits_changed & (1 << i))
			input_report_key(keypad->input_dev,
					pdata->direct_key_map[i],
					(new_state & (1 << i)));
	}
	input_sync(keypad->input_dev);
	keypad->direct_key_state = new_state;
}

static irqreturn_t pxa27x_keypad_irq_handler(int irq, void *dev_id)
{
	struct pxa27x_keypad *keypad = dev_id;
	unsigned long kpc = keypad_readl(KPC);

	if (kpc & KPC_DI)
		pxa27x_keypad_scan_direct(keypad);

	if (kpc & KPC_MI)
		pxa27x_keypad_scan_matrix(keypad);

	return IRQ_HANDLED;
}

static void pxa27x_keypad_config(struct pxa27x_keypad *keypad)
{
	struct pxa27x_keypad_platform_data *pdata = keypad->pdata;
	unsigned int mask = 0, direct_key_num = 0;
	unsigned long kpc = 0;

	/* enable matrix keys with automatic scan */
	if (pdata->matrix_key_rows && pdata->matrix_key_cols) {
		kpc |= KPC_ASACT | KPC_MIE | KPC_ME | KPC_MS_ALL;
		kpc |= KPC_MKRN(pdata->matrix_key_rows) |
		       KPC_MKCN(pdata->matrix_key_cols);
	}

	/* enable rotary key, debounce interval same as direct keys */
	if (pdata->enable_rotary0) {
		mask |= 0x03;
		direct_key_num = 2;
		kpc |= KPC_REE0;
	}

	if (pdata->enable_rotary1) {
		mask |= 0x0c;
		direct_key_num = 4;
		kpc |= KPC_REE1;
	}

	if (pdata->direct_key_num > direct_key_num)
		direct_key_num = pdata->direct_key_num;

	keypad->direct_key_mask = ((2 << direct_key_num) - 1) & ~mask;

	/* enable direct key */
	if (direct_key_num)
		kpc |= KPC_DE | KPC_DIE | KPC_DKN(direct_key_num);

	keypad_writel(KPC, kpc | KPC_RE_ZERO_DEB);
	keypad_writel(KPREC, DEFAULT_KPREC);
	keypad_writel(KPKDI, pdata->debounce_interval);
}

static int pxa27x_keypad_open(struct input_dev *dev)
{
	struct pxa27x_keypad *keypad = input_get_drvdata(dev);

	/* Enable unit clock */
	clk_enable(keypad->clk);
	pxa27x_keypad_config(keypad);

	return 0;
}

static void pxa27x_keypad_close(struct input_dev *dev)
{
	struct pxa27x_keypad *keypad = input_get_drvdata(dev);

	/* Disable clock unit */
	clk_disable(keypad->clk);
}

#ifdef CONFIG_PM
static int pxa27x_keypad_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct pxa27x_keypad *keypad = platform_get_drvdata(pdev);

	clk_disable(keypad->clk);

	if (device_may_wakeup(&pdev->dev))
		enable_irq_wake(keypad->irq);

	return 0;
}

static int pxa27x_keypad_resume(struct platform_device *pdev)
{
	struct pxa27x_keypad *keypad = platform_get_drvdata(pdev);
	struct input_dev *input_dev = keypad->input_dev;

	if (device_may_wakeup(&pdev->dev))
		disable_irq_wake(keypad->irq);

	mutex_lock(&input_dev->mutex);

	if (input_dev->users) {
		/* Enable unit clock */
		clk_enable(keypad->clk);
		pxa27x_keypad_config(keypad);
	}

	mutex_unlock(&input_dev->mutex);

	return 0;
}
#else
#define pxa27x_keypad_suspend	NULL
#define pxa27x_keypad_resume	NULL
#endif

#define res_size(res)	((res)->end - (res)->start + 1)

static int __devinit pxa27x_keypad_probe(struct platform_device *pdev)
{
	struct pxa27x_keypad *keypad;
	struct input_dev *input_dev;
	struct resource *res;
	int irq, error;

	keypad = kzalloc(sizeof(struct pxa27x_keypad), GFP_KERNEL);
	if (keypad == NULL) {
		dev_err(&pdev->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}

	keypad->pdata = pdev->dev.platform_datypedef struct _CHALLENGE_MESSAGE {
	__u8 Signature[sizeof(NTLMSSP_SIGNATURE)];
	__le32 MessageType;   /* NtLmChallenge = 2 */
	SECURITY_BUFFER TargetName;
	__le32 NegotiateFlags;
	__u8 Challenge[CIFS_CRYPTO_KEY_SIZE];
	__u8 Reserved[8];
	SECURITY_BUFFER TargetInfoArray;
	/* SECURITY_BUFFER for version info not present since we
	   do not set the version is present flag */
} __attribute__((packed)) CHALLENGE_MESSAGE, *PCHALLENGE_MESSAGE;

typedef struct _AUTHENTICATE_MESSAGE {
	__u8 Signature[sizeof(NTLMSSP_SIGNATURE)];
	__le32 MessageType;  /* NtLmsAuthenticate = 3 */
	SECURITY_BUFFER LmChallengeResponse;
	SECURITY_BUFFER NtChallengeResponse;
	SECURITY_BUFFER DomainName;
	SECURITY_BUFFER UserName;
	SECURITY_BUFFER WorkstationName;
	SECURITY_BUFFER SessionKey;
	__le32 NegotiateFlags;
	/* SECURITY_BUFFER for version info not present since we
	   do not set the version is present flag */
	char UserString[0];
} __attribute__((packed)) AUTHENTICATE_MESSAGE, *PAUTHENTICATE_MESSAGE;
