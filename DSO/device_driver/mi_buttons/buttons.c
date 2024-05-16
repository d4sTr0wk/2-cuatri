#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/miscdevice.h> // misc dev
#include <linux/fs.h>         // file operations
#include <linux/uaccess.h>    // copy to/from user space
#include <linux/gpio.h>       // gpio_get/set_value
#include <linux/mutex.h>
#include <linux/interrupt.h>

#define DRIVER_AUTHOR "Rodrigo Hernández Barba / Máximo García Aroca"
#define DRIVER_DESC   "Device driver for berryclip buttons"

// Defines 
#define GPIO_BUTTON_1	2
#define GPIO_BUTTON_2	3
#define GPIO_BUTTON_1_DESC "BUTTON_1"
#define GPIO_BUTTON_2_DESC "BUTTON_2"
#define GPIO_BUTTON_DEVICE_DESC "Berryclip"

// Buffer for the buttons
static char	buffer[64] = {0};
static int	buffer_ptr = 0;
static int	irq_BUTTON_1 = 0;
static int	irq_BUTTON_2 = 0;

// Synchronization variables
static	DECLARE_WAIT_QUEUE_HEAD(my_wait_queue);
static	DEFINE_MUTEX(counter_mutex);

// Locking condition
int	should_block = 1;

// IRQ handler for Button1
static irqreturn_t r_irq_handler_button1(int irq, void *dev_id)
{
	printk(KERN_NOTICE "%s: Button 1 pressed.\n", KBUILD_MODNAME);
	mutex_lock(&counter_mutex);
	if (buffer_ptr == 64)
	{

		printk(KERN_NOTICE "%s: Buffer is full.\n", KBUILD_MODNAME);
		mutex_unlock(&counter_mutex);
		return (-EFAULT);
	}
	buffer[buffer_ptr++] = '1';
	should_block = 0;
	mutex_unlock(&counter_mutex);
	wake_up_interruptible(&my_wait_queue);
	return (IRQ_HANDLED);
}

// IRQ handler for Button2
static irqreturn_t r_irq_handler_button2(int irq, void *dev_id)
{
	printk(KERN_NOTICE "%s: Button 2 pressed.\n", KBUILD_MODNAME);
	mutex_lock(&counter_mutex);
	if (buffer_ptr == 64)
	{
		printk(KERN_NOTICE "%s: Buffer is full.\n", KBUILD_MODNAME);
		mutex_unlock(&counter_mutex);
		return (-EFAULT);
	}
	buffer[buffer_ptr++] = '2';
	should_block = 0;
	mutex_unlock(&counter_mutex);
	wake_up_interruptible(&my_wait_queue);
	return (IRQ_HANDLED);
}

// Read function for the buttons
static ssize_t buttons_read(struct file *file, char __user *out, size_t size, loff_t *off)
{
	int ret;
	if (should_block)
	{
		ret = wait_event_interruptible(my_wait_queue, !should_block);
		if (ret)
		{
			printk(KERN_NOTICE "%s: wait_event_interruptible failed.\n", KBUILD_MODNAME);
			return (ret);
		}
	}
	
	mutex_lock(&counter_mutex);
	should_block = 1;
	if (buffer_ptr < 0)
	{
		printk(KERN_NOTICE "%s: Buffer pointer is negative.\n", KBUILD_MODNAME);
		mutex_unlock(&counter_mutex);
		return (-1);
	}
	if (buffer_ptr == 0)
	{
		printk(KERN_NOTICE "%s: Buffer is empty.\n", KBUILD_MODNAME);
		mutex_unlock(&counter_mutex);
		return (0);
	}
	if (buffer_ptr < size || size > 64)
	{
		size = buffer_ptr;
	}
	if (copy_to_user(out, buffer, size))
	{
		printk(KERN_NOTICE "%s: Copy to user failed.\n", KBUILD_MODNAME);
		mutex_unlock(&counter_mutex);
		return (-1);
	}
	buffer_ptr = 0;
	for(int i = 0; i < 64; i++)
	{
		buffer[i] = 0;
	}
	mutex_unlock(&counter_mutex);

	return (size);
}


// struct file operations
static const struct file_operations buttons_fops = {
	.owner = THIS_MODULE,
	.read = buttons_read,
};

static struct miscdevice buttons_miscdev = {
        .minor = MISC_DYNAMIC_MINOR,
        .name ="buttons",
        .fops = &buttons_fops,
        .mode = S_IRUGO | S_IWUGO,
};

static int r_devices_config(void)
{
        int     res;
        if ((res = misc_register(&buttons_miscdev)) < 0)
        {
                printk(KERN_ERR "%s: misc_register buttons failed\n", KBUILD_MODNAME);
                return (res);
        }
        printk(KERN_NOTICE "%s: misc_register buttons succeded\n", KBUILD_MODNAME);
        return (0);
}

