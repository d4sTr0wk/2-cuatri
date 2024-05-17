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

#define DRIVER_AUTHOR "Máximo García Aroca / Rodrigo Hernández Barba"
#define DRIVER_DESC   "Device driver for berryclip"

//Minor=0: all leds, minor=i: i'th led (from 1 to 6)
#define NUM_LEDS         6
#define NUM_MINORS       (NUM_LEDS+1)

// GPIO for the leds
#define GPIO_SPEAKER            4
#define GPIO_RED_LED1           9
#define GPIO_RED_LED2           10
#define GPIO_YELLOW_LED1        11
#define GPIO_YELLOW_LED2        17
#define GPIO_GREEN_LED1         22
#define GPIO_GREEN_LED2         27

static int gpio_led[] = { GPIO_RED_LED1, GPIO_RED_LED2,
                          GPIO_YELLOW_LED1, GPIO_YELLOW_LED2,
                          GPIO_GREEN_LED1, GPIO_GREEN_LED2 };

static char *desc_led[] = { "RED1", "RED2",
                            "YELLOW1", "YELLOW2",
                            "GREEN1", "GREEN2" };

// GPIO for the speaker
#define GPIO_SPEAKER		4

// GPIO for the buttons
#define GPIO_BUTTON_1		2
#define GPIO_BUTTON_2		3
#define GPIO_BUTTON_1_DESC	"BUTTON_1"
#define GPIO_BUTTON_2_DESC	"BUTTON_2"
#define GPIO_BUTTON_DEVICE_DESC	"Berryclip"

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
int	busy = 0;

/**********************************************************************************************/
/*************************************LED FILE OPERATIONS**************************************/
/**********************************************************************************************/
static ssize_t leds_write(struct file *file, const char __user *buf,
                          size_t count, loff_t *ppos)
{
        int led;
        char    ch;

        if (*ppos > 0) return count;
        if (copy_from_user(&ch, buf, 1)) return -EFAULT;
        if ((ch & 0xC0) == 0) //00xxxxxx
        {
                for (led = 0; led < NUM_LEDS; led++) {
                        gpio_set_value(gpio_led[led], ((int)ch >> led) & 1);
                        printk(KERN_INFO "%s: set led %d to %d\n", KBUILD_MODNAME, led, (int)(ch >> led) & 1);
                }
        } 
        else if ((ch & 0x40) == 0x40 && (ch & 0x80) == 0)//01xxxxxx
        {
                for (led = 0; led < NUM_LEDS; led++) 
                {
                        if ((((int)ch >> led) & 1) == 1)
                        {
                                gpio_set_value(gpio_led[led], 1);
                                printk(KERN_INFO "%s: set led %d to %d\n", KBUILD_MODNAME, led, 1);
                        }
                }
        }
        else if ((ch & 0x80) == 0x80 && (ch & 0x40) == 0) //10xxxxxx
        {
                for (led = 0; led < NUM_LEDS; led++) 
                {
                                if ((((int)ch >> led) & 1) == 1)
                                {
                                        gpio_set_value(gpio_led[led], 0);
                                        printk(KERN_INFO "%s: set led %d to %d\n", KBUILD_MODNAME, led, 0);
                                }
                }
        }
        else //11xxxxxx
        {
                for (led = 0; led < NUM_LEDS; led++) 
                {
                        if ((((int)ch >> led) & 1) == 1)
                        {
                                if(gpio_get_value(gpio_led[led]))
                                {
                                        gpio_set_value(gpio_led[led], 0);
                                        printk(KERN_INFO "%s: set led %d to %d\n", KBUILD_MODNAME, led, 0);
                                }
                                else
                                {
                                                gpio_set_value(gpio_led[led], 1);
                                                printk(KERN_INFO "%s: set led %d to %d\n", KBUILD_MODNAME, led, 1);
                                }
                        }
                }
        }
        *ppos += 1;
        return (1);
}

