 called ep93xx_keypad.

config KEYBOARD_GPIO
	tristate "GPIO Buttons"
	depends on GENERIC_GPIO
	help
	  This driver implements support for buttons connected
	  to GPIO pins of various CPUs (and some other chips).

	  Say Y here if your device has buttons connected
	  directly to such GPIO pins.  Your board-specific
	  setup logic must also provide a platform device,
	  with configuration data saying which GPIOs are used.

	  To compile this driver as a module, choose M here: the
	  module will be called gpio_keys.

config KEYBOARD_MATRIX
	tristate "GPIO driven matrix keypad support"
	depends on GENERIC_GPIO
	help
	  Enable support for GPIO driven matrix keypad.

	  To compile this driver as a module, choose M here: the
	  module will be called matrix_keypad.

config KEYBOARD_HIL_OLD
	tristate "HP HIL keyboard support (simple driver)"
	depends on GSC || HP300
	default y
	help
	  The "Human Interface Loop" is a older, 8-channel USB-like
	  controller used in several Hewlett Packard models. This driver
	  was adapted from the one written for m68k/hp300, and implements
	  support for a keyboard attached to the HIL port, but not for
	  any other types of HIL input devices like mice or tablets.
	  However, it has been thoroughly tested and is stable.

	  If you want full HIL support including support for multiple
	  keyboards, mice, and tablets, 