AREA
	select FB_CFB_IMAGEBLIT
	select FB_MACMODES
	help
	  This driver supports a frame buffer for the "valkyrie" graphics
	  adapter in some Power Macintoshes.

config FB_CT65550
	bool "Chips 65550 display support"
	depends on (FB = y) && PPC32 && PCI
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This is the frame buffer device driver for the Chips & Technologies
	  65550 graphics chip in PowerBooks.

config FB_ASILIANT
	bool "Asiliant (Chips) 69000 display support"
	depends on (FB = y) && PCI
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This is the frame buffer device driver for the Asiliant 69030 chipset

config FB_IMSTT
	bool "IMS Twin Turbo display support"
	depends on (FB = y) && PCI
	select FB_CFB_IMAGEBLIT
	select FB_MACMODES if PPC
	help
	  The IMS Twin Turbo is a PCI-based frame buffer card bundled with
	  many Macintosh and compatible computers.

config FB_VGA16
	tristate "VGA 16-color graphics support"
	depends on FB && (X86 || PPC)
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select VGASTATE
	select FONT_8x16 if FRAMEBUFFER_CONSOLE
	help
	  This is the frame buffer device driver for VGA 16 color graphic
	  cards. Say Y if you have such a card.

	  To compile this driver as a module, choose M here: the
	  module will be called vga16fb.

config FB_BF54X_LQ043
	tristate "SHARP LQ043 TFT LCD (BF548 EZKIT)"
	depends on FB && (BF54x) && !BF542
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	 This is the framebuffer device driver for a SHARP LQ043T1DG01 TFT LCD

config FB_BFIN_T350MCQB
	tristate "Varitronix COG-T350MCQB TFT LCD display (BF527 EZKIT)"
	depends on FB && BLACKFIN
	select BFIN_GPTIMERS
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	 This is the framebuffer device driver for a Varitronix VL-PS-COG-T350MCQB-01 display TFT LCD
	 This display is a QVGA 320x240 24-bit RGB display interfaced by an 8-bit wide PPI
	 It uses PPI[0..7] PPI_FS1, PPI_FS2 and PPI_CLK.


config FB_STI
	tristate "HP STI frame buffer device support"
	depends on FB && PARISC
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select STI_CONSOLE
	select VT
	default y
	---help---
	  STI refers to the HP "Standard Text Interface" which is a set of
	  BIOS routines contained in a ROM chip in HP PA-RISC based machines.
	  Enabling this option will implement the linux framebuffer device
	  using calls to the STI BIOS routines for initialisation.
	
	  If you enable this option, you will get a planar framebuffer device
	  /dev/fb which will work on the most common HP graphic cards of the
	  NGLE family, including the artist chips (in the 7xx and Bxxx series),
	  HCRX, HCRX24, CRX, CRX24 and VisEG series.

	  It is safe to enable this option, so you should probably say "Y".

config FB_MAC
	bool "Generic Macintosh display support"
	depends on (FB = y) && MAC
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select FB_MACMODES

config FB_HP300
	bool
	depends on (FB = y) && DIO
	select FB_CFB_IMAGEBLIT
	default y

config FB_TGA
	tristate "TGA/SFB+ framebuffer support"
	depends on FB && (ALPHA || TC)
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select BITREVERSE
	---help---
	  This is the frame buffer device driver for generic TGA and SFB+
	  graphic cards.  These include DEC ZLXp-E1, -E2 and -E3 PCI cards,
	  also known as PBXGA-A, -B and -C, and DEC ZLX-E1, -E2 and -E3
	  TURBOchannel cards, also known as PMAGD-A, -B and -C.

	  Due to hardware limitations ZLX-E2 and E3 cards are not supported
	  for DECstation 5000/200 systems.  Additionally due to firmware
	  limitations these cards may cause troubles with booting DECstation
	  5000/240 and /260 systems, but are fully supported under Linux if
	  you manage to get it going. ;-)

	  Say Y if you have one of those.

config FB_UVESA
	tristate "Userspace VESA VGA graphics support"
	depends on FB && CONNECTOR
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select FB_MODE_HELPERS
	help
	  This is the frame buffer driver for generic VBE 2.0 compliant
	  graphic cards. It can also take advantage of VBE 3.0 features,
	  such as refresh rate adjustment.

	  This driver generally provides more features than vesafb but
	  requires a userspace helper application called 'v86d'. See
	  <file:Documentation/fb/uvesafb.txt> for more information.

	  If unsure, say N.

config FB_VESA
	bool "VESA VGA graphics support"
	depends on (FB = y) && X86
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select FB_BOOT_VESA_SUPPORT
	help
	  This is the frame buffer device driver for generic VESA 2.0
	  compliant graphic cards. The older VESA 1.2 cards are not supported.
	  You will get a boot time penguin logo at no additional cost. Please
	  read <file:Documentation/fb/vesafb.txt>. If unsure, say Y.

config FB_EFI
	bool "EFI-based Framebuffer Support"
	depends on (FB = y) && X86 && EFI
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This is the EFI frame buffer device driver. If the firmware on
	  your platform is EFI 1.10 or UEFI 2.0, select Y to add support for
	  using the EFI framebuffer as your console.

config FB_N411
       tristate "N411 Apollo/Hecuba devkit support"
       depends on FB && X86 && MMU
       select FB_SYS_FILLRECT
       select FB_SYS_COPYAREA
       select FB_SYS_IMAGEBLIT
       select FB_SYS_FOPS
       select FB_DEFERRED_IO
       select FB_HECUBA
       help
         This enables support for the Apollo display controller in its
         Hecuba form using the n411 devkit.

config FB_HGA
	tristate "Hercules mono graphics support"
	depends on FB && X86
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  Say Y here if you have a Hercules mono graphics card.

	  To compile this driver as a module, choose M here: the
	  module will be called hgafb.

	  As this card technology is 15 years old, most people will answer N
	  here.

config FB_HGA_ACCEL
	bool "Hercules mono Acceleration functions (EXPERIMENTAL)"
	depends on FB_HGA && EXPERIMENTAL
	---help---
	This will compile the Hercules mono graphics with
	acceleration functions.

config FB_SGIVW
	tristate "SGI Visual Workstation framebuffer support"
	depends on FB && X86_VISWS
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  SGI Visual Workstation support for framebuffer graphics.

config FB_GBE
	bool "SGI Graphics Backend frame buffer support"
	depends on (FB = y) && (SGI_IP32 || X86_VISWS)
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
 	help
	  This is the frame buffer device driver for SGI Graphics Backend.
	  This chip is used in SGI O2 and Visual Workstation 320/540.

config FB_GBE_MEM
	int "Video memory size in MB"
	depends on FB_GBE
	default 4
	help
	  This is the amount of memory reserved for the framebuffer,
	  which can be any value between 1MB and 8MB.

config FB_SBUS
	bool "SBUS and UPA framebuffers"
	depends on (FB = y) && SPARC
	help
	  Say Y if you want support for SBUS or UPA based frame buffer device.

config FB_BW2
	bool "BWtwo support"
	depends on (FB = y) && (SPARC && FB_SBUS)
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This is the frame buffer device driver for the BWtwo frame buffer.

config FB_CG3
	bool "CGthree support"
	depends on (FB = y) && (SPARC && FB_SBUS)
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This is the frame buffer device driver for the CGthree frame buffer.

config FB_CG6
	bool "CGsix (GX,TurboGX) support"
	depends on (FB = y) && (SPARC && FB_SBUS)
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This is the frame buffer device driver for the CGsix (GX, TurboGX)
	  frame buffer.

config FB_FFB
	bool "Creator/Creator3D/Elite3D support"
	depends on FB_SBUS && SPARC64
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This is the frame buffer device driver for the Creator, Creator3D,
	  and Elite3D graphics boards.

config FB_TCX
	bool "TCX (SS4/SS5 only) support"
	depends on FB_SBUS
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This is the frame buffer device driver for the TCX 24/8bit frame
	  buffer.

config FB_CG14
	bool "CGfourteen (SX) support"
	depends on FB_SBUS
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This is the frame buffer device driver for the CGfourteen frame
	  buffer on Desktop SPARCsystems with the SX graphics option.

config FB_P9100
	bool "P9100 (Sparcbook 3 only) support"
	depends on FB_SBUS
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This is the frame buffer device driver for the P9100 card
	  supported on Sparcbook 3 machines.

config FB_LEO
	bool "Leo (ZX) support"
	depends on FB_SBUS
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This is the frame buffer device driver for the SBUS-based Sun ZX
	  (leo) frame buffer cards.

config FB_IGA
	bool "IGA 168x display support"
	depends on (FB = y) && SPARC32
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This is the framebuffer device for the INTERGRAPHICS 1680 and
	  successor frame buffer cards.

config FB_XVR500
	bool "Sun XVR-500 3DLABS Wildcat support"
	depends on (FB = y) && PCI && SPARC64
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This is the framebuffer device for the Sun XVR-500 and similar
	  graphics cards based upon the 3DLABS Wildcat chipset.  The driver
	  only works on sparc64 systems where the system firmware has
	  mostly initialized the card already.  It is treated as a
	  completely dumb framebuffer device.

config FB_XVR2500
	bool "Sun XVR-2500 3DLABS Wildcat support"
	depends on (FB = y) && PCI && SPARC64
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This is the framebuffer device for the Sun XVR-2500 and similar
	  graphics cards based upon the 3DLABS Wildcat chipset.  The driver
	  only works on sparc64 systems where the system firmware has
	  mostly initialized the card already.  It is treated as a
	  completely dumb framebuffer device.

config FB_PVR2
	tristate "NEC PowerVR 2 display support"
	depends on FB && SH_DREAMCAST
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	---help---
	  Say Y here if you have a PowerVR 2 card in your box.  If you plan to
	  run linux on your Dreamcast, you will have to say Y here.
	  This driver may or may not work on other PowerVR 2 cards, but is
	  totally untested.  Use at your own risk.  If unsure, say N.

	  To compile this driver as a module, choose M here: the
	  module will be called pvr2fb.

	  You can pass several parameters to the driver at boot time or at
	  module load time.  The parameters look like "video=pvr2:XXX", where
	  the meaning of XXX can be found at the end of the main source file
	  (<file:drivers/video/pvr2fb.c>). Please see the file
	  <file:Documentation/fb/pvr2fb.txt>.

config FB_EPSON1355
	bool "Epson 1355 framebuffer support"
	depends on (FB = y) && ARCH_CEIVA
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  Build in support for the SED1355 Epson Research Embedded RAMDAC
	  LCD/CRT Controller (since redesignated as the S1D13505) as a
	  framebuffer.  Product specs at
	  <http://www.erd.epson.com/vdc/html/products.htm>.

config FB_S1D13XXX
	tristate "Epson S1D13XXX framebuffer support"
	depends on FB
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  Support for S1D13XXX framebuffer device family (currently only
	  working with S1D13806). Product specs at
	  <http://www.erd.epson.com/vdc/html/legacy_13xxx.htm>

config SPLC501C
	tristate "LCD_SPLC501C support"
	select LCD_SPLC501C
	help
	  Support for LCD_SPLC501C
	  
config FB_ATMEL
	tristate "AT91/AT32 LCD Controller support"
	depends on FB && (ARCH_AT91SAM9261 || ARCH_AT91SAM9263 || ARCH_AT91SAM9RL || ARCH_AT91CAP9 || AVR32)
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This enables support for the AT91/AT32 LCD Controller.

config FB_INTSRAM
	bool "Frame Buffer in internal SRAM"
	depends on FB_ATMEL && ARCH_AT91SAM9261
	help
	  Say Y if you want to map Frame Buffer in internal SRAM. Say N if you want
	  to let frame buffer in external SDRAM.

config FB_ATMEL_STN
	bool "Use a STN display with AT91/AT32 LCD Controller"
	depends on FB_ATMEL && MACH_AT91SAM9261EK
	default n
	help
	  Say Y if you want to connect a STN LCD display to the AT91/AT32 LCD
	  Controller. Say N if you want to connect a TFT.

	  If unsure, say N.

config FB_NVIDIA
	tristate "nVidia Framebuffer Support"
	depends on FB && PCI
	select FB_BACKLIGHT if FB_NVIDIA_BACKLIGHT
	select FB_MODE_HELPERS
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select BITREVERSE
	select VGASTATE
	help
	  This driver supports graphics boards with the nVidia chips, TNT
	  and newer. For very old chipsets, such as the RIVA128, then use
	  the rivafb.
	  Say Y if you have such a graphics board.

	  To compile this driver as a module, choose M here: the
	  module will be called nvidiafb.

config FB_NVIDIA_I2C
       bool "Enable DDC Support"
       depends on FB_NVIDIA
       select FB_DDC
       help
	  This enables I2C support for nVidia Chipsets.  This is used
	  only for getting EDID information from the attached display
	  allowing for robust video mode handling and switching.

	  Because fbdev-2.6 requires that drivers must be able to
	  independently validate video mode parameters, you should say Y
	  here.

config FB_NVIDIA_DEBUG
	bool "Lots of debug output"
	depends on FB_NVIDIA
	default n
	help
	  Say Y here if you want the nVidia driver to output all sorts
	  of debugging information to provide to the maintainer when
	  something goes wrong.

config FB_NVIDIA_BACKLIGHT
	bool "Support for backlight control"
	depends on FB_NVIDIA
	default y
	help
	  Say Y here if you want to control the backlight of your display.

config FB_RIVA
	tristate "nVidia Riva support"
	depends on FB && PCI
	select FB_BACKLIGHT if FB_RIVA_BACKLIGHT
	select FB_MODE_HELPERS
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select BITREVERSE
	select VGASTATE
	help
	  This driver supports graphics boards with the nVidia Riva/Geforce
	  chips.
	  Say Y if you have such a graphics board.

	  To compile this driver as a module, choose M here: the
	  module will be called rivafb.

config FB_RIVA_I2C
       bool "Enable DDC Support"
       depends on FB_RIVA
       select FB_DDC
       help
	  This enables I2C support for nVidia Chipsets.  This is used
	  only for getting EDID information from the attached display
	  allowing for robust video mode handling and switching.

	  Because fbdev-2.6 requires that drivers must be able to
	  independently validate video mode parameters, you should say Y
	  here.

config FB_RIVA_DEBUG
	bool "Lots of debug output"
	depends on FB_RIVA
	default n
	help
	  Say Y here if you want the Riva driver to output all sorts
	  of debugging information to provide to the maintainer when
	  something goes wrong.

config FB_RIVA_BACKLIGHT
	bool "Support for backlight control"
	depends on FB_RIVA
	default y
	help
	  Say Y here if you want to control the backlight of your display.

config FB_I810
	tristate "Intel 810/815 support (EXPERIMENTAL)"
	depends on EXPERIMENTAL && FB && PCI && X86_32 && AGP_INTEL
	select FB_MODE_HELPERS
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select VGASTATE
	help
	  This driver supports the on-board graphics built in to the Intel 810 
          and 815 chipsets.  Say Y if you have and plan to use such a board.

          To compile this driver as a module, choose M here: the
	  module will be called i810fb.

          For more information, please read 
	  <file:Documentation/fb/intel810.txt>

config FB_I810_GTF
	bool "use VESA Generalized Timing Formula"
	depends on FB_I810
	help
	  If you say Y, then the VESA standard, Generalized Timing Formula 
          or GTF, will be used to calculate the required video timing values
	  per video mode.  Since the GTF allows nondiscrete timings 
          (nondiscrete being a range of values as opposed to discrete being a
          set of values), you'll be able to use any combination of horizontal 
	  and vertical resolutions, and vertical refresh rates without having
	  to specify your own timing parameters.  This is especially useful
	  to maximize the performance of an aging display, or if you just 
          have a display with nonstandard dimensions. A VESA compliant 
	  monitor is recommended, but can still work with non-compliant ones.
	  If you need or want this, then select this option. The timings may 
	  not be compliant with Intel's recommended values. Use at your own 
	  risk.

          If you say N, the driver will revert to discrete video timings 
	  using a set recommended by Intel in their documentation.
  
          If unsure, say N.

config FB_I810_I2C
	bool "Enable DDC Support"
	depends on FB_I810 && FB_I810_GTF
	select FB_DDC
	help

config FB_LE80578
	tristate "Intel LE80578 (Vermilion) support"
	depends on FB && PCI && X86
	select FB_MODE_HELPERS
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	help
	  This driver supports the LE80578 (Vermilion Range) chipset

config FB_CARILLO_RANCH
	tristate "Intel Carillo Ranch support"
	depends on FB_LE80578 && FB && PCI && X86
	help
	  This driver supports the LE80578 (Carillo Ranch) board

config FB_INTEL
	tristate "Intel 830M/845G/852GM/855GM/865G/915G/945G/945GM/965G/965GM support (EXPERIMENTAL)"
	depends on EXPERIMENTAL && FB && PCI && X86 && AGP_INTEL && EMBEDDED
	select FB_MODE_HELPERS
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select FB_BOOT_VESA_SUPPORT if FB_INTEL = y
	depends on !DRM_I915
	help
	  This driver supports the on-board graphics built in to the Intel
          830M/845G/852GM/855GM/865G/915G/915GM/945G/945GM/965G/965GM chipsets.
          Say Y if you have and plan to use such a board.

	  To make FB_INTELFB=Y work you need to say AGP_INTEL=y too.

	  To compile this driver as a module, choose M here: the
	  module will be called intelfb.

	  For more information, please read <file:Documentation/fb/intelfb.txt>

config FB_INTEL_DEBUG
	bool "Intel driver Debug Messages"
	depends on FB_INTEL
	---help---
	  Say Y here if you want the Intel driver to output all sorts
	  of debugging information to provide to the maintainer when
	  something goes wrong.

config FB_INTEL_I2C
	bool "DDC/I2C for Intel framebuffer support"
	depends on FB_INTEL
	select FB_DDC
	default y
	help
	  Say Y here if you want DDC/I2C support for your on-board Intel graphics.

config FB_MATROX
	tristate "Matrox acceleration"
	depends on FB && PCI
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select FB_TILEBLITTING
	select FB_MACMODES if PPC_PMAC
	---help---
	  Say Y here if you have a Matrox Millennium, Matrox Millennium II,
	  Matrox Mystique, Matrox Mystique 220, Matrox Productiva G100, Matrox
	  Mystique G200, Matrox Millennium G200, Matrox Marvel G200 video,
	  Matrox G400, G450 or G550 card in your box.

	  To compile this driver as a module, choose M here: the
	  module will be called matroxfb.

	  You can pass several parameters to the driver at boot time or at
	  module load time. The parameters look like "video=matrox:XXX", and
	  are described in <file:Documentation/fb/matroxfb.txt>.

config FB_MATROX_MILLENIUM
	bool "Millennium I/II support"
	depends on FB_MATROX
	help
	  Say Y here if you have a Matrox Millennium or Matrox Millennium II
	  video card. If you select "Advanced lowlevel driver options" below,
	  you should check 4 bpp packed pixel, 8 bpp packed pixel, 16 bpp
	  packed pixel, 24 bpp packed pixel and 32 bpp packed pixel. You can
	  also use font widths different from 8.

config FB_MATROX_MYSTIQUE
	bool "Mystique support"
	depends on FB_MATROX
	help
	  Say Y here if you have a Matrox Mystique or Matrox Mystique 220
	  video card. If you select "Advanced lowlevel driver options" below,
	  you should check 8 bpp packed pixel, 16 bpp packed pixel, 24 bpp
	  packed pixel and 32 bpp packed pixel. You can also use font widths
	  different from 8.

