#
# Input core configuration
#
menuconfig INPUT_KEYBOARD
	bool "Keyboards" if EMBEDDED || !X86
	default y
	help
	  Say Y here, and a list of supported keyboards will be displayed.
	  This option doesn't affect the kernel.

	  If unsure, say Y.

if INPUT_KEYBOARD

config KEYBOARD_AAED2000
	tristate "AAED-2000 keyboard"
	depends on MACH_AAED2000
	select INPUT_POLLDEV
	default y
	help
	  Say Y here to enable the keyboard on the Agilent AAED-2000
	  development board.

	  To compile this driver as a module, choose M here: the
	  module will be called aaed2000_kbd.

config KEYBOARD_AMIGA
	tristate "Amiga keyboard"
	depends on AMIGA
	help
	  Say Y here if you are running Linux on any AMIGA and have a keyboard
	  attached.

	  To compile this driver as a module, choose M here: the
	  module will be called amikbd.

config ATARI_KBD_CORE
	bool

config KEYBOARD_ATARI
	tristate "Atari keyboard"
	depends on ATARI
	select ATARI_KBD_CORE
	help
	  Say Y here if you are running Linux on any Atari and have a keyboard
	  attached.

	  To compile this driver as a module, choose M here: the
	  module will be called atakbd.

config KEYBOARD_ATKBD
	tristate "AT keyboard" if EMBEDDED || !X86
	default y
	select SERIO
	select SERIO_LIBPS2
	select SERIO_I8042 if X86
	select SERIO_GSCPS2 if GSC
	help
	  Say Y here if you want to use a standard AT or PS/2 keyboard. Usually
	  you'll need this, unless you have a different type keyboard (USB, ADB
	  or other). This also works for AT and PS/2 keyboards connected over a
	  PS/2 to serial converter.

	  If unsure, say Y.

	  To compile this driver as a module, choose M here: the
	  module will be called atkbd.

config KEYBOARD_ATKBD_HP_KEYCODES
	bool "Use HP keyboard scancodes"
	depends on PARISC && KEYBOARD_ATKBD
	default y
	help
	  Say Y here if you have a PA-RISC machine and want to use an AT or
	  PS/2 keyboard, and your keyboard uses keycodes that are specific to
	  PA-RISC keyboards.

	  Say N if you use a standard keyboard.

config KEYBOARD_ATKBD_RDI_KEYCODES
	bool "Use PrecisionBook keyboard scancodes"
	depends on KEYBOARD_ATKBD_HP_KEYCODES
	default n
	help
	  If you have an RDI PrecisionBook, say Y here if you want to use its
	  built-in keyboard (as opposed to an external keyboard).

	  The PrecisionBook has five keys that conflict with those used by most
	  AT and PS/2 keyboards. These are as follows:

	    PrecisionBook    Standard AT or PS/2

	    F1               F12
	    Left Ctrl        Left Alt
	    Caps Lock        Left Ctrl
	    Right Ctrl       Caps Lock
	    Left             102nd key (the key to the right of Left Shift)

	  If you say N here, and use the PrecisionBook keyboard, then each key
	  in the left-hand column will be interpreted as the corresponding key
	  in the right-hand column.

	  If you say Y here, and use an external keyboard, then each key in the
	  right-hand column will be interpreted as the key shown in the
	  left-hand column.

config KEYBOARD_BFIN
	tristate "Blackfin BF54x keypad support"
	depends on (BF54x && !BF544)
	help
	  Say Y here if you want to use the BF54x keypad.

	  To compile this driver as a module, choose M here: the
	  module will be called bf54x-keys.

config KEYBOARD_CORGI
	tristate "Corgi keyboard"
	depends on PXA_SHARPSL
	default y
	help
	  Say Y here to enable the keyboard on the Sharp Zaurus SL-C7xx
	  series of PDAs.

	  To compile this driver as a module, choose M here: the
	  module will be called corgikbd.

config KEYBOARD_LKKBD
	tristate "DECstation/VAXstation LK201/LK401 keyboard"
	select SERIO
	help
	  Say Y here if you want to use a LK201 or LK401 style serial
	  keyboard. This keyboard is also useable on PCs if you attach
	  it with the inputattach program. The connector pinout is
	  described within lkkbd.c.

	  To compile this driver as a module, choose M here: the
	  module will be called lkkbd.

config KEYBOARD_EP93XX
	tristate "EP93xx Matrix Keypad support"
	depends on ARCH_EP93XX
	help
	  Say Y here to enable the matrix keypad on the Cirrus EP93XX.

	  To compile this driver as a module, choose M here: the
	  module will be called ep93xx_keypad.

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
	 