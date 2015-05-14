/*
 * Copyright (c) 2007-2008 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*                                                                      */
/*  Module Name : mm.c                                                  */
/*                                                                      */
/*  Abstract                                                            */
/*      This module contains common functions for handle AP             */
/*      management frame.                                               */
/*                                                                      */
/*  NOTES                                                               */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#include "cprecomp.h"
#include "ratectrl.h"

extern const u8_t zcUpToAc[];

void zfMmApTimeTick(zdev_t* dev)
{
    u32_t now;
    zmw_get_wlan_dev(dev);

    //zm_debug_msg1("wd->wlanMode : ", wd->wlanMode);
    if (wd->wlanMode == ZM_MODE_AP)
    {
        /* => every 1.28 seconds */
        /* AP : aging STA that does not active for wd->ap.staAgingTime    */
        now = wd->tick & 0x7f;
        if (now == 0x0)
        {
            zfApAgingSta(dev);
        }
        else if (now == 0x1f)
        {
            zfQueueAge(dev, wd->ap.uapsdQ, wd->tick, 10000);
        }
        /* AP : check (wd->ap.protectedObss) and (wd->ap.bStaAssociated)  */
        /*      to enable NonErp and Protection mode                      */
        else if (now == 0x3f)
        {
            //zfApProtctionMonitor(dev);
        }
    }
}

/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfApInitStaTbl              */
/*      Init AP's station table.                                        */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      None                                                            */
/*                                                                      */
/*    AUTHOR                                                            */
/*      Stephen Chen        ZyDAS Technology Corporation    2005.10     */
/*                                                                      */
/************************************************************************/
void zfApInitStaTbl(zdev_t* dev)
{
    u16_t i;

    zmw_get_wlan_dev(dev);

    for (i=0; i<ZM_MAX_STA_SUPPORT; i++)
    {
        wd->ap.staTable[i].valid = 0;
        wd->ap.staTable[i].state = 0;
        wd->ap.staTable[i].addr[0] = 0;
        wd->ap.staTable[i].addr[1] = 0;
        wd->ap.staTable[i].addr[2] = 0;
        wd->ap.staTable[i].time = 0;
        wd->ap.staTable[i].vap = 0;
        wd->ap.staTable[i].encryMode = ZM_NO_WEP;
    }
    return;
}


/****************************************************
/******************************************************************************
 *
 * Module Name: exoparg1 - AML execution - opcodes with 1 argument
 *
 *****************************************************************************/

/*
 * Copyright (C) 2000 - 2008, Intel Corp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 */

#include <acpi/acpi.h>
#include "accommon.h"
#include "acparser.h"
#include "acdispat.h"
#include "acinterp.h"
#include "amlcode.h"
#include "acnamesp.h"

#define _COMPONENT          ACPI_EXECUTER
ACPI_MODULE_NAME("exoparg1")

/*!
 * Naming convention for AML interpreter execution routines.
 *
 * The routines that begin execution of AML opcodes are named with a common
 * convention based upon the number of arguments, the number of target operands,
 * and whether or not a value is returned:
 *
 *      AcpiExOpcode_xA_yT_zR
 *
 * Where:
 *
 * xA - ARGUMENTS:    The number of arguments (input operands) that are
 *                    required for this opcode type (0 through 6 args).
 * yT - TARGETS:      The number of targets (output operands) that are required
 *                    for this opcode type (0, 1, or 2 targets).
 * zR - RETURN VALUE: Indicates whether this opcode type returns a value
 *                    as the function return (0 or 1).
 *
 * The AcpiExOpcode* functions are called via the Dispatcher component with
 * fully resolved operands.
!*/
/*******************************************************************************
 *
 * FUNCTION:    acpi_ex_opcode_0A_0T_1R
 *
 * PARAMETERS:  walk_state          - Current state (contains AML opcode)
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Execute operator with no operands, one return value
 *
 ******************************************************************************/
acpi_status acpi_ex_opcode_0A_0T_1R(struct acpi_walk_state *walk_state)
{
	acpi_status status = AE_OK;
	union acpi_operand_object *return_desc = NULL;

	ACPI_FUNCTION_TRACE_STR(ex_opcode_0A_0T_1R,
				acpi_ps_get_opcode_name(walk_state->opcode));

	/* Examine the AML opcode */

	switch (walk_state->opcode) {
	case AML_TIMER_OP:	/*  Timer () */

		/* Create a return object of type Integer */

		return_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
		if (!return_desc) {
			status = AE_NO_MEMORY;
			goto cleanup;
		}
		return_desc->integer.value = acpi_os_get_timer();
		break;

	default:		/*  Unknown opcode  */

		ACPI_ERROR((AE_INFO, "Unknown AML opcode %X",
			    walk_state->opcode));
		status = AE_AML_BAD_OPCODE;
		break;
	}

      cleanup:

	/* Delete return object on error */

	if ((ACPI_FAILURE(status)) || walk_state->result_obj) {
		acpi_ut_remove_reference(return_desc);
		walk_state->result_obj = NULL;
	} else {
		/* Save the return value */

		walk_state->result_obj = return_desc;
	}

	return_ACPI_STATUS(status);
}

/*******************************************************************************
 *
 * FUNCTION:    acpi_ex_opcode_1A_0T_0R
 *
 * PARAMETERS:  walk_state          - Current state (contains AML opcode)
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Execute Type 1 monadic operator with numeric operand on
 *              object stack
 *
 ******************************************************************************/

acpi_status acpi_ex_opcode_1A_0T_0R(struct acpi_walk_state *walk_state)
{
	union acpi_operand_object **operand = &walk_state->operands[0];
	acpi_status status = AE_OK;

	ACPI_FUNCTION_TRACE_STR(ex_opcode_1A_0T_0R,
				acpi_ps_get_opcode_name(walk_state->opcode));

	/* Examine the AML opcode */

	switch (walk_state->opcode) {
	case AML_RELEASE_OP:	/*  Release (mutex_object) */

		status = acpi_ex_release_mutex(operand[0], walk_state);
		break;

	case AML_RESET_OP:	/*  Reset (event_object) */

		status = acpi_ex_system_reset_event(operand[0]);
		break;

	case AML_SIGNAL_OP:	/*  Signal (event_object) */

		status = acpi_ex_system_signal_event(operand[0]);
		break;

	case AML_SLEEP_OP:	/*  Sleep (msec_time) */

		status = acpi_ex_system_do_suspend(operand[0]->integer.value);
		break;

	case AML_STALL_OP:	/*  Stall (usec_time) */

		status =
		    acpi_ex_system_do_stall((u32) operand[0]->integer.value);
		break;

	case AML_UNLOAD_OP:	/*  Unload (Handle) */

		status = acpi_ex_unload_table(operand[0]);
		break;

	default:		/*  Unknown opcode  */

		ACPI_ERROR((AE_INFO, "Unknown AML opcode %X",
			    walk_state->opcode));
		status = AE_AML_BAD_OPCODE;
		break;
	}

	return_ACPI_STATUS(status);
}

/*******************************************************************************
 *
 * FUNCTION:    acpi_ex_opcode_1A_1T_0R
 *
 * PARAMETERS:  walk_state          - Current state (contains AML opcode)
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Execute opcode with one argument, one target, and no
 *              return value.
 *
 ******************************************************************************/

acpi_status acpi_ex_opcode_1A_1T_0R(struct acpi_walk_state *walk_state)
{
	acpi_status status = AE_OK;
	union acpi_operand_object **operand = &walk_state->operands[0];

	ACPI_FUNCTION_TRACE_STR(ex_opcode_1A_1T_0R,
				acpi_ps_get_opcode_name(walk_state->opcode));

	/* Examine the AML opcode */

	switch (walk_state->opcode) {
	case AML_LOAD_OP:

		status = acpi_ex_load_op(operand[0], operand[1], walk_state);
		break;

	default:		/* Unknown opcode */

		ACPI_ERROR((AE_INFO, "Unknown AML opcode %X",
			    walk_state->opcode));
		status = AE_AML_BAD_OPCODE;
		goto cleanup;
	}

      cleanup:

	return_ACPI_STATUS(status);
}

/*******************************************************************************
 *
 * FUNCTION:    acpi_ex_opcode_1A_1T_1R
 *
 * PARAMETERS:  walk_state          - Current state (contains AML opcode)
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Execute opcode with one argument, one target, and a
 *              return value.
 *
 ******************************************************************************/

