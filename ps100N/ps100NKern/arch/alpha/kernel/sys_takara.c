c.invert = invert;
			}

		}

	} else {
		rc = PMIC_PARAMETER_ERROR;
	}

	/* Exit the critical section. */
	up(&mutex);

	return rc;
}

/*!
 * @brief Get the Voice CODEC clock source and operating characteristics.
 *
 * Get the current Voice CODEC clock source and operating characteristics.
 *
 * @param  	handle          Device handle from pmic_audio_open() call.
 * @param  	clockIn         The clock signal source.
 * @param  	clockFreq       The clock signal frequency.
 * @param  	samplingRate    The audio data sampling rate.
 * @param       invert          Inversion of the frame sync and/or
 *                              bit clock inputs is enabled/disabled.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC clock settings were
 *                                   successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC clock configuration
 *                                   could not be retrieved.
 */
PMIC_STATUS pmic_audio_vcodec_get_clock(const PMIC_AUDIO_HANDLE handle,
					PMIC_AUDIO_CLOCK_IN_SOURCE *
					const clockIn,
					PMIC_AUDIO_VCODEC_CLOCK_IN_FREQ *
					const clockFreq,
					PMIC_AUDIO_VCODEC_SAMPLING_RATE *
					const samplingRate,
					PMIC_AUDIO_CLOCK_INVERT * const invert)
{
	PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

	/* Use a critical section to ensure that we return a consistent state. */
	if (down_interruptible(&mutex))
		return PMIC_SYSTEM_ERROR_EINTR;

	if ((handle == vCodec.handle) &&
	    (vCodec.handleState == HANDLE_IN_USE) &&
	    (clockIn != (PMIC_AUDIO_CLOCK_IN_SOURCE *) NULL) &&
	    (clockFreq != (PMIC_AUDIO_VCODEC_CLOCK_IN_FREQ *) NULL) &&
	    (samplingRate != (PMIC_AUDIO_VCODEC_SAMPLING_RATE *) NULL) &&
	    (invert != (PMIC_AUDIO_CLOCK_INVERT *) NULL)) {
		*clockIn = vCodec.clockIn;
		*clockFreq = vCodec.clockFreq;
		*samplingRate = vCodec.samplingRate;
		*invert = vCodec.invert;

		rc = PMIC_SUCCESS;
	}

	/* Exit the critical section. */
	up(&mutex);

	return rc;
}

/*!
 * @brief Set the Voice CODEC primary audio channel timeslot.
 *
 * Set the Voice CODEC primary audio channel timeslot. This function must be
 * used if the default timeslot for the primary audio channel is to be changed.
 *
 * @param       handle          Device handle from pmic_audio_open() call.
 * @param       timeslot        Select the primary audio channel timeslot.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC primary audio channel
 *                                   timeslot was successfully configured.
 * @retval      PMIC_PARAMETER_ERROR If the handle or audio channel timeslot
 *                                   was invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC primary audio channel
 *                                   timeslot could not be set.
 */
PMIC_STATUS pmic_audio_vcodec_set_rxtx_timeslot(const PMIC_AUDIO_HANDLE handle,
						const PMIC_AUDIO_VCODEC_TIMESLOT
						timeslot)
{
	PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
	unsigned int reg_write = 0;
	const unsigned int reg_mask = SET_BITS(regSSI_NETWORK, CDCTXRXSLOT, 3);

	/* Use a critical section to ensure a consistent hardware state. */
	if (down_interruptible(&mutex))
		return PMIC_SYSTEM_ERROR_EINTR;

	if ((handle == vCodec.handle) &&
	    (vCodec.handleState == HANDLE_IN_USE) &&
	    ((timeslot == USE_TS0) || (timeslot == USE_TS1) ||
	     (timeslot == USE_TS2) || (timeslot == USE_TS3))) {
		reg_write = SET_BITS(regSSI_NETWORK, CDCTXRXSLOT, timeslot);

		rc = pmic_write_reg(REG_AUDIO_SSI_NETWORK, reg_write, reg_mask);

		if (rc == PMIC_SUCCESS) {
			vCodec.timeslot = timeslot;
		}
	}

	/* Exit the critical section. */
	up(&mutex);

	return rc;
}