static ssize_t leds_read(struct file *file, char __user *buf, size_t size, loff_t *off)
{
	int led;
	char    ch = 0;

	if (*off > 0) return 0;
	for (led = 0; led < NUM_LEDS; led++) 
	{
		ch |= (gpio_get_value(gpio_led[led]) << led);
	}
	if (copy_to_user(buf, &ch, 1)) return -EFAULT;
	*off += 1;
	return (1);
}

/**********************************************************************************************/
/*********************************SPEAKER FILE OPERATIONS**************************************/
/**********************************************************************************************/

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

/**********************************************************************************************/
/*********************************BUTTONS FILE OPERATIONS**************************************/
/**********************************************************************************************/

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
static ssize_t buttons_read(struct file *file, char __user *buf, size_t size, loff_t *off)
{
	int ret;
	if (busy)
	{
		printk(KERN_NOTICE "%s: Device is busy.\n", KBUILD_MODNAME);
		return (-EBUSY);
	}
	mutex_lock(&counter_mutex);
	busy = 1;
	mutex_unlock(&counter_mutex);
	if (should_block)
	{
		ret = wait_event_interruptible(my_wait_queue, !should_block);
		if (ret)
		{
			printk(KERN_NOTICE "%s: wait_event_interruptible failed.\n", KBUILD_MODNAME);
			mutex_lock(&counter_mutex);
			busy = 0;
			mutex_unlock(&counter_mutex);
			return (-ERESTARTSYS);
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
	if (copy_to_user(buf, buffer, size))
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
	busy = 0;
	mutex_unlock(&counter_mutex);

	return (size);
}


// struct file operations

static const struct file_operations leds_fops = {
    .owner = THIS_MODULE,
    .write = leds_write,
    .read = leds_read,
};

static const struct file_operations speaker_fops = {
    .owner = THIS_MODULE,
    .write = speaker_write,
};

static const struct file_operations buttons_fops = {
	.owner = THIS_MODULE,
	.read = buttons_read,
};

static struct miscdevice leds_miscdev = {
        .minor = MISC_DYNAMIC_MINOR,
        .name ="leds",
        .fops = &leds_fops,
        .mode = S_IRUGO | S_IWUGO,
};

static struct miscdevice speaker_miscdev = {
        .minor = MISC_DYNAMIC_MINOR,
        .name ="speaker",
        .fops = &speaker_fops,
        .mode = S_IRUGO | S_IWUGO,
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

	/*********LEDS********/
	if ((res = misc_register(&leds_miscdev)) < 0)
        {
                printk(KERN_ERR "%s: misc_register speaker failed\n", KBUILD_MODNAME);
                return (res);
        }
        printk(KERN_NOTICE "%s: misc_register leds succeded\n", KBUILD_MODNAME);

	/*********SPEAKER********/
	if ((res = misc_register(&speaker_miscdev)) < 0)
        {
                printk(KERN_ERR "%s: misc_register speaker failed\n", KBUILD_MODNAME);
                return (res);
        }
        printk(KERN_NOTICE "%s: misc_register speaker succeded\n", KBUILD_MODNAME);

	/*********BUTTONS********/
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

	/*****LEDS*****/
	for(int i = 0; i < NUM_LEDS; i++) 
        {
                if ((res = gpio_is_valid(gpio_led[i])) < 0) 
                {
                        printk(KERN_ERR "%s:   Invalid GPIO %d\n", KBUILD_MODNAME, gpio_led[i]);
                        return (res);
                }
                if ((res = gpio_request_one(gpio_led[i],GPIOF_INIT_LOW,desc_led[i])) < 0) {
                            printk(KERN_ERR "%s:   GPIO request failure: led %s GPIO %d\n",
                                        KBUILD_MODNAME, desc_led[i], gpio_led[i]);
                        return (res);
                }
                if ((res = gpio_direction_output(gpio_led[i], 0)) < 0) 
                {
                        printk(KERN_ERR "%s:   GPIO set direction output failure: led %s GPIO %d\n",
                                        KBUILD_MODNAME, desc_led[i], gpio_led[i]);
                        for (i--; i >= 0; i--) gpio_free(gpio_led[i]);
                        return (res);
                }
        }
	/*****SPEAKER*****/

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

	/*****BUTTONS*****/
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
	if ((res = gpio_direction_input(GPIO_BUTTON_1)) < 0)
	{
		printk(KERN_ERR "%s: GPIO set direction input failure: %s.\n", KBUILD_MODNAME, GPIO_BUTTON_1_DESC);
		gpio_free(GPIO_BUTTON_1);
		return (res);
	}
	if ((irq_BUTTON_1 = gpio_to_irq(GPIO_BUTTON_1)) < 0)
	{
		printk(KERN_ERR "%s: GPIO mapping to IRQ number failed%s.\n", KBUILD_MODNAME, GPIO_BUTTON_1_DESC);
		gpio_free(GPIO_BUTTON_1);
		return (irq_BUTTON_1);
	}
	if ((res = request_irq(irq_BUTTON_1, (irq_handler_t) r_irq_handler_button1, IRQF_TRIGGER_FALLING, GPIO_BUTTON_1_DESC, GPIO_BUTTON_DEVICE_DESC)))
	{
		printk(KERN_NOTICE "%s: IRQ request failure.\n", KBUILD_MODNAME);
		gpio_free(GPIO_BUTTON_1);
		return (res);
	}
	printk(KERN_NOTICE "%s: BUTTON_1 configured.\n", KBUILD_MODNAME);

	// Config BUTTON_2 GPIO
	if ((res = gpio_is_valid(GPIO_BUTTON_2)) < 0)
	{
		printk(KERN_ERR "%s: Invalid GPIO %d.\n" KBUILD_MODNAME, GPIO_BUTTON_2);
		gpio_free(GPIO_BUTTON_1);
		return (res);
	}
	if ((res = gpio_request(GPIO_BUTTON_2, GPIO_BUTTON_2_DESC)) < 0)
	{
		printk(KERN_ERR "%s: GPIO request failure failure: %s .\n", KBUILD_MODNAME, GPIO_BUTTON_2_DESC);
		gpio_free(GPIO_BUTTON_1);
		return (res);
	}
	if ((res = gpio_direction_input(GPIO_BUTTON_2)) < 0)
	{
		printk(KERN_ERR "%s: GPIO set direction input failure: %s.\n", KBUILD_MODNAME, GPIO_BUTTON_2_DESC);
		gpio_free(GPIO_BUTTON_1);
		gpio_free(GPIO_BUTTON_2);
		return (res);
	}
	if ((irq_BUTTON_2 = gpio_to_irq(GPIO_BUTTON_2)) < 0)
	{
		printk(KERN_ERR "%s: GPIO mapping to IRQ number failed %s.\n", KBUILD_MODNAME, GPIO_BUTTON_2_DESC);
		gpio_free(GPIO_BUTTON_1);
		gpio_free(GPIO_BUTTON_2);
		return (irq_BUTTON_2);
	}
	if ((res = request_irq(irq_BUTTON_2, (irq_handler_t) r_irq_handler_button2, IRQF_TRIGGER_FALLING, GPIO_BUTTON_2_DESC, GPIO_BUTTON_DEVICE_DESC)))
	{
		printk(KERN_NOTICE "%s: IRQ request failure.\n", KBUILD_MODNAME);
		gpio_free(GPIO_BUTTON_1);
		gpio_free(GPIO_BUTTON_2);
		return (res);
	}
	printk(KERN_NOTICE "%s: BUTTON_2 configured.\n", KBUILD_MODNAME);

	return (0);
}


// Module init & cleanup
static void r_cleanup(void)
{
	printk(KERN_NOTICE "%s: module clean up\n", KBUILD_MODNAME);
	/*********LEDS********/
	if (leds_miscdev.this_device) misc_deregister(&leds_miscdev);
	for(int i = 0; i < NUM_LEDS; i++)
	{
        	gpio_free(gpio_led[i]);
	}

	/*********SPEAKER********/   
        if (speaker_miscdev.this_device) misc_deregister(&speaker_miscdev);
        gpio_free(GPIO_SPEAKER);
 
	/*********BUTTONS********/
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

