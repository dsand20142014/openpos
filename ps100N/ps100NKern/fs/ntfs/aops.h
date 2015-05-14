dev;
	unsigned int *key;
	int i;

	key = &pdata->matrix_key_map[0];
	for (i = 0; i < pdata->matrix_key_map_size; i++, key++) {
		int row = ((*key) >> 28) & 0xf;
		int col = ((*key) >> 24) & 0xf;
		int code = (*key) & 0xffffff;

		keypad->matrix_keycodes[(row << 3) + col] = code;
		set_bit(code, input_dev->keybit);
	}

	for (i = 0; i < pdata->direct_key_num; i++)
		set_bit(pdata->direct_key_map[i], input_dev->keybit);

	keypad->rotary_up_key[0] = pdata->rotary0_up_key;
	keypad->rotary_up_key[1] = pdata->rotary1_up_key;
	keypad->rotary_down_key[0] = pdata->rotary0_down_key;
	keypad->rotary_down_key[1] = pdata->rotary1_down_key;
	keypad->rotary_rel_code[0] = pdata->rotary0_rel_code;
	keypad->rotary_rel_code[1] = pdata->rotary1_rel_code;

	if (pdata->enable_rotary0) {
		if (pdata->rotary0_up_key && pdata->rotary0_down_key) {
			set_bit(pdata->rotary0_up_key, input_dev->keybit);
			set_bit(pdata->rotary0_down_key, input_dev->keybit);
		} else
			set_bit(pdata->rotary0_rel_code, input_dev->relbit);
	}

	if (pdata->enable_rotary1) {
		if (pdata->rotary1_up_key && pdata->rotary1_down_key) {
			set_bit(pdata->rotary1_up_key, input_dev->keybit);
			set_bit(pdata->rotary1_down_key, input_dev->keybit);
		} else
			set_bit(pdata->rotary1_rel_code, input_dev->relbit);
	}
}

static inline unsigned int lookup_matrix_keycode(
		struct pxa27x_keypad *keypad, int row, int col)
{
	return keypad->matrix_keycodes[(row << 3) + col];
}

static void pxa27x_keypad_scan_matrix(struct pxa27x_keypad *keypad)
{
	struct pxa27x_keypad_platform_data *pdata = keypad->pdata;
	int row, col, num_keys_pressed = 0;
	uint32_t new_state[MAX_MATRIX_KEY_COLS];
	uint32_t kpas = keypad_readl(KPAS);

	num_keys_pressed = KPAS_MUKP(kpas);

	memset(new_state, 0, sizeof(new_state));

	if (num_keys_pressed == 0)
		goto scan;

	if (num_keys_pressed == 1) {
		col = KPAS_CP(kpas);
		row = KPAS_RP(kpas);

		/* if invalid row/col, treat as no key pressed */
		if (col >= pdata->matrix_key_cols ||
		    row >= pdata->matrix_key_rows)
			goto scan;

		new_state[col] = (1 << row);
		goto scan;
	}

	if (num_keys_pressed > 1) {
		uint32_t kpasmkp0 = keypad_readl(KPASMKP0);
		uint32_t kpasmkp1 = keypad_readl(KPASMKP1);
		uint32_t kpasmkp2 = keypad_readl(KPASMKP2);
		uint32_t kpasmkp3 = keypad_readl(KPASMKP3);

		new_state[0] = kpasmkp0 & KPASMKP_MKC_MASK;
		new_state[1] = (kpasmkp0 >> 16) & KPASMKP_MKC_MASK;
		new_state[2] = kpasmkp1 & KPASMKP_MKC_MASK;
		new_state[3] = (kpasmkp1 >> 16) & KPASMKP_MKC_MASK;
		new_state[4] = kpasmkp2 & KPASMKP_MKC_MASK;
		new_state[5] = (kpasmkp2 >> 16) & KPASMKP_MKC_MASK;
		new_state[6] = kpasmkp3 & KPASMKP_MKC_MASK;
		new_state[7] = (kpasmkp3 >> 16) & KPASMKP_MKC_MASK;
	}
scan:
	for (col = 0; col < pdata->matrix_key_cols; col++) {
		uint32_t bits_changed;

		bits_changed = keypad->matrix_key_state[col] ^ new_state[col];
		if (bits_changed == 0)
			continue;

		for (row = 0; row < pdata->matrix_key_rows; row++) {
			if ((bits_changed & (1 << row)) == 0)
				continue;

			input_report_key(keypad->input_dev,
				lookup_matrix_keycode(keypad, row, col),
				new_state[col] & (1 << row));
		}
	}
	input_sync(keypad->input_dev);
	memcpy(keypad->matrix_key_state, new_state, sizeof(new_state));
}

#define DEFAULT_KPREC	(0x007f007f)

static inline int rotary_delta(uint32_t kprec)
{
	if (kprec & KPREC_OF0)
		return (kprec & 0xff) + 0x7f;
	else if (kprec & KPREC_UF0)
		return (kprec & 0xff) - 0x7f - 0xff;
	else
		return (kprec & 0xff) - 0x7f;
}

static void report_rotary_event(struct pxa27x_keypad *keypad, int r, int delta)
{
	struct input_dev *dev = keypad->input_dev;

	if (delta == 0)
		return;

	if (keypad->rotary_up_key[r] && keypad->rotary_down_key[r]) {
		int keycode = (delta > 0) ? keypad->rotary_up_key[r] :
					    keypad->rotary_down_key[r];

		/* simulate a press-n-release */
		input_report_key(dev, keycode, 1);
		input_sync(dev);
		input_report_key(dev, keycode, 0);
		input_sync(dev);
	} else {
		input_report_rel(dev, keypad->rotary_rel_code[r], delta);
		input_sync(dev);
