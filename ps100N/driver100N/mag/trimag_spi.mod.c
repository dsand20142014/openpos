#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xe9d6291d, "module_layout" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0xa2d03227, "mem_map" },
	{ 0xfc9d408e, "device_create" },
	{ 0xf7a01cef, "__class_create" },
	{ 0x68153528, "wake_up_process" },
	{ 0x2c9df22b, "kthread_create" },
	{ 0x45a55ec8, "__iounmap" },
	{ 0xa8f59416, "gpio_direction_output" },
	{ 0x34e2ae24, "mxc_iomux_set_pad" },
	{ 0x8a873583, "mxc_request_iomux" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x2dfa9905, "__mxc_ioremap" },
	{ 0xdc989c7, "cdev_add" },
	{ 0x248a8f3a, "cdev_init" },
	{ 0xfb58121b, "__init_waitqueue_head" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0xf06ed609, "spi_cpld_read" },
	{ 0x2ac3016d, "sem" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xcc5005fe, "msleep_interruptible" },
	{ 0x4f0ea0c0, "up" },
	{ 0x27374cc0, "spi_single_read" },
	{ 0x432fd7f6, "__gpio_set_value" },
	{ 0xfb326a5d, "down" },
	{ 0x51493d94, "finish_wait" },
	{ 0xd62c833f, "schedule_timeout" },
	{ 0x8085c7b1, "prepare_to_wait" },
	{ 0x4661e311, "__tracepoint_kmalloc" },
	{ 0x77bf8cb, "malloc_sizes" },
	{ 0x43b0c9c3, "preempt_schedule" },
	{ 0x9d669763, "memcpy" },
	{ 0xdc74cc24, "kmem_cache_alloc" },
	{ 0xb0300ff9, "remap_pfn_range" },
	{ 0xb6c70a7d, "__wake_up" },
	{ 0xfa2a45e, "__memzero" },
	{ 0x37a0cba, "kfree" },
	{ 0x8aeb89, "cdev_del" },
	{ 0xb176694b, "class_destroy" },
	{ 0xd6a59a55, "device_destroy" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x33257c6d, "mxc_free_iomux" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x6c7e5cfb, "kthread_stop" },
	{ 0xea147363, "printk" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

