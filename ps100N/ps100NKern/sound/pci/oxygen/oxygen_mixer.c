/*
 * C-Media CMI8788 driver - mixer code
 *
 * Copyright (c) Clemens Ladisch <clemens@ladisch.de>
 *
 *
 *  This driver is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License, version 2.
 *
 *  This driver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this driver; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <linux/mutex.h>
#include <sound/ac97_codec.h>
#include <sound/asoundef.h>
#include <sound/control.h>
#include <sound/tlv.h>
#include "oxygen.h"
#include "cm9780.h"

static int dac_volume_info(struct snd_kcontrol *ctl,
			   struct snd_ctl_elem_info *info)
{
	struct oxygen *chip = ctl->private_data;

	info->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	info->count = chip->model.dac_channels;
	info->value.integer.min = chip->model.dac_volume_min;
	info->value.integer.max = chip->model.dac_volume_max;
	return 0;
}

static int dac_volume_get(struct snd_kcontrol *ctl,
			  struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	unsigned int i;

	mutex_lock(&chip->mutex);
	for (i = 0; i < chip->model.dac_channels; ++i)
		value->value.integer.value[i] = chip->dac_volume[i];
	mutex_unlock(&chip->mutex);
	return 0;
}

static int dac_volume_put(struct snd_kcontrol *ctl,
			  struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	unsigned int i;
	int changed;

	changed = 0;
	mutex_lock(&chip->mutex);
	for (i = 0; i < chip->model.dac_channels; ++i)
		if (value->value.integer.value[i] != chip->dac_volume[i]) {
			chip->dac_volume[i] = value->value.integer.value[i];
			changed = 1;
		}
	if (changed)
		chip->model.update_dac_volume(chip);
	mutex_unlock(&chip->mutex);
	return changed;
}

static int dac_mute_get(struct snd_kcontrol *ctl,
			struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;

	mutex_lock(&chip->mutex);
	value->value.integer.value[0] = !chip->dac_mute;
	mutex_unlock(&chip->mutex);
	return 0;
}

static int dac_mute_put(struct snd_kcontrol *ctl,
			  struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	int changed;

	mutex_lock(&chip->mutex);
	changed = !value->value.integer.value[0] != chip->dac_mute;
	if (changed) {
		chip->dac_mute = !value->value.integer.value[0];
		chip->model.update_dac_mute(chip);
	}
	mutex_unlock(&chip->mutex);
	return changed;
}

static int upmix_info(struct snd_kcontrol *ctl, struct snd_ctl_elem_info *info)
{
	static const char *const names[3] = {
		"Front", "Front+Surround", "Front+Surround+Back"
	};
	struct oxygen *chip = ctl->private_data;
	unsigned int count = 2 + (chip->model.dac_channels == 8);

	info->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
	info->count = 1;
	info->value.enumerated.items = count;
	if (info->value.enumerated.item >= count)
		info->value.enumerated.item = count - 1;
	strcpy(info->value.enumerated.name, names[info->value.enumerated.item]);
	return 0;
}

static int upmix_get(struct snd_kcontrol *ctl, struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;

	mutex_lock(&chip->mutex);
	value->value.enumerated.item[0] = chip->dac_routing;
	mutex_unlock(&chip->mutex);
	return 0;
}

void oxygen_update_dac_routing(struct oxygen *chip)
{
	/* DAC 0: front, DAC 1: surround, DAC 2: center/LFE, DAC 3: back */
	static const unsigned int reg_values[3] = {
		/* stereo -> front */
		(0 << OXYGEN_PLAY_DAC0_SOURCE_SHIFT) |
		(1 << OXYGEN_PLAY_DAC1_SOURCE_SHIFT) |
		(2 << OXYGEN_PLAY_DAC2_SOURCE_SHIFT) |
		(3 << OXYGEN_PLAY_DAC3_SOURCE_SHIFT),
		/* stereo -> front+surround */
		(0 << OXYGEN_PLAY_DAC0_SOURCE_SHIFT) |
		(0 << OXYGEN_PLAY_DAC1_SOURCE_SHIFT) |
		(2 << OXYGEN_PLAY_DAC2_SOURCE_SHIFT) |
		(3 << OXYGEN_PLAY_DAC3_SOURCE_SHIFT),
		/* stereo -> front+surround+back */
		(0 << OXYGEN_PLAY_DAC0_SOURCE_SHIFT) |
		(0 << OXYGEN_PLAY_DAC1_SOURCE_SHIFT) |
		(2 << OXYGEN_PLAY_DAC2_SOURCE_SHIFT) |
		(0 << OXYGEN_PLAY_DAC3_SOURCE_SHIFT),
	};
	u8 channels;
	unsigned int reg_value;

	channels = oxygen_read8(chip, OXYGEN_PLAY_CHANNELS) &
		OXYGEN_PLAY_CHANNELS_MASK;
	if (channels == OXYGEN_PLAY_CHANNELS_2)
		reg_value = reg_values[chip->dac_routing];
	else if (channels == OXYGEN_PLAY_CHANNELS_8)
		/* in 7.1 mode, "rear" channels go to the "back" jack */
		reg_value = (0 << OXYGEN_PLAY_DAC0_SOURCE_SHIFT) |
			    (3 << OXYGEN_PLAY_DAC1_SOURCE_SHIFT) |
			    (2 << OXYGEN_PLAY_DAC2_SOURCE_SHIFT) |
			    (1 << OXYGEN_PLAY_DAC3_SOURCE_SHIFT);
	else
		reg_value = (0 << OXYGEN_PLAY_DAC0_SOURCE_SHIFT) |
			    (1 << OXYGEN_PLAY_DAC1_SOURCE_SHIFT) |
			    (2 << OXYGEN_PLAY_DAC2_SOURCE_SHIFT) |
			    (3 << OXYGEN_PLAY_DAC3_SOURCE_SHIFT);
	oxygen_write16_masked(chip, OXYGEN_PLAY_ROUTING, reg_value,
			      OXYGEN_PLAY_DAC0_SOURCE_MASK |
			      OXYGEN_PLAY_DAC1_SOURCE_MASK |
			      OXYGEN_PLAY_DAC2_SOURCE_MASK |
			      OXYGEN_PLAY_DAC3_SOURCE_MASK);
}

