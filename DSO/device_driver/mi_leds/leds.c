#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/miscdevice.h> // misc dev
#include <linux/fs.h>         // file operations
#include <linux/cdev.h>       // cdev structure
#include <linux/uaccess.h>    // copy to/from user space
#include <linux/gpio.h>       // gpio_get/set_value

#define DRIVER_AUTHOR "DAC-UMA"
#define DRIVER_DESC   "Device driver for berryclip leds"

//Minor=0: all leds, minor=i: i'th led (from 1 to 6)
#define NUM_LEDS         6
#define NUM_MINORS       (NUM_LEDS+1)

//GPIOS numbers as in BCM RPir_GPIO_config
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


// LEDs device file operations
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

// struct file operations
static const struct file_operations leds_fops = {
    .owner = THIS_MODULE,
    .write = leds_write,
    //.read = leds_read,
};


/*static const struct file_operations speaker_fops = {
    .owner = THIS_MODULE,
    .write = speaker_write,
};*/

static struct miscdevice leds_miscdev = {
        .minor = MISC_DYNAMIC_MINOR,
        .name ="leds",
        .fops = &leds_fops,
        .mode = S_IRUGO | S_IWUGO,
};

/*static struct miscdevice speaker_miscdev = {
        .minor = MISC_DYNAMIC_MINOR,
        .name ="speaker",
        .fops = &speaker_fops,
        .mode = S_IRUGO | S_IWUGO,
};
*/
static int r_devices_config(void)
{
        int     res;
        if ((res = misc_register(&leds_miscdev)) < 0)
        {
                printk(KERN_ERR "%s: misc_register speaker failed\n", KBUILD_MODNAME);
                return (res);
        }
        printk(KERN_NOTICE "%s: misc_register leds succeded\n", KBUILD_MODNAME);
        /*if ((res = misc_register(&speaker_miscdev)) < 0)
        {
                printk(KERN_ERR "%s: misc_register speaker failed\n", KBUILD_MODNAME);
                return (res);
        }
        printk(KERN_NOTICE "%s: misc_register speaker succeded\n", KBUILD_MODNAME);*/
        return (0);
}

static int r_GPIO_config(void)
{
    int i;
    int res;
        for(i = 0; i < NUM_LEDS; i++) 
        {
                if ((res = gpio_is_valid(gpio_led[i])) < 0) 
                {
                        printk(KERN_ERR "%s:   Invalid GPIO %d\n", KBUILD_MODNAME, gpio_led[i]);
                        return res;
                }
                if ((res = gpio_request_one(gpio_led[i],GPIOF_INIT_LOW,desc_led[i])) < 0) {
                            printk(KERN_ERR "%s:   GPIO request failure: led %s GPIO %d\n",
                                        KBUILD_MODNAME, desc_led[i], gpio_led[i]);
                        return res;
                }
                if ((res = gpio_direction_output(gpio_led[i], 0)) < 0) 
                {
                        printk(KERN_ERR "%s:   GPIO set direction output failure: led %s GPIO %d\n",
                                        KBUILD_MODNAME, desc_led[i], gpio_led[i]);
			r_cleanup();
                        return res;
                }
        }
        return (0);
}


// Module init & cleanup
static void r_cleanup(void)
{
    int i;
    printk(KERN_NOTICE "%s: cleaning up ", KBUILD_MODNAME);
    
    for(i = 0; i < NUM_LEDS; i++) 
    {
        gpio_free(gpio_led[i]);
    }
    
    /*if (leds_miscdev.this_device)
    {
            misc_deregister(&leds_miscdev);
    }*/
    
    printk(KERN_NOTICE "%s: DONE\n", KBUILD_MODNAME);
    return;
}

static int r_init(void)
{
        int res;
    printk(KERN_NOTICE "%s: module loading\n", KBUILD_MODNAME);

    printk(KERN_NOTICE "%s:  devices config\n", KBUILD_MODNAME);
    if ((res = r_devices_config())) {
        printk(KERN_ERR "%s:  failed\n", KBUILD_MODNAME);
        return res;
    }
    printk(KERN_NOTICE "%s:  ok\n", KBUILD_MODNAME);

    printk(KERN_NOTICE "%s:  GPIO config\n", KBUILD_MODNAME);
    if((res = r_GPIO_config())) {
        printk(KERN_ERR "%s:  failed\n", KBUILD_MODNAME);
        r_cleanup();
        return res;
    }
    printk(KERN_NOTICE "%s:  ok\n", KBUILD_MODNAME);

    return 0;
}

module_init(r_init);
module_exit(r_cleanup);


// Module licensing & description
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

