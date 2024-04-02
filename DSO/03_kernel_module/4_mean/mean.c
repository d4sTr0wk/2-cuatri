#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>

#define PROC_ENTRY_NAME "mean"
#define MAX_COOKIE_LENGTH       PAGE_SIZE

static struct proc_dir_entry *proc_entry;

static	int num_sum = 0;
static int num_cnt = 0;

// file operations
ssize_t mean_read(struct file *filp, char __user *buf, size_t count, loff_t *off)
{
	char	result[1024];
	int mean, length;

	if (num_cnt > 0)
	{
		mean = (num_sum / num_cnt) * 100;
		len = sprintf(result, "Media = %d.%d, de %d numeros\n", mean/100, mean%100, num_cnt);
	}
	else
		len = sprintf(result, "No hay valores disponibles\n");
	if (copy_to_user(buf, respuesta, len))
	{
		printk(KERN_INFO "mean_read: error: copy_to_user");
		return -EFAULT;
	}

	++off += len;
	return len;	
}

ssize_t mean_write(struct file *filp, const char __user *buf, size_t count, loff_t *off)
{
	// This code must write the received data in cookie_pot	behind the last written byte separated by a comma and if the buffer has the word "CLEAR" it must clear the cookie_pot
	
	char	*tmp_buf[count];
	int	space_available = 512;
	int	num;

	if (count > space_available) // No space left on device
		count = space_available;
	if (copy_from_user(tmp_buf, buf, count)) // Error copying data from user space
		return -EFAULT;
	if (strncmp(tmp_buf, "CLEAR", 5) == 0)
	{
		num_sum = 0;
		num_cnt = 0;
	}
	else if (sscanf(tmp_buf, "%d", &num) == 1) // If the buffer contains a number
	{
		num_sum += num;
		num_cnt++;
		// printk(KERN_INFO "mean: received number %d\n", count);
	}
	else // If the buffer does not contain a number it must be ignored and show an error message
	{
		printk(KERN_WARNING "bash: echo: error de escritura: Argumento inválido\n");
		return -EINVAL;
	}

	*off += count;
	return count;
}	

static struct proc_ops my_fops={
	.proc_read = mean_read,
	.proc_write = mean_write
};

// Module init & cleanup

int	init_mean_module( void )
{
	printk(KERN_NOTICE "Loading module '%s'\n", KBUILD_MODNAME);
	proc_entry = proc_create(PROC_ENTRY_NAME, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH, NULL, &my_fops);

	if (proc_entry == NULL) // Fallo creación entrada en /proc
	{
		printk(KERN_INFO "%s: Couldn't create proc entry\n", KBUILD_MODNAME);
	    return -ENOMEM;
	}
	printk(KERN_INFO "%s: Created /proc/%s\n", KBUILD_MODNAME, PROC_ENTRY_NAME);

	return (0);
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
MODULE_AUTHOR("D. Rodrigo Hernández Barba, D. Máximo García Aroca, UMA");