static int upmix_put(struct snd_kcontrol *ctl, struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	unsigned int count = 2 + (chip->model.dac_channels == 8);
	int changed;

	mutex_lock(&chip->mutex);
	changed = value->value.enumerated.item[0] != chip->dac_routing;
	if (changed) {
		chip->dac_routing = min(value->value.enumerated.item[0],
					count - 1);
		spin_lock_irq(&chip->reg_lock);
		oxygen_update_dac_routing(chip);
		spin_unlock_irq(&chip->reg_lock);
	}
	mutex_unlock(&chip->mutex);
	return changed;
}

static int spdif_switch_get(struct snd_kcontrol *ctl,
			    struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;

	mutex_lock(&chip->mutex);
	value->value.integer.value[0] = chip->spdif_playback_enable;
	mutex_unlock(&chip->mutex);
	return 0;
}

static unsigned int oxygen_spdif_rate(unsigned int oxygen_rate)
{
	switch (oxygen_rate) {
	case OXYGEN_RATE_32000:
		return IEC958_AES3_CON_FS_32000 << OXYGEN_SPDIF_CS_RATE_SHIFT;
	case OXYGEN_RATE_44100:
		return IEC958_AES3_CON_FS_44100 << OXYGEN_SPDIF_CS_RATE_SHIFT;
	default: /* OXYGEN_RATE_48000 */
		return IEC958_AES3_CON_FS_48000 << OXYGEN_SPDIF_CS_RATE_SHIFT;
	case OXYGEN_RATE_64000:
		return 0xb << OXYGEN_SPDIF_CS_RATE_SHIFT;
	case OXYGEN_RATE_88200:
		return IEC958_AES3_CON_FS_88200 << OXYGEN_SPDIF_CS_RATE_SHIFT;
	case OXYGEN_RATE_96000:
		return IEC958_AES3_CON_FS_96000 << OXYGEN_SPDIF_CS_RATE_SHIFT;
	case OXYGEN_RATE_176400:
		return IEC958_AES3_CON_FS_176400 << OXYGEN_SPDIF_CS_RATE_SHIFT;
	case OXYGEN_RATE_192000:
		return IEC958_AES3_CON_FS_192000 << OXYGEN_SPDIF_CS_RATE_SHIFT;
	}
}