acpi_status acpi_ex_opcode_1A_1T_1R(struct acpi_walk_state *walk_state)
{
	acpi_status status = AE_OK;
	union acpi_operand_object **operand = &walk_state->operands[0];
	union acpi_operand_object *return_desc = NULL;
	union acpi_operand_object *return_desc2 = NULL;
	u32 temp32;
	u32 i;
	acpi_integer power_of_ten;
	acpi_integer digit;

	ACPI_FUNCTION_TRACE_STR(ex_opcode_1A_1T_1R,
				acpi_ps_get_opcode_name(walk_state->opcode));

	/* Examine the AML opcode */

	switch (walk_state->opcode) {
	case AML_BIT_NOT_OP:
	case AML_FIND_SET_LEFT_BIT_OP:
	case AML_FIND_SET_RIGHT_BIT_OP:
	case AML_FROM_BCD_OP:
	case AML_TO_BCD_OP:
	case AML_COND_REF_OF_OP:

		/* Create a return object of type Integer for these opcodes */

		return_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
		if (!return_desc) {
			status = AE_NO_MEMORY;
			goto cleanup;
		}

		switch (walk_state->opcode) {
		case AML_BIT_NOT_OP:	/* Not (Operand, Result)  */

			return_desc->integer.value = ~operand[0]->integer.value;
			break;

		case AML_FIND_SET_LEFT_BIT_OP:	/* find_set_left_bit (Operand, Result) */

			return_desc->integer.value = operand[0]->integer.value;

			/*
			 * Acpi specification describes Integer type as a little
			 * endian unsigned value, so this boundary condition is valid.
			 */
			for (temp32 = 0; return_desc->integer.value &&
			     temp32 < ACPI_INTEGER_BIT_SIZE; ++temp32) {
				return_desc->integer.value >>= 1;
			}

			return_desc->integer.value = temp32;
			break;

		case AML_FIND_SET_RIGHT_BIT_OP:	/* find_set_right_bit (Operand, Result) */

			return_desc->integer.value = operand[0]->integer.value;

			/*
			 * The Acpi specification describes Integer type as a little
			 * endian unsigned value, so this boundary condition is valid.
			 */
			for (temp32 = 0; return_desc->integer.value &&
			     temp32 < ACPI_INTEGER_BIT_SIZE; ++temp32) {
				return_desc->integer.value <<= 1;
			}

			/* Since the bit position is one-based, subtract from 33 (65) */

			return_desc->integer.value =
			    temp32 ==
			    0 ? 0 : (ACPI_INTEGER_BIT_SIZE + 1) - temp32;
			break;

		case AML_FROM_BCD_OP:	/* from_bcd (BCDValue, Result) */

			/*
			 * The 64-bit ACPI integer can hold 16 4-bit BCD characters
			 * (if table is 32-bit, integer can hold 8 BCD characters)
			 * Convert each 4-bit BCD value
			 */
			power_of_ten = 1;
			return_desc->integer.value = 0;
			digit = operand[0]->integer.value;

			/* Convert each BCD digit (each is one nybble wide) */

			for (i = 0;
			     (i < acpi_gbl_integer_nybble_width) && (digit > 0);
			     i++) {

				/* Get the least significant 4-bit BCD digit */

				temp32 = ((u32) digit) & 0xF;

				/* Check the range of the digit */

				if (temp32 > 9) {
					ACPI_ERROR((AE_INFO,
						    "BCD digit too large (not decimal): 0x%X",
						    temp32));

					status = AE_AML_NUMERIC_OVERFLOW;
					goto cleanup;
				}

				/* Sum the digit into the result with the current power of 10 */

				return_desc->integer.value +=
				    (((acpi_integer) temp32) * power_of_ten);

				/* Shift to next BCD digit */

				digit >>= 4;

				/* Next power of 10 */

				power_of_ten *= 10;
			}
			break;

		case AML_TO_BCD_OP:	/* to_bcd (Operand, Result) */

			return_desc->integer.value = 0;
			digit = operand[0]->integer.value;

			/* Each BCD digit is one nybble wide */

			for (i = 0;
			     (i < acpi_gbl_integer_nybble_width) && (digit > 0);
			     i++) {
				(void)acpi_ut_short_divide(digit, 10, &digit,
							   &temp32);

				/*
				 * Insert the BCD digit that resides in the
				 * remainder from above
				 */
				return_desc->integer.value |=
				    (((acpi_integer) temp32) << ACPI_MUL_4(i));
			}

			/* Overflow if there is any data left in Digit */

			if (digit > 0) {
				ACPI_ERROR((AE_INFO,
					    "Integer too large to convert to BCD: %8.8X%8.8X",
					    ACPI_FORMAT_UINT64(operand[0]->
							       integer.value)));
				status = AE_AML_NUMERIC_OVERFLOW;
				goto cleanup;
			}
			break;

		case AML_COND_REF_OF_OP:	/* cond_ref_of (source_object, Result) */

			/*
			 * This op is a little strange because the internal return value is
			 * different than the return value stored in the result descriptor
			 * (There are really two return values)
			 */
			if ((struct acpi_namespace_node *)operand[0] ==
			    acpi_gbl_root_node) {
				/*
				 * This means that the object does not exist in the namespace,
				 * return FALSE
				 */
				return_desc->integer.value = 0;
				goto cleanup;
			}

			/* Get the object reference, store it, and remove our reference */

			status = acpi_ex_get_object_reference(operand[0],
							      &return_desc2,
							      walk_state);
			if (ACPI_FAILURE(status)) {
				goto cleanup;
			}

			status =
			    acpi_ex_store(return_desc2, operand[1], walk_state);
			acpi_ut_remove_reference(return_desc2);

			/* The object exists in the namespace, return TRUE */

			return_desc->integer.value = ACPI_INTEGER_MAX;
			goto cleanup;

		default:
			/* No other opcodes get here */
			break;
		}
		break;

	case AML_STORE_OP:	/* Store (Source, Target) */

		/*
		 * A store operand is typically a number, string, buffer or lvalue
		 * Be careful about deleting the source object,
		 * since the object itself may have been stored.
		 */
		status = acpi_ex_store(operand[0], operand[1], walk_state);
		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}

		/* It is possible that the Store already produced a return object */

		if (!walk_state->result_obj) {
			/*
			 * Normally, we would remove a reference on the Operand[0]
			 * parameter; But since it is being used as the internal return
			 * object (meaning we would normally increment it), the two
			 * cancel out, and we simply don't do anything.
			 */
			walk_state->result_obj = operand[0];
			walk_state->operands[0] = NULL;	/* Prevent deletion */
		}
		return_ACPI_STATUS(status);

		/*
		 * ACPI 2.0 Opcodes
		 */
	case AML_COPY_OP:	/* Copy (Source, Target) */

		status =
		    acpi_ut_copy_iobject_to_iobject(operand[0], &return_desc,
						    walk_state);
		break;

	case AML_TO_DECSTRING_OP:	/* to_decimal_string (Data, Result) */

		status = acpi_ex_convert_to_string(operand[0], &return_desc,
						   ACPI_EXPLICIT_CONVERT_DECIMAL);
		if (return_desc == operand[0]) {

			/* No conversion performed, add ref to handle return value */
			acpi_ut_add_reference(return_desc);
		}
		break;

	case AML_TO_HEXSTRING_OP:	/* to_hex_string (Data, Result) */

		status = acpi_ex_convert_to_string(operand[0], &return_desc,
						   ACPI_EXPLICIT_CONVERT_HEX);
		if (return_desc == operand[0]) {

			/* No conversion performed, add ref to handle return value */
			acpi_ut_add_reference(return_desc);
		}
		break;

	case AML_TO_BUFFER_OP:	/* to_buffer (Data, Result) */

		status = acpi_ex_convert_to_buffer(operand[0], &return_desc);
		if (return_desc == operand[0]) {

			/* No conversion performed, add ref to handle return value */
			acpi_ut_add_reference(return_desc);
		}
		break;

	case AML_TO_INTEGER_OP:	/* to_integer (Data, Result) */

		status = acpi_ex_convert_to_integer(operand[0], &return_desc,
						    ACPI_ANY_BASE);
		if (return_desc == operand[0]) {

			/* No conversion performed, add ref to handle return value */
			acpi_ut_add_reference(return_desc);
		}
		break;

	case AML_SHIFT_LEFT_BIT_OP:	/* shift_left_bit (Source, bit_num) */
	case AML_SHIFT_RIGHT_BIT_OP:	/* shift_right_bit (Source, bit_num) */

		/* These are two obsolete opcodes */

		ACPI_ERROR((AE_INFO,
			    "%s is obsolete and not implemented",
			    acpi_ps_get_opcode_name(walk_state->opcode)));
		status = AE_SUPPORT;
		goto cleanup;

	default:		/* Unknown opcode */

		ACPI_ERROR((AE_INFO, "Unknown AML opcode %X",
			    walk_state->opcode));
		status = AE_AML_BAD_OPCODE;
		goto cleanup;
	}

	if (ACPI_SUCCESS(status)) {

		/* Store the return value computed above into the target object */

		status = acpi_ex_store(return_desc, operand[1], walk_state);
	}

      cleanup:

	/* Delete return object on error */

	if (ACPI_FAILURE(status)) {
		acpi_ut_remove_reference(return_desc);
	}

	/* Save return object on success */

	else if (!walk_state->result_obj) {
		walk_state->result_obj = return_desc;
	}

	return_ACPI_STATUS(status);
}

/*******************************************************************************
 *
 * FUNCTION:    acpi_ex_opcode_1A_0T_1R
 *
 * PARAMETERS:  walk_state          - Current state (contains AML opcode)
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Execute opcode with one argument, no target, and a return value
 *
 ******************************************************************************/

