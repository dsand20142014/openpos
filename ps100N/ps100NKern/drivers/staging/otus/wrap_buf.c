E64_TEST_LO, 0xFFFFFFFF, 0xFFFFFFFF },
	{ E1000_RA2,	   0, 8, TABLE64_TEST_HI, 0x83FFFFFF, 0xFFFFFFFF },
	{ E1000_MTA,	   0, 128,TABLE32_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ 0, 0, 0, 0 }
};

/* 82575 register test */
static struct igb_reg_test reg_test_82575[] = {
	{ E1000_FCAL,      0x100, 1, PATTERN_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ E1000_FCAH,      0x100, 1, PATTERN_TEST, 0x0000FFFF, 0xFFFFFFFF },
	{ E1000_FCT,       0x100, 1, PATTERN_TEST, 0x0000FFFF, 0xFFFFFFFF },
	{ E1000_VET,       0x100, 1, PATTERN_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ E1000_RDBAL(0),  0x100, 4, PATTERN_TEST, 0xFFFFFF80, 0xFFFFFFFF },
	{ E1000_RDBAH(0),  0x100, 4, PATTERN_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ E1000_RDLEN(0),  0x100, 4, PATTERN_TEST, 0x000FFF80, 0x000FFFFF },
	/* Enable all four RX queues before testing. */
	{ E1000_RXDCTL(0), 0x100, 4, WRITE_NO_TEST, 0, E1000_RXDCTL_QUEUE_ENABLE },
	/* RDH is read-only for 82575, only test RDT. */
	{ E1000_RDT(0),    0x100, 4, PATTERN_TEST, 0x0000FFFF, 0x0000FFFF },
	{ E1000_RXDCTL(0), 0x100, 4, WRITE_NO_TEST, 0, 0 },
	{ E1000_FCRTH,     0x100, 1, PATTERN_TEST, 0x0000FFF0, 0x0000FFF0 },
	{ E1000_FCTTV,     0x100, 1, PATTERN_TEST, 0x0000FFFF, 0x0000FFFF },
	{ E1000_TIPG,      0x100, 1, PATTERN_TEST, 0x3FFFFFFF, 0x3FFFFFFF },
	{ E1000_TDBAL(0),  0x100, 4, PATTERN_TEST, 0xFFFFFF80, 0xFFFFFFFF },
	{ E1000_TDBAH(0),  0x100, 4, PATTERN_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ E1000_TDLEN(0),  0x100, 4, PATTERN_TEST, 0x000FFF80, 0x000FFFFF },
	{ E1000_RCTL,      0x100, 1, SET_READ_TEST, 0xFFFFFFFF, 0x00000000 },
	{ E1000_RCTL,      0x100, 1, SET_READ_TEST, 0x04CFB3FE, 0x003FFFFB },
	{ E1000_RCTL,      0x100, 1, SET_READ_TEST, 0x04CFB3FE, 0xFFFFFFFF },
	{ E1000_TCTL,      0x100, 1, SET_READ_TEST, 0xFFFFFFFF, 0x00000000 },
	{ E1000_TXCW,      0x100, 1, PATTERN_TEST, 0xC000FFFF, 0x0000FFFF },
	{ E1000_RA,        0, 16, TABLE64_TEST_LO, 0xFFFFFFFF, 0xFFFFFFFF },
	{ E1000_RA,        0, 16, TABLE64_TEST_HI, 0x800FFFFF, 0xFFFFFFFF },
	{ E1000_MTA,       0, 128, TABLE32_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ 0, 0, 0, 0 }
};

static bool reg_pattern_test(struct igb_adapter *adapter, u64 *data,
			     int reg, u32 mask, u32 write)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 pat, val;
	u32 _test[] =
		{0x5A5A5A5A, 0xA5A5A5A5, 0x00000000, 0xFFFFFFFF};
	for (pat = 0; pat < ARRAY_SIZE(_test); pat++) {
		wr32(reg, (_test[pat] & write));
		val = rd32(reg);
		if (val != (_test[pat] & write & mask)) {
			dev_err(&adapter->pdev->dev, "pattern test reg %04X "
				"failed: got 0x%08X expected 0x%08X\n",
				reg, val, (_test[pat] & write & mask));
			*data = reg;
			return 1;
		}
	}
	return 0;
}

static bool reg_set_and_check(struct igb_adapter *adapter, u64 *data,
			      int reg, u32 mask, u32 write)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 val;
	wr32(reg, write & mask);
	val = rd32(reg);
	if ((write & mask) != (val & mask)) {
		dev_err(&adapter->pdev->dev, "set/check reg %04X test failed:"
			" got 0x%08X expected 0x%08X\n", reg,
			(val & mask), (write & mask));
		*data = reg;
		return 1;
	}
	return 0;
}

#define REG_PATTERN_TEST(reg, mask, write) \
	do { \
		if (reg_pattern_test(adapter, data, reg, mask, write)) \
			return 1; \
	} while (0)

#define REG_SET_AND_CHECK(reg, mask, write) \
	do { \
		if (reg_set_and_check(adapter, data, reg, mask, wri