void oxygen_update_spdif_source(struct oxygen *chip)
{
	u32 old_control, new_control;
	u16 old_routing, new_routing;
	unsigned int oxygen_rate;

	old_control = oxygen_read32(chip, OXYGEN_SPDIF_CONTROL);
	old_routing = oxygen_read16(chip, OXYGEN_PLAY_ROUTING);
	if (chip->pcm_active & (1 << PCM_SPDIF)) {
		new_control = old_control | OXYGEN_SPDIF_OUT_ENABLE;
		new_routing = (old_routing & ~OXYGEN_PLAY_SPDIF_MASK)
			| OXYGEN_PLAY_SPDIF_SPDIF;
		oxygen_rate = (old_control >> OXYGEN_SPDIF_OUT_RATE_SHIFT)
			& OXYGEN_I2S_RATE_MASK;
		/* S/PDIF rate was already set by the caller */
	} else if ((chip->pcm_active & (1 << PCM_MULTICH)) &&
		   chip->spdif_playback_enable) {
		new_routing = (old_routing & ~OXYGEN_PLAY_SPDIF_MASK)
			| OXYGEN_PLAY_SPDIF_MULTICH_01;
		oxygen_rate = oxygen_read16(chip, OXYGEN_I2S_MULTICH_FORMAT)
			& OXYGEN_I2S_RATE_MASK;
		new_control = (old_control & ~OXYGEN_SPDIF_OUT_RATE_MASK) |
			(oxygen_rate << OXYGEN_SPDIF_OUT_RATE_SHIFT) |
			OXYGEN_SPDIF_OUT_ENABLE;
	} else {
		new_control = old_control & ~OXYGEN_SPDIF_OUT_ENABLE;
		new_routing = old_routing;
		oxygen_rate = OXYGEN_RATE_44100;
	}
	if (old_routing != new_routing) {
		oxygen_write32(chip, OXYGEN_SPDIF_CONTROL,
			       new_control & ~OXYGEN_SPDIF_OUT_ENABLE);
		oxygen_write16(chip, OXYGEN_PLAY_ROUTING, new_routing);
	}
	if (new_control & OXYGEN_SPDIF_OUT_ENABLE)
		oxygen_write32(chip, OXYGEN_SPDIF_OUTPUT_BITS,
			       oxygen_spdif_rate(oxygen_rate) |
			       ((chip->pcm_active & (1 << PCM_SPDIF)) ?
				chip->spdif_pcm_bits : chip->spdif_bits));
	oxygen_write32(chip, OXYGEN_SPDIF_CONTROL, new_control);
}

static int spdif_switch_put(struct snd_kcontrol *ctl,
			    struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	int changed;

	mutex_lock(&chip->mutex);
	changed = value->value.integer.value[0] != chip->spdif_playback_enable;
	if (changed) {
		chip->spdif_playback_enable = !!value->value.integer.value[0];
		spin_lock_irq(&chip->reg_lock);
		oxygen_update_spdif_source(chip);
		spin_unlock_irq(&chip->reg_lock);
	}
	mutex_unlock(&chip->mutex);
	return changed;
}

static int spdif_info(struct snd_kcontrol *ctl, struct snd_ctl_elem_info *info)
{
	info->type = SNDRV_CTL_ELEM_TYPE_IEC958;
	info->count = 1;
	return 0;
}

static void oxygen_to_iec958(u32 bits, struct snd_ctl_elem_value *value)
{
	value->value.iec958.status[0] =
		bits & (OXYGEN_SPDIF_NONAUDIO | OXYGEN_SPDIF_C |
			OXYGEN_SPDIF_PREEMPHASIS);
	value->value.iec958.status[1] = /* category and original */
		bits >> OXYGEN_SPDIF_CATEGORY_SHIFT;
}

static u32 iec958_to_oxygen(struct snd_ctl_elem_value *value)
{
	u32 bits;

	bits = value->value.iec958.status[0] &
		(OXYGEN_SPDIF_NONAUDIO | OXYGEN_SPDIF_C |
		 OXYGEN_SPDIF_PREEMPHASIS);
	bits |= value->value.iec958.status[1] << OXYGEN_SPDIF_CATEGORY_SHIFT;
	if (bits & OXYGEN_SPDIF_NONAUDIO)
		bits |= OXYGEN_SPDIF_V;
	return bits;
}

static inline void write_spdif_bits(struct oxygen *chip, u32 bits)
{
	oxygen_write32_masked(chip, OXYGEN_SPDIF_OUTPUT_BITS, bits,
			      OXYGEN_SPDIF_NONAUDIO |
			      OXYGEN_SPDIF_C |
			      OXYGEN_SPDIF_PREEMPHASIS |
			      OXYGEN_SPDIF_CATEGORY_MASK |
			      OXYGEN_SPDIF_ORIGINAL |
			      OXYGEN_SPDIF_V);
}