acpi_status acpi_ex_opcode_1A_0T_1R(struct acpi_walk_state *walk_state)
{
	union acpi_operand_object **operand = &walk_state->operands[0];
	union acpi_operand_object *temp_desc;
	union acpi_operand_object *return_desc = NULL;
	acpi_status status = AE_OK;
	u32 type;
	acpi_integer value;

	ACPI_FUNCTION_TRACE_STR(ex_opcode_1A_0T_1R,
				acpi_ps_get_opcode_name(walk_state->opcode));

	/* Examine the AML opcode */

	switch (walk_state->opcode) {
	case AML_LNOT_OP:	/* LNot (Operand) */

		return_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
		if (!return_desc) {
			status = AE_NO_MEMORY;
			goto cleanup;
		}

		/*
		 * Set result to ONES (TRUE) if Value == 0.  Note:
		 * return_desc->Integer.Value is initially == 0 (FALSE) from above.
		 */
		if (!operand[0]->integer.value) {
			return_desc->integer.value = ACPI_INTEGER_MAX;
		}
		break;

	case AML_DECREMENT_OP:	/* Decrement (Operand)  */
	case AML_INCREMENT_OP:	/* Increment (Operand)  */

		/*
		 * Create a new integer.  Can't just get the base integer and
		 * increment it because it may be an Arg or Field.
		 */
		return_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
		if (!return_desc) {
			status = AE_NO_MEMORY;
			goto cleanup;
		}

		/*
		 * Since we are expecting a Reference operand, it can be either a
		 * NS Node or an internal object.
		 */
		temp_desc = operand[0];
		if (ACPI_GET_DESCRIPTOR_TYPE(temp_desc) ==
		    ACPI_DESC_TYPE_OPERAND) {

			/* Internal reference object - prevent deletion */

			acpi_ut_add_reference(temp_desc);
		}

		/*
		 * Convert the Reference operand to an Integer (This removes a
		 * reference on the Operand[0] object)
		 *
		 * NOTE:  We use LNOT_OP here in order to force resolution of the
		 * reference operand to an actual integer.
		 */
		status =
		    acpi_ex_resolve_operands(AML_LNOT_OP, &temp_desc,
					     walk_state);
		if (ACPI_FAILURE(status)) {
			ACPI_EXCEPTION((AE_INFO, status,
					"While resolving operands for [%s]",
					acpi_ps_get_opcode_name(walk_state->
								opcode)));

			goto cleanup;
		}

		/*
		 * temp_desc is now guaranteed to be an Integer object --
		 * Perform the actual increment or decrement
		 */
		if (walk_state->opcode == AML_INCREMENT_OP) {
			return_desc->integer.value =
			    temp_desc->integer.value + 1;
		} else {
			return_desc->integer.value =
			    temp_desc->integer.value - 1;
		}

		/* Finished with this Integer object */

		acpi_ut_remove_reference(temp_desc);

		/*
		 * Store the result back (indirectly) through the original
		 * Reference object
		 */
		status = acpi_ex_store(return_desc, operand[0], walk_state);
		break;

	case AML_TYPE_OP:	/* object_type (source_object) */

		/*
		 * Note: The operand is not resolved at this point because we want to
		 * get the associated object, not its value.  For example, we don't
		 * want to resolve a field_unit to its value, we want the actual
		 * field_unit object.
		 */

		/* Get the type of the base object */

		status =
		    acpi_ex_resolve_multiple(walk_state, operand[0], &type,
					     NULL);
		if (ACPI_FAILURE(status)) {
			goto cleanup;
		}

		/* Allocate a descriptor to hold the type. */

		return_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
		if (!return_desc) {
			status = AE_NO_MEMORY;
			goto cleanup;
		}

		return_desc->integer.value = type;
		break;

	case AML_SIZE_OF_OP:	/* size_of (source_object) */

		/*
		 * Note: The operand is not resolved at this point because we want to
		 * get the associated object, not its value.
		 */

		/* Get the base object */

		status = acpi_ex_resolve_multiple(walk_state,
						  operand[0], &type,
						  &temp_desc);
		if (ACPI_FAILURE(status)) {
			goto cleanup;
		}

		/*
		 * The type of the base object must be integer, buffer, string, or
		 * package.  All others are not supported.
		 *
		 * NOTE: Integer is not specifically supported by the ACPI spec,
		 * but is supported implicitly via implicit operand conversion.
		 * rather than bother with conversion, we just use the byte width
		 * global (4 or 8 bytes).
		 */
		switch (type) {
		case ACPI_TYPE_INTEGER:
			value = acpi_gbl_integer_byte_width;
			break;

		case ACPI_TYPE_STRING:
			value = temp_desc->string.length;
			break;

		case ACPI_TYPE_BUFFER:

			/* Buffer arguments may not be evaluated at this point */

			status = acpi_ds_get_buffer_arguments(temp_desc);
			value = temp_desc->buffer.length;
			break;

		case ACPI_TYPE_PACKAGE:

			/* Package arguments may not be evaluated at this point */

			status = acpi_ds_get_package_arguments(temp_desc);
			value = temp_desc->package.count;
			break;

		default:
			ACPI_ERROR((AE_INFO,
				    "Operand must be Buffer/Integer/String/Package - found type %s",
				    acpi_ut_get_type_name(type)));
			status = AE_AML_OPERAND_TYPE;
			goto cleanup;
		}

		if (ACPI_FAILURE(status)) {
			goto cleanup;
		}

		/*
		 * Now that we have the size of the object, create a result
		 * object to hold the value
		 */
		return_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
		if (!return_desc) {
			status = AE_NO_MEMORY;
			goto cleanup;
		}

		return_desc->integer.value = value;
		break;

	case AML_REF_OF_OP:	/* ref_of (source_object) */

		status =
		    acpi_ex_get_object_reference(operand[0], &return_desc,
						 walk_state);
		if (ACPI_FAILURE(status)) {
			goto cleanup;
		}
		break;

	case AML_DEREF_OF_OP:	/* deref_of (obj_reference | String) */

		/* Check for a method local or argument, or standalone String */

		if (ACPI_GET_DESCRIPTOR_TYPE(operand[0]) ==
		    ACPI_DESC_TYPE_NAMED) {
			temp_desc =
			    acpi_ns_get_attached_object((struct
							 acpi_namespace_node *)
							operand[0]);
			if (temp_desc
			    && ((temp_desc->common.type == ACPI_TYPE_STRING)
				|| (temp_desc->common.type ==
				    ACPI_TYPE_LOCAL_REFERENCE))) {
				operand[0] = temp_desc;
				acpi_ut_add_reference(temp_desc);
			} else {
				status = AE_AML_OPERAND_TYPE;
				goto cleanup;
			}
		} else {
			switch ((operand[0])->common.type) {
			case ACPI_TYPE_LOCAL_REFERENCE:
				/*
				 * This is a deref_of (local_x | arg_x)
				 *
				 * Must resolve/dereference the local/arg reference first
				 */
				switch (operand[0]->reference.class) {
				case ACPI_REFCLASS_LOCAL:
				case ACPI_REFCLASS_ARG:

					/* Set Operand[0] to the value of the local/arg */

					status =
					    acpi_ds_method_data_get_value
					    (operand[0]->reference.class,
					     operand[0]->reference.value,
					     walk_state, &temp_desc);
					if (ACPI_FAILURE(status)) {
						goto cleanup;
					}

					/*
					 * Delete our reference to the input object and
					 * point to the object just retrieved
					 */
					acpi_ut_remove_reference(operand[0]);
					operand[0] = temp_desc;
					break;

				case ACPI_REFCLASS_REFOF:

					/* Get the object to which the reference refers */

					temp_desc =
					    operand[0]->reference.object;
					acpi_ut_remove_reference(operand[0]);
					operand[0] = temp_desc;
					break;

				default:

					/* Must be an Index op - handled below */
					break;
				}
				break;

			case ACPI_TYPE_STRING:
				break;

			default:
				status = AE_AML_OPERAND_TYPE;
				goto cleanup;
			}
		}

		if (ACPI_GET_DESCRIPTOR_TYPE(operand[0]) !=
		    ACPI_DESC_TYPE_NAMED) {
			if ((operand[0])->common.type == ACPI_TYPE_STRING) {
				/*
				 * This is a deref_of (String). The string is a reference
				 * to a named ACPI object.
				 *
				 * 1) Find the owning Node
				 * 2) Dereference the node to an actual object. Could be a
				 *    Field, so we need to resolve the node to a value.
				 */
				status =
				    acpi_ns_get_node(walk_state->scope_info->
						     scope.node,
						     operand[0]->string.pointer,
						     ACPI_NS_SEARCH_PARENT,
						     ACPI_CAST_INDIRECT_PTR
						     (struct
						      acpi_namespace_node,
						      &return_desc));
				if (ACPI_FAILURE(status)) {
					goto cleanup;
				}

				status =
				    acpi_ex_resolve_node_to_value
				    (ACPI_CAST_INDIRECT_PTR
				     (struct acpi_namespace_node, &return_desc),
				     walk_state);
				goto cleanup;
			}
		}

		/* Operand[0] may have changed from the code above */

		if (ACPI_GET_DESCRIPTOR_TYPE(operand[0]) ==
		    ACPI_DESC_TYPE_NAMED) {
			/*
			 * This is a deref_of (object_reference)
			 * Get the actual object from the Node (This is the dereference).
			 * This case may only happen when a local_x or arg_x is
			 * dereferenced above.
			 */
			return_desc = acpi_ns_get_attached_object((struct
								   acpi_namespace_node
								   *)
								  operand[0]);
			acpi_ut_add_reference(return_desc);
		} else {
			/*
			 * This must be a reference object produced by either the
			 * Index() or ref_of() operator
			 */
			switch (operand[0]->reference.class) {
			case ACPI_REFCLASS_INDEX:

				/*
				 * The target type for the Index operator must be
				 * either a Buffer or a Package
				 */
				switch (operand[0]->reference.target_type) {
				case ACPI_TYPE_BUFFER_FIELD:

					temp_desc =
					    operand[0]->reference.object;

					/*
					 * Create a new object that contains one element of the
					 * buffer -- the element pointed to by the index.
					 *
					 * NOTE: index into a buffer is NOT a pointer to a
					 * sub-buffer of the main buffer, it is only a pointer to a
					 * single element (byte) of the buffer!
					 */
					return_desc =
					    acpi_ut_create_internal_object
					    (ACPI_TYPE_INTEGER);
					if (!return_desc) {
						status = AE_NO_MEMORY;
						goto cleanup;
					}

					/*
					 * Since we are returning the value of the buffer at the
					 * indexed location, we don't need to add an additional
					 * reference to the buffer itself.
					 */
					return_desc->integer.value =
					    temp_desc->buffer.
					    pointer[operand[0]->reference.
						    value];
					break;

				case ACPI_TYPE_PACKAGE:

					/*
					 * Return the referenced element of the package.  We must
					 * add another reference to the referenced object, however.
					 */
					return_desc =
					    *(operand[0]->reference.where);
					if (return_desc) {
						acpi_ut_add_reference
						    (return_desc);
					}
					break;

				default:

					ACPI_ERROR((AE_INFO,
						    "Unknown Index TargetType %X in reference object %p",
						    operand[0]->reference.
						    target_type, operand[0]));
					status = AE_AML_OPERAND_TYPE;
					goto cleanup;
				}
				break;

			case ACPI_REFCLASS_REFOF:

				return_desc = operand[0]->reference.object;

				if (ACPI_GET_DESCRIPTOR_TYPE(return_desc) ==
				    ACPI_DESC_TYPE_NAMED) {
					return_desc =
					    acpi_ns_get_attached_object((struct
									 acpi_namespace_node
									 *)
									return_desc);
				}

				/* Add another reference to the object! */

				acpi_ut_add_reference(return_desc);
				break;

			default:
				ACPI_ERROR((AE_INFO,
					    "Unknown class in reference(%p) - %2.2X",
					    operand[0],
					    operand[0]->reference.class));

				status = AE_TYPE;
				goto cleanup;
			}
		}
		break;

	default:

		ACPI_ERROR((AE_INFO, "Unknown AML opcode %X",
			    walk_state->opcode));
		status = AE_AML_BAD_OPCODE;
		goto cleanup;
	}

      cleanup:

	/* Delete return object on error */

	if (ACPI_FAILURE(status)) {
		acpi_ut_remove_reference(return_desc);
	}

	/* Save return object on success */

	else {
		walk_state->result_obj = return_desc;
	}

	return_ACPI_STATUS(status);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                /*
 * Linux driver for digital TV devices equipped with B2C2 FlexcopII(b)/III
 * flexcop.c - main module part
 * Copyright (C) 2004-9 Patrick Boettcher <patrick.boettcher@desy.de>
 * based on skystar2-driver Copyright (C) 2003 Vadim Catana, skystar@moldova.cc
 *
 * Acknowledgements:
 *   John Jurrius from BBTI, Inc. for extensive support
 *                    with code examples and data books
 *   Bjarne Steinsbo, bjarne at steinsbo.com (some ideas for rewriting)
 *
 * Contributions to the skystar2-driver have been done by
 *   Vincenzo Di Massa, hawk.it at tiscalinet.it (several DiSEqC fixes)
 *   Roberto Ragusa, r.ragusa at libero.it (polishing, restyling the code)
 *   Uwe Bugla, uwe.bugla at gmx.de (doing tests, restyling code, writing docu)
 *   Niklas Peinecke, peinecke at gdv.uni-hannover.de (hardware pid/mac
 *               filtering)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "flexcop.h"

#define DRIVER_NAME "B2C2 FlexcopII/II(b)/III digital TV receiver chip"
#define DRIVER_AUTHOR "Patrick Boettcher <patrick.boettcher@desy.de"

#ifdef CONFIG_DVB_B2C2_FLEXCOP_DEBUG
#define DEBSTATUS ""
#else
#define DEBSTATUS " (debugging is not enabled)"
#endif

int b2c2_flexcop_debug;
module_param_named(debug, b2c2_flexcop_debug,  int, 0644);
MODULE_PARM_DESC(debug,
		"set debug level (1=info,2=tuner,4=i2c,8=ts,"
		"16=sram,32=reg (|-able))."
		DEBSTATUS);
#undef DEBSTATUS

DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_nr);

/* global zero for ibi values */
flexcop_ibi_value ibi_zero;

static int flexcop_dvb_start_feed(struct dvb_demux_feed *dvbdmxfeed)
{
	struct flexcop_device *fc = dvbdmxfeed->demux->priv;
	return flexcop_pid_feed_control(fc, dvbdmxfeed, 1);
}

static int flexcop_dvb_stop_feed(struct dvb_demux_feed *dvbdmxfeed)
{
	struct flexcop_device *fc = dvbdmxfeed->demux->priv;
	return flexcop_pid_feed_control(fc, dvbdmxfeed, 0);
}

static int flexcop_dvb_init(struct flexcop_device *fc)
{
	int ret = dvb_register_adapter(&fc->dvb_adapter,
			"FlexCop Digital TV device", fc->owner,
			fc->dev, adapter_nr);
	if (ret < 0) {
		err("error registering DVB adapter");
		return ret;
	}
	fc->dvb_adapter.priv = fc;

	fc->demux.dmx.capabilities = (DMX_TS_FILTERING | DMX_SECTION_FILTERING
			| DMX_MEMORY_BASED_FILTERING);
	fc->demux.priv = fc;
	fc->demux.filternum = fc->demux.feednum = FC_MAX_FEED;
	fc->demux.start_feed = flexcop_dvb_start_feed;
	fc->demux.stop_feed = flexcop_dvb_stop_feed;
	fc->demux.write_to_decoder = NULL;

	if ((ret = dvb_dmx_init(&fc->demux)) < 0) {
		err("dvb_dmx failed: error %d", ret);
		goto err_dmx;
	}

	fc->hw_frontend.source = DMX_FRONTEND_0;

	fc->dmxdev.filternum = fc->demux.feednum;
	fc->dmxdev.demux = &fc->demux.dmx;
	fc->dmxdev.capabilities = 0;
	if ((ret = dvb_dmxdev_init(&fc->dmxdev, &fc->dvb_adapter)) < 0) {
		err("dvb_dmxdev_init failed: error %d", ret);
		goto err_dmx_dev;
	}

	if ((ret = fc->demux.dmx.add_frontend(&fc->demux.dmx, &fc->hw_frontend)) < 0) {
		err("adding hw_frontend to dmx failed: error %d", ret);
		goto err_dmx_add_hw_frontend;
	}

	fc->mem_frontend.source = DMX_MEMORY_FE;
	if ((ret = fc->demux.dmx.add_frontend(&fc->demux.dmx, &fc->mem_frontend)) < 0) {
		err("adding mem_frontend to dmx failed: error %d", ret);
		goto err_dmx_add_mem_frontend;
	}

	if ((ret = fc->demux.dmx.connect_frontend(&fc->demux.dmx, &fc->hw_frontend)) < 0) {
		err("connect frontend failed: error %d", ret);
		goto err_connect_frontend;
	}

	dvb_net_init(&fc->dvb_adapter, &fc->dvbnet, &fc->demux.dmx);

	fc->init_state |= FC_STATE_DVB_INIT;
	return 0;

err_connect_frontend:
	fc->demux.dmx.remove_frontend(&fc->demux.dmx, &fc->mem_frontend);
err_dmx_add_mem_frontend:
	fc->demux.dmx.remove_frontend(&fc->demux.dmx, &fc->hw_frontend);
err_dmx_add_hw_frontend:
	dvb_dmxdev_release(&fc->dmxdev);
err_dmx_dev:
	dvb_dmx_release(&fc->demux);
err_dmx:
	dvb_unregister_adapter(&fc->dvb_adapter);
	return ret;
}

static void flexcop_dvb_exit(struct flexcop_device *fc)
{
	if (fc->init_state & FC_STATE_DVB_INIT) {
		dvb_net_release(&fc->dvbnet);

		fc->demux.dmx.close(&fc->demux.dmx);
		fc->demux.dmx.remove_frontend(&fc->demux.dmx,
			&fc->mem_frontend);
		fc->demux.dmx.remove_frontend(&fc->demux.dmx,
			&fc->hw_frontend);
		dvb_dmxdev_release(&fc->dmxdev);
		dvb_dmx_release(&fc->demux);
		dvb_unregister_adapter(&fc->dvb_adapter);
		deb_info("deinitialized dvb stuff\n");
	}
	fc->init_state &= ~FC_STATE_DVB_INIT;
}

/* these methods are necessary to achieve the long-term-goal of hiding the
 * struct flexcop_device from the bus-parts */
void flexcop_pass_dmx_data(struct flexcop_device *fc, u8 *buf, u32 len)
{
	dvb_dmx_swfilter(&fc->demux, buf, len);
}
EXPORT_SYMBOL(flexcop_pass_dmx_data);

void flexcop_pass_dmx_packets(struct flexcop_device *fc, u8 *buf, u32 no)
{
	dvb_dmx_swfilter_packets(&fc->demux, buf, no);
}
EXPORT_SYMBOL(flexcop_pass_dmx_packets);

static void flexcop_reset(struct flexcop_device *fc)
{
	flexcop_ibi_value v210, v204;

	/* reset the flexcop itself */
	fc->write_ibi_reg(fc,ctrl_208,ibi_zero);

	v210.raw = 0;
	v210.sw_reset_210.reset_block_000 = 1;
	v210.sw_reset_210.reset_block_100 = 1;
	v210.sw_reset_210.reset_block_200 = 1;
	v210.sw_reset_210.reset_block_300 = 1;
	v210.sw_reset_210.reset_block_400 = 1;
	v210.sw_reset_210.reset_block_500 = 1;
	v210.sw_reset_210.reset_block_600 = 1;
	v210.sw_reset_210.reset_block_700 = 1;
	v210.sw_reset_210.Block_reset_enable = 0xb2;
	v210.sw_reset_210.Special_controls = 0xc259;
	fc->write_ibi_reg(fc,sw_reset_210,v210);
	msleep(1);

	/* reset the periphical devices */

	v204 = fc->read_ibi_reg(fc,misc_204);
	v204.misc_204.Per_reset_sig = 0;
	fc->write_ibi_reg(fc,misc_204,v204);
	msleep(1);
	v204.misc_204.Per_reset_sig = 1;
	fc->write_ibi_reg(fc,misc_204,v204);
}

void flexcop_reset_block_300(struct flexcop_device *fc)
{
	flexcop_ibi_value v208_save = fc->read_ibi_reg(fc, ctrl_208),
			  v210 = fc->read_ibi_reg(fc, sw_reset_210);

	deb_rdump("208: %08x, 210: %08x\n", v208_save.raw, v210.raw);
	fc->write_ibi_reg(fc,ctrl_208,ibi_zero);

	v210.sw_reset_210.reset_block_300 = 1;
	v210.sw_reset_210.Block_reset_enable = 0xb2;

	fc->write_ibi_reg(fc,sw_reset_210,v210);
	fc->write_ibi_reg(fc,ctrl_208,v208_save);
}

struct flexcop_device *flexcop_device_kmalloc(size_t bus_specific_len)
{
	void *bus;
	struct flexcop_device *fc = kzalloc(sizeof(struct flexcop_device),
				GFP_KERNEL);
	if (!fc) {
		err("no memory");
		return NULL;
	}

	bus = kzalloc(bus_specific_len, GFP_KERNEL);
	if (!bus) {
		err("no memory");
		kfree(fc);
		return NULL;
	}

	fc->bus_specific = bus;

	return fc;
}
EXPORT_SYMBOL(flexcop_device_kmalloc);

void flexcop_device_kfree(struct flexcop_device *fc)
{
	kfree(fc->bus_specific);
	kfree(fc);
}
EXPORT_SYMBOL(flexcop_device_kfree);

int flexcop_device_initialize(struct flexcop_device *fc)
{
	int ret;
	ibi_zero.raw = 0;

	flexcop_reset(fc);
	flexcop_determine_revision(fc);
	flexcop_sram_init(fc);
	flexcop_hw_filter_init(fc);
	flexcop_smc_ctrl(fc, 0);

	if ((ret = flexcop_dvb_init(fc)))
		goto error;

	/* i2c has to be done before doing EEProm stuff -
	 * because the EEProm is accessed via i2c */
	ret = flexcop_i2c_init(fc);
	if (ret)
		goto error;

	/* do the MAC address reading after initializing the dvb_adapter */
	if (fc->get_mac_addr(fc, 0) == 0) {
		u8 *b = fc->dvb_adapter.proposed_mac;
		info("MAC address = %pM", b);
		flexcop_set_mac_filter(fc,b);
		flexcop_mac_filter_ctrl(fc,1);
	} else
		warn("reading of MAC address failed.\n");

	if ((ret = flexcop_frontend_init(fc)))
		goto error;

	flexcop_device_name(fc,"initialization of","complete");
	return 0;

error:
	flexcop_device_exit(fc);
	return ret;
}
EXPORT_SYMBOL(flexcop_device_initialize);

void flexcop_device_exit(struct flexcop_device *fc)
{
	flexcop_frontend_exit(fc);
	flexcop_i2c_exit(fc);
	flexcop_dvb_exit(fc);
}
EXPORT_SYMBOL(flexcop_device_exit);

static int flexcop_module_init(void)
{
	info(DRIVER_NAME " loaded successfully");
	return 0;
}

static void flexcop_module_cleanup(void)
{
	info(DRIVER_NAME " unloaded successfully");
}

module_init(flexcop_module_init);
module_exit(flexcop_module_cleanup);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_NAME);
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          /* OmniVision OV6620/OV6120 Camera Chip Support Code
 *
 * Copyright (c) 1999-2004 Mark McClelland <mark@alpha.dyndns.org>
 * http://alpha.dyndns.org/ov511/
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. NO WARRANTY OF ANY KIND is expressed or implied.
 */

