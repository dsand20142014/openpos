atus = acpi_ns_initialize_objects();
		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}
	}

	/*
	 * Initialize all device objects in the namespace. This runs the device
	 * _STA and _INI methods.
	 */
	if (!(flags & ACPI_NO_DEVICE_INIT)) {
		ACPI_DEBUG_PRINT((ACPI_DB_EXEC,
				  "[Init] Initializing ACPI Devices\n"));

		status = acpi_ns_initialize_devices();
		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}
	}

	/*
	 * Complete the GPE initialization for the GPE blocks defined in the FADT
	 * (GPE block 0 and 1).
	 *
	 * Note1: This is where the _PRW methods are executed for the GPEs. These
	 * methods can only be executed after the SCI and Global Lock handlers are
	 * installed and initialized.
	 *
	 * Note2: Currently, there seems to be no need to run the _REG methods
	 * before execution of the _PRW methods and enabling of the GPEs.
	 */
	if (!(flags & ACPI_NO_EVENT_INIT)) {
		status = acpi_ev_install_fadt_gpes();
		if (ACPI_FAILURE(status))
			return (status);
	}

	/*
	 * Empty the caches (delete the cached objects) on the assumption that
	 * the table load filled them up more than they will be at runtime --
	 * thus wasting non-paged memory.
	 */
	status = acpi_purge_cached_objects();

	acpi_gbl_startup_flags |= ACPI_INITIALIZED_OK;
	return_ACPI_STATUS(status);
}

ACPI_EXPORT_SYMBOL(acpi_initialize_objects)

#endif
/*******************************************************************************
 *
 * FUNCTION:    acpi_terminate
 *
 * PARAMETERS:  None
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Shutdown the ACPI subsystem.  Release all resources.
 *
 ******************************************************************************/
acpi_status acpi_terminate(void)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(acpi_terminate);

	/* Terminate the AML Debugger if present */

	ACPI_DEBUGGER_EXEC(acpi_gbl_db_terminate_threads = TRUE);

	/