static int spdif_default_get(struct snd_kcontrol *ctl,
			     struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;

	mutex_lock(&chip->mutex);
	oxygen_to_iec958(chip->spdif_bits, value);
	mutex_unlock(&chip->mutex);
	return 0;
}

static int spdif_default_put(struct snd_kcontrol *ctl,
			     struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	u32 new_bits;
	int changed;

	new_bits = iec958_to_oxygen(value);
	mutex_lock(&chip->mutex);
	changed = new_bits != chip->spdif_bits;
	if (changed) {
		chip->spdif_bits = new_bits;
		if (!(chip->pcm_active & (1 << PCM_SPDIF)))
			write_spdif_bits(chip, new_bits);
	}
	mutex_unlock(&chip->mutex);
	return changed;
}

static int spdif_mask_get(struct snd_kcontrol *ctl,
			  struct snd_ctl_elem_value *value)
{
	value->value.iec958.status[0] = IEC958_AES0_NONAUDIO |
		IEC958_AES0_CON_NOT_COPYRIGHT | IEC958_AES0_CON_EMPHASIS;
	value->value.iec958.status[1] =
		IEC958_AES1_CON_CATEGORY | IEC958_AES1_CON_ORIGINAL;
	return 0;
}

static int spdif_pcm_get(struct snd_kcontrol *ctl,
			 struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;

	mutex_lock(&chip->mutex);
	oxygen_to_iec958(chip->spdif_pcm_bits, value);
	mutex_unlock(&chip->mutex);
	return 0;
}

static int spdif_pcm_put(struct snd_kcontrol *ctl,
			 struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	u32 new_bits;
	int changed;

	new_bits = iec958_to_oxygen(value);
	mutex_lock(&chip->mutex);
	changed = new_bits != chip->spdif_pcm_bits;
	if (changed) {
		chip->spdif_pcm_bits = new_bits;
		if (chip->pcm_active & (1 << PCM_SPDIF))
			write_spdif_bits(chip, new_bits);
	}
	mutex_unlock(&chip->mutex);
	return changed;
}

static int spdif_input_mask_get(struct snd_kcontrol *ctl,
				struct snd_ctl_elem_value *value)
{
	value->value.iec958.status[0] = 0xff;
	value->value.iec958.status[1] = 0xff;
	value->value.iec958.status[2] = 0xff;
	value->value.iec958.status[3] = 0xff;
	return 0;
}

static int spdif_input_default_get(struct snd_kcontrol *ctl,
				   struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	u32 bits;

	bits = oxygen_read32(chip, OXYGEN_SPDIF_INPUT_BITS);
	value->value.iec958.status[0] = bits;
	value->value.iec958.status[1] = bits >> 8;
	value->value.iec958.status[2] = bits >> 16;
	value->value.iec958.status[3] = bits >> 24;
	return 0;
}

static int spdif_loopback_get(struct snd_kcontrol *ctl,
			      struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;

	value->value.integer.value[0] =
		!!(oxygen_read32(chip, OXYGEN_SPDIF_CONTROL)
		   & OXYGEN_SPDIF_LOOPBACK);
	return 0;
}

static int spdif_loopback_put(struct snd_kcontrol *ctl,
			      struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	u32 oldreg, newreg;
	int changed;

	spin_lock_irq(&chip->reg_lock);
	oldreg = oxygen_read32(chip, OXYGEN_SPDIF_CONTROL);
	if (value->value.integer.value[0])
		newreg = oldreg | OXYGEN_SPDIF_LOOPBACK;
	else
		newreg = oldreg & ~OXYGEN_SPDIF_LOOPBACK;
	changed = newreg != oldreg;
	if (changed)
		oxygen_write32(chip, OXYGEN_SPDIF_CONTROL, newreg);
	spin_unlock_irq(&chip->reg_lock);
	return changed;
}

static int monitor_volume_info(struct snd_kcontrol *ctl,
			       struct snd_ctl_elem_info *info)
{
	info->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	info->count = 1;
	info->value.integer.min = 0;
	info->value.integer.max = 1;
	return 0;
}

static int monitor_get(struct snd_kcontrol *ctl,
		       struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	u8 bit = ctl->private_value;
	int invert = ctl->private_value & (1 << 8);

	value->value.integer.value[0] =
		!!invert ^ !!(oxygen_read8(chip, OXYGEN_ADC_MONITOR) & bit);
	return 0;
}

