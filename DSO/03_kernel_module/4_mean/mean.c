#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>

#define PROC_ENTRY_NAME "mean"
#define MAX_COOKIE_LENGTH       PAGE_SIZE

static struct proc_dir_entry *proc_entry;
static int	first_number;
static int	second_number;



static struct proc_ops my_fops={
	.proc_read = mean_read,
	.proc_write = mean_write
};

// Module init & cleanup

int	init_mean_module( void )
{
	printk(KERN_NOTICE "Loading module '%s'\n", KBUILD_MODNAME);
	
	proc_entry = proc_create(PROC_ENTRY_NAME, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOT, NULL, &my_fops);

	if (proc_entry == NULL) // Fallo creación entrada en /proc
	{
		printk(KERN_INFO "%s: Couldn't create proc entry\n", KBUILD_MODNAME);
	        return -ENOMEM;
	}
	


}

void	cleanup_mean_module( void )
{
	printk(KERN_INFO "%s: Cleaning up module\n", KBUILD_MODNAME);
	remove_proc_entry(PROC_ENTRY_NAME, NULL);
	printk(KERN_INFO "Done.\n");
}

module_init ( init_mean_module );
module_exit( cleanup_mean_module );

// Module licesing & description
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Mean Kernel Proc Module");
MODULE_AUTHOR("M. Rodrigo Hernández Barba, M. Máximo García Aroca, UMA");
