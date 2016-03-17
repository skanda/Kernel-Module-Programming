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
	{ 0xd75266a7, __VMLINUX_SYMBOL_STR(i2c_del_driver) },
	{ 0xe05804b7, __VMLINUX_SYMBOL_STR(i2c_register_driver) },
	{ 0x42618d38, __VMLINUX_SYMBOL_STR(input_event) },
	{ 0x4f7d7233, __VMLINUX_SYMBOL_STR(i2c_master_recv) },
	{ 0x276efcff, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0x20fe762d, __VMLINUX_SYMBOL_STR(input_register_polled_device) },
	{ 0x676bbc0f, __VMLINUX_SYMBOL_STR(_set_bit) },
	{ 0xe213a198, __VMLINUX_SYMBOL_STR(input_allocate_polled_device) },
	{ 0x564e08ed, __VMLINUX_SYMBOL_STR(dev_set_drvdata) },
	{ 0x26daf986, __VMLINUX_SYMBOL_STR(devm_kmalloc) },
	{ 0x2e5810c6, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr1) },
	{ 0x8e865d3c, __VMLINUX_SYMBOL_STR(arm_delay_ops) },
	{ 0xa5b73b99, __VMLINUX_SYMBOL_STR(i2c_master_send) },
	{ 0xefd6cf06, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr0) },
	{ 0xaa8d7faf, __VMLINUX_SYMBOL_STR(input_free_polled_device) },
	{ 0x5f1f7c2f, __VMLINUX_SYMBOL_STR(input_unregister_polled_device) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x9ac547ab, __VMLINUX_SYMBOL_STR(dev_get_drvdata) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=input-polldev";

MODULE_ALIAS("i2c:nunchuck");
MODULE_ALIAS("of:N*T*Cnintendo,nunchuck*");

MODULE_INFO(srcversion, "009D3EC7F03FC0F8D238397");