static int monitor_put(struct snd_kcontrol *ctl,
		       struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	u8 bit = ctl->private_value;
	int invert = ctl->private_value & (1 << 8);
	u8 oldreg, newreg;
	int changed;

	spin_lock_irq(&chip->reg_lock);
	oldreg = oxygen_read8(chip, OXYGEN_ADC_MONITOR);
	if ((!!value->value.integer.value[0] ^ !!invert) != 0)
		newreg = oldreg | bit;
	else
		newreg = oldreg & ~bit;
	changed = newreg != oldreg;
	if (changed)
		oxygen_write8(chip, OXYGEN_ADC_MONITOR, newreg);
	spin_unlock_irq(&chip->reg_lock);
	return changed;
}

static int ac97_switch_get(struct snd_kcontrol *ctl,
			   struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	unsigned int codec = (ctl->private_value >> 24) & 1;
	unsigned int index = ctl->private_value & 0xff;
	unsigned int bitnr = (ctl->private_value >> 8) & 0xff;
	int invert = ctl->private_value & (1 << 16);
	u16 reg;

	mutex_lock(&chip->mutex);
	reg = oxygen_read_ac97(chip, codec, index);
	mutex_unlock(&chip->mutex);
	if (!(reg & (1 << bitnr)) ^ !invert)
		value->value.integer.value[0] = 1;
	else
		value->value.integer.value[0] = 0;
	return 0;
}

static void mute_ac97_ctl(struct oxygen *chip, unsigned int control)
{
	unsigned int priv_idx;
	u16 value;

	if (!chip->controls[control])
		return;
	priv_idx = chip->controls[control]->private_value & 0xff;
	value = oxygen_read_ac97(chip, 0, priv_idx);
	if (!(value & 0x8000)) {
		oxygen_write_ac97(chip, 0, priv_idx, value | 0x8000);
		if (chip->model.ac97_switch)
			chip->model.ac97_switch(chip, priv_idx, 0x8000);
		snd_ctl_notify(chip->card, SNDRV_CTL_EVENT_MASK_VALUE,
			       &chip->controls[control]->id);
	}
}

static int ac97_switch_put(struct snd_kcontrol *ctl,
			   struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	unsigned int codec = (ctl->private_value >> 24) & 1;
	unsigned int index = ctl->private_value & 0xff;
	unsigned int bitnr = (ctl->private_value >> 8) & 0xff;
	int invert = ctl->private_value & (1 << 16);
	u16 oldreg, newreg;
	int change;

	mutex_lock(&chip->mutex);
	oldreg = oxygen_read_ac97(chip, codec, index);
	newreg = oldreg;
	if (!value->value.integer.value[0] ^ !invert)
		newreg |= 1 << bitnr;
	else
		newreg &= ~(1 << bitnr);
	change = newreg != oldreg;
	if (change) {
		oxygen_write_ac97(chip, codec, index, newreg);
		if (codec == 0 && chip->model.ac97_switch)
			chip->model.ac97_switch(chip, index, newreg & 0x8000);
		if (index == AC97_LINE) {
			oxygen_write_ac97_masked(chip, 0, CM9780_GPIO_STATUS,
						 newreg & 0x8000 ?
						 CM9780_GPO0 : 0, CM9780_GPO0);
			if (!(newreg & 0x8000)) {
				mute_ac97_ctl(chip, CONTROL_MIC_CAPTURE_SWITCH);
				mute_ac97_ctl(chip, CONTROL_CD_CAPTURE_SWITCH);
				mute_ac97_ctl(chip, CONTROL_AUX_CAPTURE_SWITCH);
			}
		} else if ((index == AC97_MIC || index == AC97_CD ||
			    index == AC97_VIDEO || index == AC97_AUX) &&
			   bitnr == 15 && !(newreg & 0x8000)) {
			mute_ac97_ctl(chip, CONTROL_LINE_CAPTURE_SWITCH);
			oxygen_write_ac97_masked(chip, 0, CM9780_GPIO_STATUS,
						 CM9780_GPO0, CM9780_GPO0);
		}
	}
	mutex_unlock(&chip->mutex);
	return change;
}

static int ac97_volume_info(struct snd_kcontrol *ctl,
			    struct snd_ctl_elem_info *info)
{
	int stereo = (ctl->private_value >> 16) & 1;

	info->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	info->count = stereo ? 2 : 1;
	info->value.integer.min = 0;
	info->value.integer.max = 0x1f;
	return 0;
}