#define DEBUG

#include <linux/slab.h>
#include "ovcamchip_priv.h"

/* Registers */
#define REG_GAIN		0x00	/* gain [5:0] */
#define REG_BLUE		0x01	/* blue gain */
#define REG_RED			0x02	/* red gain */
#define REG_SAT			0x03	/* saturation */
#define REG_CNT			0x05	/* Y contrast */
#define REG_BRT			0x06	/* Y brightness */
#define REG_WB_BLUE		0x0C	/* WB blue ratio [5:0] */
#define REG_WB_RED		0x0D	/* WB red ratio [5:0] */
#define REG_EXP			0x10	/* exposure */

/* Window parameters */
#define HWSBASE 0x38
#define HWEBASE 0x3A
#define VWSBASE 0x05
#define VWEBASE 0x06

struct ov6x20 {
	int auto_brt;
	int auto_exp;
	int backlight;
	int bandfilt;
	int mirror;
};

/* Initial values for use with OV511/OV511+ cameras */
static struct ovcamchip_regvals regvals_init_6x20_511[] = {
	{ 0x12, 0x80 }, /* reset */
	{ 0x11, 0x01 },
	{ 0x03, 0x60 },
	{ 0x05, 0x7f }, /* For when autoadjust is off */
	{ 0x07, 0xa8 },
	{ 0x0c, 0x24 },
	{ 0x0d, 0x24 },
	{ 0x0f, 0x15 }, /* COMS */
	{ 0x10, 0x75 }, /* AEC Exposure time */
	{ 0x12, 0x24 }, /* Enable AGC and AWB */
	{ 0x14, 0x04 },
	{ 0x16, 0x03 },
	{ 0x26, 0xb2 }, /* BLC enable */
	/* 0x28: 0x05 Selects RGB format if RGB on */
	{ 0x28, 0x05 },
	{ 0x2a, 0x04 }, /* Disable framerate adjust */
	{ 0x2d, 0x99 },
	{ 0x33, 0xa0 }, /* Color Processing Parameter */
	{ 0x34, 0xd2 }, /* Max A/D range */
	{ 0x38, 0x8b },
	{ 0x39, 0x40 },