config FB_MATROX_G
	bool "G100/G200/G400/G450/G550 support"
	depends on FB_MATROX
	---help---
	  Say Y here if you have a Matrox G100, G200, G400, G450 or G550 based
	  video card. If you select "Advanced lowlevel driver options", you
	  should check 8 bpp packed pixel, 16 bpp packed pixel, 24 bpp packed
	  pixel and 32 bpp packed pixel. You can also use font widths
	  different from 8.

	  If you need support for G400 secondary head, you must say Y to
	  "Matrox I2C support" and "G400 second head support" right below.
	  G450/G550 secondary head and digital output are supported without
	  additional modules.

	  The driver starts in monitor mode. You must use the matroxset tool 
	  (available at <ftp://platan.vc.cvut.cz/pub/linux/matrox-latest/>) to 
	  swap primary and secondary head outputs, or to change output mode.  
	  Secondary head driver always start in 640x480 resolution and you 
	  must use fbset to change it.

	  Do not forget that second head supports only 16 and 32 bpp
	  packed pixels, so it is a good idea to compile them into the kernel
	  too. You can use only some font widths, as the driver uses generic
	  painting procedures (the secondary head does not use acceleration
	  engine).

	  G450/G550 hardware can display TV picture only from secondary CRTC,
	  and it performs no scaling, so picture must have 525 or 625 lines.

config FB_MATROX_I2C
	tristate "Matrox I2C support"
	depends on FB_MATROX
	select FB_DDC
	---help---
	  This drivers creates I2C buses which are needed for accessing the
	  DDC (I2C) bus present on all Matroxes, an I2C bus which
	  interconnects Matrox optional devices, like MGA-TVO on G200 and
	  G400, and the secondary head DDC bus, present on G400 only.

	  You can say Y or M here if you want to experiment with monitor
	  detection code. You must say Y or M here if you want to use either
	  second head of G400 or MGA-TVO on G200 or G400.

	  If you compile it as module, it will create a module named
	  i2c-matroxfb.

config FB_MATROX_MAVEN
	tristate "G400 second head support"
	depends on FB_MATROX_G && FB_MATROX_I2C
	---help---
	  WARNING !!! This support does not work with G450 !!!

	  Say Y or M here if you want to use a secondary head (meaning two
	  monitors in parallel) on G400 or MGA-TVO add-on on G200. Secondary
	  head is not compatible with accelerated XFree 3.3.x SVGA servers -
	  secondary head output is blanked while you are in X. With XFree
	  3.9.17 preview you can use both heads if you use SVGA over fbdev or
	  the fbdev driver on first head and the fbdev driver on second head.

	  If you compile it as module, two modules are created,
	  matroxfb_crtc2 and matroxfb_maven. Matroxfb_maven is needed for
	  both G200 and G400, matroxfb_crtc2 is needed only by G400. You must
	  also load i2c-matroxfb to get it to run.

	  The driver starts in monitor mode and you must use the matroxset
	  tool (available at
	  <ftp://platan.vc.cvut.cz/pub/linux/matrox-latest/>) to switch it to
	  PAL or NTSC or to swap primary and secondary head outputs.
	  Secondary head driver also always start in 640x480 resolution, you
	  must use fbset to change it.

	  Also do not forget that second head supports only 16 and 32 bpp
	  packed pixels, so it is a good idea to compile them into the kernel
	  too.  You can use only some font widths, as the driver uses generic
	  painting procedures (the secondary head does not use acceleration
	  engine).

config FB_MATROX_MULTIHEAD
	bool "Multihead support"
	depends on FB_MATROX
	---help---
	  Say Y here if you have more than one (supported) Matrox device in
	  your computer and you want to use all of them for different monitors
	  ("multihead"). If you have only one device, you should say N because
	  the driver compiled with Y is larger and a bit slower, especially on
	  ia32 (ix86).

	  If you said M to "Matrox unified accelerated driver" and N here, you
	  will still be able to use several Matrox devices simultaneously:
	  insert several instances of the module matroxfb into the kernel
	  with insmod, supplying the parameter "dev=N" where N is 0, 1, etc.
	  for the different Matrox devices. This method is slightly faster but
	  uses 40 KB of kernel memory per Matrox card.

	  There is no need for enabling 'Matrox multihead support' if you have
	  only one Matrox card in the box.

config FB_RADEON
	tristate "ATI Radeon display support"
	depends on FB && PCI
	select FB_BACKLIGHT if FB_RADEON_BACKLIGHT
	select FB_MODE_HELPERS
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select FB_MACMODES if PPC_OF
	help
	  Choose this option if you want to use an ATI Radeon graphics card as
	  a framebuffer device.  There are both PCI and AGP versions.  You
	  don't need to choose this to run the Radeon in plain VGA mode.

	  There is a product page at
	  http://apps.ati.com/ATIcompare/

config FB_RADEON_I2C
	bool "DDC/I2C for ATI Radeon support"
	depends on FB_RADEON
	select FB_DDC
	default y
	help
	  Say Y here if you want DDC/I2C support for your Radeon board. 

config FB_RADEON_BACKLIGHT
	bool "Support for backlight control"
	depends on FB_RADEON
	default y
	help
	  Say Y here if you want to control the backlight of your display.

config FB_RADEON_DEBUG
	bool "Lots of debug output from Radeon driver"
	depends on FB_RADEON
	default n
	help
	  Say Y here if you want the Radeon driver to output all sorts
	  of debugging information to provide to the maintainer when
	  something goes wrong.

config FB_ATY128
	tristate "ATI Rage128 display support"
	depends on FB && PCI
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select FB_BACKLIGHT if FB_ATY128_BACKLIGHT
	select FB_MACMODES if PPC_PMAC
	help
	  This driver supports graphics boards with the ATI Rage128 chips.
	  Say Y if you have such a graphics board and read
	  <file:Documentation/fb/aty128fb.txt>.

	  To compile this driver as a module, choose M here: the
	  module will be called aty128fb.

config FB_ATY128_BACKLIGHT
	bool "Support for backlight control"
	depends on FB_ATY128
	default y
	help
	  Say Y here if you want to control the backlight of your display.

config FB_ATY
	tristate "ATI Mach64 display support" if PCI || ATARI
	depends on FB && !SPARC32
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select FB_BACKLIGHT if FB_ATY_BACKLIGHT
	select FB_MACMODES if PPC
	help
	  This driver supports graphics boards with the ATI Mach64 chips.
	  Say Y if you have such a graphics board.

	  To compile this driver as a module, choose M here: the
	  module will be called atyfb.

config FB_ATY_CT
	bool "Mach64 CT/VT/GT/LT (incl. 3D RAGE) support"
	depends on PCI && FB_ATY
	default y if SPARC64 && PCI
	help
	  Say Y here to support use of ATI's 64-bit Rage boards (or other
	  boards based on the Mach64 CT, VT, GT, and LT chipsets) as a
	  framebuffer device.  The ATI product support page for these boards
	  is at <http://support.ati.com/products/pc/mach64/>.

config FB_ATY_GENERIC_LCD
	bool "Mach64 generic LCD support (EXPERIMENTAL)"
	depends on FB_ATY_CT
	help
	  Say Y if you have a laptop with an ATI Rage LT PRO, Rage Mobility,
	  Rage XC, or Rage XL chipset.

config FB_ATY_GX
	bool "Mach64 GX support" if PCI
	depends on FB_ATY
	default y if ATARI
	help
	  Say Y here to support use of the ATI Mach64 Graphics Expression
	  board (or other boards based on the Mach64 GX chipset) as a
	  framebuffer device.  The ATI product support page for these boards
	  is at
	  <http://support.ati.com/products/pc/mach64/graphics_xpression.html>.

config FB_ATY_BACKLIGHT
	bool "Support for backlight control"
	depends on FB_ATY
	default y
	help
	  Say Y here if you want to control the backlight of your display.

config FB_S3
	tristate "S3 Trio/Virge support"
	depends on FB && PCI
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select FB_TILEBLITTING
	select FB_SVGALIB
	select VGASTATE
	select FONT_8x16 if FRAMEBUFFER_CONSOLE
	---help---
	  Driver for graphics boards with S3 Trio / S3 Virge chip.

config FB_SAVAGE
	tristate "S3 Savage support"
	depends on FB && PCI && g received.
 *	Return 0 if permission is granted.
 *
 * Security hook for dentry
 *
 * @dentry_open
 *	Save open-time permission checking state for later use upon
 *	file_permission, and recheck access if anything has changed
 *	since inode_permission.
 *
 * Security hooks for task operations.
 *
 * @task_create:
 *	Check permission before creating a child process.  See the clone(2)
 *	manual page for definitions of the @clone_flags.
 *	@clone_flags contains the flags indicating what should be shared.
 *	Return 0 if permission is granted.
 * @cred_free:
 *	@cred points to the credentials.
 *	Deallocate and clear the cred->security field in a set of credentials.
 * @cred_prepare:
 *	@new points to the new credentials.
 *	@old points to the original credentials.
 *	@gfp indicates the atomicity of any memory allocations.
 *	Prepare a new set of credentials by copying the data from the old set.
 * @cred_commit:
 *	@new points to the new credentials.
 *	@old points to the original credentials.
 *	Install a new set of credentials.
 * @kernel_act_as:
 *	Set the credentials for a kernel service to act as (subjective context).
 *	@new points to the credentials to be modified.
 *	@secid specifies the security ID to be set
 *	The current task must be the one that nominated @secid.
 *	Return 0 if successful.
 * @kernel_create_files_as:
 *	Set the file creation context in a set of credentials to be the same as
 *	the objective context of the specified inode.
 *	@new points to the credentials to be modified.
 *	@inode points to the inode to use as a reference.
 *	The current task must be the one that nominated @inode.
 *	Return 0 if successful.
 * @task_setuid:
 *	Check permission before setting one or more of the user identity
 *	attributes of the current process.  The @flags parameter indicates
 *	which of the set*uid system calls invoked this hook and how to
 *	interpret the @id0, @id1, and @id2 parameters.  See the LSM_SETID
 *	definitions at the beginning of this file for the @flags values and
 *	their meanings.
 *	@id0 contains a uid.
 *	@id1 contains a uid.
 *	@id2 contains a uid.
 *	@flags contains one of the LSM_SETID_* values.
 *	Return 0 if permission is granted.
 * @task_fix_setuid:
 *	Update the module's state after setting one or more of the user
 *	identity attributes of the current process.  The @flags parameter
 *	indicates which of the set*uid system calls invoked this hook.  If
 *	@new is the set of credentials that will be installed.  Modifications
 *	should be made to this rather than to @current->cred.
 *	@old is the set of credentials that are being replaces
 *	@flags contains one of the LSM_SETID_* values.
 *	Return 0 on success.
 * @task_setgid:
 *	Check permission before setting one or more of the group identity
 *	attributes of the current process.  The @flags parameter indicates
 *	which of the set*gid system calls invoked this hook and how to
 *	interpret the @id0, @id1, and @id2 parameters.  See the LSM_SETID
 *	definitions at the beginning of this file for the @flags values and
 *	their meanings.
 *	@id0 contains a gid.
 *	@id1 contains a gid.
 *	@id2 contains a gid.
 *	@flags contains one of the LSM_SETID_* values.
 *	Return 0 if permission is granted.
 * @task_setpgid:
 *	Check permission before setting the process group identifier of the
 *	process @p to @pgid.
 *	@p contains the task_struct for process being modified.
 *	@pgid contains the new pgid.
 *	Return 0 if permission is granted.
 * @task_getpgid:
 *	Check permission before getting the process group identifier of the
 *	process @p.
 *	@p contains the task_struct for the process.
 *	Return 0 if permission is granted.
 * @task_getsid:
 *	Check permission before getting the session identifier of the process
 *	@p.
 *	@p contains the task_struct for the process.
 *	Return 0 if permission is granted.
 * @task_getsecid:
 *	Retrieve the security identifier of the process @p.
 *	@p contains the task_struct for the process and place is into @secid.
 *	In case of failure, @secid will be set to zero.
 *
 * @task_setgroups:
 *	Check permission before setting the supplementary group set of the
 *	current process.
 *	@group_info contains the new group information.
 *	Return 0 if permission is granted.
 * @task_setnice:
 *	Check permission before setting the nice value of @p to @nice.
 *	@p contains the task_struct of process.
 *	@nice contains the new nice value.
 *	Return 0 if permission is granted.
 * @task_setioprio
 *	Check permission before setting the ioprio value of @p to @ioprio.
 *	@p contains the task_struct of process.
 *	@ioprio contains the new ioprio value
 *	Return 0 if permission is granted.
 * @task_getioprio
 *	Check permission before getting the ioprio value of @p.
 *	@p contains the task_struct of process.
 *	Return 0 if permission is granted.
 * @task_setrlimit:
 *	Check permission before setting the resource limits of the current
 *	process for @resource to @new_rlim.  The old resource limit values can
 *	be examined by dereferencing (current->signal->rlim + resource).
 *	@resource contains the resource whose limit is being set.
 *	@new_rlim contains the new limits for @resource.
 *	Return 0 if permission is granted.
 * @task_setscheduler:
 *	Check permission before setting scheduling policy and/or parameters of
 *	process @p based on @policy and @lp.
 *	@p contains the task_struct for process.
 *	@policy contains the scheduling policy.
 *	@lp contains the scheduling parameters.
 *	Return 0 if permission is granted.
 * @task_getscheduler:
 *	Check permission before obtaining scheduling information for process
 *	@p.
 *	@p contains the task_struct for process.
 *	Return 0 if permission is granted.
 * @task_movememory
 *	Check permission before moving memory owned by process @p.
 *	@p contains the task_struct for process.
 *	Return 0 if permission is granted.
 * @task_kill:
 *	Check permission before sending signal @sig to @p.  @info can be NULL,
 *	the constant 1, or a pointer to a siginfo structure.  If @info is 1 or
 *	SI_FROMKERNEL(info) is true, then the signal should be viewed as coming
 *	from the kernel and should typically be permitted.
 *	SIGIO signals are handled separately by the send_sigiotask hook in
 *	file_security_ops.
 *	@p contains the task_struct for process.
 *	@info contains the signal information.
 *	@sig contains the signal value.
 *	@secid contains the sid of the process where the signal originated
 *	Return 0 if permission is granted.
 * @task_wait:
 *	Check permission before allowing a process to reap a child process @p
 *	and collect its status information.
 *	@p contains the task_struct for process.
 *	Return 0 if permission is granted.
 * @task_prctl:
 *	Check permission before performing a process control operation on the
 *	current process.
 *	@option contains the operation.
 *	@arg2 contains a argument.
 *	@arg3 contains a argument.
 *	@arg4 contains a argument.
 *	@arg5 contains a argument.
 *	Return -ENOSYS if no-one wanted to handle this op, any other value to
 *	cause prctl() to return immediately with that value.
 * @task_to_inode:
 *	Set the security attributes for an inode based on an associated task's
 *	security attributes, e.g. for /proc/pid inodes.
 *	@p contains the task_struct for the task.
 *	@inode contains the inode structure for the inode.
 *
 * Security hooks for Netlink messaging.
 *
 * @netlink_send:
 *	Save security information for a netlink message so that permission
 *	checking can be performed when the message is processed.  The security
 *	information can be saved using the eff_cap field of the
 *	netlink_skb_parms structure.  Also may be used to provide fine
 *	grained control over message transmission.
 *	@sk associated sock of task sending the message.,
 *	@skb contains the sk_buff structure for the netlink message.
 *	Return 0 if the information was successfully saved and message
 *	is allowed to be transmitted.
 * @netlink_recv:
 *	Check permission before processing the received netlink message in
 *	@skb.
 *	@skb contains the sk_buff structure for the netlink message.
 *	@cap indicates the capability required
 *	Return 0 if permission is granted.
 *
 * Security hooks for Unix domain networkA TURBOchannel framebuffer support"
	depends on (FB = y) && TC
 	select FB_CFB_FILLRECT
 	select FB_CFB_COPYAREA
 	select FB_CFB_IMAGEBLIT
	help
	  Support for the PMAG-AA TURBOchannel framebuffer card (1280x1024x1)
	  used mainly in the MIPS-based DECstation series.

config FB_PMAG_BA
	tristate "PMAG-BA TURBOchannel framebuffer support"
	depends on FB && TC
 	select FB_CFB_FILLRECT
 	select FB_CFB_COPYAREA
 	select FB_CFB_IMAGEBLIT
	help
	  Support for the PMAG-BA TURBOchannel framebuffer card (1024x864x8)
	  used mainly in the MIPS-based DECstation series.

config FB_PMAGB_B
	tristate "PMAGB-B TURBOchannel framebuffer support"
	depends on FB && TC
 	select FB_CFB_FILLRECT
 	select FB_CFB_COPYAREA
 	select FB_CFB_IMAGEBLIT
	help
	  Support for the PMAGB-B TURBOchannel framebuffer card used mainly
	  in the MIPS-based DECstation series. The card is currently only
	  supported in 1280x1024x8 mode.

config FB_MAXINE
	bool "Maxine (Personal DECstation) onboard framebuffer support"
	depends on (FB = y) && MACH_DECSTATION
 	select FB_CFB_FILLRECT
 	select FB_CFB_COPYAREA
 	select FB_CFB_IMAGEBLIT
	help
	  Support for the onboard framebuffer (1024x768x8) in the Personal
	  DECstation series (Personal DECstation 5000/20, /25, /33, /50,
	  Codename "Maxine").

config FB_G364
	bool "G364 frame buffer support"
	depends on (FB = y) && (MIPS_MAGNUM_4000 || OLIVETTI_M700)
 	select FB_CFB_FILLRECT
 	select FB_CFB_COPYAREA
 	select FB_CFB_IMAGEBLIT
	help
	  The G364 driver is the framebuffer used in MIPS Magnum 4000 and
	  Olivetti M700-10 systems.

config FB_68328
	bool "Motorola 68328 native frame buffer support"
	depends on (FB = y) && (M68328 || M68EZ328 || M68VZ328)
 	select FB_CFB_FILLRECT
 	select FB_CFB_COPYAREA
 	select FB_CFB_IMAGEBLIT
	help
	  Say Y here if you want to support the built-in frame buffer of
	  the Motorola 68328 CPU family.

config FB_PXA168
	tristate "PXA168/910 LCD framebuffer support"
	depends on FB && (CPU_PXA168 || CPU_PXA910)
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	---help---
	  Frame buffer driver for the built-in LCD controller in the Marvell
	  MMP processor.

config FB_PXA
	tristate "PXA LCD framebuffer support"
	depends on FB && ARCH_PXA
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	---help---
	  Frame buffer driver for the built-in LCD controller in the Intel
	  PXA2x0 processor.

	  This driver is also available as a module ( = code which can be
	  inserted and removed from the running kernel whenever you want). The
	  module will be called pxafb. If you want to compile it as a module,
	  say M here and read <file:Documentation/kbuild/modules.txt>.

	  If unsure, say N.

config FB_PXA_OVERLAY
	bool "Support PXA27x/PXA3xx Overlay(s) as framebuffer"
	default n
	depends on FB_PXA && (PXA27x || PXA3xx)

config FB_PXA_SMARTPANEL
	bool "PXA Smartpanel LCD support"
	default n
	depends on FB_PXA

config FB_PXA_PARAMETERS
	bool "PXA LCD command line parameters"
	default n
	depends on FB_PXA
	---help---
	  Enable the use of kernel command line or module parameters
	  to configure the physical properties of the LCD panel when
	  using the PXA LCD driver.

	  This option allows you to override the panel parameters
	  supplied by the platform in order to support multiple
	  different models of flatpanel. If you will only be using a
	  single model of flatpanel then you can safely leave this
	  option disabled.

	  <file:Documentation/fb/pxafb.txt> describes the available parameters.

config FB_MBX
	tristate "2700G LCD framebuffer support"
	depends on FB && ARCH_PXA
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	---help---
	  Framebuffer driver for the Intel 2700G (Marathon) Graphics
	  Accelerator

config FB_MBX_DEBUG
       bool "Enable debugging info via debugfs"
       depends on FB_MBX && DEBUG_FS
       default n
       ---help---
         Enable this if you want debugging information using the debug
         filesystem (debugfs)

         If unsure, say N.

config FB_FSL_DIU
	tristate "Freescale DIU framebuffer support"
	depends on FB && FSL_SOC
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	select PPC_LIB_RHEAP
	---help---
	  Framebuffer driver for the Freescale SoC DIU

config FB_W100
	tristate "W100 frame buffer support"
	depends on FB && ARCH_PXA
 	select FB_CFB_FILLRECT
 	select FB_CFB_COPYAREA
 	select FB_CFB_IMAGEBLIT
	---help---
	  Frame buffer driver for the w100 as found on the Sharp SL-Cxx series.
	  It can also drive the w3220 chip found on iPAQ hx4700.

	  This driver is also available as a module ( = code which can be
	  inserted and removed from the running kernel whenever you want). The
	  module will be called w100fb. If you want to compile it as a module,
	  say M here and read <file:Documentation/kbuild/modules.txt>.

	  If unsure, say N.

config FB_SH_MOBILE_LCDC
	tristate "SuperH Mobile LCDC framebuffer support"
	depends on FB && SUPERH
	select FB_SYS_FILLRECT
	select FB_SYS_COPYAREA
	select FB_SYS_IMAGEBLIT
	select FB_SYS_FOPS
	select FB_DEFERRED_IO
	---help---
	  Frame buffer driver for the on-chip SH-Mobile LCD controller.

config FB_TMIO
	tristate "Toshiba Mobile IO FrameBuffer support"
	depends on FB && MFD_CORE
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	---help---
	  Frame buffer driver for the Toshiba Mobile IO integrated as found
	  on the Sharp SL-6000 series

	  This driver is also available as a module ( = code which can be
	  inserted and removed from the running kernel whenever you want). The
	  module will be called tmiofb. If you want to compile it as a module,
	  say M here and read <file:Documentation/kbuild/modules.txt>.

	  If unsure, say N.

config FB_TMIO_ACCELL
	bool "tmiofb acceleration"
	depends on FB_TMIO
	default y

config FB_S3C
	tristate "Samsung S3C framebuffer support"
	depends on FB && ARCH_S3C64XX
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	---help---
	  Frame buffer driver for the built-in FB controller in the Samsung
	  SoC line from the S3C2443 onwards, including the S3C2416, S3C2450,
	  and the S3C64XX series such as the S3C6400 and S3C6410.

	  These chips all have the same basic framebuffer design with the
	  actual capabilities depending on the chip. For instance the S3C6400
	  and S3C6410 support 4 hardware windows whereas the S3C24XX series
	  currently only have two.

	  Currently the support is only for the S3C6400 and S3C6410 SoCs.

config FB_S3C_DEBUG_REGWRITE
       bool "Debug register writes"
       depends on FB_S3C
       ---help---
         Show all register writes via printk(KERN_DEBUG)

config FB_S3C2410
	tristate "S3C2410 LCD framebuffer support"
	depends on FB && ARCH_S3C2410
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	---help---
	  Frame buffer driver for the built-in LCD controller in the Samsung
	  S3C2410 processor.

	  This driver is also available as a module ( = code which can be
	  inserted and removed from the running kernel whenever you want). The
	  module will be called s3c2410fb. If you want to compile it as a module,
	  say M here and read <file:Documentation/kbuild/modules.txt>.

	  If unsure, say N.
config FB_S3C2410_DEBUG
	bool "S3C2410 lcd debug messages"
	depends on FB_S3C2410
	help
	  Turn on debugging messages. Note that you can set/unset at run time
	  through sysfs

config FB_SM501
	tristate "Silicon Motion SM501 framebuffer support"
	depends on FB && MFD_SM501
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	---help---
	  Frame buffer driver for the CRT and LCD controllers in the Silicon
	  Motion SM501.

	  This driver is also available as a module ( = code which can be
	  inserted and removed from the running kernel whenever you want). The
	  module will be called sm501fb. If you want to compile it as a module,
	  say M here and read <file:Documentation/kbuild/modules.txt>.

	  If unsure, say N.


config FB_PNX4008_DUM
	tristate "Display Update Module support on Philips PNX4008 board"
	depends on FB && ARCH_PNX4008
	---help---
	  Say Y here to enable support for PNX4008 Display Update Module (DUM)

config FB_PNX4008_DUM_RGB
	tristate "RGB Framebuffer support on Philips PNX4008 board"
	depends on FB_PNX4008_DUM
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	---help---
	  Say Y here to enable support for PNX4008 RGB Framebuffer

config FB_STMP37XX
	tristate "STMP 37XX LCD Framebuffer driver"
	depends on FB && (ARCH_STMP37XX || ARCH_STMP378X)
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	---help---
	  Say Y here to enable support for the framebuffer driver for the
	  Sigmatel STMP37XX board.

config FB_IBM_GXT4500
	tristate "Framebuffer support for IBM GXT4500P adaptor"
	depends on FB && PPC
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	---help---
	  Say Y here to enable support for the IBM GXT4500P display
	  adaptor, found on some IBM System P (pSeries) machines.

config FB_PS3
	tristate "PS3 GPU framebuffer driver"
	depends on FB && PS3_PS3AV
	select FB_SYS_FILLRECT
	select FB_SYS_COPYAREA
	select FB_SYS_IMAGEBLIT
	select FB_SYS_FOPS
	select VT_HW_CONSOLE_BINDING if FRAMEBUFFER_CONSOLE
	---help---
	  Include support for the virtual frame buffer in the PS3 platform.

config FB_PS3_DEFAULT_SIZE_M
	int "PS3 default frame buffer size (in MiB)"
	depends on FB_PS3
	default 9
	---help---
	  This is the default size (in MiB) of the virtual frame buffer in
	  the PS3.
	  The default value can be overridden on the kernel command line
	  using the "ps3fb" option (e.g. "ps3fb=9M");

config FB_XILINX
	tristate "Xilinx frame buffer support"
	depends on FB && (XILINX_VIRTEX || MICROBLAZE)
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	---help---
	  Include support for the Xilinx ML300/ML403 reference design
	  framebuffer. ML300 carries a 640*480 LCD display on the board,
	  ML403 uses a standard DB15 VGA connector.

config FB_COBALT
	tristate "Cobalt server LCD frame buffer support"
	depends on FB && MIPS_COBALT

config FB_SH7760
	bool "SH7760/SH7763/SH7720/SH7721 LCDC support"
	depends on FB && (CPU_SUBTYPE_SH7760 || CPU_SUBTYPE_SH7763 \
		|| CPU_SUBTYPE_SH7720 || CPU_SUBTYPE_SH7721)
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	---help---
	  Support for the SH7760/SH7763/SH7720/SH7721 integrated
	  (D)STN/TFT LCD Controller.
	  Supports display resolutions up to 1024x1024 pixel, grayscale and
	  color operation, with depths ranging from 1 bpp to 8 bpp monochrome
	  and 8, 15 or 16 bpp color; 90 degrees clockwise display rotation for
	  panels <= 320 pixel horizontal resolution.

config FB_VIRTUAL
	tristate "Virtual Frame Buffer support (ONLY FOR TESTING!)"
	depends on FB
	select FB_SYS_FILLRECT
	select FB_SYS_COPYAREA
	select FB_SYS_IMAGEBLIT
	select FB_SYS_FOPS
	---help---
	  This is a `virtual' frame buffer device. It operates on a chunk of
	  unswappable kernel memory instead of on the memory of a graphics
	  board. This means you cannot see any output sent to this frame
	  buffer device, while it does consume precious memory. The main use
	  of this frame buffer device is testing and debugging the frame
	  buffer subsystem. Do NOT enable it for normal systems! To protect
	  the innocent, it has to be enabled explicitly at boot time using the
	  kernel option `video=vfb:'.

	  To compile this driver as a module, choose M here: the
	  module will be called vfb. In order to load it, you must use
	  the vfb_enable=1 option.

	  If unsure, say N.

config XEN_FBDEV_FRONTEND
	tristate "Xen virtual frame buffer support"
	depends on FB && XEN
	select FB_SYS_FILLRECT
	select FB_SYS_COPYAREA
	select FB_SYS_IMAGEBLIT
	select FB_SYS_FOPS
	select FB_DEFERRED_IO
	default y
	help
	  This driver implements the front-end of the Xen virtual
	  frame buffer driver.  It communicates with a back-end
	  in another domain.

config FB_METRONOME
	tristate "E-Ink Metronome/8track controller support"
	depends on FB
	select FB_SYS_FILLRECT
	select FB_SYS_COPYAREA
	select FB_SYS_IMAGEBLIT
	select FB_SYS_FOPS
	select FB_DEFERRED_IO
	help
	  This driver implements support for the E-Ink Metronome
	  controller. The pre-release name for this device was 8track
	  and could also have been called by some vendors as PVI-nnnn.

config FB_MB862XX
	tristate "Fujitsu MB862xx GDC support"
	depends on FB
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	---help---
	  Frame buffer driver for Fujitsu Carmine/Coral-P(A)/Lime controllers.

config FB_MB862XX_PCI_GDC
	bool "Carmine/Coral-P(A) GDC"
	depends on PCI && FB_MB862XX
	---help---
	  This enables framebuffer support for Fujitsu Carmine/Coral-P(A)
	  PCI graphics controller devices.

config FB_MB862XX_LIME
	bool "Lime GDC"
	depends on FB_MB862XX
	depends on OF && !FB_MB862XX_PCI_GDC
	depends on PPC
	select FB_FOREIGN_ENDIAN
	select FB_LITTLE_ENDIAN
	---help---
	  Framebuffer support for Fujitsu Lime GDC on host CPU bus.

config FB_PRE_INIT_FB
	bool "Don't reinitialize, use bootloader's GDC/Display configuration"
	depends on FB_MB862XX_LIME
	---help---
	  Select this option if display contents should be inherited as set by
	  the bootloader.

config FB_MX3
	tristate "MX3 Framebuffer support"
	depends on FB && MX3_IPU
	select FB_CFB_FILLRECT
	select FB_CFB_COPYAREA
	select FB_CFB_IMAGEBLIT
	default y
	help
	  This is a framebuffer device for the i.MX31 LCD Controller. So
	  far only synchronous displays are supported. If you plan to use
	  an LCD display with your i.MX31 system, say Y here.

config FB_BROADSHEET
	tristate "E-Ink Broadsheet/Epson S1D13521 controller support"
	depends on FB
	select FB_SYS_FILLRECT
	select FB_SYS_COPYAREA
	select FB_SYS_IMAGEBLIT
	select FB_SYS_FOPS
	select FB_DEFERRED_IO
	help
	  This driver implements support for the E-Ink Broadsheet
	  controller. The release name for this device was Epson S1D13521
	  and could also have been called by other names when coupled with
	  a bridge adapter.

source "drivers/video/omap/Kconfig"

source "drivers/video/backlight/Kconfig"
source "drivers/video/display/Kconfig"

if VT
	source "drivers/video/console/Kconfig"
endif

if FB || SGI_NEWPORT_CONSOLE
	source "drivers/video/logo/Kconfig"
endif

endmenu
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       ŒÄrÿ…Àt<¶Hƒùu3ƒx@@ v‹ · ƒøu ‰t$p‹„$”   ‰D$D$p‰$è tÿé·  …ÿ„ù   ‰4$ÇD$    è4Ärÿ…Àt<¶Hƒùu+ƒx@@ v‹ · ƒø)u‰t$h‹„$”   ‰D$D$hé_  …ÿ„©   ‰4$ÇD$    èäÃrÿ…Àt<¶Hƒùu3ƒx@@ v‹ · ƒø(u ‰t$`‹„$”   ‰D$D$`‰$èiŸtÿé  …ÿtU‰4$ÇD$    èÃrÿ…ÀtA¶Hƒùu8ƒx@@ v‹ ‹Œ$”   · ƒø$tƒø;u‰t$X‰L$D$X‰$è Ÿtÿé¶  ‰÷‰,$èásÿ„Àt ‰|$P‹„$”   ‰D$D$P‰$èŸtÿéˆ  1ö…ÿ„¼   ‹w…ö„¯   ‰<$ÇD$    èøÂrÿ…ÀtK¶Hƒùu>ƒx@@ v‹ · Hƒøw*¹ T £Ás ‰|$H‹„$”   ‰D$D$H‰$è¢tÿé  …ötP‰<$ÇD$    è™Ârÿ…Àt<¶Hƒùu3ƒx@@ ‹Œ$”   v‹ · ƒø.u‰|$@‰L$D$@‰$è^tÿéÄ  ‰ş‰,$ètÿ„Àt ‰t$8‹„$”   ‰D$D$8‰$è@tÿé–  …ö„  ‹~…ÿ„\  ‰4$ÇD$    èÂrÿ…ÀtK¶Hƒùu:ƒx@@ v‹ ¹şÿ  #ñ   u ‰t$0‹„$”   ‰D$D$0‰$èætÿé,  …ÿ„ı   ‰4$ÇD$    è©Árÿ…Àt>¶Hƒùu5ƒx@@ v‹ · = B  u ‰t$(‹„$”   ‰D$D$(‰$èœtÿéÒ   …ÿ„£   ‰4$ÇD$    èOÁrÿ…Àt=¶Hƒùu0ƒx@@ v‹ · ƒø9u‰t$ ‹„$”   ‰D$D$ ‰$èTtÿë}…ÿtR‰4$ÇD$    èşÀrÿ…Àt>¶Hƒùu5ƒx@@ v‹ · ƒø8ƒø(ƒøt$ƒøuëƒø9të
ƒø)tƒø.t‰,$èôsÿ„Àt‰t$‹„$”   ‰D$D$‰$èåœtÿƒÄ|^_[]Ã             USWVƒìè    [Ã\`S ‹l$4‹D$0‹ 1Ò…À¿    „Œ   1Òƒx¿    r‰$ÇD$   è7Àrÿ1Ò…À¿    td1Ò¶Hƒù¿    uT‹Pƒú@w7‹p ‹x$¹@   )Ñ‰ğÓà¥÷1ÒöÁ EøDĞ­ú‰øÓøöÁ EĞÁÿöÁ Døë‹@ ‹‹x‰Ğ!øƒøÿ„‰  ‰|$‰T$³¥îÉÿ‰4$ÇD$   èš]rÿ‰Ç‹M‹E)Á9Ïv‰|$‰t$‰,$èŒ]rÿ‰Åë‰|$‰t$‰$èÈ]rÿ}‹D$0‹ 1Ò…À¿    t1Òƒx¿    rr‰$ÇD$   èG¿rÿ1Ò…À¿    tW1Ò¶Hƒù¿    uG‹Pƒú@w7‹p ‹x$¹@   )Ñ‰ğÓà¥÷1ÒöÁ EøDĞ­ú‰øÓøöÁ EĞÁÿöÁ Døë‹@ ‹‹x‰|$‰T$‰,$è5]rÿ‰Æ«ÙSÉÿ‰,$ÇD$   è­\rÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$èŸ\rÿ‰Æë‰|$‰l$‰$èÛ\rÿ~‹D$ƒÀÿ‹L$ƒÑÿ‰D$‰4$‰L$èÊ\rÿ‹H;HsQ‰PÆ]ë‰$ÇD$]   èg\rÿƒÄ^_[]Ã‰î«x|Ëÿ‰,$ÇD$   è\rÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$è	\rÿëÀ‰|$‰l$‰$èG\rÿ~ë«  USWVƒìè    [ÃÌ]S ‹l$4‹D$0‰l$‰$èštÿ»¥îÉÿ‰<$ÇD$   è£[rÿ‰Æ‹M‹E)Á9Îv‰t$‰|$‰,$‰îè“[rÿë‰t$‰|$‰$èÑ[rÿu‰î‹l$0‹E 1ÿ…Àt4ƒx‰õr.‰$ÇD$   èW½rÿ…Àt¶Hƒùuƒx@@ v‹ ‹8ë‰õ‰<$è‘™tÿ‰Æ…öt0‰4$ÇD$ÿÿÿÿè[rÿ‰Ç‹M‹E)Á9ÏvL‰|$‰t$‰,$èıZrÿë|‰|$»…|Ëÿ‰<$ÇD$   èÑZrÿ‰Æ‹M‹E)Á9Îv'‰t$‰|$‰,$èÃZrÿë*‰|$‰t$‰$è[rÿ}ë-‰t$‰|$‰$èìZrÿu‰è‹L$‰L$‰$ÇD$    è¿jtÿ‹E;EsH‰MÆ ]ë‰,$ÇD$]   è|ZrÿƒÄ^_[]Ã    USWVƒì,è    [Ã,\S ‹t$D«¥îÉÿ‰,$ÇD$   èZrÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$èZrÿ‰Æë‰|$‰l$‰$èAZrÿ~‹D$@‰D$D$ ‰$è:ÿrÿƒì‹D$ ‹|$$‹V‹N)Ê9×v‰|$‰D$‰4$è³Yrÿ‰Æë‰|$‰D$‰$èïYrÿ~« LÊÿ‰,$ÇD$   èvYrÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$èhYrÿ‰Æë‰|$‰l$‰$è¤Yrÿ~‹D$@‰D$D$‰$èşrÿƒì‹D$‹|$‹V‹N)Ê9×v‰|$‰D$‰4$èYrÿ‰Æ‹~ë‰|$‰D$‰$èOYrÿ~‰~;~sG‰FÆ]ë‰4$ÇD$]   èùXrÿƒÄ,^_[]Ã USWVƒìè    [Ã¬ZS ‹t$4«¥îÉÿ‰,$ÇD$   è“Xrÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$è…Xrÿ‰Æë‰|$‰l$‰$èÁXrÿ~‹D$0‹ …Àt@ƒxr:‰$ÇD$   èNºrÿ…Àt&¶Hƒùu‰D$D$‰$è¡arÿƒì‹D$‹|$ëÇD$    ÇD$    1ÿ1À‹V‹N)Ê9×v‰|$‰D$‰4$èôWrÿ‰Æë‰|$‰D$‰$è0Xrÿ~«|Ëÿ‰,$ÇD$   è·Wrÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$è©Wrÿ‰Æë‰|$‰l$‰$èåWrÿ~‹D$0‹ 1Ò…À¿    t1Òƒx¿    rr‰$ÇD$   èd¹rÿ1Ò…À¿    tW1Ò¶Hƒù¿    uG‹Pƒú@w7‹h ‹x$¹@   )Ñ‰èÓà¥ï1ÒöÁ EøDĞ­ú‰øÓøöÁ EĞÁÿöÁ Døë‹@ ‹‹x‰|$‰T$‰4$èRWrÿ‹H;HsQ‰PÆ]ë‰$ÇD$]   èïVrÿƒÄ^_[]Ã       USWVƒì,è    [ÃœXS ‹D$@‹ …À„ÿ
  ‹t$Dƒx‚‰   ‰$ÇD$   è¸rÿ…Àtu¶Hƒùul‰D$D$ ‰$èÔ_rÿƒìƒ|$$ tb»¥îÉÿ‰<$ÇD$   è4Vrÿ‰õ‰ÆE‰D$U‹M‹E)Á9Î†ã  ‰T$‰t$‰|$‰,$èVrÿ‰Æ‹Féã  ÇD$     ÇD$$    nF‰D$‰l$»’|Ëÿ‰<$ÇD$   èÄUrÿ‰Æ‹M ‹l$‹E )Á9Îv‰t$‰|$‹D$D‰$è®Urÿ‰Æë‰t$‰|$‰$èêUrÿu ‹t$D‹D$@‹ 1ÿ…Àt0ƒxr*‰$ÇD$   èq·rÿ…Àt¶Hƒùuƒx@@ v‹ ‹8‰|$‰4$è[Vrÿ‰Æ«š|Ëÿ‰,$ÇD$   è#Urÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$èUrÿ‰Æë‰|$‰l$‰$èQUrÿ~‹D$@‹ 1É…Àº    tH1Éƒxº    r;‰$ÇD$   èĞ¶rÿ1É…Àº    t 1É¶Pƒúº    uƒx@@ v‹ ‹‹P‰T$‰L$‰4$èÕ¥rÿ‰Æ«xqËÿ‰,$ÇD$	   èmTrÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$è_Trÿ‰Æë‰|$‰l$‰$è›Trÿ~‹D$@‹ 1É…Àº    tH1Éƒxº    r;‰$ÇD$   è¶rÿ1É…Àº    t 1É¶Pƒúº    uƒx@@ v‹ ‹‹P‰T$‰L$‰4$è¥rÿ‰Æ«¢|Ëÿ‰,$ÇD$
   è·Srÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$è©Srÿ‰Æë‰|$‰l$‰$èåSrÿ~‹D$@‹ 1É…Àº    tH1Éƒxº    r;‰$ÇD$   èdµrÿ1É…Àº    t 1É¶Pƒúº    uƒx@@ v‹ ‹‹P‰T$‰L$‰4$èi¤rÿ‹D$@‹0…ö„Ü  ‹~…ÿ„Ñ  ‰4$ÇD$    è µrÿ…À„¹  ¶Hƒù…¬  ƒx@@ v‹ · ƒø$t	ƒø;…  1íƒÿ
r*‰4$ÇD$	   è¸´rÿ…Àt¶Hƒùuƒx@@ v‹ ‹(‰,$è‘tÿ‰Æ…ö„M  «¬|Ëÿ‰,$ÇD$   èfRrÿ‰Ç‹D$‹‹T$‹)Á9Ï†Ã   ‰|$‰l$‹D$D‰$èJRrÿ‰ÇéÁ   ‰t$‰|$‰$‰×‰|$è}Rrÿ‹ğ‰‰î‹L$ ‹|$$‹V)Â9×v‰|$‰L$‰4$èRrÿ‰Æë‰|$‰L$‰$è@Rrÿ~«ÜİÉÿ‰,$ÇD$   èÇQrÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$è¹Qrÿë‰|$‰l$‰$è÷Qrÿ~‹l$é­ûÿÿ‰|$‰l$‰$‰ÕèÙQrÿ} ‹|$D‰4$ÇD$ÿÿÿÿèbQrÿ‰Å‹O‹G)Á9Ív‰l$‰t$‰<$èTQrÿë‰l$‰t$‰$è’Qrÿo»ÜİÉÿ‰<$ÇD$   èQrÿ‰Æ‹D$‹‹l$‹E )Á9Îv‰t$‰|$‹D$D‰$è Qrÿë‰t$‰|$‰$è>Qrÿu ‹D$@‹0…ö„=  ‹~ƒÿ	‚Â   ‰4$ÇD$   èÁ²rÿ…Àt1¶Hƒùu#ƒx@@ v‹ ö t»³|Ëÿ‰<$ÇD$   ëCƒÿ	r}‰4$ÇD$   è|²rÿ…Àti¶Hƒùu`ƒx@@ v‹ ö tP»¾|Ëÿ‰<$ÇD$   è9Prÿ‰Æ‹D$‹‹E )Á9Îv‰t$‰|$‹D$D‰$è$Prÿë‰t$‰|$‰$èbPrÿu ‹D$@‹ …À„a  ƒx	r}‰$ÇD$   èë±rÿ…Àti¶Hƒùu`ƒx@@ v‹ ö @tP»Ë|Ëÿ‰<$ÇD$   è¨Orÿ‰Æ‹D$‹‹E )Á9Îv‰t$‰|$‹D$D‰$è“Orÿë‰t$‰|$‰$èÑOrÿu ‹D$@‹0…ö„Ğ  ƒ~	rK‰4$ÇD$   èZ±rÿ…Àt/¶Hƒùu&ƒx@@ v‹ ö t»Ù|Ëÿ‰<$ÇD$   é   …ö„7  ‹~…ÿ„,  ‰4$ÇD$    è±rÿ…Àt(¶Hƒùuƒx@@ v‹ · ƒø„¨   …ÿ„ğ   ‰4$ÇD$    èÈ°rÿ…Àt¶Hƒùuƒx@@ v‹ · ƒøtp…ÿ„¸   ‰4$ÇD$    è°rÿ…Àt$¶Hƒùuƒx@@ v‹ · ƒøt8…ÿ„€   ‰4$ÇD$    èX°rÿ…Àtl¶Hƒùucƒx@@ v‹ · ƒøuP»á|Ëÿ‰<$ÇD$   èNrÿ‰Æ‹D$‹‹E )Á9Îv‰t$‰|$‹D$D‰$èıMrÿë‰t$‰|$‰$è;Nrÿu ‹D$@‹ …À„:  ƒx	r~‰$ÇD$   èÄ¯rÿ…Àtj¶Hƒùuaƒx@@ v‹ ö@tP»è|Ëÿ‰<$ÇD$
   è€Mrÿ‰Æ‹D$‹‹E )Á9Îv‰t$‰|$‹D$D‰$èkMrÿë‰t$‰|$‰$è©Mrÿu ‹D$@‹ …À„¨  ƒx	r~‰$ÇD$   è2¯rÿ…Àtj¶Hƒùuaƒx@@ v‹ ö@tP»ò|Ëÿ‰<$ÇD$
   èîLrÿ‰Æ‹D$‹‹E )Á9Îv‰t$‰|$‹D$D‰$èÙLrÿë‰t$‰|$‰$èMrÿu ‹D$@‹ …À„  ƒx	r~‰$ÇD$   è ®rÿ…Àtj¶Hƒùuaƒx@@ v‹ ö@@tP»ü|Ëÿ‰<$ÇD$   è\Lrÿ‰Æ‹D$‹‹E )Á9Îv‰t$‰|$‹D$D‰$èGLrÿë‰t$‰|$‰$è…Lrÿu ‹D$@‹ …À„„   ƒx	r~‰$ÇD$   è®rÿ…Àtj¶Hƒùuaƒx@@ v‹ ö@€tP»	}Ëÿ‰<$ÇD$   èÊKrÿ‰Æ‹D$‹‹E )Á9Îv‰t$‰|$‹D$D‰$èµKrÿë‰t$‰|$‰$èóKrÿu ƒÄ,^_[]Ã        USWVƒìè    [ÃlMS ‹t$4‹D$0‰t$‰$è9‰tÿ«}Ëÿ‰,$ÇD$   èCKrÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$è5Krÿ‰Æë‰|$‰l$‰$èqKrÿ~‹D$0‹ …Àtcƒx
r]‰$ÇD$	   èş¬rÿ…ÀtI¶Hƒù‰Át1É…Ét#ƒyr2‰$ÇD$   èÓ¬rÿ…Àt¶Hƒùu‰D$D$‰$è&TrÿƒìëÇD$    ÇD$    ‹D$‹|$‹V‹N)Ê9×v‰|$‰D$‰4$è}Jrÿ‰Æ‹~ë‰|$‰D$‰$è¶Jrÿ~‰~;~sG‰FÆ]ë‰4$ÇD$]   è`JrÿƒÄ^_[]Ã        USWVƒìè    [ÃLS ‹t$4‹|$0‰t$‰<$èÙ‡tÿ‹…Àt3ƒxr7ÇD$    ‰$ÇD$
   èå«rÿ…Àt#¶Hƒùt1À‰D$ëÇD$    ëÇD$    «¥îÉÿ‰,$ÇD$   è˜Irÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$èŠIrÿ‰Æë‰|$‰l$‰$èÆIrÿ~‹L$1À…Ét‹A‰D$‰4$èjJrÿ‰Æ«%}Ëÿ‰,$ÇD$   è2Irÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$è$Irÿë‰|$‰l$‰$èbIrÿ~ƒÄ^_[]Ã       USWVƒìè    [ÃÜJS ‹l$4³’|Ëÿ‰4$ÇD$   èÃHrÿ‰Ç‹M‹E)Á9Ïv‰|$‰t$‰,$èµHrÿ‰Çë‰|$‰t$‰$èñHrÿ}‰ï‹D$0‹ 1ö…Àt0ƒxr*‰$ÇD$   èzªrÿ…Àt¶Hƒùuƒx@@ v‹ ‹0‰t$‰<$èdIrÿ‹H;HsQ‰PÆ]ë‰$ÇD$]   èQHrÿ‹D$0‹ …À„ç  ƒx	rv‰$ÇD$   èªrÿ…Àtb¶HƒùuYƒx@@ v‹ ƒ8 tI³0}Ëÿ‰4$ÇD$	   èÊGrÿ‰Ç‹M‹E)Á9Ïv‰|$‰t$‰,$è¼Grÿë‰|$‰t$‰$èúGrÿ}‹D$0‹ …À„]  ƒx
rv‰$ÇD$	   èƒ©rÿ…Àtb¶HƒùuYƒx@@ v‹ ƒ8 tI³á|Ëÿ‰4$ÇD$   è@Grÿ‰Ç‹M‹E)Á9Ïv‰|$‰t$‰,$è2Grÿë‰|$‰t$‰$èpGrÿ}‹D$0‹8…ÿ„Ó  ‹wƒş½    r.‰<$ÇD$   èò¨rÿ1í…Àt1í¶Hƒùuƒx@@ v‹ ‹(ƒşr,‰<$ÇD$   è¿¨rÿ…Àt¶Hƒùuƒx@@ v‹ ‹ ë
1Àë1Àë1À9Å„ä   ³9}Ëÿ‰4$ÇD$	   èkFrÿ‰Ç‹l$4‹M‹E)Á9Ïv‰|$‰t$‰,$èYFrÿ‰Çë‰|$‰t$‰$è•Frÿ}‰ï‹D$0‹ 1ö…Àt0ƒxr*‰$ÇD$   è¨rÿ…Àt¶Hƒùuƒx@@ v‹ ‹0‰t$‰<$èGrÿ‰Ç³ÜİÉÿ‰4$ÇD$   èĞErÿ‰Å‹O‹G)Á9Ív‰l$‰t$‰<$èÂErÿë‰l$‰t$‰$è Frÿo‹D$0‹8…ÿ‹l$4„_  ‹wƒş‚»   ‰<$ÇD$   è§rÿ…Àt1¶Hƒùu#ƒx@@ v‹ ö t³³|Ëÿ‰4$ÇD$   ëCƒşrv‰<$ÇD$   è:§rÿ…Àtb¶HƒùuYƒx@@ v‹ ö tI³¾|Ëÿ‰4$ÇD$   è÷Drÿ‰Ç‹M‹E)Á9Ïv‰|$‰t$‰,$èéDrÿë‰|$‰t$‰$è'Erÿ}‹D$0‹ …À„Š  ƒxrw‰$ÇD$   è°¦rÿ…Àtc¶HƒùuZƒx@@ v‹ ö@@tI³ü|Ëÿ‰4$ÇD$   èlDrÿ‰Ç‹M‹E)Á9Ïv‰|$‰t$‰,$è^Drÿë‰|$‰t$‰$èœDrÿ}‹D$0‹ …À„ÿ   ƒxrw‰$ÇD$   è%¦rÿ…Àtc¶HƒùuZƒx@@ v‹ ö@€tI³	}Ëÿ‰4$ÇD$   èáCrÿ‰Ç‹M‹E)Á9Ïv‰|$‰t$‰,$èÓCrÿë‰|$‰t$‰$èDrÿ}‹D$0‹ …Àtxƒxrr‰$ÇD$   è¥rÿ…Àt^¶HƒùuU‰D$D$‰$èñLrÿƒìƒ|$ tK³¥îÉÿ‰4$ÇD$   èQCrÿ‰Ç‹M‹E)Á9Ïv/‰|$‰t$‰,$èCCrÿ‰Å‹Eë0ÇD$    ÇD$    ƒÄ^_[]Ã‰|$‰t$‰$èdCrÿ‹Eø‰E‹L$‹t$‹U)Â9Öv‰t$‰L$‰,$èëBrÿ‰Å‹uë‰t$‰L$‰$è$Crÿu‰u;usF‰EÆ]ë’‰,$ÇD$]   èÎBrÿë€            USWVƒì,è    [Ã|DS ‹l$D‹D$@‹ …À„™   ƒx‚   ‰$ÇD$   èa¤rÿ…Àt{¶Hƒùur‰D$D$ ‰$è´Krÿƒìƒ|$$ thƒ¥îÉÿ‰D$‰$ÇD$   èBrÿ‰ÆU}‹M‹E)Á9Î†Ÿ  ‰T$‰|$‰t$‹D$‰D$‰,$èìArÿ‰Æ‹Fé  ÇD$     ÇD$$    }E‰D$«’|Ëÿ‰,$ÇD$   è¢Arÿ‰Æ‹‹D$‹ )Á9Îv ‰|$‰t$‰l$‹D$D‰$èŠArÿ‰Æ‹l$ë ‰|$‰t$‰l$‰$è¾Arÿ‹D$0‹t$D‰Å‹D$@‹ 1ÿ…Àt0ƒxr*‰$ÇD$   è@£rÿ…Àt¶Hƒùuƒx@@ v‹ ‹8‰|$‰4$è*Brÿ‹H;HsQ‰PÆ]ë‰$ÇD$]   èArÿ‹D$@‹ …À„Æ  ƒx
‚Ü   ‰$ÇD$	   èÏ¢rÿ…À„Ä   ¶Hƒù…·   ƒx@@ v‹ ƒ8 „£   »0}Ëÿ‰<$ÇD$	   è€@rÿ‰Æ‹D$‹‹E )Á9Îvi‰t$‰|$‹D$D‰$èk@rÿëf‰T$‰t$‹L$‰L$‰$è¡@rÿ‹ğ‰‰|$‰î‹L$ ‹|$$‹V)Â9×†»   ‰|$‰L$‰4$è @rÿ‰Æ‹~é·   ‰t$‰|$‰$èV@rÿu ‹D$@‹ …À„Ò   ƒx‚È   ‰$ÇD$
   èÛ¡rÿ…À„°   ¶Hƒù…£   ƒx@@ v‹ ƒ8 „   »á|Ëÿ‰<$ÇD$   èŒ?rÿ‰Æ‹D$‹‹E )Á9ÎvU‰t$‰|$‹D$D‰$èw?rÿëR‰|$‰L$‰$èµ?rÿ~‰~;~sG‰FÆ]ë‰4$ÇD$]   è_?rÿ‹|$énıÿÿ‰t$‰|$‰$èv?rÿu ƒÄ,^_[]Ã           USWVƒì,è    [Ãì@S ‹t$D‹D$@‹ …À„•   ƒx‚‹   ‰$ÇD$   èÑ rÿ…Àtw¶Hƒùun‰D$D$ ‰$è$Hrÿƒìƒ|$$ tdƒ¥îÉÿ‰D$‰$ÇD$   è€>rÿ‰ÇF‰D$n‹N‹F)Á9Ï†  ‰|$‹D$‰D$‰4$è`>rÿ‰Æ‹Fé  ÇD$     ÇD$$    F‰D$v«’|Ëÿ‰,$ÇD$   è>rÿ‰Ç‹D$‹‹)Á9Ïv‰|$‰l$‹D$D‰$è>rÿ‰Æë‰|$‰l$‰$è>>rÿ>‹t$D‹D$@‹ 1ÿ…Àt5ƒxr/‰$ÇD$   èÆŸrÿ…Àt¶Hƒùuƒx@@ v‹ ¿ÿÿÿ #8‰|$‰4$è«>rÿ‹H;HsQ‰PÆ]ë‰$ÇD$]   è˜=rÿƒÄ,^_[]Ã‰|$‹L$‰L$‰$è¬=rÿ‹E ø‰E ‹L$ ‹|$$‹V)Â9×v‰|$‰L$‰4$è3=rÿ‰Æ‹~ë‰|$‰L$‰$èl=rÿ~‰~;~sG‰FÆ]ë‰4$ÇD$]   è=rÿ‰îé³şÿÿ               USWVƒì,è    [Ã¼>S ‹t$D‹D$@‹ …À„•   ƒx‚‹   ‰$ÇD$   è¡rÿ…Àtw¶Hƒùun‰D$D$ ‰$èôErÿƒìƒ|$$ tdƒ¥îÉÿ‰D$‰$ÇD$   èP<rÿ‰ÇF‰D$n‹N‹F)Á9Ï†”  ‰|$‹D$‰D$‰4$è0<rÿ‰Æ‹Fé’  ÇD$     ÇD$$    F‰D$v«’|Ëÿ‰,$ÇD$   èæ;rÿ‰Ç‹D$‹‹)Á9Ïv‰|$‰l$‹D$D‰$èÒ;rÿ‰Æë‰|$‰l$‰$è<rÿ>‹t$D‹D$@‹ 1ÿ…Àt0ƒxr*‰$ÇD$   è–rÿ…Àt¶Hƒùuƒx@@ v‹ ‹8‰|$‰4$è€<rÿ‰Æ«B}Ëÿ‰,$ÇD$   èH;rÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$è:;rÿ‰Æë‰|$‰l$‰$èv;rÿ~‹D$@‹ 1ÿ…Àt0ƒxr*‰$ÇD$   èrÿ…Àt¶Hƒùuƒx@@ v‹ ‹8‰|$‰4$èë;rÿ‹H;HsQ‰PÆ]ë‰$ÇD$]   èØ:rÿƒÄ,^_[]Ã‰|$‹L$‰L$‰$èì:rÿ‹E ø‰E ‹L$ ‹|$$‹V)Â9×v‰|$‰L$‰4$ès:rÿ‰Æ‹~ë‰|$‰L$‰$è¬:rÿ~‰~;~sG‰FÆ]ë‰4$ÇD$]   èV:rÿ‰îé#şÿÿ               USWVƒì,è    [Ãü;S ‹t$D‹D$@‹ …À„•   ƒx‚‹   ‰$ÇD$   èá›rÿ…Àtw¶Hƒùun‰D$D$ ‰$è4Crÿƒìƒ|$$ tdƒ¥îÉÿ‰D$‰$ÇD$   è9rÿ‰ÇF‰D$n‹N‹F)Á9Ï†ÿ   ‰|$‹D$‰D$‰4$èp9rÿ‰Æ‹Féı   ÇD$     ÇD$$    F‰D$v«’|Ëÿ‰,$ÇD$   è&9rÿ‰Ç‹D$‹‹)Á9Ïv‰|$‰l$‹D$D‰$è9rÿ‰Æë‰|$‰l$‰$èN9rÿ>‹t$D‹D$@‹ 1ÿ…Àt0ƒxr*‰$ÇD$   èÖšrÿ…Àt¶Hƒùuƒx@@ v‹ ‹8‰|$‰4$èÀ9rÿ‹H;HsQ‰PÆ]ë‰$ÇD$]   è­8rÿƒÄ,^_[]Ã‰|$‹L$‰L$‰$èÁ8rÿ‹E ø‰E ‹L$ ‹|$$‹V)Â9×v‰|$‰L$‰4$èH8rÿ‰Æ‹~ë‰|$‰L$‰$è8rÿ~‰~;~sG‰FÆ]ë‰4$ÇD$]   è+8rÿ‰îé¸şÿÿ    USWVƒì<è    [ÃÜ9S ‹|$T‹l$P‹E ‰$èj<rÿ‰Æ‹E …Àtpƒxrj‰$ÇD$   è»™rÿ…ÀtV¶HƒùuM‰D$D$0‰$èArÿƒìƒ|$4 tC‰t$‹D$0‹t$4‹W‹O)Ê9Ö†Ã   ‰t$‰D$‰<$èh7rÿ‰ÆéÁ   ÇD$0    ÇD$4    ‹E …À„Ö  ƒx‚Ì  ‰$ÇD$   è,™rÿ…À„´  ¶Hƒù…§  ‰D$D$(‰$è×árÿƒìƒ|$, „‰  ‰t$ƒP}Ëÿ‰D$‰$ÇD$   èË6rÿ‰Æ‰ı‹M‹E)Á9Îvp‰t$‹D$‰D$‰,$è·6rÿëq‰t$‰D$‰$èõ6rÿw‰şƒä2Éÿ‰D$‰$ÇD$   èv6rÿ‹N‹V)Ñ9È†Š   ‰D$‹D$‰D$‰4$èb6rÿ‰Áé   ‰t$‹L$‰L$‰$è—6rÿu‹T$òD$(òD$ òD$ ò$‰éèâ   »ÜİÉÿ‰<$ÇD$   èü5rÿ‰Æ‹M‹E)Á9Îv{‰t$‰|$‰,$èî5rÿë|‰D$‹L$‰L$‰$‰D$è$6rÿ‰ñ‹D$A‹E 1ö…Àt^ƒxrX‰L$‰$ÇD$   è¨—rÿ…Àt<¶Hƒù‹L$u3ƒx@@ v‹ ¾ÿÿÿ #0ë‰t$‰|$‰$èÃ5rÿuƒÄ<^_[]Ã‹L$‰t$‰$èh6rÿ‹t$éşıÿÿ               USWVƒì,‰×‰Îè    [Ã7S ƒ|$D „  D$@‰|$‰$è>àrÿ…Àtcƒxr]‰$ÇD$   èô–rÿ…ÀtI¶Hƒùu@ƒx t:‰$ÇD$    èÑ–rÿ…Àt&¶Hƒùu‰D$D$ ‰$è$>rÿƒì‹D$ ‹l$$ëÇD$     ÇD$$    1í1À‹V‹N)Ê9Õv‰l$‰D$‰4$èw4rÿ‹në‰l$‰D$‰$è²4rÿn‰n;nsE‰FÆE :‰ğë‰4$ÇD$:   èY4rÿl$@‹M áÿÿÿ ‰L$‰$è05rÿ}    r5‹F;FsH‰NÆ :‰ğë‰4$ÇD$:   è4rÿ¶M‰L$‰$èò4rÿ‰|$‰,$è–ãsÿ‰D$D$‰$è–Şrÿƒìƒ|$ „¸   ‰|$»€}Ëÿ‰<$ÇD$   è3rÿ‰Å‹N‹F)Á9Ív‰l$‰|$‰4$è€3rÿë‰l$‰|$‰$è¾3rÿnòD$òD$òD$ò$‰ñ‹T$è	şÿÿ«%ùÉÿ‰,$ÇD$   è#3rÿ‰Ç‹N‹F)Á9Ïv‰|$‰l$‰4$è3rÿë‰|$‰l$‰$èS3rÿ~ƒÄ,^_[]Ã        ‹D$‹L$‰Ã     ‹D$‹L$‰Ã     Sƒìè    [Ã¯4S ‹D$‹1À…Ét‹T$9Qv‰T$‰$èŸ”rÿ‹L$‰ƒÄ[Â   Sƒìè    [Ão4S ‹D$‹1À…Ét‹T$9Qv‰T$‰$è_”rÿ‹L$‰ƒÄ[Â   USWVƒì<è    [Ã,4S ‹|$Pƒğ«Éÿ‰D$0‰$ÇD$   è2rÿ‰D$4òD$0òD$‰<$è'Ÿrÿ‰Æ…öu!ÇD$    ë,‰$è—rÿ‰$è6rÿ‹F…Àué‹‰4$ÿP°‰D$ƒT}Ëÿ‰D$(‰$ÇD$   è¡1rÿ‰D$,òD$(òD$‰<$è¹rÿ‰Æ…öuë&‰$è¸–rÿ‰$è06rÿ‹F…Àué‹‰4$ÿP°‰D$G(‰D$‹o09Å„   ‰ï‹o‰|$D$ ‰$èİ’rÿƒì³c}Ëÿ‰4$ÇD$
   è1rÿ9D$$r…Àt*‹L$ ‰D$‰t$‰$è†:rÿ…Àt;l$‰ïuªë ‰<$èhotÿ°‰D$;l$…ÿÿÿ‹|$P‹GƒÇÜ9øtS‹\$H(‹P@ë‹R9Êt)‹r ë‹v 9Ötéƒ~, tóÇF,    ÇF(    ³ëá‹@89øu¹ë‹\$€ã¶ÃƒÄ<^_[]Ã     SVƒì$è    [Ã>2S ‹t$0ƒm}Ëÿ‰D$‰$ÇD$   è!0rÿ‰D$òD$òD$‰4$è©ntÿ1É…Àtƒx@@ v‹ ‹‰ÈƒÄ$^[Ã              USWVƒìLè    [ÃÌ1S ‹L$`‹D$dÇA    Ç    ÇA    ÇA    ÆD$I‹*ºÉÿ‰L$@ÆD$HL$@‰L$‰$è&Èrÿ‰D$ …À„  ‹D$ ‰$è>‘rÿ‰D$…À„ı   1ö‰t$‹D$ ‰$è0‘rÿ…À„Ø   ƒx‚Î   ‰$ÇD$	   è>‘rÿ‰Â‰T$…Ò„°   ¶Bƒø…£   ‹B‰D$$…À„”   1í9jv{‰l$‰$‰×èırÿ‰ú‰Ç…ÿte¶Gƒøu\ƒrV‰<$ÇD$   èÖrÿ‹T$…Àt>¶Hƒùu5‰D$8‰|$<D$8‰D$‹D$`‰D$D$(‰$‰×è?mtÿƒì‰úE9l$$…uÿÿÿF;t$…ÿÿÿƒÄL^_[]Â            USWVƒìè    [Ã<0S ‹l$8‹|$4‹t$0‹G1Ò…À„   ‰\$‹‹u ‰Â‰ğÁè‰õÁí	1Å‰ĞPÿ‰T$!Õé‹é9ŞtB‰D$1À¿   ƒûüt?ƒûø•Ã…À•ÇßDÂı#l$é‹é9ŞuØë‰D$‹D$Á‹L$0‰‰AÆA ëA…ÀEĞ‹t$0‹|$4‹l$8‹\$‰T$‰l$‰<$è7ltÿ‹M ‰‹M‰H‹OÁá‰‰NÆFƒÄ^_[]Â  USWVƒìè    [Ã</S ‹|$4‹l$0‹M‹E   4@9òs÷ÑÁ+M‰ÂÁê9Ñ†   ‹D$8é  À‰D$‰,$èÆktÿ‹M1À…É„ê   ‹D$0‹‹7‰t$‰ğÁè‰óÁë	1ÃI‰L$!ËÚ‹<Ú9ş„½   1í¾   ƒÿü„£   ƒÿø•Á…í•ÅÍDèóv#\$Ú‹<Ú9|$uÒé~   ‰D$‰,$èAktÿ‹M1À…Éti‹D$0‹‹7‰t$‰ğÁè‰óÁë	1ÃI‰L$!ËÚ‹<Ú9şt@1í¾   ƒÿüt'ƒÿø•Á…í•ÅÍDèóv#\$Ú‹<Ú9|$uÖë…íEÅ‹L$0ÿAƒ8ütÿIƒÄ^_[]Ã        USWVƒìè    [ÃÌ-S ‹|$ ‹D$$‹7‹oH‰ÁÑé	Á‰ÈÁè	È‰ÁÁé	Á‰ÈÁè	È‰ÂÁê	ÂBƒú@¹@   GÊ‰O1À…ÉtÁá‰$è,rÿ‰…ötî‰D$‰t$‰<$èDjtÿ‰4$è<-rÿë,ÇG    ÇG    ‹O…ÉtÁá…ÀtÇ üÿÿÿƒÀƒÁøuîƒÄ^_[]Ã      USWVƒì‹D$Ç@    Ç@    ‹@…Àt"‹L$‹	Áà…ÉtÇüÿÿÿƒÁƒÀøuî‹L$$‹T$ ëƒÂ9Ê„±   ‹2‰ğƒÈƒøüté‰t$‹D$‹x1Û…ÿtx‹D$‹(‹L$‰ÈÁè‰ÎÁî	1ÆO‰<$!ş\õ ‹Dõ 9ÁtP¿   1Éƒøüt(ƒøø•À…É•ÄÄDËş#4$\õ ‹Dõ 9D$uÕë…ÉEÙ‹D$‰‹B‰C‹D$ÿ@‹L$$éDÿÿÿƒÄ^_[]Ã            SWVƒìè    [Ãí+S ‹t$ ‹F1ÿ…Àt'Hº    t½Ğƒò¹!   )Ñ¸   Óàƒø?¿@   Oø;~u9ÇF    ÇF    …ÿ„‡   ‹Áç…ÀtÇ üÿÿÿƒÀƒÇøuîëd‹‰$èR+rÿ‰~…ÿt?Áç‰<$è *rÿ‰ÇF    ÇF    ‹N…Ét1Áá…ÀtÇ üÿÿÿƒÀƒÁøuîëÇ    ÇF    ÇF    ƒÄ^_[Ã USWVƒìè    [Ãü*S ‹|$4‹l$0‹M‹E   4@9òs÷ÑÁ+M‰ÂÁê9Ñ†   ‹D$8é  À‰D$‰,$è¦gtÿ‹M1À…É„ê   ‹D$0‹‹7‰t$‰ğÁè‰óÁë	1ÃI‰L$!ËÚ‹<Ú9ş„½   1í¾   ƒÿü„£   ƒÿø•Á…í•ÅÍDèóv#\$Ú‹<Ú9|$uÒé~   ‰D$‰,$è!gtÿ‹M1À…Éti‹D$0‹‹7‰t$‰ğÁè‰óÁë	1ÃI‰L$!ËÚ‹<Ú9şt@1í¾   ƒÿüt'ƒÿø•Á…í•ÅÍDèóv#\$Ú‹<Ú9|$uÖë…íEÅ‹L$0ÿAƒ8ütÿIƒÄ^_[]Ã        USWVƒìè    [ÃŒ)S ‹|$ ‹D$$‹7‹oH‰ÁÑé	Á‰ÈÁè	È‰ÁÁé	Á‰ÈÁè	È‰ÂÁê	ÂBƒú@¹@   GÊ‰O1À…ÉtÁá‰$èİ'rÿ‰…ötî‰D$‰t$‰<$è$ftÿ‰4$èü(rÿë,ÇG    ÇG    ‹O…ÉtÁá…ÀtÇ üÿÿÿƒÀƒÁøuîƒÄ^_[]Ã      USWVƒì‹D$Ç@    Ç@    ‹@…Àt"‹L$‹	Áà…ÉtÇüÿÿÿƒÁƒÀøuî‹L$$‹T$ ëƒÂ9Ê„±   ‹2‰ğƒÈƒøüté‰t$‹D$‹x1Û…ÿtx‹D$‹(‹L$‰ÈÁè‰ÎÁî	1ÆO‰<$!ş\õ ‹Dõ 9ÁtP¿   1Éƒøüt(ƒøø•À…É•ÄÄDËş#4$\õ ‹Dõ 9D$uÕë…ÉEÙ‹D$‰‹B‰C‹D$ÿ@‹L$$éDÿÿÿƒÄ^_[]Ã            USWVƒìè    [Ã¬'S ‹|$4‹t$0‹N‹F   ,@9ês÷ÑÁ+N‰ÂÁê9Ñv.‹D$8ëLÀ‰D$‰4$è}dtÿD$‰D$‰|$‰4$èYbtÿ‹D$ë$‰D$‰4$èWdtÿD$‰D$‰|$‰4$è3btÿ‹D$ÿFƒ8üuƒxÿtÿNƒÄ^_[]Ã      USWVƒìè    [Ãü&S ‹t$0‹D$4‹>‹nH‰ÁÑé	Á‰ÈÁè	È‰ÁÁé	Á‰ÈÁè	È‰ÂÁê	ÂBƒú@¹@   GÊ‰N1À…ÉtkÁ‰$èM%rÿ‰…ÿ„ï   ‰|$ÇF    ÇF    ‹N…Ét#kÉ…ÀtÇ@ÿÿÿÿÇ üÿÿÿƒÀƒÁôuç‰ò…í„›   ‹D$‰éhkù‰ÆEü‹Müƒùøuƒ} ştb‰|$ë‰|$ƒùü‰éuƒ} ÿ‹|$t>N‰L$L$‰L$‰D$‰×‰<$èî`tÿ‹D$‹Mü‰‹L$‹	‰H‹M‰HÿG‰ú‹|$ƒÆƒÅƒÇôu‚‹D$‰$è–%rÿë=ÇF    ÇF    ‹N…Ét(kÉ…ÀtÇ@ÿÿÿÿÇ üÿÿÿƒÀƒÁôuçƒÄ^_[]Ã               USWVƒì‹D$,‹l$$‹]…Û„  ‹D$(‹‰L$‹P‰$‰ÈÁèÁé	1ÁkÂ%‰Â÷ÒƒÀÿÊ‰Ñ¤Á
‰ÖÁî1Ö1Á‰ò¤Ê‰ÏÁç÷Ò÷×Ïò‰Ğ¤ø‰ÑÁé1Ñ1øÉº	   ÷âÊ‰Ñ¤Á‰ÖÁî1Ö1Á‰ğ¤È‰ÏÁç÷×Ï‹m ÷Ğğ¤ø1øKÇD$   ÇD$    ë …ö•ÇßDñ‰t$‹L$ÈA‰L$‰Ó!ØkğL5 ‹|5 9|$u	‹$;T5tG‰Úƒÿøuƒ|5ş•Ãë(³ƒÿüuƒ|5ÿ‹t$uë$‹t$ëŠÇ     ë‹D$,‰°ë…öEÎ‹D$,‰1À¶ÀƒÄ^_[]Ã‹D$‹H1À…Ét)‹D$‹ ~kÉ‹€Ğ  ‹Dôë÷Ñ‹€ì  kÉ0‹DƒàüÃ         ‹D$‹H1À…Éy‹D$‹ ÷Ñ‹€ì  kÉ0‹D$ƒàüÃ          SWV‹D$‹L$‹T$‹r…öt"‹|$‹~(kÖ‹³Ğ  ‹T2ôƒâü‰Ç     ë;Ç     Ç    ë-÷Ö‹›ì  kö0‹t3ƒæü‰1‹‹R÷Ò‹‰ì  kÒ0‹L$ƒáü‰^_[Ã         SWVƒìè    [Ãı"S ‹t$$‹D$ ‹H…Éx1À…ÉtW‹kÉ‹€Ğ  ‹DôëC‹÷Ñ‹ì  kÁ0‹L$ƒáüt!‰L$|$‰<$è’Ërÿƒì‰t$‰<$èÃ_tÿë‹‹‰ì  ‹DƒàüƒÄ^_[Ã           USWVƒì,è    [Ãl"S ‹t$@‹D$D‰D$(|$(‰|$D$ ‰$ÇD$   èE¡rÿƒì‹l$ …í„¡   ‰<$ÇD$    èÆÂrÿ‰Æ‰<$ÇD$   è´Ârÿ‰ù‰Ç‰L$D$‰$ÇD$   èø rÿƒì‹D$‰D$‹D$@Ç@    1Àşÿÿÿ Gğ‰ùÁáÿÿ   GÈ	ñ‹t$@‰‰,$èY$rÿ‹L$‹ …Ét)‰L$‰l$‰$ÇD$    èÇ^tÿë#Ç    ÇF    ë‰l$‰$ÇD$    è²^tÿ‰FƒÄ,^_[]Â      SWVƒì è    [ÃM!S ‹t$0‹D$4‹L$8‰L$‰$èV^tÿ‰D$|$‰<$èVÄrÿƒì‰<$è[¶rÿ„Àt‹D$ƒxr‰<$ÇD$   ëÇ    ÇF    ëR‰<$ÇD$   èÁrÿ‹|$Ç    ÇF    …ÿt-1É=ÿÿÿ FÈ‰‰<$èX#rÿ‹ ‰|$‰$ÇD$    èâ]tÿ‰FƒÄ ^_[Â       USWVƒìè    [Ã| S ‹|$<‹t$0Ç    ÇF    …ÿth‹l$8‹L$41Òùÿÿÿ GÊ‰èÁàıÿ   GÂ	È‰‰<$è×"rÿ‹L$@‹ …Ét‰L$‰|$‰$ÇD$    èE]tÿë‰|$‰$ÇD$    è?]tÿ‰FƒÄ^_[]Â   USWVƒì<è    [ÃÜS ‹|$T‹D$P‰|$8‹¨Ì  1ö…ítp‰\$‹ˆÀ  ‰øÁè‰úÁê	1ÂM!ê4Ñ‹Ñ9øtv‰ë1ÿ½   ƒøüt%ƒøø•À…ÿ•ÄÄDşêm!Ú4Ñ‹Ñ;D$TuØë:…ÿE÷‹|$T‹\$‹D$P‰\$€À  ‰t$L$8‰L$‰$è‡\tÿ‰Æ‰>ÇF    ‹F…À…e  ‹D$X…Àt‰FéU  ‹T$PªĞ  ‹ŠĞ  ‹‚Ô  9Á‹\$u ‰,$ÇD$€   ‰×èA\tÿ‰ú‹ŠĞ  ‹‚Ô  ‰l$)ÈÁøiø«ªªªG‰~‹D$8ÇD$$   ÇD$(    ‰D$,ƒàüƒÀƒøw
¹  £ÁrD$$‰$‰Õè5}rÿ‰ê‹«ÀüÿÿƒÅ‰l$ ‰T$0‰|$4‹ºÔ  ;ºØ  sh1À…ÿtWÇG   ÇG    ‹D$,‰GƒàüƒÀƒøw
¹  £ÁrG‹L$$ƒáü‰L$‰$èJrÿ‹T$P‰/òD$0òG‹‚Ô  ƒÀ‰‚Ô  ëD$ ‰D$‹D$‰$èT[tÿ‹D$,ƒàüƒÀƒøw
¹  £ÁrD$$‰$èO{rÿ‹FƒÄ<^_[]Ã    USWVìŒ   è    [Ã™S ‹Œ$¨   ‹¼$¤   ‹„$    °Ü  ‰¼$€   ‰Œ$„   „$ˆ   ‰D$¬$€   ‰l$‰4$èÓZtÿ‹”$ˆ   „Àt‹B…À…a  ë%‰T$‰l$‰4$è»Ztÿ‰Â‰:‹„$¨   ‰BÇB    ‹„$¬   …Àt‰Bé'  ‹„$    ‰Æ†ì  ‰D$‹ì  ‹†ğ  ‰õ9Áu$‹D$‰$ÇD$€   ‰ÖèhZtÿ‰ò‹ì  ‹…ğ  )ÈÁøiğ«ªªª÷Ö‰rÇD$<   ÇD$@    ‰|$DƒçüƒÇƒÿw
¸  £ørD$<‰$‰×è#{rÿ‰ú‰T$‹»ÀüÿÿƒÇ‰|$8‰é‰L$H‰t$L‹rÇD$$   ÇD$(    ‹”$¨   ‰T$,ƒâüƒÂƒúw
¸  £ĞrD$$‰$‰ÍèÅzrÿ‰é‰|$ ‰L$0‰Í‰t$4T$TÇD$T   ÇD$X    ‹D$D‰D$\ƒàüƒÀƒøw
¹  £Ár‹D$<ƒàü‰D$‰$èñ|rÿt$<‰|$PòD$HòD$`ÇD$l   ÇD$p    ‹D$,‰D$tƒàüƒÀƒøw
¹  £Ár‹D$$ƒàü‰D$D$l‰$è™|rÿ‰|$hòD$0òD$x‰é‹©ğ  ;©ô  ƒ¾   1À…í„©   ÇE   ÇE    ‹D$\‰EƒàüƒÀƒøw
¹  £ÁrE‹L$Tƒáü‰L$‰$è+|rÿ‰} òD$`òEÇE   ÇE     ‹D$t‰E$ƒàüƒÀƒøw
¹  £ÁrE‹L$lƒáü‰L$‰$èİ{rÿ‰}òD$xòE(‹Œ$    ‹ğ  ƒÀ0‰ğ  ëD$P‰D$‹D$‰$è#Xtÿ‹D$tƒàüƒÀƒøw
¹  £ÁrD$l‰$èŞwrÿ‹D$\ƒàüƒÀƒøw
¹  £ÁrD$T‰$è¹wrÿ‹D$,ƒàüƒÀƒøw
¹  £ÁrD$$‰$è”wrÿ‹D$DƒàüƒÀƒøw
¹  £Ár‰4$èswrÿ‹D$‹@ÄŒ   ^_[]Ã USWVƒì<è    [Ã¼S ‹D$P‹H1À…É„Ù   ‹D$T‹ ~kÉ‹€Ğ  DôÇD$    ë÷Ñ‹ì  kÉ0D
‹L
$ƒáü‰L$‹(ƒåü‰,$èrÿ‰D$‰$èê$rÿ‰Ç‹D$P‰Æ‹%ÿÿÿ ‰D$‰<$ÇD$    ÇD$    è?~rÿ‰D$,¶F‰D$‰<$ÇD$    ÇD$    è~rÿ‰D$0‰l$4‹D$‰D$8D$,‰D$ ÇD$$   òD$ òD$‹D$‰$èyrÿƒÄ<^_[]Ã           USWVƒìè    [Ã¬S ‹t$0‹D$4‰D$|$‰|$D$‰$ÇD$   è…—rÿƒì‹l$…íte‰<$ÇD$   è
¹rÿ‰Æ‰<$ÇD$   èø¸rÿ1Éşÿÿÿ Gñ‰ÂÁâ=ÿ   GÑ	ò‹t$0‰‰,$èĞrÿ‹ ‰l$‰$ÇD$    èZUtÿ‰FëÇ    ÇF    ƒÄ^_[]Â               Ã               USWVƒì,è    [ÃÌS ‹l$@‹E…À„5  ‹|$H‹L$D‹	~kÀ‹‰Ğ  Dôë÷Ğ‹‰ì  kÀ0D‹ ƒàü‰D$(t=D$(‰D$D$ ‰$èÕºrÿƒì‹D$ ‹t$$‹W‹O)Ê9ÖvH‰t$‰D$‰<$è^rÿëb«Í×Éÿ‰,$ÇD$
   è6rÿ‰Æ‹O‹G)Á9Îv'‰t$‰l$‰<$è(rÿë(‰t$‰D$‰$èfrÿwë‰t$‰l$‰$èQrÿw‹l$@‹G;GsH‰OÆ :‰øë‰<$ÇD$:   èõrÿ‹M áÿÿÿ ‰L$‰$èĞrÿ}    r5‹G;GsH‰OÆ :‰øë‰<$ÇD$:   è²rÿ¶M‰L$‰$è’rÿ‹M1À…Éy‹D$D‹ ÷Ñ‹€ì  kÉ0‹D$ƒàü‰D$D$‰$è!¿rÿƒìƒ|$ „ª   «€}Ëÿ‰,$ÇD$   èrÿ‰Æ‹O‹G)Á9Îv‰t$‰l$‰<$èrÿë‰t$‰l$‰$èMrÿw‰|$‹D$D‰D$D$‰$è2rÿ«%ùÉÿ‰,$ÇD$   è¼rÿ‰Æ‹O‹G)Á9Îv‰t$‰l$‰<$è®rÿë‰t$‰l$‰$èìrÿwƒÄ,^_[]Ã USWVƒìè    [ÃlS ‹³lôÿÿŠ„Àu.‰4$è	-rÿ…Àt"‹ƒpôÿÿ‹ …À¹ÍŒUíEÈ‹ƒtôÿÿ‰‰4$èó,rÿ‹D$ ‹“tôÿÿ‹‹x‰ËÁëÁáƒÁƒÓ 3:1ù½i-8ë‰È÷å‰ÆiÁêßĞiËi-8ëÁiÙi-8ëÁé1ş1Î‰ğ÷åiÎêßÑÙÁé1ÁiÁi-8ëƒÄ^_[]Ã         USWVƒì,è    [Ã¬S ‹L$D‹t$@‹‹F)ĞÁøiÀ«ªªª9ÈƒS  ‹n‰è)ĞÁøiÀ«ªªª1ÿ…Ét‰ÅkD$D‰$èrÿ‰Ç‰è‹‹n‰D$ kÈ‰L$(kD$Dø9Õ„ô   ‰L$‰D$Eè)Ğ‰Ö‰t$$º«ªªª÷âÁê÷Ú‹D$ Dÿ‰D$‰ò‹D$(tø‰|$ ‹ƒÀüÿÿƒÀ‰D$(}èÇFô   ÇFø    ‹Eô‰FüƒàüƒÀƒøw¹  £ÁrFô‹Mìƒáü‰L$‰$èÛtrÿ‹T$$‹D$(‰FğòEøòƒÆè9ú‰ıukD$‹L$ Á‹D$@‹(‹p‰‹L$‰H‹L$‰H9ît ‹FèƒÆè‰4$ÿP9õuğë‰‰N‰F…ít‰,$èrÿƒÄ,^_[]Ã  USWVƒì,è    [ÃS ‹|$D‹t$@‹‹N)ÁÁùiÉ«ªªª9ùƒ  N‹V)ÂÁúiê«ªªªÇD$$    ‰L$(1À…ÿtkÇ0‰$èorÿ‰D$kÍ0Á‰L$ ‰L$kÏ0Á‰L$$D$‰D$‰4$èePtÿ‹l$‹t$ 9î„•   FĞ)è¹«ªªª÷áÁê÷Ò‰T$‰t$‰÷wĞ‹GôƒàüƒÀƒøw¹  £ÁrGì‰$èÕorÿ‹GÜƒàüƒÀƒøw¹  £ÁrƒÇÔ‰<$è¥orÿ9õ‰÷uŸkD$0‹L$Á‰L$ ‹D$…Àt‰$èĞrÿƒÄ,^_[]Ã        USWVƒì,è    XÀÌS ‰D$‹|$@‹_‹G‰Ùƒáü…À„|   ‹W¢   ‹²Ì  …ö„F  ‰T$‹’À  ‰ØÁè‰İÁí	1ÅN!õ‹ê9Át"¿   ƒøü„  ı!õ‹ê9ÁuéÇêøÿÿÿ‹D$ÿˆÄ  ÿ€È  ‹|$@‹_éá   ƒùøtƒùüt…É„  G‰$‹\$è¥nrÿ‹_ƒã‰_éä   ÷Ğ‹Šì  ‰L$kğ0‹D1‹L1$ƒàüƒáü‰Õ•Ü  ‰D$ ‰L$$D$(‰D$D$ ‰D$‰$‹\$è>Ntÿ„Àt‹D$(Ç øÿÿÿÇ@øÿÿÿÿà  ÿ…ä  ‹G‰Áƒáüƒùøtƒùüt…ÉtG‰$‹\$ènrÿ‹Gƒà‰G‹D$ÇD0,    ÇD0    ë6‹|$@‰Øƒàüƒøøtƒøüt…ÀtG‰$‹\$èÃmrÿ‹_ƒã‰_ÇG    ƒÄ,^_[]Ã           USWVƒì,è    [ÃüS ‹l$D¶Eƒø‰ét1É‹|$@…É„š   ‹G…À„   ‰L$‹o…Àì   ‹G‹Ì  …É„  ‰Æƒæü‹•À  ‰l$‰ÇÁï‰D$Áè	1øI!È‹,Â9ît%¿   ƒıü„¾  ø!È‹,Â9îuéÇÂøÿÿÿ‹D$ÿˆÄ  ÿ€È  ‹|$@‹Gé•  ‹‹@ƒÄ,^_[]ÿà‹G‰Áƒáü9é„÷  wƒÁƒùw
º  £Êr‰4$è›lrÿ‹Gƒà	è‰GƒåüƒÅƒıw¸  £è‚¶  ‰4$èmrÿé©  ÷Ğ‹ì  ‰L$kĞ0‰T$‹D‹L$ƒàüƒáü•Ü  ‰D$ ‰L$$D$(‰D$D$ ‰D$‰$èLtÿ„Àt‹D$(Ç øÿÿÿÇ@øÿÿÿÿà  ÿ…ä  ‹G‰Áƒáü‹T$9ÑtG‰ÕwƒÁƒùw
º  £Êr‰4$èÓkrÿ‹Gƒà	è‰GƒåüƒÅƒıw
¸  £èr‰4$èËlrÿ‹L$‹T$DL$‹ ƒàü‹	ƒáü‹W‹w‰t$‰L$‰D$‰$èKtÿ‹O9È„«   ‹G÷Ñ‹€ì  kÉ0ÇD,    ÇD    éˆ   ‹|$@‹D$‰Á‰Íƒàü‹T$9ĞtOwƒÀƒøw
¹  £Ár‰4$‰Õèkrÿ‰ê‹oƒå	Õ‰o‰ĞƒàüƒÀƒøw
¹  £Ár‰4$‰Öèlrÿ‰ò‹G‹O‰L$‰T$‰$è}Jtÿ;GtÇG    ƒÄ,^_[]Ã         Sƒìè    [ÃS ‹D$‹HƒáüƒÁƒùw
º  £ÊrƒÀ‰$èyjrÿ‹ƒîÿÿƒÄ[ÿà             USWVƒì‹D$,‹l$$‹]…Û„,  ‹D$(‹‰L$‹P‰$‰ÈÁèÁé	1Á‰ĞÁèÁê	1Â‰Ğ÷ĞƒÂÿÈ‰Á¤Ñ
‰ÆÁî1Æ1Ñ‰ò¤Ê‰ÏÁç÷Ò÷×Ïò‰Ğ¤ø‰ÑÁé1Ñ1øÉº	   ÷âÊ‰Ñ¤Á‰ÖÁî1Ö1Á‰ğ¤È‰ÏÁç÷×Ï‹m ÷Ğğ¤ø1øKÇD$   ÇD$    ë)…ö•ÇßDñ‰t$‹L$ÈA‰L$‰Ó!ØkğL5 ‹|5 9|$u	‹$;T5tG‰Úƒÿøuƒ|5ø•Ãë(³ƒÿüuƒ|5ü‹t$uë$‹t$ëŠÇ     ë‹D$,‰°ë…öEÎ‹D$,‰1À¶ÀƒÄ^_[]ÃUSWVƒì,è    [ÃLS ‹t$@F‹‹V)ÊÁúiê«ªªª‹V)ÊÁúiú«ªªªÿªªªs'Mÿ9ÏBùL$$ÇD$$    ‰D$(1À…ÿ¾    uë(L$$ÇD$$    ‰D$(¿UUUkÇ0‰$‰Îèw	rÿ‰ñ‰Æ‰ø‰t$kí0kÀ0ğ‰÷ï‰|$ ‰|$‰„½   ÇD.   ÇD.    ‹T$D‹B‰D.ƒàüƒÀƒøw
¹  £ÁrD.‹Jƒáü‰L$‰$è›krÿ‹T$D‹ƒÀüÿÿƒÀ‰D$‰òBòD.ÇD.   ÇD.     ‹B$‰D.$ƒàüƒÀƒøw
¹  £ÁrD.‹Jƒáü‰L$‰$‰×è9krÿ‰ú‹D$‰D.òB(òD.(D50‰D$ D$‰D$‹D$@‰$è˜Gtÿ‹l$‹t$ 9î„˜   FĞ)è¹«ªªª÷áÁê÷Ò‰T$‰t$‰÷wĞ‹GôƒàüƒÀƒøw¹  £ÁrGì‰$ègrÿ‹GÜƒàüƒÀƒøw¹  £ÁrƒÇÔ‰<$èÕfrÿ9õ‰÷uŸkD$0‹L$Á‰L$ ‹D$…Àt‰$è 	rÿƒÄ,^_[]Ã        USWVƒìè    [ÃüS ‹t$4‹D$0‹‰L$‹h9Í‹~„Ú   ‹ƒÀüÿÿƒÀ‰D$EĞ‰D$ÇGÔ   ÇGØ    ‹EÜ‰GÜƒàüƒÀƒøw
¹  £ÁrGÔ‹MÔƒáü‰L$‰$è»irÿ‹D$‰GĞòEàòGàÇGì   ÇGğ    ‹Eô‰GôƒàüƒÀƒøw¹  £ÁrGì‹Mìƒáü‰L$‰$èkirÿ‹D$‰GèòEøòGø‹~ƒÇĞ‰~‹D$‹L$9È‰Í…?ÿÿÿ‹T$0‹
ë‹T$0‰:‰N‹B‹N‰J‰F‹B‹N‰J‰F‹F‰ƒÄ^_[]Ã         USWVƒìè    [Ã¼S ‹|$4‹t$0‹N‹F   ,@9ês÷ÑÁ+N‰ÂÁê9Ñv.‹D$8ëLÀ‰D$‰4$èMEtÿD$‰D$‰|$‰4$èéDtÿ‹D$ë$‰D$‰4$è'EtÿD$‰D$‰|$‰4$èÃDtÿ‹D$ÿFƒ8üuƒxütÿNƒÄ^_[]Ã      USWVƒìè    [ÃS ‹t$0‹D$4‹‰L$‹~H‰ÁÑé	Á‰ÈÁè	È‰ÁÁé	Á‰ÈÁè	È‰ÂÁê	ÂBƒú@¹@   GÊ‰N1À…É½üÿÿÿtkÁ‰$èTrÿ‰ƒ|$ „ã   ÇF    ÇF    ‹N…Ét#kÉ…Àt‰(‰hƒÀƒÁôuï‰ò…ÿ„“   ‹D$hkÿ‰ÆEü‹Müƒùøuƒ} øtb‰|$ë‰|$ƒùü‰éuƒ} ü‹|$t>N‰L$L$‰L$‰D$‰×‰<$è~Ctÿ‹D$‹Mü‰‹L$‹	‰H‹M‰HÿG‰ú‹|$ƒÆƒÅƒÇôu‚‹D$‰$è¦rÿë5ÇF    ÇF    ‹N…Ét kÉ…Àt‰(‰hƒÀƒÁôuïƒÄ^_[]Ã       USWVƒì,è    [ÃlS ‹T$@‹‹J)ÁÁùiÉ«ªªª‰L$ i‹J)ÁÁùiÁ«ªªª¾ªªª
=TUUwÀ9èBÅ1ÿ…À‰Æ¸    tkÆ‰$è¹rÿ‰÷kL$ ‰ÂÊ‰T$(‰Æ„„   ‰|$ÇD   ÇD    ‹T$D‹B‰DƒàüƒÀƒøw‰L$$¹  £Á‹L$$r*‰L$$D‹Jƒáü‰L$‰$‰l$‰ÕèÖerÿ‹L$$‰ê‹l$‹ƒÀüÿÿƒÀ‹|$(‰òBòD‹|$‰L$$kÏñkÕò‹D$@‹(‹x9ï„ñ   ‰T$‰L$Gè)è‰l$(¹«ªªª÷áÁê÷Ú‹D$ Dÿ‰D$ ‰t$‹D$$tø‹ƒÀüÿÿƒÀ‰D$$oèÇFô   ÇFø    ‹Gô‰FüƒàüƒÀƒøw¹  £ÁrFô‹Oìƒáü‰L$‰$èûdrÿ‹D$$‰FğòGøòƒÆè9l$(‰ïu kD$ ‹L$Á‹D$@‹(‹p‰‹L$‰H‹L$‰H9ît&‹FèƒÆè‰4$ÿP9õuğë‹t$(‰0‰P‰H…ít‰,$è6rÿƒÄ,^_[]Ã              USWVƒìè    [Ã,S ‹|$4‹l$0‹M‹E   4@9òs÷ÑÁ+M‰ÂÁê9Ñ†   ‹D$8é  À‰D$‰,$èÆ@tÿ‹M1À…É„ê   ‹D$0‹‹7‰t$‰ğÁè‰óÁë	1ÃI‰L$!ËÚ‹<Ú9ş„½   1í¾   ƒÿü„£   ƒÿø•Á…í•ÅÍDèóv#\$Ú‹<Ú9|$uÒé~   ‰D$‰,$èA@tÿ‹M1À…Éti‹D$0‹‹7‰t$‰ğÁè‰óÁë	1ÃI‰L$!ËÚ‹<Ú9şt@1í¾   ƒÿüt'ƒÿø•Á…í•ÅÍDèóv#\$Ú‹<Ú9|$uÖë…íEÅ‹L$0ÿAƒ8ütÿIƒÄ^_[]Ã        USWVƒìè    [Ã¼S ‹|$ ‹D$$‹7‹oH‰ÁÑé	Á‰ÈÁè	È‰ÁÁé	Á‰ÈÁè	È‰ÂÁê	ÂBƒú@¹@   GÊ‰O1À…ÉtÁá‰$è rÿ‰…ötî‰D$‰t$‰<$èD?tÿ‰4$è,rÿë,ÇG    ÇG    ‹O…ÉtÁá…ÀtÇ üÿÿÿƒÀƒÁøuîƒÄ^_[]Ã      USWVƒì‹D$Ç@    Ç@    ‹@…Àt"‹L$‹	Áà…ÉtÇüÿÿÿƒÁƒÀøuî‹L$$‹T$ ëƒÂ9Ê„±   ‹2‰ğƒÈƒøüté‰t$‹D$‹x1Û…ÿtx‹D$‹(‹L$‰ÈÁè‰ÎÁî	1ÆO‰<$!ş\õ ‹Dõ 9ÁtP¿   1Éƒøüt(ƒøø•À…É•ÄÄDËş#4$\õ ‹Dõ 9D$uÕë…ÉEÙ‹D$‰‹B‰C‹D$ÿ@‹L$$éDÿÿÿƒÄ^_[]Ã            Sƒìè    [ÃßÿR ‹D$‹@…Àt‰$èı=tÿƒÄ[Ã        Ã               Sƒìè    [ÃŸÿR ƒø±  ‰$èÒ=tÿƒÄ[Ã             USWVƒìè    [ÃlÿR ‹|$4‹t$0‹D$8‹L$<ÇF    ‰N‹‹ÄüÿÿƒÁ‰ÇF    ‰F‰~«8ÌÉÿ‰,$ÇD$   è%ıqÿƒ, ufƒ yU‰D$‰l$òD$òD$‰<$èl¼rÿ…Àt5ƒx t/‰$ÇD$    èò^rÿ¶Hƒùt1À…Àtƒx@@ v‹ ‹ ‰FƒÄ^_[]Ã           SWVƒìè    [ÃşR ‹|$$‹t$ ‹‹N‰L$‰<$ÿP@ƒ~ t!‹‹ã~Ëÿ‰L$‰<$ÿP‹‹V‰T$‰$ÿQ4ƒÄ^_[Ã         SVƒìè    [Ã>şR ‹t$ ‹D$$‹“í~Ëÿ‰T$‰$ÿQ‹‹V‰T$‰$ÿQ4‹“	Ëÿ‰T$‰$ÿQ‹‹V‰T$‰$ÿQDƒÄ^[Ã               SVƒìè    [ÃÎıR ‹t$ ‹D$$‹“Ëÿ‰T$‰$ÿQ‹‹V‰T$‰$ÿQ4‹“	Ëÿ‰T$‰$ÿQ‹‹V‰T$‰$ÿQHƒÄ^[Ã               USWVƒìè    [Ã\ıR ‹t$$‹|$ ‹G…ÀtZ‹o‹‰D$‰4$ÿQƒı ‹‹It2“i˜Éÿ‰T$‰$ÿÑ‹‹W‰T$‰$ÿQ4‹“k˜Éÿ‰T$‰$ÿQë“k˜Éÿ‰T$‰$ÿÑ‹‹O‰L$‰4$ÿP@ƒÄ^_[]Ã             ‹D$ƒx •À¶ÀÃ USWVƒì,è    [Ã¬üR ‹l$D‹t$@‹FƒÆ‰$ègÿqÿ‰D$‰4$è;“sÿ‰D$ t$ ‰t$|$(‰<$ÇD$   èk{rÿƒì‰|$D$‰$è¸ŸrÿƒìòD$òE ‰4$ÇD$    èÚœrÿ‹L$H‰‰4$ÇD$   èÄœrÿ‹L$L‰ƒÄ,^_[]Ã      USWVìŒ   è    [ÃùûR ‹¼$¤   ƒÍ×Éÿ‰D$p‰$ÇD$
   èÙùqÿ‰D$t1öƒ ¸    „‡   ‹GƒÇ‰$è†şqÿ‰D$‰<$èZ’sÿ‰„$€   ¼$€   ‰|$´$ˆ   ‰4$ÇD$   èzrÿƒì‰t$D$x‰$èÎrÿƒìòD$xòD$p‰<$ÇD$    èï›rÿ‰Æ‰<$ÇD$   èİ›rÿ‹¬$    ÆD$8ÆD$9L$p‰L$0»i˜Éÿ‰|$4L$0‰L$@‰t$DÆD$HÆD$I·T$HÆD$)‰|$ ÆD$(¶Êƒùu‹L$(‰L$XòD$ òD$P·L$XˆÊëC„ÒuÆD$X ÆD$Yµ1Òë/‰Ñá ÿ  ù   u‹L$@ë²L$@‰L$P‰|$TˆT$XÆD$YµÆD$ÆD$‰D$¶òƒşu‹D$‰D$hòD$òD$`ë5„ÒuÆD$h ÆD$ië%¶Íƒùu‹L$Pë²L$P‰L$`‰D$dˆT$hÆD$iD$`‰D$‰,$èXXrÿÄˆ   ^_[]Â            USWVƒìè    [ÃìùR ‹L$0‹T$4‹‰Ö‹h‰L$|$‰<$è8tÿƒì‰|$‰4$ÿÕ‹“k˜Éÿ‰T$‰$ÿQ‹‹T$0‹R‰T$‰$ÿQ@öD$t‹D$‰$èjùqÿƒÄ^_[]Ã  SVƒì$è    [ÃnùR ‹³T	 …öt=‹D$0‹@‰D$‰$ÇD$ÿÿÿÿèJ÷qÿ‰D$òD$òD$‰4$ÇD$    èZ[sÿë1À¶ÀƒÄ$^[Ã             SVƒì$è    [ÃşøR ‹³\	 …öt=‹D$0‹@‰D$‰$ÇD$ÿÿÿÿèÚöqÿ‰D$òD$òD$‰4$ÇD$    èêZsÿë1À¶ÀƒÄ$^[Ã             SVƒì$è    [ÃøR ‹³d	 …öt=‹D$0‹@‰D$‰$ÇD$ÿÿÿÿèjöqÿ‰D$òD$òD$‰4$ÇD$    èzZsÿë1À¶ÀƒÄ$^[Ã             SWVƒì0è    [ÃøR ‹D$@‹L$P‹T$L‹t$H‹|$DÇD$   ÇD$   ‰|$‰t$ òòD$$‰L$,‹‹ÈüÿÿƒÁ‰L$L$‰L$‰$èg3sÿƒÄ0^_[ÃSWVƒì0è    [Ã­÷R ‹D$@‹L$P‹T$L‹t$H‹|$DÇD$   ÇD$   ‰|$‰t$ òòD$$‰L$,‹‹ÌüÿÿƒÁ‰L$L$‰L$‰$è÷2sÿƒÄ0^_[ÃSWVƒì0è    [Ã=÷R ‹D$@‹L$P‹T$L‹t$H‹|$DÇD$   ÇD$   ‰|$‰t$ òòD$$‰L$,‹‹ĞüÿÿƒÁ‰L$L$‰L$‰$è‡2sÿƒÄ0^_[ÃÃ               è    XÀÃöR ‹€îÿÿÿà            è    XÀ£öR ‹€îÿÿÿà            è    XÀƒöR ‹€îÿÿÿà            è    XÀcöR ‹€îÿÿÿà            è    XÀCöR ‹€îÿÿÿà            è    XÀ#öR ‹€îÿÿÿà            è    XÀöR ‹€îÿÿÿà            è    XÀãõR ‹€îÿÿÿà            USWVƒì|è    [Ã¼õR ‹´$   ÇD$$    ÇD$     ÇD$(    ‹„$    …Àt‰D$‹¬$¤   ƒı
w(D- ˆD$`t$aë9ÇD$d    ÇD$`    ÇD$h    ë>}ƒçğ‰<$èíóqÿ‰Æ‰t$hƒÏ‰|$`‰l$d‰l$‹D$‰D$‰4$èˆóqÿÆ. ‹´$   ÆD$! ÆD$  D$ ‰$ÇD$    èo2rÿ‹D$h‰D$(òD$`òD$ ‹~(¶D$ ¨t‹D$$ëÑè…À„Ğ   Ç$   ècóqÿ‰Æ½   …öt]ÇF    ÇF    ‹ƒÔüÿÿƒÀ‰n¶D$ ¨t‹D$(‰D$p‹D$$ë
L$!‰L$pÑè‰D$tòD$pòD$‰,$ÇD$    è{Vsÿ‰/‹G‰w…Àt
‰$èw2tÿ‹/ÇD$d    ÇD$`    ÇD$h    D$`‰D$‰,$è}2tÿ„ÀtAöD$`‹´$   t‹D$h‰$èßóqÿ‹„$”   ‰FöD$ t‹D$(‰$èÂóqÿ1ÀƒÄ|^_[]ÃÇD$4    ÇD$0    ÇD$8    ƒ=Ëÿ‰D$‰$ÇD$   è¦ñqÿ‰ÆŠD$ ˆD$¨u	¶D$Ñèë‹D$$‰D$‹D$,0ƒı
w6ˆD$0|$1ë'ƒÅƒåğ‰,$è òqÿ‰Ç‰|$8ƒÍ‰l$0‰t$4ŠD$ ˆD$‰t$‹D$‰D$‰<$è“ñqÿÆ7 öD$uD$!ë‹D$(‹L$‰L$‰D$t$0‰4$èf0rÿ»ZËÿ‰<$ÇD$   èğğqÿ‰D$‰|$‰4$è@0rÿ‹H‰L$Hò òD$@Ç@    Ç     Ç@    ¶D$`¨uL$aÑèë‹D$d‹L$h‰D$‰L$D$@‰$èî/rÿ‹H‰L$Xò òD$PÇ@    Ç     Ç@    D$P‰$ÇD$    èåˆrÿ     ¸   Ã          Sƒìè    [Ã/òR ‹D$‰D$ƒÀ0‰$èfrÿƒÄ[Ã        Sƒìè    [ÃÿñR ‹D$$‹L$ ‰D$‰L$ƒÁ0‰$èåerÿƒÄ[ÃSƒìè    [ÃÏñR ŠD$(„Àt)‹D$$‹L$ ‹“dñÿÿƒÂ‰T$‰D$‰L$D$‰$è0tÿƒÄ[Ã          Ã               è    XÀsñR ‹€îÿÿÿà            Sƒìè    [ÃOñR ‹D$‹‹ÔüÿÿƒÁ‰ƒÀ‰$èvSsÿ‹ƒØüÿÿƒÄ[ÿà          SVPè    [ÃñR ‹t$‹ƒÔüÿÿƒÀ‰F‰$è7Ssÿ‰4$èo/tÿ‹ƒîÿÿƒÄ^[ÿà  Sƒìè    [ÃÏğR ‹D$ƒÀ‰$èSsÿƒÄ[Ã            è    XÀ£ğR ‹€îÿÿÿà            è    XÀƒğR ‹€îÿÿÿà            1ÀÃ             SWVƒìè    [ÃMğR ŠL$$‹t$ ‹F‹P;Ps
z‰xˆ
ë¶É‰L$‰$èRîqÿ‰ğƒÄ^_[Ã         SWVƒìè    [ÃıïR ŠL$$‹t$ ‹F‹P;Ps
z‰xˆ
ë¶É‰L$‰$èîqÿ‰ğƒÄ^_[Ã         SWVƒìè    [Ã­ïR ŠL$$‹t$ ‹F‹P;Ps
z‰xˆ
ë¶É‰L$‰$è²íqÿ‰ğƒÄ^_[Ã         USWVƒìè    [Ã\ïR ‹t$ ‹~‹D$$‹l$(‹W‹O)Ê9Õv‰l$‰D$‰<$èBíqÿë‰l$‰D$‰$è€íqÿo‰ğƒÄ^_[]Ã   USWVƒìè    [ÃüîR ‹t$$‹D$ ‹h‰4$ÇD$ÿÿÿÿèâìqÿ‰Ç‹M‹E)Á9Ïv‰|$‰t$‰,$èÔìqÿë‰|$‰t$‰$èíqÿ}‹D$ ƒÄ^_[]Ã   SVƒìè    [ÃîR ‹D$$‹t$ ‹N¶öÂt‹P‹@ë@Ñê‰T$‰D$‰$èqìqÿ‰ğƒÄ^[Ã         SVƒìè    [Ã>îR ‹D$$‹t$ ‹N‰D$‰$èHíqÿ‰ğƒÄ^[ÃSVƒìè    [ÃîR ‹D$$‹t$ ‹N‰D$‰$è(íqÿ‰ğƒÄ^[ÃSVƒìè    [ÃŞíR ‹D$$‹L$(‹t$ ‹V‰L$‰D$‰$è=rÿ‰ğƒÄ^[Ã        SVƒìè    [ÃíR ‹D$$‹L$(‹t$ ‹V‰L$‰D$‰$èğëqÿ‰ğƒÄ^[Ã        SVƒìè    [Ã^íR ‹D$$‹t$ ‹N‰D$‰$èÈÊrÿ‰ğƒÄ^[ÃSVƒìè    [Ã.íR ‹D$$‹t$ ‹N‰D$‰$è8ìqÿ‰ğƒÄ^[ÃSVƒìè    [ÃşìR ‹D$$‹t$ ‹N‰D$‰$èìqÿ‰ğƒÄ^[ÃSVƒìè    [ÃÎìR òD$$‹t$ ‹FòD$‰$èD‚rÿ‰ğƒÄ^[Ã            SVƒìè    [ÃìR ‹D$$‹t$ ‹N‰L$‰$èx‚rÿ‰ğƒÄ^[ÃUSWVƒìè    [Ã\ìR ‹t$0‹D$4‹~‰D$D$‰$èÂóqÿƒì‹D$‹l$‹W‹O)Ê9Õv‰l$‰D$‰<$è+êqÿë‰l$‰D$‰$èiêqÿo‰ğƒÄ^_[]Ã            SVƒìè    [ÃŞëR ‹D$$‹t$ ‹N¶P`öÂt‹Pd‹@hëƒÀ`@Ñê‰T$‰D$‰$è½éqÿ‰ğƒÄ^[Ã     Ã               è    XÀƒëR ‹€îÿÿÿà            ‹L$‹D$‰‹	‰HÇ@    Ç@     H0Ç@    Ç@    Ç@    Ç@    ‰H$‰H(H@‰H,HL‰H@‰HDH|‰HHˆˆ   ‰H|‰ˆ€   ˆ˜   ‰ˆ„   ˆ¤   ‰ˆ˜   ‰ˆœ   ˆ´   ‰ˆ    ˆÀ   ‰ˆ´   ‰ˆ¸   ˆğ   ‰ˆ¼   Ã USWVìœ  è    [Ã©êR ‹´$°  ‹F$‹N()ÁÁù‹V‰Œ$ä   ‰„$à   ò„$à   òD$‰$è°Jrÿ‹N‰Œ$ˆ  ‰D$„$ˆ  ‰$ès+tÿ”$D  ‰”$8  ‰”$<  „$„  ‰„$@  „$ô   ‰„$à   ‰„$ä   Ç„$è      Ç„$0  ÿÿÿÿÇ„$,  ÿÿÿÿÇ„$(  ÿÿÿÿÇ„$$  ÿÿÿÿÇ„$   ÿÿÿÿÇ„$  ÿÿÿÿÇ„$  ÿÿÿÿÇ„$  ÿÿÿÿÇ„$  ÿÿÿÿÇ„$  ÿÿÿÿÇ„$  ÿÿÿÿÇ„$  ÿÿÿÿÇ„$   ÿÿÿÿÇ„$ü   ÿÿÿÿÇ„$ø   ÿÿÿÿÇ„$ô   ÿÿÿÿÇ„$ì       Ç„$ğ       ‹F@‹vD)Æ‰Ñ„º   Áşiö«ªªª‰t$$1ÿ½   ëƒÅ‹„$°  ‹@@‹(ƒàü‰D$„$à   ‰$èZ÷qÿ„Àt.‹„$°  ‹@@‹4(ƒæü‹„$<  ;„$@  s‰0ƒ„$<  ‹t$$G9÷r£ë3„$D  ‰D$„$8  ‰$ÇD$   ÇD$    èëqÿ‹„$<  ë¸‹”$8  ‹Œ$<  )ÑÁù‹´$°  ‹F‰T$0‰L$4òD$0òD$‰$è•Hrÿ‹N‰Œ$Ø   ‰D$„$Ø   ‰$èX)tÿ‹F|‹€   )ÁÁù‹V‰L$4‰D$0òD$0òD$‰$èKHrÿ‰„$Ğ   ‹N‰Œ$È   ‰D$„$È   ‰$è)tÿ„$Ğ   ‰$èÈfrÿ‰D$…À„ª  ´$¤   1í‰l$„$Ğ   ‰D$„$¸   ‰$è¢frÿƒì‹„$¸   ‰„$À   ‰´$˜   ‰´$œ   Œ$´   ‰Œ$    ‹Œ$°  ‹	‰„$   ‹„$   ‰D$‰$è‚(tÿ…À„¢   ‰l$ ‰$‰D$$è
Grÿ‰Å1ÿ…íuBëq„$¤   ‰D$„$˜   ‰$ÇD$   ÇD$    è†éqÿ‹„$œ   ë/‰|$‹D$$‰$èÀFrÿ‰Æ‹„$œ   ;„$    s‰0ƒ„$œ   G9ıuÏ‹D$$‰$èC#tÿ´$¤   ‹l$ „$À   ‰$èÙ'tÿ‰Ç…ÿtS‹„$˜   ‹Œ$œ   )ÁÁù‹”$°  ‹R‰L$4‰D$0òD$0òD$‰$èšFrÿ‰¼$ˆ   ‰D$„$ˆ   ‰$è`'tÿ‹„$˜   9ğt‰$èİåqÿE;l$…bşÿÿ‹´$°  ‹†˜   ‹œ   )ÁÁù‹V‰L$4‰D$0òD$0òD$‰$è+Frÿ‹N‰Œ$€   ‰D$„$€   ‰$èî&tÿl$<‰l$0‰l$4L$|‰L$8‹–´   ‹¾¸   )×‰è‰î„ˆ   ÁÿiÇ«ªªª‰D$$‰è1í¾   ëƒÆ‹Œ$°  ‹‘´   ‹L$8‹<2ƒçü9Ès‰8‹D$4ƒÀ‰D$4E;l$$rÎë*D$<‰D$D$0‰$ÇD$   ÇD$    è¨çqÿ‹D$4ëÀ‹t$0l$<)ğÁø‹Œ$°  ‰Ï‹O‰´$  ‰„$”  ò„$  òD$‰$è$Erÿ‹O‰L$(‰D$D$(‰$èí%tÿ‹D$09èt‰$èmäqÿ„$à   ‰$èóqÿ‹„$8  Œ$D  9Èt‰$èDäqÿÄœ  ^_[]Ã         SVƒì$è    [ÃnäR ‹t$0‹D$4‹@òD$8òD$òD$òD$‰$è„Drÿ‰ƒÄ$^[Â           USWVìÌ   è    [ÃäR ‹¼$ä   ‹G‰$è˜ïqÿ‰$ÇD$    ÇD$    ÇD$$  èHIrÿ‰„$¤   ‹G´$¤   ‰´$˜   Ç„$œ      ò„$˜   òD$‰$èşjrÿ‰G‹G‰´$   Ç„$”      ò„$   òD$‰$èÏjrÿ‰G‹G‰´$ˆ   Ç„$Œ      ò„$ˆ   òD$‰$è jrÿ‰G‹G‰´$€   Ç„$„      ò„$€   òD$‰$èqjrÿ‰G‹G‰t$xÇD$|   òD$xòD$‰$èKjrÿ‰G‹G‰$è}îqÿ‰$ÇD$    ÇD$    ÇD$  è-Hrÿ‰D$@‹w‹„$ì   ‹Œ$ğ   ‹¼$ô   ‹¬$ø   ‰Œ$¼   ‰„$¸   ò„$¸   òD$‰4$è¥krÿ‰„$Ä   ‰¬$´   ‰¼$°   ‹¼$ä   ò„$°   òD$‰4$èrkrÿ‰„$È   „$Ä   ‰„$¨   Ç„$¬      ò„$¨   òD$‰4$è[Brÿ‰D$D‹G‰$èœíqÿ‹Œ$è   ‰L$‰$ÇD$    ÇD$    èIGrÿ‰D$H‹Gò„$ü   òD$8òD$8òD$‰$èßjrÿ‰D$L‹G‰$è`çqÿ¶Œ$  ‰L$‰$ÇD$    ÇD$    èìFrÿ‰D$P‹Gò„$  òD$0òD$0òD$‰$è‚jrÿ‰D$T‹G‰$èãìqÿ‹Œ$  ‰L$‰$ÇD$    ÇD$    èFrÿ‰D$X‹G‰D$\‹G‰D$`‹G‰D$d‹G‰D$h‹G‰D$l‹Gò„$  òD$(òD$(òD$‰$èjrÿ‰D$p‹G‰$èdìqÿ‹Œ$  ‰L$‰$ÇD$    ÇD$    èFrÿ‰D$t‹GL$@‰L$ ÇD$$   òD$ òD$‰$èÆ@rÿ‰Æ€¼$    t@‹?ƒ*ºÉÿ‰D$‰$ÇD$   è^Şqÿ‰D$òD$òD$‰<$è@rÿ‰t$‰$è*@rÿ‹„$à   ‰0ÄÌ   ^_[]Â     SWVƒì0è    [ÃàR ‹L$@‹D$P‹t$D‹V‹|$H‰|$(‹|$LÇD$     ÇD$$    Æ´   ‰t$òD$ òD$‰D$‰|$‹D$(‰D$Ç$:   è   ƒÄ0^_[Â         USWVƒìl‰×è    [·„$€   ‹´$ˆ   ÃŠßR ƒ¼$”    „¹   ·è‰<$‰L$èïêqÿÍ   ‰l$‰$ÇD$    ÇD$    èDrÿ‰D$<„$„   ‹ ‰D$@‰t$D‰<$è°êqÿ‹Œ$Œ   ‰L$‰$ÇD$    ÇD$    è]Drÿ‰D$H„$   ò òD$0òD$0òD$‰<$èôgrÿ‰D$LD$<‰D$(ÇD$,   òD$(é‹   ·è‰<$‰L$è6êqÿÍ   ‰l$‰$ÇD$    ÇD$    èäCrÿ‰D$\„$„   ‹ ‰D$`‰t$d‰<$è÷éqÿ‹Œ$Œ   ‰L$‰$ÇD$    ÇD$    è¤Crÿ‰D$hD$\‰D$PÇD$T   òD$PòD$‰<$è\>rÿ‹L$‹´$˜   ‰ÇD$   ÇD$    ‰D$ ƒàüƒÀƒøw
¹  £ÁrD$‰$èŠ<rÿ‹F;Fsq…Àt<Ç    Ç@    ‹L$ ‰HƒáüƒÁƒùw
º  £Êr‹L$ƒáü‰L$‰$èÂ>rÿƒF‹D$ ƒàüƒÀƒøw
¹  £ÁrD$‰$èù:rÿƒÄl^_[]Ã‰4$ÇD$    è¡tÿ‹Féwÿÿÿ         SWVƒì0è    [Ã-İR ‹L$@‹D$P‹t$D‹V‹|$H‰|$(‹|$LÇD$     ÇD$$    Æ´   ‰t$òD$ òD$‰D$‰|$‹D$(‰D$Ç$:   è1ıÿÿƒÄ0^_[Â         USWVƒì<è    [Ã¬ÜR ‹l$`‹t$T‹~‹D$X‰D$8D$\‰D$D$0‰$è&~rÿƒì‹D$0òD$dòD$(Æ´   ‰t$òD$(òD$‰l$‰D$‹D$8‰D$Ç$   ‹L$P‰úè›üÿÿƒÄ<^_[]Â  SWVƒì0è    [ÃÜR ‹L$@‹D$P‹t$D‹V‹|$H‰|$(‹|$LòD$TòD$ Æ´   ‰t$òD$ òD$‰D$‰|$‹D$(‰D$Ç$   è%üÿÿƒÄ0^_[Â             USWVƒìLè    [ÃœÛR ‹L$d‹A‰Î‰$èçqÿ‰$ÇD$    ÇD$    ÇD$)  èÌ@rÿ‰D$ ‹n‹D$h‹L$l‹t$p‹|$t‰L$<‰D$8òD$8òD$‰,$èYdrÿ‰D$D‰|$4‰t$0òD$0òD$‰,$è9drÿ‰D$HD$D‰D$(ÇD$,   òD$(òD$‰,$è1;rÿ‰D$$‹D$d‹@L$ ‰L$ÇD$   òD$òD$‰$è;rÿ‹L$`‰ƒÄL^_[]Â   USWVƒì,è    [ÃœÚR ‹l$P‹t$T‹|$D‹G‰$èæqÿ‰$ÇD$    ÇD$    ÇD$(  èÆ?rÿ‰D$ ‹GòD$HòD$òD$òD$‰$è_crÿ‰D$$‹G‰$è0?rÿ‰t$‰l$‰$ÇD$    èx?rÿ‰D$(‹GL$ ‰L$ÇD$   òD$òD$‰$è-:rÿ‹L$@‰ƒÄ,^_[]Â              SWVƒìPè    [Ã½ÙR ‹t$`‹|$d‹G‰$è;åqÿ‰$ÇD$    ÇD$    ÇD$;  èë>rÿ‰D$(ÇD$,    ÇD$0    ‹GòD$hòD$ òD$ òD$‰$ètbrÿ‰D$4‹G‰$èÕäqÿ‰$ÇD$    ÇD$    ÇD$    è…>rÿ‰D$8‹G‰$è>rÿ‰$ÇD$    ÇD$    ÇD$    èV>rÿ‰D$<‹G‰$èç=rÿ‰$ÇD$    ÇD$    ÇD$    è'>rÿ‰D$@‹G‰$è¸=rÿ‰$ÇD$    ÇD$    ÇD$    èø=rÿ‰D$D‹G‰$èäqÿ‰$ÇD$    ÇD$    ÇD$    èÉ=rÿ‰D$H‹G‰$èêãqÿ‰$ÇD$    ÇD$    ÇD$    èš=rÿ‰D$L‹GL$(‰L$ÇD$
   òD$òD$‰$èO8rÿ‰ƒÄP^_[Â     SWVƒì è    [Ãí×R ‹t$0‹|$4ƒoËÿ‰D$‰$ÇD$   èÌÕqÿ‰D$òD$òD$‰|$‰4$ètÿƒÄ^_[Â        USWVƒìLè    [‹l$p‹t$t‹|$d‹GÃŒ×R ‰$èãqÿ‰$ÇD$    ÇD$    ÇD$$  è¶<rÿ‰D$$ÇD$(    ÇD$,    ‹GòD$hòD$òD$òD$‰$è?`rÿ‰D$0‹G‰$è âqÿ‰$ÇD$    ÇD$    ÇD$    èP<rÿ‰D$4‹G‰$èá;rÿ‰t$‰l$‰$ÇD$    è)<rÿ‰D$8‹G‰$èº;rÿ‹L$|‰L$‹L$x‰L$‰$ÇD$    èú;rÿ‰D$<‹G‰$è‹;rÿ‰$ÇD$    ÇD$    ÇD$    èË;rÿ‰D$@‹G‰$èìáqÿ‰$ÇD$    ÇD$    ÇD$    èœ;rÿ‰D$D‹G‰$è½áqÿ‹Œ$€   ‰L$‰$ÇD$    ÇD$    èj;rÿ‰D$H‹GL$$‰L$ÇD$
   òD$òD$‰$è6rÿ‹L$`‰ƒÄL^_[]Â                SWVƒìPè    [Ã­ÕR ‹t$`‹|$d‹G‰$è+áqÿ¹   L$h‰L$‰$ÇD$    ÇD$    èÖ:rÿ‰D$(ÇD$,    ÇD$0    ‹GÇD$     ÇD$$    òD$ òD$‰$è[^rÿ‰D$4‹G‰$è¼àqÿ‰$ÇD$    ÇD$    ÇD$    èl:rÿ‰D$8‹G‰$èı9rÿ‰$ÇD$    ÇD$    ÇD$    è=:rÿ‰D$<‹G‰$èÎ9rÿ‰$ÇD$    ÇD$    ÇD$    è:rÿ‰D$@‹G‰$èŸ9rÿ‰$ÇD$    ÇD$    ÇD$    èß9rÿ‰D$D‹G‰$è àqÿ‰$ÇD$    ÇD$    ÇD$    è°9rÿ‰D$HD$l‰D$D$‰$èØurÿƒì‹D$‰D$L‹GL$(‰L$ÇD$
   òD$òD$‰$èF4rÿ‰ƒÄP^_[Â            USWVƒì\è    [‹l$|‹´$€   ‹|$t‹GÃÜÓR ‰$èSßqÿ‰$ÇD$    ÇD$    ÇD$  è9rÿ‰D$4ÇD$8    ÇD$<    ‹Gò„$Œ   òD$(òD$(òD$‰$è‰\rÿ‰D$@‹G‰$èêŞqÿ‰$ÇD$    ÇD$    ÇD$    èš8rÿ‰D$D‹G‰$è+8rÿ‰t$‰l$‰$ÇD$    ès8rÿ‰D$H‹G‰$è8rÿ‹Œ$ˆ   ‰L$‹Œ$„   ‰L$‰$ÇD$    è>8rÿ‰D$L‹G‰$èÏ7rÿ‰$ÇD$    ÇD$    ÇD$    è8rÿ‰D$P‹G‰$è0Şqÿ‰$ÇD$    ÇD$    ÇD$    èà7rÿ‰D$TD$x‰D$D$ ‰$ètrÿƒì‹D$ ‰D$X‹GL$4‰L$ÇD$
   òD$òD$‰$èv2rÿ‹L$p‰ƒÄ\^_[]Â       SWVƒìPè    [ÃÒR ‹t$`‹|$d‹G‰$è‹İqÿ‰$ÇD$    ÇD$    ÇD$  è;7rÿ‰D$$ÇD$(    ÇD$,    ÇD$0    ‹G‰$èDİqÿ‰$ÇD$    ÇD$    ÇD$    èô6rÿ‰D$4‹G‰$è…6rÿ‰$ÇD$    ÇD$    ÇD$    èÅ6rÿ‰D$8‹G‰$èV6rÿ‰$ÇD$    ÇD$    ÇD$    è–6rÿ‰D$<‹G‰$è'6rÿ‰$ÇD$    ÇD$    ÇD$    èg6rÿ‰D$@‹G‰$èˆÜqÿ‰$ÇD$    ÇD$    ÇD$    è86rÿ‰D$DD$h‰D$D$ ‰$è`rrÿƒì‹D$ ‰D$HD$l‰D$D$‰$èArrÿƒì‹D$‰D$L‹GL$$‰L$ÇD$   òD$òD$‰$è¯0rÿ‰ƒÄP^_[Â     SWVƒìPè    [ÃMĞR ‹t$`‹|$d‹G‰$èËÛqÿ¹   L$h‰L$‰$ÇD$    ÇD$    èv5rÿ‰D$(ÇD$,    ÇD$0    ÇD$4    ‹G‰$èÛqÿ‰$ÇD$    ÇD$    ÇD$    è/5rÿ‰D$8‹G‰$èÀ4rÿ‰$ÇD$    ÇD$    ÇD$    è 5rÿ‰D$<‹G‰$è‘4rÿ‰$ÇD$    ÇD$    ÇD$    èÑ4rÿ‰D$@‹G‰$èb4rÿ‰$ÇD$    ÇD$    ÇD$    è¢4rÿ‰D$D‹G‰$èÃÚqÿ‰$ÇD$    ÇD$    ÇD$    ès4rÿ‰D$HD$l‰D$D$ ‰$è›prÿƒì‹D$ ‰D$L‹GL$(‰L$ÇD$
   òD$òD$‰$è	/rÿ‰ƒÄP^_[Â               USWVƒìlè    [ÃœÎR ‹´$€   ‹¼$„   ‹G‰$èÚqÿ‰$ÇD$    ÇD$    ÇD$  èÄ3rÿ‰D$@„$”   ‰$èÁtÿ‰D$D‹¬$œ   ‰l$hD$h‰$èÖ€rÿ1É„ÀDÍ‰L$8D$8‰D$D$0‰$è·orÿƒì‹D$0‰D$H‹Gò„$Œ   òD$(òD$(òD$‰$èWrÿ‰D$L‹G‰$ègÙqÿ‹Œ$˜   ‰L$‰$ÇD$    ÇD$    è3rÿ‰D$P‹G‰$è¥2rÿ‰$ÇD$    ÇD$    ÇD$    èå2rÿ‰D$T‹G‰$èv2rÿ‰$ÇD$    ÇD$    ÇD$    è¶2rÿ‰D$X‹G‰$èG2rÿ‰$ÇD$    ÇD$    ÇD$    è‡2rÿ‰D$\‹G‰$è¨Øqÿ‰$ÇD$    ÇD$    ÇD$    èX2rÿ‰D$`„$ˆ   ‰D$D$ ‰$è}nrÿƒì‹D$ ‰D$d‹GL$@‰L$ÇD$
   òD$òD$‰$èë,rÿ‰ƒÄl^_[]Â                SWVƒìPè    [Ã}ÌR ‹t$`‹|$d‹G‰$èû×qÿ‰$ÇD$    ÇD$    ÇD$*  è«1rÿ‰D$(ÇD$,    D$h‰D$D$ ‰$èËmrÿƒì‹D$ ‰D$0ÇD$4    ‹G‰$è×qÿ‰$ÇD$    ÇD$    ÇD$    èM1rÿ‰D$8‹G‰$èŞ0rÿ‰$ÇD$    ÇD$    ÇD$    è1rÿ‰D$<‹G‰$è¯0rÿ‰$ÇD$    ÇD$    ÇD$    èï0rÿ‰D$@‹G‰$è€0rÿ‰$ÇD$    ÇD$    ÇD$    èÀ0rÿ‰D$D‹G‰$èáÖqÿ‰$ÇD$    ÇD$    ÇD$    è‘0rÿ‰D$HD$l‰D$D$‰$è¹lrÿƒì‹D$‰D$L‹GL$(‰L$ÇD$
   òD$òD$‰$è'+rÿ‰ƒÄP^_[Â             USWVƒìLè    [Ã¼ÊR ‹l$p‹t$t‹|$d‹G‰$è6Öqÿ‰$ÇD$    ÇD$    ÇD$  èæ/rÿ‰D$$ÇD$(    D$h‰D$D$ ‰$èlrÿƒì‹D$ ‰D$,ÇD$0    ‹G‰$èØÕqÿ‰$ÇD$    ÇD$    ÇD$    èˆ/rÿ‰D$4‹G‰$è/rÿ‰$ÇD$    ÇD$    ÇD$    èY/rÿ‰D$8‹G‰$èê.rÿ‰$ÇD$    ÇD$    ÇD$    è*/rÿ‰D$<‹G‰$è».rÿ‰t$‰l$‰$ÇD$    è/rÿ‰D$@‹G‰$è$Õqÿ‹L$x‰L$‰$ÇD$    ÇD$    èÔ.rÿ‰D$DD$l‰D$D$‰$èüjrÿƒì‹D$‰D$H‹GL$$‰L$ÇD$
   òD$òD$‰$èj)rÿ‹L$`‰ƒÄL^_[]Â           USWVƒìlè    [‹´$    ‹¼$„   ‹GÃüÈR ‰$ètÔqÿ‰$ÇD$    ÇD$    ÇD$  è$.rÿ‰D$@„$”   ‰$è!
tÿ‰D$D‹¬$ˆ   ‰l$hD$h‰$è6{rÿ1É„ÀDÍ‰L$8D$8‰D$D$0‰$èjrÿƒì‹D$0‰D$H‹Gò„$Œ   òD$(òD$(òD$‰$èfQrÿ‰D$L‹G‰$èÇÓqÿ‹Œ$˜   ‰L$‰$ÇD$    ÇD$    èt-rÿ‰D$P‹G‰$è-rÿ‰t$‹Œ$œ   ‰L$‰$ÇD$    èF-rÿ‰D$T‹G‰$è×,rÿ‹Œ$¨   ‰L$‹Œ$¤   ‰L$‰$ÇD$    è-rÿ‰D$X‹G‰$è¢,rÿ‹Œ$°   ‰L$‹Œ$¬   ‰L$‰$ÇD$    èÜ,rÿ‰D$\‹G‰$èıÒqÿ‹Œ$´   ‰L$‰$ÇD$    ÇD$    èª,rÿ‰D$`„$¸   ‰D$D$ ‰$èÏhrÿƒì‹D$ ‰D$d‹GL$@‰L$ÇD$
   òD$òD$‰$è='rÿ‹Œ$€   ‰ƒÄl^_[]Â           USWVƒìlè    [ÃÌÆR ‹¬$„   ¾   ´$    ‹E‰$è?Òqÿ‰$ÇD$    ÇD$    ÇD$  èï+rÿ‰D$<„$”   ‰$èìtÿ‰D$@‹¼$ˆ   ‰|$hD$h‰$èyrÿ1É„ÀDÏ‰L$8D$8‰D$D$0‰$èâgrÿƒì‹D$0‰D$D‹Eò„$Œ   òD$(òD$(òD$‰$è1Orÿ‰D$H‹E‰$è’Ñqÿ‹Œ$˜   ‰L$‰$ÇD$    ÇD$    è?+rÿ‰D$L‹E‰$èĞ*rÿ‰$ÇD$    ÇD$    ÇD$    è+rÿ‰D$P‹E‰$è¡*rÿ‰$ÇD$    ÇD$    ÇD$    èá*rÿ‰D$T‹E‰$èr*rÿ‰$ÇD$    ÇD$    ÇD$    è²*rÿ‰D$X‹E‰$èÓĞqÿ‰t$‰$ÇD$    ÇD$    è‡*rÿ‰D$\„$œ   ‰D$D$ ‰$è¬frÿƒì‹D$ ‰D$`‹„$¤   ‰D$d‹EL$<‰L$ÇD$   òD$òD$‰$è%rÿ‹Œ$€   ‰ƒÄl^_[]Â             USWVƒì|è    [‹´$¬   ‹¼$”   ‹GÃœÄR ‰$èĞqÿ‰$ÇD$    ÇD$    ÇD$  èÄ)rÿ‰D$@„$    ‰$èÁtÿ‰D$D‹¬$    ‰l$xD$x‰$èÖvrÿ1É„ÀDÍ‰L$H‹Gò„$˜   òD$8òD$8òD$‰$è%Mrÿ‰D$L‹G‰$è†Ïqÿ‹Œ$¤   ‰L$‰$ÇD$    ÇD$    è3)rÿ‰D$P‹G‰$èÄ(rÿ‰t$‹Œ$¨   ‰L$‰$ÇD$    è)rÿ‰D$T‹G‰$è–(rÿ‹Œ$´   ‰L$‹Œ$°   ‰L$‰$ÇD$    èĞ(rÿ‰D$X‹G‰$èa(rÿ‹Œ$¼   ‰L$‹Œ$¸   ‰L$‰$ÇD$    è›(rÿ‰D$\‹G‰$è¼Îqÿ‹Œ$À   ‰L$‰$ÇD$    ÇD$    èi(rÿ‰D$`‹„$Ä   ‰D$d‹Gò„$È   òD$0òD$0òD$‰$èôKrÿ‰D$h‹Gò„$Ğ   òD$(òD$(òD$‰$èÊKrÿ‰D$l‹Gò„$Ø   òD$ òD$ òD$‰$è Krÿ‰D$p‹G‰$èÎqÿ‹Œ$à   ‰L$‰$ÇD$    ÇD$    è®'rÿ‰D$t‹GL$@‰L$ÇD$   òD$òD$‰$èc"rÿ‹Œ$   ‰ƒÄ|^_[]Â USWVƒì\è    [‹¼$Œ   ‹l$t‹EÃüÁR ‰$èwÍqÿ‰$ÇD$    ÇD$    ÇD$  è''rÿ‰D$,„$€   ‰$è$tÿ‰D$0‹´$€   ‰t$XD$X‰$è9trÿ1É„ÀDÎ‰L$4‹EòD$xòD$ òD$ òD$‰$è‹Jrÿ‰D$8‹E‰$èìÌqÿ‹Œ$„   ‰L$‰$ÇD$    ÇD$    è™&rÿ‰D$<‹E‰$è*&rÿ‰|$‹Œ$ˆ   ‰L$‰$ÇD$    èk&rÿ‰D$@‹E‰$èü%rÿ‹Œ$”   ‰L$‹Œ$   ‰L$‰$ÇD$    è6&rÿ‰D$D‹E‰$èÇ%rÿ‹Œ$œ   ‰L$‹Œ$˜   ‰L$‰$ÇD$    è&rÿ‰D$H‹E‰$è"Ìqÿ‹Œ$    ‰L$‰$ÇD$    ÇD$    èÏ%rÿ‰D$L‹„$¤   ‰D$P‹„$¨   ‰D$T‹EL$,‰L$ÇD$   òD$òD$‰$èn rÿ‹L$p‰ƒÄ\^_[]Â               SWVƒìPè    [Ãı¿R ‹t$`‹|$d‹G‰$è{Ëqÿ‰$ÇD$    ÇD$    ÇD$ B è+%rÿ‰D$0‹GòD$hòD$(òD$(òD$‰$èÄHrÿ‰D$4‹D$p‰D$8‹G‰$èËqÿ‹L$t‰L$‰$ÇD$    ÇD$    èÍ$rÿ‰D$<‹GòD$xòD$ òD$ òD$‰$èfHrÿ‰D$@‹Gò„$€   òD$òD$òD$‰$è<Hrÿ‰D$D‹G‰$èÊqÿ‹Œ$ˆ   ‰L$‰$ÇD$    ÇD$    èJ$rÿ‰D$H‹„$Œ   ‰D$L‹GL$0‰L$ÇD$   òD$òD$‰$èôrÿ‰ƒÄP^_[Â          USWVƒì\è    [ÃŒ¾R ‹¬$ˆ   ‹|$t‹G‰$èÊqÿ‰$ÇD$    ÇD$    ÇD$/  è·#rÿ‰D$<‹t$x‰t$XD$X‰$èßprÿ1É„ÀDÎ‰L$8D$8‰D$D$0‰$èÀ_rÿƒì‹D$0‰D$@‹GòD$|òD$(òD$(òD$‰$èGrÿ‰D$D„$„   ‰D$D$ ‰$èw_rÿƒì‹D$ ‰D$H‰l$L‹G‰$èMÉqÿ‹Œ$Œ   ‰L$‰$ÇD$    ÇD$    èú"rÿ‰D$P‹G‰$èÉqÿ‹Œ$   ‰L$‰$ÇD$    ÇD$    èÈ"rÿ‰D$T‹GL$<‰L$ÇD$   òD$òD$‰$è}rÿ‹L$p‰ƒÄ\^_[]Â              USWVƒì\è    [Ã½R ‹´$Œ   ‹|$t‹G‰$è‡Èqÿ¹   L$x‰L$‰$ÇD$    ÇD$    è2"rÿ‰D$8‹l$|‰l$XD$X‰$èZorÿ1É„ÀDÍ‰L$0D$0‰D$D$(‰$è;^rÿƒì‹D$(‰D$<‹Gò„$€   òD$ òD$ òD$‰$èŠErÿ‰D$@„$ˆ   ‰D$D$‰$èï]rÿƒì‹D$‰D$D‰t$H‹„$   ‰D$L‹G‰$èºÇqÿ‹Œ$”   ‰L$‰$ÇD$    ÇD$    èg!rÿ‰D$P‹G‰$èˆÇqÿ‹Œ$˜   ‰L$‰$ÇD$    ÇD$    è5!rÿ‰D$T‹GL$8‰L$ÇD$   òD$òD$‰$èêrÿ‹L$p‰ƒÄ\^_[]Â           USWVƒìLè    [Ã|»R ‹D$`‹L$d‹T$x‹t$|‹¼$€   ‹l$h‰l$H‹l$t‰l$8‹¬$„   òD$lòD$@‰l$(‰|$$‰t$ ‰T$‹T$8‰T$òD$@òD$‹T$H‰T$‰L$‰$ÇD$0   èƒüsÿƒÄH^_[]Â          USWVƒìLè    [ÃÜºR ‹¬$„   ‹´$ˆ   ‹|$d‹D$h‰D$HòD$lòD$@‹D$t‰D$8‹GòD$xòD$0òD$0òD$‰$è¬Crÿ‰t$(‰l$$‹Œ$€   ‰L$ ‰D$‹D$8‰D$òD$@òD$‹D$H‰D$‰|$‹D$`‰$ÇD$A  èÁûsÿƒÄH^_[]Â        USWVƒìLè    [ÃºR ‹D$`‹L$d‹T$|‹´$€   ‹¼$„   ‹l$h‰l$HòD$lòD$@‹l$t‰l$8‹l$x‰|$(‰t$$‰T$ ‰l$‹T$8‰T$òD$@òD$‹T$H‰T$‰L$‰$ÇD$A  è#ûsÿƒÄH^_[]Â          USWVìŒ   è    [‹¼$¤   ‹GÃy¹R ‰$èøÄqÿ‰$ÇD$    ÇD$    ÇD$  è¨rÿ‰D$L„$´   ‰$è¥úsÿ‰D$P‹¬$¨   ‰¬$ˆ   „$ˆ   ‰$è´krÿ1ö„ÀEî‰l$HD$H‰D$D$@‰$è•Zrÿƒì‹D$@‰D$T‹Gò„$¬   òD$8òD$8òD$‰$èäArÿ‰D$X‹G‰$èEÄqÿ‹Œ$¸   ‰L$‰$ÇD$    ÇD$    èòrÿ‰D$\‹G‰$èƒrÿ‹Œ$À   ‰L$‹Œ$¼   ‰L$‰$ÇD$    è½rÿ‰D$`‹G‰$èNrÿ‹Œ$È   ‰L$‹Œ$Ä   ‰L$‰$ÇD$    èˆrÿ‰D$d‹G‰$è©Ãqÿ‹Œ$Ğ   ‰L$‹Œ$Ì   ‰L$‰$ÇD$    èSrÿ‰D$h‹G‰$ètÃqÿ‹Œ$Ô   ‰L$‰$ÇD$    ÇD$    è!rÿ‰D$l„$Ø   ‰D$D$0‰$èFYrÿƒì‹D$0‰D$p‹„$Ü   ‰D$t‹G‰$èÃqÿ‰$ÇD$    ÇD$    ÇD$    èÅrÿ‰D$x„$à   ‰D$D$(‰$èêXrÿƒì‹D$(‰D$|‹„$ä   ‰„$€   ‹¬$ì   …ít*„$è   ‹Oò òD$ òD$ òD$‰$è@rÿ‰Æ‰´$„   ‹GL$L‰L$ÇD$   òD$òD$‰$èrÿ…í‹Œ$    ‰t‰D$‹D$‰D$‰<$è=øsÿÄŒ   ^_[]Â SWVƒì è    [Ã¶R ‹t$0~@‹D$4ÇD$   ÇD$    ‰D$ƒàüƒÀƒøw
¹  £ÁrD$‰$èşrÿ‹FD;FHsp…Àt<Ç    Ç@    ‹L$‰HƒáüƒÁƒùw
º  £Êr‹L$ƒáü‰L$‰$è6rÿƒFD‹D$ƒàüƒÀƒøw
¹  £ÁrD$‰$èmrÿƒÄ ^_[Ã‰<$ÇD$    è÷sÿ‹FDéxÿÿÿ              USWVìŒ   è    [‹¼$¤   ‹GÃ™µR ‰$èÁqÿ‰$ÇD$    ÇD$    ÇD$  èÈrÿ‰D$L„$´   ‰$èÅösÿ‰D$P‹¬$¨   ‰¬$ˆ   „$ˆ   ‰$èÔgrÿ1ö„ÀEî‰l$HD$H‰D$D$@‰$èµVrÿƒì‹D$@‰D$T‹Gò„$¬   òD$8òD$8òD$‰$è>rÿ‰D$X‹G‰$èeÀqÿ‹Œ$¸   ‰L$‰$ÇD$    ÇD$    èrÿ‰D$\‹G‰$è£rÿ‹Œ$À   ‰L$‹Œ$¼   ‰L$‰$ÇD$    èİrÿ‰D$`‹G‰$ènrÿ‹Œ$È   ‰L$‹Œ$Ä   ‰L$‰$ÇD$    è¨rÿ‰D$d‹G‰$èÉ¿qÿ‰$ÇD$    ÇD$    ÇD$    èyrÿ‰D$h‹G‰$èš¿qÿ‹Œ$Ì   ‰L$‰$ÇD$    ÇD$    èGrÿ‰D$l„$Ğ   ‰D$D$0‰$èlUrÿƒì‹D$0‰D$p‹„$Ô   ‰D$t‹G‰$è;¿qÿ‹Œ$Ø   ‰L$‰$ÇD$    ÇD$    èèrÿ‰D$x„$Ü   ‰D$D$(‰$èUrÿƒì‹D$(‰D$|Ç„$€       ‹¬$ä   …ít*„$à   ‹Oò òD$ òD$ òD$‰$èD<rÿ‰Æ‰´$„   ‹GL$L‰L$ÇD$   òD$òD$‰$è4rÿ…í‹Œ$    ‰t‰D$‹D$‰D$‰<$ècôsÿÄŒ   ^_[]Â       USWVƒì|è    [‹¼$”   ‹GÃ¬²R ‰$è+¾qÿ‰$ÇD$    ÇD$    ÇD$  èÛrÿ‰D$<„$¤   ‰$èØósÿ‰D$@‹¬$˜   ‰l$xD$x‰$èídrÿ1ö„ÀEî‰l$8D$8‰D$D$0‰$èÎSrÿƒì‹D$0‰D$D‹Gò„$œ   òD$(òD$(òD$‰$è;rÿ‰D$H‹G‰$è~½qÿ‹Œ$¨   ‰L$‰$ÇD$    ÇD$    è+rÿ‰D$L‹G‰$è¼rÿ‹Œ$°   ‰L$‹Œ$¬   ‰L$‰$ÇD$    èörÿ‰D$P‹G‰$è‡rÿ‹Œ$¸   ‰L$‹Œ$´   ‰L$‰$ÇD$    èÁrÿ‰D$T‹G‰$èRrÿ‰$ÇD$    ÇD$    ÇD$    è’rÿ‰D$X‹G‰$è³¼qÿ‹Œ$¼   ‰L$‰$ÇD$    ÇD$    è`rÿ‰D$\ÇD$`    ‹„$À   ‰D$d‹G‰$èn¼qÿ‹Œ$Ä   ‰L$‰$ÇD$    ÇD$    èrÿ‰D$hÇD$l    ÇD$p    ‹¬$Ì   …ít*„$È   ‹Oò òD$ òD$ òD$‰$è”9rÿ‰Æ‰t$t‹GL$<‰L$ÇD$   òD$òD$‰$è‡rÿ…í‹Œ$   ‰t‰D$‹D$‰D$‰<$è¶ñsÿƒÄ|^_[]Â             USWVƒìlè    [Ãü¯R ‹´$€   ‹¼$„   ‹G‰$èt»qÿ‰$ÇD$    ÇD$    ÇD$  è$rÿ‰D$$‹G‰$èE»qÿ‰$èıºqÿ‰D$(ÇD$,    ‹oƒ=&Éÿ‰D$`‰$ÇD$   è„­qÿ‰D$dòD$`òD$‰,$èŒ8rÿ‰D$0‹G‰$èíºqÿ‰$ÇD$    ÇD$    ÇD$    èrÿ‰D$4‹G‰$è.rÿ‰$ÇD$    ÇD$    ÇD$    ènrÿ‰D$8‹G‰$èÿrÿ‰$ÇD$    ÇD$    ÇD$    è?rÿ‰D$<‹G‰$èĞrÿ‰$ÇD$    ÇD$    ÇD$    èrÿ‰D$@‹G‰$è1ºqÿ‹Œ$   ‰L$‰$ÇD$    ÇD$    èŞrÿ‰D$DÇD$H    ‹„$Œ   ‰D$L‹G‰$èì¹qÿ‰$ÇD$    ÇD$    ÇD$    èœrÿ‰D$PÇD$T    ÇD$X    ÇD$\    ‹GL$$‰L$ÇD$   òD$òD$‰$è9rÿ‰ƒÄl^_[]Â              USWVìŒ   è    [‹¼$¤   ‹GÃÉ­R ‰$èH¹qÿ‰$ÇD$    ÇD$    ÇD$  èørÿ‰D$L„$´   ‰$èõîsÿ‰D$P‹¬$¨   ‰¬$ˆ   „$ˆ   ‰$è`rÿ1ö„ÀEî‰l$HD$H‰D$D$@‰$èåNrÿƒì‹D$@‰D$T‹Gò„$¬   òD$8òD$8òD$‰$è46rÿ‰D$X‹G‰$è•¸qÿ‹Œ$¸   ‰L$‰$ÇD$    ÇD$    èBrÿ‰D$\‹G‰$èÓrÿ‹Œ$À   ‰L$‹Œ$¼   ‰L$‰$ÇD$    èrÿ‰D$`‹G‰$èrÿ‹Œ$È   ‰L$‹Œ$Ä   ‰L$‰$ÇD$    èØrÿ‰D$d‹G‰$èù·qÿ‰$ÇD$    ÇD$    ÇD$    è©rÿ‰D$h‹G‰$èÊ·qÿ‰$ÇD$    ÇD$    ÇD$    èzrÿ‰D$l„$Ğ   ‰D$D$0‰$èŸMrÿƒì‹D$0‰D$p‹„$Ì   ‰D$t‹G‰$èn·qÿ‰$ÇD$    ÇD$    ÇD$    èrÿ‰D$xÇD$|    Ç„$€       ‹¬$Ø   …ít*„$Ô   ‹Oò òD$(òD$(òD$‰$è”4rÿ‰Æ‰´$„   ‹GL$L‰L$ ÇD$$   òD$ òD$‰$è„rÿ‰Æ‹„$    ‰0‹G(;G,s+‰0ƒG(…ít‰t$‹D$‰D$‰<$è£ìsÿÄŒ   ^_[]Â G$O0‰L$‰$ÇD$   ÇD$    èd­qÿ‹G(ë®               USWVƒìlè    [‹´$Œ   ‹¼$„   ‹GÃ¼ªR ‰$è4¶qÿ‰$ÇD$    ÇD$    ÇD$  èärÿ‰D$$ÇD$(    ÇD$,    ‹oƒ=&Éÿ‰D$`‰$ÇD$   èS¨qÿ‰D$dòD$`òD$‰,$è[3rÿ‰D$0‹G‰$è¼µqÿ‰$ÇD$    ÇD$    ÇD$    èlrÿ‰D$4‹G‰$èırÿ‰t$‹Œ$ˆ   ‰L$‰$ÇD$    è>rÿ‰D$8‹G‰$èÏrÿ‹Œ$”   ‰L$‹Œ$   ‰L$‰$ÇD$    è	rÿ‰D$<‹G‰$è*µqÿ‰$ÇD$    ÇD$    ÇD$    èÚrÿ‰D$@‹G‰$èû´qÿ‰$ÇD$    ÇD$    ÇD$    è«rÿ‰D$D„$˜   ‰D$D$ ‰$èĞJrÿƒì‹D$ ‰D$H‹„$œ   ‰D$L‹G‰$èŸ´qÿ‰$ÇD$    ÇD$    ÇD$    èOrÿ‰D$PÇD$T    ÇD$X    ÇD$\    ‹GL$$‰L$ÇD$   òD$òD$‰$èìrÿ‹Œ$€   ‰ƒÄl^_[]Â          USWVƒìlè    [‹´$Œ   ‹¼$„   ‹GÃ|¨R ‰$èô³qÿ‰$ÇD$    ÇD$    ÇD$  è¤rÿ‰D$$ÇD$(    ÇD$,    ‹oƒ=&Éÿ‰D$`‰$ÇD$   è¦qÿ‰D$dòD$`òD$‰,$è1rÿ‰D$0‹G‰$è|³qÿ‰$ÇD$    ÇD$    ÇD$    è,rÿ‰D$4‹G‰$è½rÿ‰t$‹Œ$ˆ   ‰L$‰$ÇD$    èşrÿ‰D$8‹G‰$èrÿ‹Œ$”   ‰L$‹Œ$   ‰L$‰$ÇD$    èÉrÿ‰D$<‹G‰$èê²qÿ‰$ÇD$    ÇD$    ÇD$    èšrÿ‰D$@‹G‰$è»²qÿ‰$ÇD$    ÇD$    ÇD$   èkrÿ‰D$D„$˜   ‰D$D$ ‰$èHrÿƒì‹D$ ‰D$H‹„$œ   ‰D$L‹G‰$è_²qÿ‰$ÇD$    ÇD$    ÇD$    èrÿ‰D$PÇD$T    ÇD$X    ÇD$\    ‹GL$$‰L$ÇD$   òD$òD$‰$è¬rÿ‹Œ$€   ‰ƒÄl^_[]Â          USWVƒì\è    [Ã<¦R |$x‹t$p‰<$ÇD$   èÅFrÿ¨@t	‹‰é%  D$4‰D$(‰D$,t$X‰t$0‹‰D$‹@‰D$…Àto1í|$4ë‹t$0‰l$‹D$‰$èÜrÿ9÷s‰‹|$,ƒÇ‰|$,E9l$uÔë.‰ÆD$4‰D$D$(‰$ÇD$   ÇD$    è¨qÿ‰ğ‹|$,ë¼‹|$tD$x‰$ÇD$   èFrÿ‰ÆƒÎ@‹G‰$èì°qÿ‰t$‰$ÇD$    ÇD$    è 
rÿ‹L$(‰A ‹G‹L$(‰L$ ‹T$,)ÊÁú‰T$$òD$ òD$‰$èMrÿ‹L$p‰‹D$(L$49Èt‰$è£¤qÿƒÄ\^_[]Â          USWVƒì\è    [ÃÌ¤R |$x‹t$p‰<$ÇD$   èUErÿöÄt	‹‰é'  D$4‰D$(‰D$,t$X‰t$0‹‰D$‹@‰D$…Àtn1í|$4ë‹t$0‰l$‹D$‰$èlrÿ9÷s‰‹|$,ƒÇ‰|$,E9l$uÔë.‰ÆD$4‰D$D$(‰$ÇD$   ÇD$    èœ¦qÿ‰ğ‹|$,ë¼‹|$tD$x‰$ÇD$   èœDrÿ‰ÆÎ@  ‹G‰$èy¯qÿ‰t$‰$ÇD$    ÇD$    è-	rÿ‹L$(‰A ‹G‹L$(‰L$ ‹T$,)ÊÁú‰T$$òD$ òD$‰$èÚrÿ‹L$p‰‹D$(L$49Èt‰$è0£qÿƒÄ\^_[]Â       SWVƒì è    [Ã]£R ‹t$0‹|$4‹G‰$èÛ®qÿ‰$ÇD$    ÇD$    ÇD$  è‹rÿ‰D$‹GL$‰L$ÇD$   òD$òD$‰$è@rÿ‰ƒÄ ^_[Â      USWVƒì|è    [‹¼$”   ‹GÃÜ¢R ‰$è[®qÿ¹   Œ$˜   ‰L$‰$ÇD$    ÇD$    èrÿ‰D$<„$¨   ‰$è äsÿ‰D$@‹¬$¤   ‰l$xD$x‰$èUrÿ1ö„ÀEî‰l$8D$8‰D$D$0‰$èöCrÿƒì‹D$0‰D$D‹Gò„$œ   òD$(òD$(òD$‰$èE+rÿ‰D$H‹G‰$è¦­qÿ‹Œ$¬   ‰L$‰$ÇD$    ÇD$    èSrÿ‰D$L‹G‰$èärÿ‹Œ$¸   ‰L$‹Œ$´   ‰L$‰$ÇD$    èrÿ‰D$P‹G‰$è¯rÿ‹Œ$À   ‰L$‹Œ$¼   ‰L$‰$ÇD$    èérÿ‰D$T‹G‰$è
­qÿ‰$ÇD$    ÇD$    ÇD$    èºrÿ‰D$X‹G‰$èÛ¬qÿ‰$ÇD$    ÇD$    ÇD$   è‹rÿ‰D$\ÇD$`    ÇD$d    ‹G‰$èœ¬qÿ‹Œ$°   ‰L$‰$ÇD$    ÇD$    èIrÿ‰D$hÇD$l    ÇD$p    ‹¬$È   …ít*„$Ä   ‹Oò òD$ òD$ òD$‰$èÂ)rÿ‰Æ‰t$t‹GL$<‰L$ÇD$   òD$òD$‰$èµ rÿ…í‹Œ$   ‰t‰D$‹D$‰D$‰<$èäásÿƒÄ|^_[]Â           USWVƒì|è    [‹¼$”   ‹GÃ, R ‰$è««qÿ¹   Œ$˜   ‰L$‰$ÇD$    ÇD$    èSrÿ‰D$<„$¨   ‰$èPásÿ‰D$@‹¬$¤   ‰l$xD$x‰$èeRrÿ1ö„ÀEî‰l$8D$8‰D$D$0‰$èFArÿƒì‹D$0‰D$D‹Gò„$œ   òD$(òD$(òD$‰$è•(rÿ‰D$H‹G‰$èöªqÿ‹Œ$¬   ‰L$‰$ÇD$    ÇD$    è£rÿ‰D$L‹G‰$è4rÿ‹Œ$¸   ‰L$‹Œ$´   ‰L$‰$ÇD$    ènrÿ‰D$P‹G‰$èÿrÿ‹Œ$À   ‰L$‹Œ$¼   ‰L$‰$ÇD$    è9rÿ‰D$T‹G‰$èZªqÿ‰$ÇD$    ÇD$    ÇD$    è
rÿ‰D$X‹G‰$è+ªqÿ‰$ÇD$    ÇD$    ÇD$   èÛrÿ‰D$\ÇD$`    ÇD$d    ‹G‰$èì©qÿ‹Œ$°   ‰L$‰$ÇD$    ÇD$    è™rÿ‰D$hÇD$l    ÇD$p    ‹¬$È   …ít*„$Ä   ‹Oò òD$ òD$ òD$‰$è'rÿ‰Æ‰t$t‹GL$<‰L$ÇD$   òD$òD$‰$è%rÿ…í‹Œ$   ‰t‰D$‹D$‰D$‰<$è4ßsÿƒÄ|^_[]Â           USWVƒì,è    [‹l$H‹t$L‹|$D‹GÃ|R ‰$èö¨qÿ‰$ÇD$    ÇD$    ÇD$!  è¦rÿ‰D$ ‹G‰$è7rÿ‰t$‰l$‰$ÇD$    èrÿ‰D$$‹G‰$èrÿ‹L$T‰L$‹L$P‰L$‰$ÇD$    èPrÿ‰D$(‹GL$ ‰L$ÇD$   òD$òD$‰$èıqÿ‹L$@‰ƒÄ,^_[]Â      USWVƒìlè    [ÃœœR ‹¬$„   ‹E‰$è¨qÿ‰$ÇD$    ÇD$    ÇD$4  èËrÿ‰D$8‹E‰$èì§qÿ‰$è¤§qÿ‰D$<ÇD$@    ‹E‹¼$ˆ   ‹´$Œ   ‰t$4‰|$0òD$0òD$‰$è;%rÿ‰D$D‹E‰t$,‰|$(òD$(òD$‰$è%rÿ‰D$H‹Eò„$   òD$ òD$ òD$‰$èî$rÿ‰D$L‹„$˜   ‰D$P‹E‰$èD§qÿ‹Œ$œ   ‰L$‰$ÇD$    ÇD$    èñ rÿ‰D$T‹„$    ‰D$X‹E‰$è§qÿ¶Œ$¤   ‰L$‰$ÇD$    ÇD$    è³ rÿ‰D$\‹E‰$èÔ¦qÿ‰$ÇD$    ÇD$    ÇD$   è„ rÿ‰D$`‹„$¨   ‰D$dÇD$h    ‹EL$8‰L$ÇD$   òD$òD$‰$è&ûqÿ‰Æ‹…œ   ;…    s‰0ƒ…œ   ‹„$€   ‰0ƒÄl^_[]Â …˜   ¤   ‰L$‰$ÇD$   ÇD$    èqÿ‹…œ   ë´          SWVƒìPè    [ÃmšR ‹t$`‹L$d‹T$tŠD$|‹|$l‰|$L‰|$D‹|$h‰|$H‰|$@‹|$p‰|$8‹|$x‰|$0‹¼$€   ‰|$(¶À‰D$$‹D$0‰D$ ‰T$‹D$8‰D$òD$@òD$òD$HòD$‰L$‰4$èŒÛsÿƒÄL^_[Â    USWVƒìlè    [ÃÌ™R ‹¬$„   ‹E‰$èK¥qÿ‰$ÇD$    ÇD$    ÇD$4  èûşqÿ‰D$4‹E‰$è¥qÿ‰$èÔ¤qÿ‰D$8‹´$ˆ   ‰t$hD$h‰$è	Lrÿ1É„ÀDÎ‰L$<‹E‹´$Œ   ‹¼$   ‰|$,‰t$(òD$(òD$‰$èQ"rÿ‰D$@‹E‰|$$‰t$ òD$ òD$‰$è."rÿ‰D$D‹Eò„$”   òD$òD$òD$‰$è"rÿ‰D$H‹„$œ   ‰D$L‹E‰$èZ¤qÿ‹Œ$    ‰L$‰$ÇD$    ÇD$    èşqÿ‰D$P‹„$¤   ‰D$T‹E‰$è¤qÿ¶Œ$¨   ‰L$‰$ÇD$    ÇD$    èÉıqÿ‰D$X‹E‰$èê£qÿ‰$ÇD$    ÇD$    ÇD$   èšıqÿ‰D$\‹„$¬   ‰D$`‹„$°   ‰D$d‹EL$4‰L$ÇD$   òD$òD$‰$è9øqÿ‰Æ‹…œ   ;…    s‰0ƒ…œ   ‹„$€   ‰0ƒÄl^_[]Â …˜   ¤   ‰L$‰$ÇD$   ÇD$    è%šqÿ‹…œ   ë´             USWVƒìLè    [Ã|—R ‹l$d‹´$Œ   ‹|$l‰|$HD$H‰$èşIrÿ‹E‰$èã¢qÿ¹   L$h‰L$‰$ÇD$    ÇD$    èüqÿ‰D$(‰|$HD$H‰$èºIrÿ1É„ÀDÏ‰L$,‹EòD$pòD$ òD$ òD$‰$è rÿ‰D$0‹D$x‰D$4‹E‰$èe¢qÿÁæt$|‰t$‰$ÇD$    ÇD$    èüqÿ‰D$8‹„$€   ‰D$<‹E‰$è(¢qÿ‹Œ$ˆ   ‰L$‰$ÇD$    ÇD$    èÕûqÿ‰D$@‹E‰$èö¡qÿ‰$è®¡qÿ‰D$D‹EL$(‰L$ÇD$   òD$òD$‰$èsöqÿ‰Æ€¼$„    t2‰|$D$‰$èG9rÿƒì‹E ‹L$‰L$‰$è±×sÿ‰t$‰$èåõqÿ‹D$`‰0ƒÄL^_[]Â      USWVƒì\è    [ÃÌ•R ‹¬$œ   ‹|$t‹G‰$èG¡qÿ¹   L$x‰L$‰$ÇD$    ÇD$    èòúqÿ‰D$4‹t$|‰t$XD$X‰$èHrÿ1É„ÀDÎ‰L$8‹Gò„$€   òD$(òD$(òD$‰$èirÿ‰D$<‹„$ˆ   ‰D$@‹G‰$è¿ qÿÁå¬$Œ   ‰l$‰$ÇD$    ÇD$    èiúqÿ‰D$D‹„$   ‰D$H‹G‰$è qÿ‰$è7 qÿ‰D$L‹G‰$èh qÿ‰$è  qÿ‰D$P‹Gò„$”   òD$ òD$ òD$‰$èæôqÿ‰D$T‹GL$4‰L$ÇD$	   òD$òD$‰$tate_pol_flow_match(struct xfrm_state *x,
			struct xfrm_policy *xp, struct flowi *fl)
{
	return 1;
}

static inline int security_xfrm_decode_session(struct sk_buff *skb, u32 *secid)
{
	return 0;
}

static inline void security_skb_classify_flow(struct sk_buff *skb, struct flowi *fl)
{
}

#endif	/* CONFIG_SECURITY_NETWORK_XFRM */

#ifdef CONFIG_SECURITY_PATH
int security_path_unlink(struct path *dir, struct dentry *dentry);
int security_path_mkdir(struct path *dir, struct dentry *dentry, int mode);
int security_path_rmdir(struct path *dir, struct dentry *dentry);
int security_path_mknod(struct path *dir, struct dentry *dentry, int mode,
			unsigned int dev);
int security_path_truncate(struct path *path, loff_t length,
			   unsigned int time_attrs);
int security_path_symlink(struct path *dir, struct dentry *dentry,
			  const char *old_name);
int security_path_link(struct dentry *old_dentry, struct path *new_dir,
		       struct dentry *new_dentry);
int security_path_rename(struct path *old_dir, struct dentry *old_dentry,
			 struct path *new_dir, struct dentry *new_dentry);
#else	/* CONFIG_SECURITY_PATH */
static inline int security_path_unlink(struct path *dir, struct dentry *dentry)
{
	return 0;
}

static inline int security_path_mkdir(struct path *dir, struct dentry *dentry,
				      int mode)
{
	return 0;
}

static inline int security_path_rmdir(struct path *dir, struct dentry *dentry)
{
	return 0;
}

static inline int security_path_mknod(struct path *dir, struct dentry *dentry,
				      int mode, unsigned int dev)
{
	return 0;
}

static inline int security_path_truncate(struct path *path, loff_t length,
					 unsigned int time_attrs)
{
	return 0;
}

static inline int security_path_symlink(struct path *dir, struct dentry *dentry,
					const char *old_name)
{
	return 0;
}

static inline int security_path_link(struct dentry *old_dentry,
				     struct path *new_dir,
				     struct dentry *new_dentry)
{
	return 0;
}

static inline int security_path_rename(struct path *old_dir,
				       struct dentry *old_dentry,
				       struct path *new_dir,
				       struct dentry *new_dentry)
{
	return 0;
}
#endif	/* CONFIG_SECURITY_PATH */

#ifdef CONFIG_KEYS
#ifdef CONFIG_SECURITY

int security_key_alloc(struct key *key, const struct cred *cred, unsigned long flags);
void security_key_free(struct key *key);
int security_key_permission(key_ref_t key_ref,
			    const struct cred *cred, key_perm_t perm);
int security_key_getsecurity(struct key *key, char **_buffer);

#else

static inline int security_key_alloc(struct key *key,
				     const struct cred *cred,
				     unsigned long flags)
{
	return 0;
}

static inline void security_key_free(struct key *key)
{
}

static inline int security_key_permission(key_ref_t key_ref,
					  const struct cred *cred,
					  key_perm_t perm)
{
	return 0;
}

static inline int security_key_getsecurity(struct key *key, char **_buffer)
{
	*_buffer = NULL;
	return 0;
}

#endif
#endif /* CONFIG_KEYS */

#ifdef CONFIG_AUDIT
#ifdef CONFIG_SECURITY
int security_audit_rule_init(u32 field, u32 op, char *rulestr, void **lsmrule);
int security_audit_rule_known(struct audit_krule *krule);
int security_audit_rule_match(u32 secid, u32 field, u32 op, void *lsmrule,
			      struct audit_context *actx);
void security_audit_rule_free(void *lsmrule);

#else

static inline int security_audit_rule_init(u32 field, u32 op, char *rulestr,
					   void **lsmrule)
{
	return 0;
}

static inline int security_audit_rule_known(struct audit_krule *krule)
{
	return 0;
}

static inline int security_audit_rule_match(u32 secid, u32 field, u32 op,
				   void *lsmrule, struct audit_context *actx)
{
	return 0;
}

static inline void security_audit_rule_free(void *lsmrule)
{ }

#endif /* CONFIG_SECURITY */
#endif /* CONFIG_AUDIT */

#ifdef CONFIG_SECURITYFS

extern struct dentry *securityfs_create_file(const char *name, mode_t mode,
					     struct dentry *parent, void *data,
					     const struct file_operations *fops);
extern struct dentry *securityfs_create_dir(const char *name, struct dentry *parent);
extern void securityfs_remove(struct dentry *dentry);

#else /* CONFIG_SECURITYFS */

static inline struct dentry *securityfs_create_dir(const char *name,
						   struct dentry *parent)
{
	return ERR_PTR(-ENODEV);
}

static inline struct dentry *securityfs_create_file(const char *name,
						    mode_t mode,
						    struct dentry *parent,
						    void *data,
						    const struct file_operations *fops)
{
	return ERR_PTR(-ENODEV);
}

static inline void securityfs_remove(struct dentry *dentry)
{}

#endif

#ifdef CONFIG_SECURITY

static inline char *alloc_secdata(void)
{
	return (char *)get_zeroed_page(GFP_KERNEL);
}

static inline void free_secdata(void *secdata)
{
	free_page((unsigned long)secdata);
}

#else

static inline char *alloc_secdata(void)
{
        return (char *)1;
}

static inline void free_secdata(void *secdata)
{ }
#endif /* CONFIG_SECURITY */

#endif /* ! __LINUX_SECURITY_H */