static int ac97_volume_get(struct snd_kcontrol *ctl,
			   struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	unsigned int codec = (ctl->private_value >> 24) & 1;
	int stereo = (ctl->private_value >> 16) & 1;
	unsigned int index = ctl->private_value & 0xff;
	u16 reg;

	mutex_lock(&chip->mutex);
	reg = oxygen_read_ac97(chip, codec, index);
	mutex_unlock(&chip->mutex);
	value->value.integer.value[0] = 31 - (reg & 0x1f);
	if (stereo)
		value->value.integer.value[1] = 31 - ((reg >> 8) & 0x1f);
	return 0;
}

static int ac97_volume_put(struct snd_kcontrol *ctl,
			   struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	unsigned int codec = (ctl->private_value >> 24) & 1;
	int stereo = (ctl->private_value >> 16) & 1;
	unsigned int index = ctl->private_value & 0xff;
	u16 oldreg, newreg;
	int change;

	mutex_lock(&chip->mutex);
	oldreg = oxygen_read_ac97(chip, codec, index);
	newreg = oldreg;
	newreg = (newreg & ~0x1f) |
		(31 - (value->value.integer.value[0] & 0x1f));
	if (stereo)
		newreg = (newreg & ~0x1f00) |
			((31 - (value->value.integer.value[1] & 0x1f)) << 8);
	else
		newreg = (newreg & ~0x1f00) | ((newreg & 0x1f) << 8);
	change = newreg != oldreg;
	if (change)
		oxygen_write_ac97(chip, codec, index, newreg);
	mutex_unlock(&chip->mutex);
	return change;
}

static int ac97_fp_rec_volume_info(struct snd_kcontrol *ctl,
				   struct snd_ctl_elem_info *info)
{
	info->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	info->count = 2;
	info->value.integer.min = 0;
	info->value.integer.max = 7;
	return 0;
}

static int ac97_fp_rec_volume_get(struct snd_kcontrol *ctl,
				  struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	u16 reg;

	mutex_lock(&chip->mutex);
	reg = oxygen_read_ac97(chip, 1, AC97_REC_GAIN);
	mutex_unlock(&chip->mutex);
	value->value.integer.value[0] = reg & 7;
	value->value.integer.value[1] = (reg >> 8) & 7;
	return 0;
}

static int ac97_fp_rec_volume_put(struct snd_kcontrol *ctl,
				  struct snd_ctl_elem_value *value)
{
	struct oxygen *chip = ctl->private_data;
	u16 oldreg, newreg;
	int change;

	mutex_lock(&chip->mutex);
	oldreg = oxygen_read_ac97(chip, 1, AC97_REC_GAIN);
	newreg = oldreg & ~0x0707;
	newreg = newreg | (value->value.integer.value[0] & 7);
	newreg = newreg | ((value->value.integer.value[0] & 7) << 8);
	change = newreg != oldreg;
	if (change)
		oxygen_write_ac97(chip, 1, AC97_REC_GAIN, newreg);
	mutex_unlock(&chip->mutex);
	return change;
}

#define AC97_SWITCH(xname, codec, index, bitnr, invert) { \
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
		.name = xname, \
		.info = snd_ctl_boolean_mono_info, \
		.get = ac97_switch_get, \
		.put = ac97_switch_put, \
		.private_value = ((codec) << 24) | ((invert) << 16) | \
				 ((bitnr) << 8) | (index), \
	}
#define AC97_VOLUME(xname, codec, index, stereo) { \
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
		.name = xname, \
		.access = SNDRV_CTL_ELEM_ACCESS_READWRITE | \
			  SNDRV_CTL_ELEM_ACCESS_TLV_READ, \
		.info = ac97_volume_info, \
		.get = ac97_volume_get, \
		.put = ac97_volume_put, \
		.tlv = { .p = ac97_db_scale, }, \
		.private_value = ((codec) << 24) | ((stereo) << 16) | (index), \
	}

static DECLARE_TLV_DB_SCALE(monitor_db_scale, -1000, 1000, 0);
static DECLARE_TLV_DB_SCALE(ac97_db_scale, -3450, 150, 0);
static DECLARE_TLV_DB_SCALE(ac97_rec_db_scale, 0, 150, 0);

