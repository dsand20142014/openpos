t % 2) {
		atmel_write8(dev, DR, *src);
		src++; len--;
	}
	for (i = len; i > 1 ; i -= 2) {
		u8 lb = *src++;
		u8 hb = *src++;
		atmel_write16(dev, DR, lb | (hb << 8));
	}
	if (i)
		atmel_write8(dev, DR, *src);
}

static void atmel_copy_to_host(struct net_device *dev, unsigned char *dest,
			       u16 src, u16 len)
{
	int i;
	atmel_writeAR(dev, src);
	if (src % 2) {
		*dest = atmel_read8(dev, DR);
		dest++; len--;
	}
	for (i = len; i > 1 ; i -= 2) {
		u16 hw = atmel_read16(dev, DR);
		*dest++ = hw;
		*dest++ = hw >> 8;
	}
	if (i)
		*dest = atmel_read8(dev, DR);
}

static void atmel_set_gcr(struct net_device *dev, u16 mask)
{
	outw(inw(dev->base_addr + GCR) | mask, dev->base_addr + GCR);
}

static void atmel_clear_gcr(struct net_device *dev, u16 mask)
{
	outw(inw(dev->base_addr + GCR) & ~mask, dev->base_addr + GCR);
}

static int atmel_lock_mac(struct atmel_private *priv)
{
	int i, j = 20;
 retry:
	for (i = 5000; i; i--) {
		if (!atmel_rmem8(priv, atmel_hi(priv, IFACE_LOCKOUT_HOST_OFFSET)))
			break;
		udelay(20);
	}

	if (!i)
		return 0; /* timed out */

	atmel_wmem8(priv, atmel_hi(priv, IFACE_LOCKOUT_MAC_OFFSET), 1);
	if (atmel_rmem8(priv, atmel_hi(priv, IFACE_LOCKOUT_HOST_OFFSET))) {
		atmel_wmem8(priv, atmel_hi(priv, IFACE_LOCKOUT_MAC_OFFSET), 0);
		if (!j--)
			return 0; /* timed out */
		goto retry;
	}

	return 1;
}

static void atmel_wmem32(struct atmel_private *priv, u16 pos, u32 data)
{
	atmel_writeAR(priv->dev, pos);
	atmel_write16(priv->dev, DR, data); /* card 