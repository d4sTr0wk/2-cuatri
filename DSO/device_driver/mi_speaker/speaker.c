#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/miscdevice.h> // misc dev
#include <linux/fs.h>         // file operations
#include <linux/cdev.h>       // cdev structure
#include <linux/uaccess.h>    // copy to/from user space
#include <linux/gpio.h>       // gpio_get/set_value

#define DRIVER_AUTHOR "Máximo García Aroca / Rodrigo Hernández Barba"
#define DRIVER_DESC   "Device driver for berryclip speaker"

//GPIOS numbers as in BCM RPir_GPIO_config
#define GPIO_SPEAKER            4

// Speaker File Operations

static ssize_t speaker_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char c;

	printk(KERN_NOTICE "%s: speaker_write\n", KBUILD_MODNAME);
	if (count != 1)
	{
		printk(KERN_ERR "%s: invalid write size\n", KBUILD_MODNAME);
		return (-EINVAL);
	}
	if (copy_from_user(&c, buf, 1))
	{
		printk(KERN_ERR "%s: copy_from_user failed\n", KBUILD_MODNAME);
		return (-EFAULT);
	}
	if (c == '0')
		gpio_set_value(GPIO_SPEAKER, 0);
	else
		gpio_set_value(GPIO_SPEAKER, 1);
	
	return (count);
}
	
// struct file operations
static const struct file_operations speaker_fops = {
    .owner = THIS_MODULE,
    .write = speaker_write,
};


static struct miscdevice speaker_miscdev = {
        .minor = MISC_DYNAMIC_MINOR,
        .name ="speaker",
        .fops = &speaker_fops,
        .mode = S_IRUGO | S_IWUGO,
};

static int r_devices_config(void)
{
        int     res;
        if ((res = misc_register(&speaker_miscdev)) < 0)
        {
                printk(KERN_ERR "%s: misc_register speaker failed\n", KBUILD_MODNAME);
                return (res);
        }
        printk(KERN_NOTICE "%s: misc_register speaker succeded\n", KBUILD_MODNAME);
        return (0);
}

static int r_GPIO_config(void)
{
	int res;
	if ((res = gpio_is_valid(GPIO_SPEAKER)) < 0)
	{
		printk(KERN_ERR "%s: Invalid GPIO %d\n", KBUILD_MODNAME, GPIO_SPEAKER);
		return (res);
	}
	if ((res = gpio_request(GPIO_SPEAKER, "speaker")) < 0)
	{
		printk(KERN_ERR "%s: gpio_request speaker failed\n", KBUILD_MODNAME);
		return (res);
	}
	if ((res = gpio_direction_output(GPIO_SPEAKER, 0)) < 0)
	{
		printk(KERN_ERR "%s: gpio_direction_output speaker failed\n", KBUILD_MODNAME);
		gpio_free(GPIO_SPEAKER);
		return (res);
	}
	printk(KERN_NOTICE "%s: gpio_request speaker succeded\n", KBUILD_MODNAME);
		
        return (0);
}


// Module init & cleanup
static void r_cleanup(void)
{
	int i;
	printk(KERN_NOTICE "%s: cleaning up ", KBUILD_MODNAME); 
	if (speaker_miscdev.this_device)
		misc_deregister(&speaker_miscdev);
	gpio_free(GPIO_SPEAKER);
 
	printk(KERN_NOTICE "%s: DONE\n", KBUILD_MODNAME);
	return;
}

static int r_init(void)
{
	int res;
	printk(KERN_NOTICE "%s: module loading\n", KBUILD_MODNAME);

	printk(KERN_NOTICE "%s:  devices config\n", KBUILD_MODNAME);
	if ((res = r_devices_config())) 
	{
        	printk(KERN_ERR "%s:  failed\n", KBUILD_MODNAME);
		return (res);
	}
	printk(KERN_NOTICE "%s:  ok\n", KBUILD_MODNAME);

	printk(KERN_NOTICE "%s:  GPIO config\n", KBUILD_MODNAME);
	if((res = r_GPIO_config())) 
	{
		printk(KERN_ERR "%s:  failed\n", KBUILD_MODNAME);
		r_cleanup();
		return (res);
	}
	printk(KERN_NOTICE "%s:  ok\n", KBUILD_MODNAME);

	return (0);
}

module_init(r_init);
module_exit(r_cleanup);


// Module licensing & description
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