static int r_GPIO_config(void)
{
	int res;
	// Config BUTTON_1 GPIO
	if ((res = gpio_is_valid(GPIO_BUTTON_1)) < 0)
	{
		printk(KERN_ERR "%s: Invalid GPIO %d.\n" KBUILD_MODNAME, GPIO_BUTTON_1);
		return (res);
	}
	if ((res = gpio_request(GPIO_BUTTON_1, GPIO_BUTTON_1_DESC)) < 0)
	{
		printk(KERN_ERR "%s: GPIO request failure failure: %s.\n", KBUILD_MODNAME, GPIO_BUTTON_1_DESC);
		return (res);
	}
	if ((irq_BUTTON_1 = gpio_to_irq(GPIO_BUTTON_1)) < 0)
	{
		printk(KERN_ERR "%s: GPIO mapping to IRQ number failed%s.\n", KBUILD_MODNAME, GPIO_BUTTON_1_DESC);
		return (irq_BUTTON_1);
	}
	if ((res = request_irq(irq_BUTTON_1, (irq_handler_t) r_irq_handler_button1, IRQF_TRIGGER_FALLING, GPIO_BUTTON_1_DESC, GPIO_BUTTON_DEVICE_DESC)))
	{
		printk(KERN_NOTICE "%s: IRQ request failure.\n", KBUILD_MODNAME);
		return (res);
	}
	printk(KERN_NOTICE "%s: BUTTON_1 configured.\n", KBUILD_MODNAME);

	// Config BUTTON_2 GPIO
	if ((res = gpio_is_valid(GPIO_BUTTON_2)) < 0)
	{
		printk(KERN_ERR "%s: Invalid GPIO %d.\n" KBUILD_MODNAME, GPIO_BUTTON_2);
		return (res);
	}
	if ((res = gpio_request(GPIO_BUTTON_2, GPIO_BUTTON_2_DESC)) < 0)
	{
		printk(KERN_ERR "%s: GPIO request failure failure: %s .\n", KBUILD_MODNAME, GPIO_BUTTON_2_DESC);
		return (res);
	}
	if ((irq_BUTTON_2 = gpio_to_irq(GPIO_BUTTON_2)) < 0)
	{
		printk(KERN_ERR "%s: GPIO mapping to IRQ number failed %s.\n", KBUILD_MODNAME, GPIO_BUTTON_2_DESC);
		return (irq_BUTTON_2);
	}
	if ((res = request_irq(irq_BUTTON_2, (irq_handler_t) r_irq_handler_button2, IRQF_TRIGGER_FALLING, GPIO_BUTTON_2_DESC, GPIO_BUTTON_DEVICE_DESC)))
	{
		printk(KERN_NOTICE "%s: IRQ request failure.\n", KBUILD_MODNAME);
		return (res);
	}
	printk(KERN_NOTICE "%s: BUTTON_2 configured.\n", KBUILD_MODNAME);

	return (0);
}


// Module init & cleanup
static void r_cleanup(void)
{
	printk(KERN_NOTICE "%s: module clean up\n", KBUILD_MODNAME);
	if (buttons_miscdev.this_device) misc_deregister(&buttons_miscdev);

	if (irq_BUTTON_1) free_irq(irq_BUTTON_1, GPIO_BUTTON_DEVICE_DESC);
	if (irq_BUTTON_2) free_irq(irq_BUTTON_2, GPIO_BUTTON_DEVICE_DESC);
	gpio_free(GPIO_BUTTON_1);
	gpio_free(GPIO_BUTTON_2);
	printk(KERN_NOTICE "%s: module unloaded\n", KBUILD_MODNAME);
	return ;
}

static int r_init(void)
{
	int res;

	// Print message to kernel log
	printk(KERN_NOTICE "%s: module loading\n", KBUILD_MODNAME);

	printk(KERN_NOTICE "%s:  devices config\n", KBUILD_MODNAME);
	if ((res = r_devices_config())) 
	{
		printk(KERN_ERR "%s:  failed\n", KBUILD_MODNAME);
		r_cleanup();
		return (res);
	}
	printk(KERN_NOTICE "%s:  OK\n", KBUILD_MODNAME);

	printk(KERN_NOTICE "%s:  GPIO config\n", KBUILD_MODNAME);
	if((res = r_GPIO_config())) 
	{
		printk(KERN_ERR "%s:  failed\n", KBUILD_MODNAME);
		r_cleanup();
		return (res);
    	}
	printk(KERN_NOTICE "%s:  OK\n", KBUILD_MODNAME);

	return (0);
}

module_init(r_init);
module_exit(r_cleanup);


// Module licensing & description
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