	{ 0x3c, 0x39 }, /* Enable AEC mode changing */
	{ 0x3c, 0x3c }, /* Change AEC mode */
	{ 0x3c, 0x24 }, /* Disable AEC mode changing */

	{ 0x3d, 0x80 },
	/* These next two registers (0x4a, 0x4b) are undocumented. They
	 * control the color balance */
	{ 0x4a, 0x80 },
	{ 0x4b, 0x80 },
	{ 0x4d, 0xd2 }, /* This reduces noise a bit */
	{ 0x4e, 0xc1 },
	{ 0x4f, 0x04 },
	{ 0xff, 0xff },	/* END MARKER */
};

/* Initial values for use with OV518 cameras */
static struct ovcamchip_regvals regvals_init_6x20_518[] = {
	{ 0x12, 0x80 }, /* Do a reset */
	{ 0x03, 0xc0 }, /* Saturation */
	{ 0x05, 0x8a }, /* Contrast */
	{ 0x0c, 0x24 }, /* AWB blue */
	{ 0x0d, 0x24 }, /* AWB red */
	{ 0x0e, 0x8d }, /* Additional 2x gain */
	{ 0x0f, 0x25 }, /* Black expanding level = 1.3V */
	{ 0x11, 0x01 }, /* Clock div. */
	{ 0x12, 0x24 }, /* Enable AGC and AWB */
	{ 0x13, 0x01 }, /* (default) */
	{ 0x14, 0x80 }, /* Set reserved bit 7 */
	{ 0x15, 0x01 }, /* (default) */
	{ 0x16, 0x03 }, /* (default) */
	{ 0x17, 0x38 }, /* (default) */
	{ 0x18, 0xea }, /* (default) */
	{ 0x19, 0x04 },
	{ 0x1a, 0x93 },
	{ 0x1b, 0x00 }, /* (default) */
	{ 0x1e, 0xc4 }, /* (default) */
	{ 0x1f, 0x04 }, /* (default) */
	{ 0x20, 0x20 }, /* Enable 1st stage aperture correction */
	{ 0x21, 0x10 }, /* Y offset */
	{ 0x22, 0x88 }, /* U offset */
	{ 0x23, 0xc0 }, /* Set XTAL power level */
	{ 0x24, 0x53 }, /* AEC bright ratio */
	{ 0x25, 0x7a }, /* AEC black ratio */
	{ 0x26, 0xb2 }, /* BLC enable */
	{ 0x27, 0xa2 }, /* Full output range */
	{ 0x28, 0x01 }, /* (default) */
	{ 0x29, 0x00 }, /* (default) */
	{ 0x2a, 0x84 }, /* (default) */
	{ 0x2b, 0xa8 }, /* Set custom frame rate */
	{ 0x2c, 0xa0 }, /* (reserved) */
	{ 0x2d, 0x95 }, /* Enable banding filter */
	{ 0x2e, 0x88 }, /* V offset */
	{ 0x33, 0x22 }, /* Luminance gamma on */
	{ 0x34, 0xc7 }, /* A/D bias */
	{ 0x36, 0x12 }, /* (reserved) */
	{ 0x37, 0x63 }, /* (reserved) */
	{ 0x38, 0x8b }, /* Quick AEC/AEB */
	{ 0x39, 0x00 }, /* (default) */
	{ 0x3a, 0x0f }, /* (default) */
	{ 0x3b, 0x3c }, /* (default) */
	{ 0x3c, 0x5c }, /* AEC controls */
	{ 0x3d, 0x80 }, /* Drop 1 (bad) frame when AEC change */
	{ 0x3e, 0x80 }, /* (default) */
	{ 0x3f, 0x02 }, /* (default) */
	{ 0x40, 0x10 }, /* (reserved) */
	{ 0x41, 0x10 }, /* (reserved) */
	{ 0x42, 0x00 }, /* (reserved) */
	{ 0x43, 0x7f }, /* (reserved) */
	{ 0x44, 0x80 }, /* (reserved) */
	{ 0x45, 0x1c }, /* (reserved) */
	{ 0x46, 0x1c }, /* (reserved) */
	{ 0x47, 0x80 }, /* (reserved) */
	{ 0x48, 0x5f }, /* (reserved) */
	{ 0x49, 0x00 }, /* (reserved) */
	{ 0x4a, 0x00 }, /* Color balance (undocumented) */
	{ 0x4b, 0x80 }, /* Color balance (undocumented) */
	{ 0x4c, 0x58 }, /* (reserved) */
	{ 0x4d, 0xd2 }, /* U *= .938, V *= .838 */
	{ 0x4e, 0xa0 }, /* (default) */
	{ 0x4f, 0x04 }, /* UV 3-point average */
	{ 0x50, 0xff }, /* (reserved) */
	{ 0x51, 0x58 }, /* (reserved) */
	{ 0x52, 0xc0 }, /* (reserved) */
	{ 0x53, 0x42 }, /* (reserved) */
	{ 0x27, 0xa6 }, /* Enable manual offset adj. (reg 21 & 22) */
	{ 0x12, 0x20 },
	{ 0x12, 0x24 },

