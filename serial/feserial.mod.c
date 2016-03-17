#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
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
	{ 0x260f334e, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x132ddebb, __VMLINUX_SYMBOL_STR(platform_driver_unregister) },
	{ 0x338e9c7d, __VMLINUX_SYMBOL_STR(__platform_driver_register) },
	{ 0x21659a1a, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0xbde277c, __VMLINUX_SYMBOL_STR(misc_register) },
	{ 0xdac11bae, __VMLINUX_SYMBOL_STR(of_property_read_u32_array) },
	{ 0x564e08ed, __VMLINUX_SYMBOL_STR(dev_set_drvdata) },
	{ 0x423b830, __VMLINUX_SYMBOL_STR(devm_request_threaded_irq) },
	{ 0xaf8e8fef, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0xd13c972a, __VMLINUX_SYMBOL_STR(debugfs_create_u8) },
	{ 0x733c3b54, __VMLINUX_SYMBOL_STR(kasprintf) },
	{ 0xd8ccdd26, __VMLINUX_SYMBOL_STR(devm_ioremap_resource) },
	{ 0x26daf986, __VMLINUX_SYMBOL_STR(devm_kmalloc) },
	{ 0xf99f30df, __VMLINUX_SYMBOL_STR(debugfs_create_dir) },
	{ 0x82ecc79e, __VMLINUX_SYMBOL_STR(platform_get_resource) },
	{ 0x1ad84252, __VMLINUX_SYMBOL_STR(__pm_runtime_resume) },
	{ 0x5bb71311, __VMLINUX_SYMBOL_STR(pm_runtime_enable) },
	{ 0xadcb92c, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0xefe1a13f, __VMLINUX_SYMBOL_STR(prepare_to_wait_event) },
	{ 0x1000e51, __VMLINUX_SYMBOL_STR(schedule) },
	{ 0xfbc74f64, __VMLINUX_SYMBOL_STR(__copy_from_user) },
	{ 0x276efcff, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0xfa2a45e, __VMLINUX_SYMBOL_STR(__memzero) },
	{ 0xac8f37b2, __VMLINUX_SYMBOL_STR(outer_cache) },
	{ 0x24560980, __VMLINUX_SYMBOL_STR(_dev_info) },
	{ 0x67c2fa54, __VMLINUX_SYMBOL_STR(__copy_to_user) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x6ed16928, __VMLINUX_SYMBOL_STR(__pm_runtime_disable) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xd85ec85b, __VMLINUX_SYMBOL_STR(misc_deregister) },
	{ 0x6c346a06, __VMLINUX_SYMBOL_STR(debugfs_remove_recursive) },
	{ 0x67afff35, __VMLINUX_SYMBOL_STR(platform_get_irq) },
	{ 0x9ac547ab, __VMLINUX_SYMBOL_STR(dev_get_drvdata) },
	{ 0xefd6cf06, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr0) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Cfree-electrons,serial*");
MODULE_ALIAS("platform:serial");

MODULE_INFO(srcversion, "A0E37EEB73C4CFF16E5630A");