static const struct snd_kcontrol_new controls[] = {
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "Master Playback Volume",
		.access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.info = dac_volume_info,
		.get = dac_volume_get,
		.put = dac_volume_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "Master Playback Switch",
		.info = snd_ctl_boolean_mono_info,
		.get = dac_mute_get,
		.put = dac_mute_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "Stereo Upmixing",
		.info = upmix_info,
		.get = upmix_get,
		.put = upmix_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = SNDRV_CTL_NAME_IEC958("", PLAYBACK, SWITCH),
		.info = snd_ctl_boolean_mono_info,
		.get = spdif_switch_get,
		.put = spdif_switch_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_PCM,
		.device = 1,
		.name = SNDRV_CTL_NAME_IEC958("", PLAYBACK, DEFAULT),
		.info = spdif_info,
		.get = spdif_default_get,
		.put = spdif_default_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_PCM,
		.device = 1,
		.name = SNDRV_CTL_NAME_IEC958("", PLAYBACK, CON_MASK),
		.access = SNDRV_CTL_ELEM_ACCESS_READ,
		.info = spdif_info,
		.get = spdif_mask_get,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_PCM,
		.device = 1,
		.name = SNDRV_CTL_NAME_IEC958("", PLAYBACK, PCM_STREAM),
		.access = SNDRV_CTL_ELEM_ACCESS_READWRITE |
			  SNDRV_CTL_ELEM_ACCESS_INACTIVE,
		.info = spdif_info,
		.get = spdif_pcm_get,
		.put = spdif_pcm_put,
	},
};

static const struct snd_kcontrol_new spdif_input_controls[] = {
	{
		.iface = SNDRV_CTL_ELEM_IFACE_PCM,
		.device = 1,
		.name = SNDRV_CTL_NAME_IEC958("", CAPTURE, MASK),
		.access = SNDRV_CTL_ELEM_ACCESS_READ,
		.info = spdif_info,
		.get = spdif_input_mask_get,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_PCM,
		.device = 1,
		.name = SNDRV_CTL_NAME_IEC958("", CAPTURE, DEFAULT),
		.access = SNDRV_CTL_ELEM_ACCESS_READ,
		.info = spdif_info,
		.get = spdif_input_default_get,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = SNDRV_CTL_NAME_IEC958("Loopback ", NONE, SWITCH),
		.info = snd_ctl_boolean_mono_info,
		.get = spdif_loopback_get,
		.put = spdif_loopback_put,
	},
};

