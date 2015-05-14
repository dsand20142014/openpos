     /* R212 */
	0x0000,     /* R213 */
	0x0000,     /* R214 */
	0x0000,     /* R215 - VCC_FAULT Masks */
	0x001F,     /* R216 - Main Bandgap Control */
	0x0000,     /* R217 - OSC Control */
	0x9000,     /* R218 - RTC Tick Control */
	0x0000,     /* R219 - Security1 */
	0x4000,     /* R220 */
	0x0000,     /* R221 */
	0x0000,     /* R222 */
	0x0000,     /* R223 */
	0x0000,     /* R224 - Signal overrides */
	0x0000,     /* R225 - DCDC/LDO status */
	0x0000,     /* R226 - Charger Overides/status */
	0x0000,     /* R227 - misc overrides */
	0x0000,     /* R228 - Supply overrides/status 1 */
	0x0000,     /* R229 - Supply overrides/status 2 */
	0xE000,     /* R230 - GPIO Pin Status */
	0x0000,     /* R231 - comparotor overrides */
	0x0000,     /* R232 */
	0x0000,     /* R233 - State Machine status */
	0x1200,     /* R234 */
	0x0000,     /* R235 */
	0x8000,     /* R236 */
	0x0000,     /* R237 */
	0x0000,     /* R238 */
	0x0000,     /* R239 */
	0x0003,     /* R240 */
	0x0000,     /* R241 */
	0x0000,     /* R242 */
	0x0004,     /* R243 */
	0x0300,     /* R244 */
	0x0000,     /* R245 */
	0x0200,     /* R246 */
	0x0000,     /* R247 */
	0x1000,     /* R248 - DCDC1 Test Controls */
	0x5000,     /* R249 */
	0x1000,     /* R250 - DCDC3 Test Controls */
	0x1000,     /* R251 - DCDC4 Test Controls */
	0x5100,     /* R252 */
	0x1000,     /* R253 - DCDC6 Test Controls */
};
#endif

#ifdef CONFIG_MFD_WM8352_CONFIG_MODE_3

#undef WM8350_HAVE_CONFIG_MODE
#define WM8350_HAVE_CONFIG_MODE

const u16 wm8352_mode3_defaults[] = {
	0x6143,     /* R0   - Reset/ID */
	0x0000,     /* R1   - ID */
	0x0002,     /* R2   - Revision */
	0x1C02,     /* R3   - System Control 1 */
	0x0204,     /* R4   - System Control 2 */
	0x0000,     /* R5   - System Hibernate */
	0x8A00,     /* R6   - Interface Control */
	0x0000,     /* R7 */
	0x8000,     /* R8   - Power mgmt (1) */
	0x00