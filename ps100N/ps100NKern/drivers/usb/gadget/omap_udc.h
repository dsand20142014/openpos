 optional "implicit return".  We save the result
 *              of every ASL operator and control method invocation in case the
 *              parent method exit.  Before storing a new return value, we
 *              delete the previous return value.
 *
 ******************************************************************************/

u8
acpi_ds_do_implicit_return(union acpi_operand_object *return_desc,
			   struct acpi_walk_state *walk_state, u8 add_reference)
{
	ACPI_FUNCTION_NAME(ds_do_implicit_return);

	/*
	 * Slack must be enabled for this feature, and we must
	 * have a valid return object
	 */
	if ((!acpi_gbl_enable_interpreter_slack) || (!return_desc)) {
		return (FALSE);
	}

	ACPI_DEBUG_PRINT((ACPI_DB_DISPATCH,
			  "Result %p will be implicitly returned; Prev=%p\n",
			  return_desc, walk_state->implicit_return_obj));

	/*
	 * Delete any "stale" implicit return value first. However, in
	 * complex statements, the implicit return value can be
	 * bubbled up several levels, so we don't clear the value if it
	 * is the same as the return_desc.
	 */
	if (walk_state->implicit_return_obj) {
		if (walk_state->implicit_return_obj == return_desc) {
			return (TRUE);
		}
		acpi_ds_clear_implicit_return(walk_state);
	}

	/* Save the implicit return value, add a reference if requested */

	walk_state->implicit_return_obj = return_desc;
	if (add_reference) {
		acpi_ut_add_reference(return_desc);
	}

	return (TRUE);
}

/*******************************************************************************
 *
 * FUNCTION:    acpi_ds_is_result_used
 *
 * PARAMETERS:  Op                  - Current Op
 *              walk_state          - Current State
 *
 * RETURN:      TRUE if result is used, FALSE otherwise
 *
 * DESCRIPTION: Check if a result object will be used by the parent
 *
 ******************************************************************************/

u8
acpi_ds_is_result_used(union acpi_parse_object * op,
		       struct acpi_walk_state * walk_state)
{
	const struct acpi_opcode_info *parent_info;

	ACPI_FUNCTION_TRACE_PTR(ds_is_result_used, op);

	/* Must have both an Op and a Result Object */

	if (!op) {
		ACPI_ERROR((AE_INFO, "Null Op"));
		return_UINT8(TRUE);
	}

	/*
	 * We know that this operator is not a
	 * Return() operator (would not come here.) The following code is the
	 * optional support for a so-called "implicit return". Some AML code
	 * assumes that the last value of the method is "implicitly" returned
	 * to the caller. Just save the last result as the return value.
	 * NOTE: this is optional because the ASL language does not actually
	 * support this behavior.
	 */
	(void)acpi_ds_do_implicit_return(walk_state->result_obj, walk_state,
					 TRUE);

	/*
	 * Now determine if the parent will use the result
	 *
	 * If there is no parent, or the parent is a scope_op, we are executing
	 * at the method level. An executing method typically has no parent,
	 * since each method is parsed separately.  A method invoked externally
	 * via execute_control_method has a scope_op as the parent.
	 */
	if ((!op->common.parent) ||
	    (op->common.parent->common.aml_opcode == AML_SCOPE_OP)) {

		/* No parent, the return value cannot possibly be used */

		ACPI_DEBUG_PRINT((ACPI_DB_DISPATCH,
				  "At Method level, result of [%s] not used\n",
				  acpi_ps_get_opcode_name(op->common.
							  aml_opcode)));
		return_UINT8(FALSE);
	}

	/* Get info on the parent. The root_op is AML_SCOPE */

	parent_info =
	    acpi_ps_get_opcode_info(op->common.parent->common.aml_opcode);
	if (parent_info->class == AML_CLASS_UNKNOWN) {
		ACPI_ERROR((AE_INFO, "Unknown parent opcode Op=%p", op));
		return_UINT8(FALSE);
	}

	/*
	 * Decide what to do with the result based on the parent.  If
	 * the parent opcode will not use the result, delete the object.
	 * Otherwise leave it as is, it will be deleted when it is used
	 * as an operand later.
	 */
	switch (parent_info->class) {
	case AML_CLASS_CONTROL:

		switch (op->common.parent->common.aml_opcode) {
		case AML_RETURN_OP:

			/* Never delete the return value associated with a return opcode */

			goto result_used;

		case AML_IF_OP:
		case AML_WHILE_OP:

			/*
			 * If we are executing the predicate AND this is the predicate op,
			 * we will use the return value
			 */
			if ((walk_state->control_state->common.state ==
			     ACPI_CONTROL_PREDICATE_EXECUTING)
			    && (walk_state->control_state->control.
				predicate_op == op)) {
				goto result_used;
			}
			break;

		default:
			/* Ignore other control opcodes */
			break;
		}

		/* The general control opcode returns no result */

		goto result_not_used;

	case AML_CLASS_CREATE:

		/*
		 * These opcodes allow term_arg(s) as operands and therefore
		 * the operands can be method calls.  The result is used.
		 */
		goto result_used;

	case AML_CLASS_NAMED_OBJECT:

		if ((op->common.parent->common.aml_opcode == AML_REGION_OP) ||
		    (op->common.parent->common.aml_opcode == AML_DATA_REGION_OP)
		    || (op->common.parent->common.aml_opcode == AML_PACKAGE_OP)
		    || (op->common.parent->common.aml_opcode ==
			AML_VAR_PACKAGE_OP)
		    || (op->common.parent->common.aml_opcode == AML_BUFFER_OP)
		    || (op->common.parent->common.aml_opcode ==
			AML_INT_EVAL_SUBTREE_OP)
		    || (op->common.parent->common.aml_opcode ==
			AML_BANK_FIELD_OP)) {
			/*
			 * These opcodes allow term_arg(s) as operands and therefore
			 * the operands can be method calls.  The result is used.
			 */
			goto result_used;
		}

		goto result_not_used;

	default:

		/*
		 * In all other cases. the parent will actually use the return
		 * object, so keep it.
		 */
		goto result_used;
	}

      result_used:
	ACPI_DEBUG_PRINT((ACPI_DB_DISPATCH,
			  "Result of [%s] used by Parent [%s] Op=%p\n",
			  acpi_ps_get_opcode_name(op->common.aml_opcode),
			  acpi_ps_get_opcode_name(op->common.parent->common.
						  aml_opcode), op));

	return_UINT8(TRUE);

      result_not_used:
	ACPI_DEBUG_PRINT((ACPI_DB_DISPATCH,
			  "Result of [%s] not used by Parent [%s] Op=%p\n",
			  acpi_ps_get_opcode_name(op->common.aml_opcode),
			  acpi_ps_get_opcode_name(op->common.parent->common.
						  aml_opcode), op));

	return_UINT8(FALSE);
}

/*******************************************************************************
 *
 * FUNCTION:    acpi_ds_delete_result_if_not_used
 *
 * PARAMETERS:  Op              - Current parse Op
 *              result_obj      - Result of the operation
 *              walk_state      - Current state
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Used after interpretation of an opcode.  If there is an internal
 *              result descriptor, check if the pa