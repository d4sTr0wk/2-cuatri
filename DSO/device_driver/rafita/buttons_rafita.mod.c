#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0xf0ef52e8, "down" },
	{ 0x581cde4e, "up" },
	{ 0x637493f3, "__wake_up" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x526c3a6c, "jiffies" },
	{ 0x92997ed8, "_printk" },
	{ 0xfcec0987, "enable_irq" },
	{ 0xca5a7528, "down_interruptible" },
	{ 0x800473f, "__cond_resched" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x1000e51, "schedule" },
	{ 0x647af474, "prepare_to_wait_event" },
	{ 0x49970de8, "finish_wait" },
	{ 0x4578f528, "__kfifo_to_user" },
	{ 0x27bbf221, "disable_irq_nosync" },
	{ 0xca54fee, "_test_and_set_bit" },
	{ 0x7f02188f, "__msecs_to_jiffies" },
	{ 0xc38c83b8, "mod_timer" },
	{ 0x9d2ab8ac, "__tasklet_schedule" },
	{ 0xc1514a3b, "free_irq" },
	{ 0xfe990052, "gpio_free" },
	{ 0xe949a784, "misc_deregister" },
	{ 0x2b68bd2f, "del_timer" },
	{ 0xea3c74e, "tasklet_kill" },
	{ 0xcf591ee4, "misc_register" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xf89f9719, "gpio_to_desc" },
	{ 0x44c72844, "gpiod_to_irq" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x52e296ce, "param_ops_int" },
	{ 0x91ade18, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "0D17E7377C3CD41F1D53A6B");