/*!
 * @brief Get the current Voice CODEC primary audio channel timeslot.
 *
 * Get the current Voice CODEC primary audio channel timeslot.
 *
 * @param	handle          Device handle from pmic_audio_open() call.
 * @param       timeslot        The primary audio channel timeslot.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC primary audio channel
 *                                   timeslot was successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC primary audio channel
 *                                   timeslot could not be retrieved.
 */
PMIC_STATUS pmic_audio_vcodec_get_rxtx_timeslot(const PMIC_AUDIO_HANDLE handle,
						PMIC_AUDIO_VCODEC_TIMESLOT *
						const timeslot)
{
	PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

	/* Use a critical section to ensure a consistent hardware state. */
	if (down_interruptible(&mutex))
		return PMIC_SYSTEM_ERROR_EINTR;

	if ((handle == vCodec.handle) &&
	    (vCodec.handleState == HANDLE_IN_USE) &&
	    (timeslot != (PMIC_AUDIO_VCODEC_TIMESLOT *) NULL)) {
		*timeslot = vCodec.timeslot;

		rc = PMIC_SUCCESS;
	}

	/* Exit the critical section. */
	up(&mutex);

	return rc;
}

/*!
 * @brief Set the Voice CODEC secondary recording audio channel timeslot.
 *
 * Set the Voice CODEC secondary audio channel timeslot. This function must be
 * used if the default timeslot for the secondary audio channel is to be
 * changed. The secondary audio channel timeslot is used to transmit the audio
 * data that was recorded by the Voice CODEC from the secondary audio input
 * channel.
 *
 * @param   handle          Device handle from pmic_audio_open() call.
 * @param   timeslot        Select the secondary audio channel timeslot.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC secondary audio channel
 *                                   timeslot was successfully configured.
 * @retval      PMIC_PARAMETER_ERROR If the handle or audio channel timeslot
 *                                   was invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC secondary audio channel
 *                                   timeslot could not be set.
 */
PMIC_STATUS pmic_audio_vcodec_set_secondary_txslot(const PMIC_AUDIO_HANDLE
						   handle,
						   const
						   PMIC_AUDIO_VCODEC_TIMESLOT
						   timeslot)
{
	PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
	unsigned int reg_mask = SET_BITS(regSSI_NETWORK, CDCTXSECSLOT, 3);
	unsigned int reg_write = 0;

	/* Use a critical section to ensure a consistent hardware state. */
	if (down_interruptible(&mutex))
		return PMIC_SYSTEM_ERROR_EINTR;

	if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE)) {
		/* How to handle primary slot and secondary slot being the same */
		if ((timeslot >= USE_TS0) && (timeslot <= USE_TS3)
		    && (timeslot != vCodec.timeslot)) {
			reg_write =
			    SET_BITS(regSSI_NETWORK, CDCTXSECSLOT, timeslot);

			rc = pmic_write_reg(REG_AUDIO_SSI_NETWORK,
					    reg_write, reg_mask);

			if (rc == PMIC_SUCCESS) {
				vCodec.secondaryTXtimeslot = timeslot;
			}
		}

	}

	/* Exit the critical section. */
	up(&mutex);

	return rc;
}

/*!
 * @brief Get the Voice CODEC secondary recording audio channel timeslot.
 *
 * Get the Voice CODEC secondary audio channel timeslot.
 *
 * @param       handle          Device handle from pmic_audio_open() call.
 * @param       timeslot        The secondary audio channel timeslot.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC secondary audio channel
 *                                   timeslot was successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC secondary audio channel
 *                                   timeslot could not be retrieved.
 */
PMIC_STATUS pmic_audio_vcodec_get_secondary_txslot(const PMIC_AUDIO_HANDLE
						   handle,
						   PMIC_AUDIO_VCODEC_TIMESLOT *
						   const timeslot)
{
	PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

	/* Use a critical section to ensure a consistent hardware state. */
	if (down_interruptible(&mutex))
		return PMIC_SYSTEM_ERROR_EINTR;

	if ((handle == vCodec.handle) &&
	    (vCodec.handleState == HANDLE_IN_USE) &&
	    (timeslot != (PMIC_AUDIO_VCODEC_TIMESLOT *) NULL)) {
		rc = PMIC_SUCCESS;
		*timeslot = vCodec.secondaryTXtimeslot;
	}

	/* Exit the cr