	{ 0xff, 0xff },	/* END MARKER */
};

/* This initializes the OV6x20 camera chip and relevant variables. */
static int ov6x20_init(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov6x20 *s;
	int rc;

	DDEBUG(4, &c->dev, "entered");

	switch (c->adapter->id) {
	case I2C_HW_SMBUS_OV511:
		rc = ov_write_regvals(c, regvals_init_6x20_511);
		break;
	case I2C_HW_SMBUS_OV518:
		rc = ov_write_regvals(c, regvals_init_6x20_518);
		break;
	default:
		dev_err(&c->dev, "ov6x20: Unsupported adapter\n");
		rc = -ENODEV;
	}

	if (rc < 0)
		return rc;

	ov->spriv = s = kzalloc(sizeof *s, GFP_KERNEL);
	if (!s)
		return -ENOMEM;

	s->auto_brt = 1;
	s->auto_exp = 1;

	return rc;
}

static int ov6x20_free(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);

	kfree(ov->spriv);
	return 0;
}

static int ov6x20_set_control(struct i2c_client *c,
			      struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov6x20 *s = ov->spriv;
	int rc;
	int v = ctl->value;

	switch (ctl->id) {
	case OVCAMCHIP_CID_CONT:
		rc = ov_write(c, REG_CNT, v >> 8);
		break;
	case OVCAMCHIP_CID_BRIGHT:
		rc = ov_write(c, REG_BRT, v >> 8);
		break;
	case OVCAMCHIP_CID_SAT:
		rc = ov_write(c, REG_SAT, v >> 8);
		break;
	case OVCAMCHIP_CID_HUE:
		rc = ov_write(c, REG_RED, 0xFF - (v >> 8));
		if (rc < 0)
			goto out;

		rc = ov_write(c, REG_BLUE, v >> 8);
		break;
	case OVCAMCHIP_CID_EXP:
		rc = ov_write(c, REG_EXP, v);
		break;
	case OVCAMCHIP_CID_FREQ:
	{
		int sixty = (v == 60);

		rc = ov_write(c, 0x2b, sixty?0xa8:0x28);
		if (rc < 0)
			goto out;

		rc = ov_write(c, 0x2a, sixty?0x84:0xa4);
		break;
	}
	case OVCAMCHIP_CID_BANDFILT:
		rc = ov_write_mask(c, 0x2d, v?0x04:0x00, 0x04);
		s->bandfilt = v;
		break;
	case OVCAMCHIP_CID_AUTOBRIGHT:
		rc = ov_write_mask(c, 0x2d, v?0x10:0x00, 0x10);
		s->auto_brt = v;
		break;
	case OVCAMCHIP_CID_AUTOEXP:
		rc = ov_write_mask(c, 0x13, v?0x01:0x00, 0x01);
		s->auto_exp = v;
		break;
	case OVCAMCHIP_CID_BACKLIGHT:
	{
		rc = ov_write_mask(c, 0x4e, v?0xe0:0xc0, 0xe0);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x29, v?0x08:0x00, 0x08);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x0e, v?0x80:0x00, 0x80);
		s->backlight = v;
		break;
	}
	case OVCAMCHIP_CID_MIRROR:
		rc = ov_write_mask(c, 0x12, v?0x40:0x00, 0x40);
		s->mirror = v;
		break;
	default:
		DDEBUG(2, &c->dev, "control not supported: %d", ctl->id);
		return -EPERM;
	}

out:
	DDEBUG(3, &c->dev, "id=%d, arg=%d, rc=%d", ctl->id, v, rc);
	return rc;
}

static int ov6x20_get_control(struct i2c_client *c,
			      struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov6x20 *s = ov->spriv;
	int rc = 0;
	unsigned char val = 0;

	switch (ctl->id) {
	case OVCAMCHIP_CID_CONT:
		rc = ov_read(c, REG_CNT, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_BRIGHT:
		rc = ov_read(c, REG_BRT, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_SAT:
		rc = ov_read(c, REG_SAT, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_HUE:
		rc = ov_read(c, REG_BLUE, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_EXP:
		rc = ov_read(c, REG_EXP, &val);
		ctl->value = val;
		break;
	case OVCAMCHIP_CID_BANDFILT:
		ctl->value = s->bandfilt;
		break;
	case OVCAMCHIP_CID_AUTOBRIGHT:
		ctl->value = s->auto_brt;
		break;
	case OVCAMCHIP_CID_AUTOEXP:
		ctl->value = s->auto_exp;
		break;
	case OVCAMCHIP_CID_BACKLIGHT:
		ctl->value = s->backlight;
		break;
	case OVCAMCHIP_CID_MIRROR:
		ctl->value = s->mirror;
		break;
	default:
		DDEBUG(2, &c->dev, "control not supported: %d", ctl->id);
		return -EPERM;
	}

	DDEBUG(3, &c->dev, "id=%d, arg=%d, rc=%d", ctl->id, ctl->value, rc);
	return rc;
}

static int ov6x20_mode_init(struct i2c_client *c, struct ovcamchip_window *win)
{
	/******** QCIF-specific regs ********/

	ov_write(c, 0x14, win->quarter?0x24:0x04);

	/******** Palette-specific regs ********/

	/* OV518 needs 8 bit multiplexed in color mode, and 16 bit in B&W */
	if (c->adapter->id == I2C_HW_SMBUS_OV518) {
		if (win->format == VIDEO_PALETTE_GREY)
			ov_write_mask(c, 0x13, 0x00, 0x20);
		else
			ov_write_mask(c, 0x13, 0x20, 0x20);
	} else {
		if (win->format == VIDEO_PALETTE_GREY)
			ov_write_mask(c, 0x13, 0x20, 0x20);
		else
			ov_write_mask(c, 0x13, 0x00, 0x20);
	}

	/******** Clock programming ********/

	/* The OV6620 needs special handling. This prevents the
	 * severe banding that normally occurs */

	/* Clock down */
	ov_write(c, 0x2a, 0x04);

	ov_write(c, 0x11, win->clockdiv);

	ov_write(c, 0x2a, 0x84);
	/* This next setting is critical. It seems to improve
	 * the gain or the contrast. The "reserved" bits seem
	 * to have some effect in this case. */
	ov_write(c, 0x2d, 0x85); /* FIXME: This messes up banding filter */

	return 0;
}

static int ov6x20_set_window(struct i2c_client *c, struct ovcamchip_window *win)
{
	int ret, hwscale, vwscale;

	ret = ov6x20_mode_init(c, win);
	if (ret < 0)
		return ret;

	if (win->quarter) {
		hwscale = 0;
		vwscale = 0;
	} else {
		hwscale = 1;
		vwscale = 1;	/* The datasheet says 0; it's wrong */
	}

	ov_write(c, 0x17, HWSBASE + (win->x >> hwscale));
	ov_write(c, 0x18, HWEBASE + ((win->x + win->width) >> hwscale));
	ov_write(c, 0x19, VWSBASE + (win->y >> vwscale));
	ov_write(c, 0x1a, VWEBASE + ((win->y + win->height) >> vwscale));

	return 0;
}

static int ov6x20_command(struct i2c_client *c, unsigned int cmd, void *arg)
{
	switch (cmd) {
	case OVCAMCHIP_CMD_S_CTRL:
		return ov6x20_set_control(c, arg);
	case OVCAMCHIP_CMD_G_CTRL:
		return ov6x20_get_control(c, arg);
	case OVCAMCHIP_CMD_S_MODE:
		return ov6x20_set_window(c, arg);
	default:
		DDEBUG(2, &c->dev, "command not supported: %d", cmd);
		return -ENOIOCTLCMD;
	}
}

struct ovcamchip_ops ov6x20_ops = {
	.init    =	ov6x20_init,
	.free    =	ov6x20_free,
	.command =	ov6x20_command,
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 ******************/
u16_t zfApAddIeWmePara(zdev_t* dev, zbuf_t* buf, u16_t offset, u16_t vap)
{
    zmw_get_wlan_dev(dev);

    /* Element ID */
    zmw_tx_buf_writeb(dev, buf, offset++, ZM_WLAN_EID_WIFI_IE);

    /* Element Length */
    zmw_tx_buf_writeb(dev, buf, offset++, 24);

    /* OUI */
    zmw_tx_buf_writeb(dev, buf, offset++, 0x00);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x50);
    zmw_tx_buf_writeb(dev, buf, offset++, 0xF2);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x02);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x01);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x01);

    /* QoS Info */
    if (wd->ap.uapsdEnabled)
    {
        zmw_tx_buf_writeb(dev, buf, offset++, 0x81);
    }
    else
    {
    zmw_tx_buf_writeb(dev, buf, offset++, 0x01);
    }

    /* Reserved */
    zmw_tx_buf_writeb(dev, buf, offset++, 0x00);

    /* Best Effort AC parameters */
    zmw_tx_buf_writeb(dev, buf, offset++, 0x03);
    zmw_tx_buf_writeb(dev, buf, offset++, 0xA4);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x00);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x00);
    /* Backfround AC parameters */
    zmw_tx_buf_writeb(dev, buf, offset++, 0x27);
    zmw_tx_buf_writeb(dev, buf, offset++, 0xA4);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x00);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x00);
    /* Video AC parameters */
    zmw_tx_buf_writeb(dev, buf, offset++, 0x42);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x43);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x5E);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x00);
    /* Voice AC parameters */
    zmw_tx_buf_writeb(dev, buf, offset++, 0x62);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x32);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x2F);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x00);

    return offset;
}


