 request and an interrupt event).
 *
 * We need to use a spinlock whenever we do need to provide mutual
 * exclusion while possibly executing in a hardware interrupt context.
 * Spinlocks should be held for the minimum time that is necessary
 * because hardware interrupts are disabled while a spinlock is held.
 *
 */

static spinlock_t lock = SPIN_LOCK_UNLOCKED;
/*!
 * @brief This mutex is used to provide mutual exclusion.
 *
 * Create a mutex that can be used to provide mutually exclusive
 * read/write access to the globally accessible data structures
 * that were defined above. Mutually exclusive access is required to
 * ensure that the audio data structures are consistent at all times
 * when possibly accessed by multiple threads of execution.
 *
 * Note that we use a mutex instead of the spinlock whenever disabling
 * interrupts while in the critical section is not required. This helps
 * to minimize kernel interrupt handling latency.
 */
static DECLARE_MUTEX(mutex);

/*!
 * @brief Global variable to track currently active interrupt events.
 *
 * This global variable is used to keep track of all of the currently
 * active interrupt events for the audio driver. Note that access to this
 * variable may occur while within an interrupt context and, therefore,
 * must be guarded by using a spinlock.
 */
/* static PMIC_CORE_EVENT eventID = 0; */

/* Prototypes for all static audio driver functions. */
/*
static PMIC_STATUS pmic_audio_mic_boost_enable(void);
static PMIC_STATUS pmic_audio_mic_boost_disable(void);*/
static PMIC_STATUS pmic_audio_close_handle(const PMIC_AUDIO_HANDLE handle);
static PMIC_STATUS pmic_audio_reset_device(const PMIC_AUDIO_HANDLE handle);

static PMIC_STATUS pmic_audio_deregister(void *callback,
					 PMIC_AUDIO_EVENTS * const eventMask);

/*************************************************************************
 * Audio device access APIs.
 *************************************************************************
 */

/*!
 * @name General Setup and Configuration APIs
 * Functions for general setup and configuration of the PMIC Audio
 * hardware.
 */
/*@{*/

PMIC_STATUS pmic_audio_set_autodetect(int val)
{
	PMIC_STATUS status;
	unsigned int reg_mask = 0, reg_write = 0;
	reg_mask = SET_BITS(regAUDIO_RX_0, VAUDIOON, 1);
	status = pmic_write_reg(REG_AUDIO_RX_0, reg_mask, reg_mask);
	if (status != PMIC_SUCCESS)
		return status;
	reg_mask = 0;
	if (val == 1) {
		reg_write = SET_BITS(regAUDIO_RX_0, HSDETEN, 1) |
		    SET_BITS(regAUDIO_RX_0, HSDETAUTOB, 1);
	} else {
		reg_write = 0;
	}
	reg_mask =
	    SET_BITS(regAUDIO_RX_0, HSDETEN, 1) | SET_BITS(regAUDIO_RX_0,
							   HSDETAUTOB, 1);
	status = pmic_write_reg(REG_AUDIO_RX_0, reg_write, reg_mask);

	return status;
}

/*!
 * @brief Request exclusive access to the PMIC Audio hardware.
 *
 * Attempt to open and gain exclusive access to a key PMIC audio hardware
 * component (e.g., the Stereo DAC or the Voice CODEC). Depending upon the
 * type of audio operation that is desired and the nature of the audio data
 * stream, the Stereo DAC and/or the Voice CODEC will be a required hardware
 * component and needs to be acquired by calling this function.
 *
 * If the open request is successful, then a numeric handle is returned
 * and this handle must be used in all subsequent function calls to complete
 * the configuration of either the Stereo DAC or the Voice CODEC and along
 * with any other associated audio hardware components that will be needed.
 *
 * The same handle must also be used in the close call when use of the PMIC
 * audio hardware is no longer required.
 *
 * The open request will fail if the requested audio hardware component has
 * already been acquired by a previous open call but not yet closed.
 *
 * @param  handle          Device handle to be used for subsequent PMIC
 *                              audio API calls.
 * @param  device          The required PMIC audio hardware component.
 *
 * @retval      PMIC_SUCCESS         If the open request was successful
 * @retval      PMIC_PARAMETER_ERROR If the handle argument is NULL.
 * @retval      PMIC_ERROR           If the audio hardware component is
 *                                   unavailable.
 */
PMIC_STATUS pmic_audio_open(PMIC_AUDIO_HANDLE * const handle,
			    const PMIC_AUDIO_SOURCE device)
{
	PMIC_STATUS rc = PMIC_ERROR;

	if (handle == (PMIC_AUDIO_HANDLE *) NULL) {
		/* Do not dereference a NULL pointer. */
		return PMIC_PARAMETER_ERROR;
	}

	/* We only need to acquire a mutex here because the interrupt handler
	 * never modifies the device handle or device handle state. Therefore,
	 * we don't need to worry about conflicts with the interrupt handler
	 * or the need to execute in an interrupt context.
	 *
	 * But we do need a critical section here to avoid problems in case
	 * multiple calls to pmic_audio_open() are made since we can only allow
	 * one of them to succeed.
	 */
	if (down_interruptible(&mutex))
		return PMIC_SYSTEM_ERROR_EINTR;

	/* Check the current device handle state and acquire the handle if
	 * it is available.
	 */

	if ((device == STEREO_DAC) && (stDAC.handleState == HANDLE_FREE)) {
		stDAC.handle = (PMIC_AUDIO_HANDLE) (&stDAC);
		stDAC.handleState = HANDLE_IN_USE;
		*handle = stDAC.handle;
		rc = PMIC_SUCCESS;
	} else if ((device == VOICE_CODEC)
		   && (vCodec.handleState == HANDLE_FREE)) {
		vCodec.handle = (PMIC_AUDIO_HANDLE) (&vCodec);
		vCodec.handleState = HANDLE_IN_USE;
		*handle = vCodec.handle;
		rc = PMIC_SUCCESS;
	} else if ((device == EXTERNAL_STEREO_IN) &&
		   (extStereoIn.handleState == HANDLE_FREE)) {
		extStereoIn.handle = (PMIC_AUDIO_HANDLE) (&extStereoIn);
		extStereoIn.handleState = HANDLE_IN_USE;
		*handle = extStereoIn.handle;
		rc = PMIC_SUCCESS;
	} else {
		*handle = AUDIO_HANDLE_NULL;
	}

	/* Exit the critical section. */
	up(&mutex);

	return rc;
}

/*!
 * @brief Terminate further access to the PMIC audio hardware.
 *
 * Terminate further access to the PMIC audio hardware that was previously
 * acquired by calling pmic_audio_open(). This now allows another thread to
 * successfully call pmic_audio_open() to gain access.
 *
 * Note that we will shutdown/reset the Voice CODEC or Stereo DAC as well as
 * any associated audio input/output components that are no longer required.
 *
 * @param   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the close request was successful.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 */
PMIC_STATUS pmic_audio_close(const PMIC_AUDIO_HANDLE handle)
{
	PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

	/* We 