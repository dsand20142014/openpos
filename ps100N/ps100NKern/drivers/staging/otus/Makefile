ods
 *          _Qxx EC methods
 *          _T_x compiler temporary variables
 *
 *      2) Predefined names that never actually exist within the AML code:
 *          Predefined resource descriptor field names
 *
 *      3) Predefined names that are implemented within ACPICA:
 *          _OSI
 *
 *      4) Some predefined names that are not documented within the ACPI spec.
 *          _WDG, _WED
 *
 * The main entries in the table each contain the following items:
 *
 * Name                 - The ACPI reserved name
 * param_count          - Number of arguments to the method
 * expected_btypes      - Allowed type(s) for the return value.
 *                        0 means that no return value is expected.
 *
 * For methods that return packages, the next entry in the table contains
 * information about the expected structure of the package. This information
 * is saved here (rather than in a separate table) in order to minimize the
 * overall size of the stored data.
 */
static const union acpi_predefined_info predefined_names[] = {
	{.info = {"_AC0", 0, ACPI_RTYPE_INTEGER}},
	{.info = {"_AC1", 0, ACPI_RTYPE_INTEGER}},
	{.info = {"_AC2", 0, ACPI_RTYPE_INTEGER}},
	{.info = {"_AC3", 0, ACPI_RTYPE_INTEGER}},
	{.info = {"_AC4", 0, ACPI_RTYPE_INTEGER}},
	{.info = {"_AC5", 0, ACPI_RTYPE_INTEGER}},
	{.info = {"_AC6", 0, ACPI_RTYPE_INTEGER}},
	{.info = {"_AC7", 0, ACPI_RTYPE_INTEGER}},
	{.info = {"_AC8", 0, ACPI_RTYPE_INTE