/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfApSendBeacon              */
/*      Sned AP mode beacon.                                            */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      none                                                            */
/*                                                                      */
/*    AUTHOR                                                            */
/*      Stephen Chen        ZyDAS Technology Corporation    2005.11     */
/*                                                                      */
/************************************************************************/
void zfApSendBeacon(zdev_t* dev)
{
    zbuf_t* buf;
    u16_t offset;
    u16_t vap;
    u16_t seq;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    wd->ap.beaconCounter++;
    if (wd->ap.beaconCounter >= wd->ap.vapNumber)
    {
        wd->ap.beaconCounter = 0;
    }
    vap = wd->ap.beaconCounter;


    zm_msg1_mm(ZM_LV_2, "Send beacon, vap=", vap);

    /* TBD : Maximum size of beacon */
    if ((buf = zfwBufAllocate(dev, 1024)) == NULL)
    {
        zm_msg0_mm(ZM_LV_0, "Alloc beacon buf Fail!");
        return;
    }

    offset = 0;

    /* wlan header */
    /* Frame control */
    zmw_tx_buf_writeh(dev, buf, offset, 0x0080);
    offset+=2;
    /* Duration */
    zmw_tx_buf_writeh(dev, buf, offset, 0x0000);
    offset+=2;
    /* Address 1 */
    zmw_tx_buf_writeh(dev, buf, offset, 0xffff);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, 0xffff);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, 0xffff);
    offset+=2;
    /* Address 2 */
    zmw_tx_buf_writeh(dev, buf, offset, wd->macAddr[0]);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, wd->macAddr[1]);
    offset+=2;
#ifdef ZM_VAPMODE_MULTILE_SSID
    zmw_tx_buf_writeh(dev, buf, offset, wd->macAddr[2]); //Multiple SSID
#else
    zmw_tx_buf_writeh(dev, buf, offset, (wd->macAddr[2]+(vap<<8))); //VAP
#endif
    offset+=2;
    /* Address 3 */
    zmw_tx_buf_writeh(dev, buf, offset, wd->macAddr[0]);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, wd->macAddr[1]);
    offset+=2;
#ifdef ZM_VAPMODE_MULTILE_SSID
    zmw_tx_buf_writeh(dev, buf, offset, wd->macAddr[2]); //Multiple SSID
#else
    zmw_tx_buf_writeh(dev, buf, offset, (wd->macAddr[2]+(vap<<8))); //VAP
#endif
    offset+=2;

    /* Sequence number */
    zmw_enter_critical_section(dev);
    seq = ((wd->mmseq++)<<4);
    zmw_leave_critical_section(dev);
    zmw_tx_buf_writeh(dev, buf, offset, seq);
    offset+=2;

    /* 24-31 Time Stamp : hardware will fill this field */
    zmw_tx_buf_writeh(dev, buf, offset, 0);
    zmw_tx_buf_writeh(dev, buf, offset+2, 0);
    zmw_tx_buf_writeh(dev, buf, offset+4, 0);
    zmw_tx_buf_writeh(dev, buf, offset+6, 0);
    offset+=8;

    /* Beacon Interval */
    zmw_tx_buf_writeh(dev, buf, offset, wd->beaconInterval);
    offset+=2;

    /* Capability */
    zmw_tx_buf_writeh(dev, buf, offset, wd->ap.capab[vap]);
    offset+=2;

    /* SSID */
    if (wd->ap.hideSsid[vap] == 0)
    {
        offset = zfApAddIeSsid(dev, buf, offset, vap);
    }
    else
    {
        zmw_tx_buf_writeb(dev, buf, offset++, ZM_WLAN_EID_SSID);
        zmw_tx_buf_writeb(dev, buf, offset++, 0);

    }

    /* Support Rate */
    if ( wd->frequency < 3000 )
    {
    offset = zfMmAddIeSupportRate(dev, buf, offset,
                                  ZM_WLAN_EID_SUPPORT_RATE, ZM_RATE_SET_CCK);
    }
    else
    {
        offset = zfMmAddIeSupportRate(dev, buf, offset,
                                  ZM_WLAN_EID_SUPPORT_RATE, ZM_RATE_SET_OFDM);
    }

    /* DS parameter set */
    offset = zfMmAddIeDs(dev, buf, offset);

    /* TIM */
    offset = zfApAddIeTim(dev, buf, offset, vap);

    /* If WLAN Type is not PURE B */
    if (wd->ap.wlanType[vap] != ZM_WLAN_TYPE_PURE_B)
    {
        if ( wd->frequency < 3000 )
        {
        /* ERP Information */
        offset = zfMmAddIeErp(dev, buf, offset);

        /* Extended Supported Rates */
        offset = zfMmAddIeSupportRate(dev, buf, offset,
                                      ZM_WLAN_EID_EXTENDED_RATE, ZM_RATE_SET_OFDM);
    }
    }

    /* TODO : country information */
    /* TODO : RSN */
    if (wd->ap.wpaSupport[vap] == 1)
    {
        offset = zfMmAddIeWpa(dev, buf, offset, vap);
    }

    /* WME Parameters */
    if (wd->ap.qosMode == 1)
    {
        offset = zfApAddIeWmePara(dev, buf, offset, vap);
    }

    /* HT Capabilities Info */
    offset = zfMmAddHTCapability(dev, buf, offset);

    /* Extended HT Capabilities Info */
    offset = zfMmAddExtendedHTCapability(dev, buf, offset);

    /* 1212 : write to beacon fifo */
    /* 1221 : write to share memory */
    zfHpSendBeacon(dev, buf, offset);

    /* Free beacon buffer */
    /* TODO: In order to fit the madwifi beacon architecture, we need to
       free beacon buffer in the HAL layer.
     */

    //zfwBufFree(dev, buf, 0);
}


/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfIntrabssForward           */
/*      Called to transmit intra-BSS frame from upper layer.            */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*      buf : buffer pointer                                            */
/*      vap : virtual AP                                                */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      1 : unicast intras-BSS frame                                    */
/*      0 : other frames                                                */
/*                                                                      */
/*    AUTHOR                                                            */
/*      Stephen             ZyDAS Technology Corporation    2005.11     */
/*                                                                      */
/************************************************************************/
u16_t zfIntrabssForward(zdev_t* dev, zbuf_t* buf, u8_t srcVap)
{
    u16_t err;
    u16_t asocFlag = 0;
    u16_t dst[3];
    u16_t aid;
    u16_t staState;
    zbuf_t* txBuf;
    u16_t len;
    u16_t i;
    u16_t temp;
    u16_t ret;
    u8_t vap = 0;
#ifdef ZM_ENABLE_NATIVE_WIFI
    dst[0] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A3_OFFSET);
    dst[1] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A3_OFFSET+2);
    dst[2] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A3_OFFSET+4);
#else
    dst[0] = zmw_rx_buf_readh(dev, buf, 0);
    dst[1] = zmw_rx_buf_readh(dev, buf, 2);
    dst[2] = zmw_rx_buf_readh(dev, buf, 4);
#endif  // ZM_ENABLE_NATIVE_WIFI

    /* Do Intra-BSS forward(data copy) if necessary*/
    if ((dst[0]&0x1) != 0x1)
    {
        aid = zfApGetSTAInfo(dev, dst, &staState, &vap);
        if ((aid != 0xffff) && (staState == ZM_STATE_ASOC) && (srcVap == vap))
        {
            asocFlag = 1;
            zm_msg0_rx(ZM_LV_2, "Intra-BSS forward : asoc STA");
        }

    }
    else
    {
        vap = srcVap;
        zm_msg0_rx(ZM_LV_2, "Intra-BSS forward : BCorMC");
    }

    /* destination address = associated STA or BC/MC */
    if ((asocFlag == 1) || ((dst[0]&0x1) == 0x1))
    {
        /* Allocate frame */
        if ((txBuf = zfwBufAllocate(dev, ZM_RX_FRAME_SIZE))
                == NULL)
        {
            zm_msg0_rx(ZM_LV_1, "Alloc intra-bss buf Fail!");
            goto zlAllocError;
        }

        /* Copy frame */
        len = zfwBufGetSize(dev, buf);
        for (i=0; i<len; i+=2)
        {
            temp = zmw_rx_buf_readh(dev, buf, i);
            zmw_tx_buf_writeh(dev, txBuf, i, temp);
        }
        zfwBufSetSize(dev, txBuf, len);

#ifdef ZM_ENABLE_NATIVE_WIFI
        /* Tx-A2 = Rx-A1, Tx-A3 = Rx-A2, Tx-A1 = Rx-A3 */
        for (i=0; i<6; i+=2)
        {
            temp = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET+i);
            zmw_tx_buf_writeh(dev, txBuf, ZM_WLAN_HEADER_A2_OFFSET+i, temp);
            temp = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A2_OFFSET+i);
            zmw_tx_buf_writeh(dev, txBuf, ZM_WLAN_HEADER_A3_OFFSET+i, temp);
            temp = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A3_OFFSET+i);
            zmw_tx_buf_writeh(dev, txBuf, ZM_WLAN_HEADER_A1_OFFSET+i, temp);
        }

        #endif

        /* Transmit frame */
        /* Return error if port is disabled */
        if ((err = zfTxPortControl(dev, txBuf, vap)) == ZM_PORT_DISABLED)
        {
            err = ZM_ERR_TX_PORT_DISABLED;
            goto zlTxError;
        }