static const struct {
	unsigned int pcm_dev;
	struct snd_kcontrol_new controls[2];
} monitor_controls[] = {
	{
		.pcm_dev = CAPTURE_0_FROM_I2S_1,
		.controls = {
			{
				.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
				.name = "Analog Input Monitor Switch",
				.info = snd_ctl_boolean_mono_info,
				.get = monitor_get,
				.put = monitor_put,
				.private_value = OXYGEN_ADC_MONITOR_A,
			},
			{
				.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
				.name = "Analog Input Monitor Volume",
				.access = SNDRV_CTL_ELEM_ACCESS_READWRITE |
					  SNDRV_CTL_ELEM_ACCESS_TLV_READ,
				.info = monitor_volume_info,
				.get = monitor_get,
				.put = monitor_put,
				.private_value = OXYGEN_ADC_MONITOR_A_HALF_VOL
						| (1 << 8),
				.tlv = { .p = monitor_db_scale, },
			},
		},
	},
	{
		.pcm_dev = CAPTURE_0_FROM_I2S_2,
		.controls = {
			{
				.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
				.name = "Analog Input Monitor Switch",
				.info = snd_ctl_boolean_mono_info,
				.get = monitor_get,
				.put = monitor_put,
				.private_value = OXYGEN_ADC_MONITOR_B,
			},
			{
				.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
				.name = "Analog Input Monitor Volume",
				.access = SNDRV_CTL_ELEM_ACCESS_READWRITE |
					  SNDRV_CTL_ELEM_ACCESS_TLV_READ,
				.info = monitor_volume_info,
				.get = monitor_get,
				.put = monitor_put,
				.private_value = OXYGEN_ADC_MONITOR_B_HALF_VOL
						| (1 << 8),
				.tlv = { .p = monitor_db_scale, },
			},
		},
	},
	{
		.pcm_dev = CAPTURE_2_FROM_I2S_2,
		.controls = {
			{
				.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
				.name = "Analog Input Monitor Switch",
				.index = 1,
				.info = snd_ctl_boolean_mono_info,
				.get = monitor_get,
				.put = monitor_put,
				.private_value = OXYGEN_ADC_MONITOR_B,
			},
			{
				.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
				.name = "Analog Input Monitor Volume",
				.index = 1,
				.access = SNDRV_CTL_ELEM_ACCE/*******************************************************************
 * This file is part of the Emulex Linux Device Driver for         *
 * Fibre Channel Host Bus Adapters.                                *
 * Copyright (C) 2004-2009 Emulex.  All rights reserved.           *
 * EMULEX and SLI are trademarks of Emulex.                        *
 * www.emulex.com                                                  *
 * Portions Copyright (C) 2004-2005 Christoph Hellwig              *
 *                                                                 *
 * This program is free software; you can redistribute it and/or   *
 * modify it under the terms of version 2 of the GNU General       *
 * Public License as published by the Free Software Foundation.    *
 * This program is distributed in the hope that it will be useful. *
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND          *
 * WARRANTIES, INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY,  *
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE      *
 * DISCLAIMED, EXCEPT TO THE EXTENT THAT SUCH DISCLAIMERS ARE HELD *
 * TO BE LEGALLY INVALID.  See the GNU General Public License for  *
 * more details, a copy of which can be found in the file COPYING  *
 * included with this package.                                     *
 *******************************************************************/
/* See Fibre Channel protocol T11 FC-LS for details */
#include <linux/blkdev.h>
#include <linux/pci.h>
#include <linux/interrupt.h>

#include <scsi/scsi.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_transport_fc.h>

#include "lpfc_hw4.h"
#include "lpfc_hw.h"
#include "lpfc_sli.h"
#include "lpfc_sli4.h"
#include "lpfc_nl.h"
#include "lpfc_disc.h"
#include "lpfc_scsi.h"
#include "lpfc.h"
#include "lpfc_logmsg.h"
#include "lpfc_crtn.h"
#include "lpfc_vport.h"
#include "lpfc_debugfs.h"

static int lpfc_els_retry(struct lpfc_hba *, struct lpfc_iocbq *,
			  struct lpfc_iocbq *);
static void lpfc_cmpl_fabric_iocb(struct lpfc_hba *, struct lpfc_iocbq *,
			struct lpfc_iocbq *);
static void lpfc_fabric_abort_vport(struct lpfc_vport *vport);
static int lpfc_issue_els_fdisc(struct lpfc_vport *vport,
				struct lpfc_nodelist *ndlp, uint8_t retry);
static int lpfc_issue_fabric_iocb(struct lpfc_hba *phba,
				  struct lpfc_iocbq *iocb);
static void lpfc_register_new_vport(struct lpfc_hba *phba,
				    struct lpfc_vport *vport,
				    struct lpfc_nodelist *ndlp);

static int lpfc_max_els_tries = 3;

/**
 * lpfc_els_chk_latt - Check host link attention event for a vport
 * @vport: pointer to a host virtual N_Port data structure.
 *
 * This routine checks whether there is an outstanding host link
 * attention event during the discovery process with the @vport. It is done
 * by reading the HBA's Host Attention (HA) register. If there is any host
 * link attention events during this @vport's discovery process, the @vport
 * shall be marked as FC_ABORT_DISCOVERY, a host link attention clear shall
 * be issued if the link state is not already in host link cleared state,
 * and a return code shall indicate whether the host link attention event
 * had happened.
 *
 * Note that, if either the host link is in state LPFC_LINK_DOWN or @vport
 * state in LPFC_VPORT_READY, the request for checking host link attention
 * event will be ignored and a return code shall indicate no host link
 * attention event had happened.
 *
 * Return codes
 *   0 - no host link attention event happened
 *   1 - host link attention event happened
 **/
int
lpfc_els_chk_latt(struct lpfc_vport *vport)
{
	struct Scsi_Host *shost = lpfc_shost_from_vport(vport);
	struct lpfc_hba  *phba = vport->phba;
	uint32_t ha_copy;

	if (vport->port_state >= LPFC_VPORT_READY ||
	    phba->link_state == LPFC_LINK_DOWN ||
	    phba->sli_rev > LPFC_SLI_REV3)
		return 0;

	/* Read the HBA Host Attention Register */
	ha_copy = readl(phba->HAregaddr);

	if (!(ha_copy & HA_LATT))
		return 0;

	/* Pending Link Event during Discovery */
	lpfc_printf_vlog(vport, KERN_ERR, LOG_DISCOVERY,
			 "0237 Pending 
		err = add_controls(chip, monitor_controls[i].controls,
				   ARRAY_SIZE(monitor_controls[i].controls));
		if (err < 0)
			return err;
	}
	if (chip->has_ac97_0) {
		err = add_controls(chip, ac97_controls,
				   ARRAY_SIZE(ac97_controls));
		if (err < 0)
			return err;
	}
	if (chip->has_ac97_1) {
		err = add_controls(chip, ac97_fp_controls,
				   ARRAY_SIZE(ac97_fp_controls));
		if (err < 0)
			return err;
	}
	return chip->model.mixer_init ? chip->model.mixer_init(chip) : 0;
}