#if 1
        /* AP : Buffer frame for power saving STA */
        if ((ret = zfApBufferPsFrame(dev, txBuf, vap)) == 0)
        {
            /* forward frame if not been buffered */
            #if 1
            /* Put to VTXQ[ac] */
            ret = zfPutVtxq(dev, txBuf);
            /* Push VTXQ[ac] */
            zfPushVtxq(dev);
            #else
            zfTxSendEth(dev, txBuf, vap, ZM_INTERNAL_ALLOC_BUF, 0);
            #endif

        }
#endif
    }
    return asocFlag;

zlTxError:
    zfwBufFree(dev, txBuf, 0);
zlAllocError:
    return asocFlag;
}

struct zsMicVar* zfApGetRxMicKey(zdev_t* dev, zbuf_t* buf)
{
    u8_t sa[6];
    u16_t id = 0, macAddr[3];

    zmw_get_wlan_dev(dev);

    zfCopyFromRxBuffer(dev, buf, sa, ZM_WLAN_HEADER_A2_OFFSET, 6);

    macAddr[0] = sa[0] + (sa[1] << 8);
    macAddr[1] = sa[2] + (sa[3] << 8);
    macAddr[2] = sa[4] + (sa[5] << 8);

    if ((id = zfApFindSta(dev, macAddr)) != 0xffff)
        return (&wd->ap.staTable[id].rxMicKey);

    return NULL;
}

struct zsMicVar* zfApGetTxMicKey(zdev_t* dev, zbuf_t* buf, u8_t* qosType)
{
    u8_t da[6];
    u16_t id = 0, macAddr[3];

    zmw_get_wlan_dev(dev);

    zfCopyFromIntTxBuffer(dev, buf, da, 0, 6);

    macAddr[0] = da[0] + (da[1] << 8);
    macAddr[1] = da[2] + (da[3] << 8);
    macAddr[2] = da[4] + (da[5] << 8);

    if ((macAddr[0] & 0x1))
    {
        return (&wd->ap.bcMicKey[0]);
    }
    else if ((id = zfApFindSta(dev, macAddr)) != 0xffff)
    {
        *qosType = wd->ap.staTable[id].qosType;
        return (&wd->ap.staTable[id].txMicKey);
    }

    return NULL;
}

u16_t zfApUpdatePsBit(zdev_t* dev, zbuf_t* buf, u8_t* vap, u8_t* uapsdTrig)
{
    u16_t staState;
    u16_t aid;
    u16_t psBit;
    u16_t src[3];
    u16_t dst[1];
    u16_t i;

    zmw_get_wlan_dev(dev);

    src[0] = zmw_rx_buf_readh(dev, buf, 10);
    src[1] = zmw_rx_buf_readh(dev, buf, 12);
    src[2] = zmw_rx_buf_readh(dev, buf, 14);

    if ((zmw_rx_buf_readb(dev, buf, 1) & 0x3) != 3)
    {
        /* AP */
        dst[0] = zmw_rx_buf_readh(dev, buf, 4);

        psBit = (zmw_rx_buf_readb(dev, buf, 1) & 0x10) >> 4;
        /* Get AID and update STA PS mode */
        aid = zfApGetSTAInfoAndUpdatePs(dev, src, &staState, vap, psBit, uapsdTrig);

        /* if STA not associated, send deauth */
        if ((aid == 0xffff) || (staState != ZM_STATE_ASOC))
        {
            if ((dst[0]&0x1)==0)
            {
                zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_DEAUTH, src, 0x7,
                        0, 0);
            }

            return ZM_ERR_STA_NOT_ASSOCIATED;
        }
    } /* if ((zmw_rx_buf_readb(dev, buf, 1) & 0x3) != 3) */
    else
    {
        /* WDS */
        for (i=0; i<ZM_MAX_WDS_SUPPORT; i++)
        {
            if ((wd->ap.wds.wdsBitmap & (1<<i)) != 0)
            {
                if ((src[0] == wd->ap.wds.macAddr[i][0])
                        && (src[1] == wd->ap.wds.macAddr[i][1])
                        && (src[2] == wd->ap.wds.macAddr[i][2]))
                {
                    *vap = 0x20 + i;
                    break;
                }
            }
        }
    }
    return ZM_SUCCESS;
}

void zfApProcessPsPoll(zdev_t* dev, zbuf_t* buf)
{
    u16_t src[3];
    u16_t dst[3];
    zbuf_t* psBuf = NULL;
    u16_t id;
    u8_t moreData = 0;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    src[0] = zmw_tx_buf_readh(dev, buf, 10);
    src[1] = zmw_tx_buf_readh(dev, buf, 12);
    src[2] = zmw_tx_buf_readh(dev, buf, 14);

    /* Find ps buffer for PsPoll */
    zmw_enter_critical_section(dev);
    id = wd->ap.uniHead;
    while (id != wd->ap.uniTail)
    {
        psBuf = wd->ap.uniArray[id];

        dst[0] = zmw_tx_buf_readh(dev, psBuf, 0);
        dst[1] = zmw_tx_buf_readh(dev, psBuf, 2);
        dst[2] = zmw_tx_buf_readh(dev, psBuf, 4);

        if ((src[0] == dst[0]) && (src[1] == dst[1]) && (src[2] == dst[2]))
        {
            moreData = zfApRemoveFromPsQueue(dev, id, src);
            break;
        }
        else
        {
            psBuf = NULL;
        }
        id = (id + 1) & (ZM_UNI_ARRAY_SIZE - 1);
    }
    zmw_leave_critical_section(dev);

    /* Send ps buffer */
    if (psBuf != NULL)
    {
        /* Send with more data bit */
        zfTxSendEth(dev, psBuf, 0, ZM_EXTERNAL_ALLOC_BUF, moreData);
    }

    return;
}

void zfApSetProtectionMode(zdev_t* dev, u16_t mode)
{
    zmw_get_wlan_dev(dev);

    if (mode == 0)
    {
        if (wd->ap.protectionMode != mode)
        {
            /* Write MAC&PHY registers to disable protection */

            wd->ap.protectionMode = mode;
        }

    }
    else
    {
        if (wd->ap.protectionMode != mode)
        {
            /* Write MAC&PHY registers to enable protection */

            wd->ap.protectionMode = mode;
        }
    }
    return;
}


/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfApSendFailure             */
/*      Send failure.                                                   */
/*                                                                      */
/*    INPUTS                                                1 = v & mask;
					*dst32++ = pval[0] | (v1 >> 8);
					pval[0] = (v ^ v1) << 8;
					v = *src32++;
					v1 = v & mask;
					*dst32++ = pval[1] | (v1 >> 8);
					pval[1] = (v ^ v1) << 8;
					v = *src32++;
					v1 = v & mask;
					*dst32++ = pval[2] | (v1 >> 8);
					pval[2] = (v ^ v1) << 8;
					v = *src32++;
					v1 = v & mask;
					*dst32++ = pval[3] | (v1 >> 8);
					pval[3] = (v ^ v1) << 8;
				}

				if (f & 2) {
					dst32[0] = (dst32[0] & mask) | pval[0];
					dst32[1] = (dst32[1] & mask) | pval[1];
					dst32[2] = (dst32[2] & mask) | pval[2];
					dst32[3] = (dst32[3] & mask) | pval[3];
				}

				src += next_line;
				dst += next_line;
			}
		} else {
			u32 *src32, *dst32;
			u32 pval[4], v, v1, mask;
			int i, j, w, f;

			src = (u8 *)info->screen_base + (sy - 1) * next_line + ((sx + width + 8) & ~15) / (8 / BPL);
			dst = (u8 *)info->screen_base + (dy - 1) * next_line + ((dx + width + 8) & ~15) / (8 / BPL);

			mask = 0xff00ff;
			f = 0;
			w = width;
			if ((dx + width) & 15)
				f = 1;
			if (sx & 15) {
				f |= 2;
				w += 8;
			}
			w >>= 4;
			for (i = height; i; i--) {
				src32 = (u32 *)src;
				dst32 = (u32 *)dst;

				if (f & 1) {
					pval[0] = dst32[-1] & mask;
					pval[1] = dst32[-2] & mask;
					pval[2] = dst32[-3] & mask;
					pval[3] = dst32[-4] & mask;
				} else {
					pval[0] = (*--src32 >> 8) & mask;
					pval[1] = (*--src32 >> 8) & mask;
					pval[2] = (*--src32 >> 8) & mask;
					pval[3] = (*--src32 >> 8) & mask;
				}

				for (j = w; j > 0; j--) {
					v = *--src32;
					v1 = v & mask;
					*--dst32 = pval[0] | (v1 << 8);
					pval[0] = (v ^ v1) >> 8;
					v = *--src32;
					v1 = v & mask;
					*--dst32 = pval[1] | (v1 << 8);
					pval[1] = (v ^ v1) >> 8;
					v = *--src32;
					v1 = v & mask;
					*--dst32 