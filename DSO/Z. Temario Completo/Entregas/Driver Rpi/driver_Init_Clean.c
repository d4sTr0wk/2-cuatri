#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kfifo.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>

// Defines & vars.
#define DRIVER_AUTHOR "Rafae' Ramírez Salas"
#define DRIVER_DESC   "Driver para plaquita aux Rpi"

//GPIOS numbers as in BCM RPi
#define GPIO_BUTTON1 2
#define GPIO_BUTTON2 3

#define GPIO_SPEAKER 4

#define GPIO_GREEN1  27
#define GPIO_GREEN2  22
#define GPIO_YELLOW1 17
#define GPIO_YELLOW2 11
#define GPIO_RED1    10
#define GPIO_RED2    9

// Buffer to keep the pulses.
DEFINE_KFIFO(buffer, unsigned char, 128);

// Declaration of 'a wait' to block processes.
DECLARE_WAIT_QUEUE_HEAD(wait);

// Open semaphore declaration (value = 1).
DEFINE_SEMAPHORE(semaphore);

// Block condition.
int bloqueo = 1;

// Tasklets functions.
static void tasklet_func1(struct tasklet_struct *);
static void tasklet_func2(struct tasklet_struct *);

// Tasklets declarations.
DECLARE_TASKLET(taskletButton1, tasklet_func1);
DECLARE_TASKLET(taskletButton2, tasklet_func2);

static unsigned long ticks_unsegundo;

// Timers functions.
static void timer_func1(struct timer_list *);
static void timer_func2(struct timer_list *);

// Timers declarations.
DEFINE_TIMER(timerButton1, timer_func1);
DEFINE_TIMER(timerButton2, timer_func2);

// Interrupts variables block.
static short int irq_BUTTON1 = 0;
static short int irq_BUTTON2 = 0;

// Text below will be seen in 'cat /proc/interrupts' command.
#define GPIO_BUTTON1_DESC "Button 1"
#define GPIO_BUTTON2_DESC "Button 2"

// Below is optional, used in more complex code, in our case, this could be.
#define GPIO_BUTTON1_DEVICE_DESC "Placa lab. DAC"
#define GPIO_BUTTON2_DEVICE_DESC "Placa lab. DAC"

// Add a parameter in the download of our driver.
static int velocity = 155;
module_param(velocity, int, S_IRUGO);

// Counter for the total of inits we've done.
unsigned int cntInit = 0;

// Array of LEDs numbers.
static int LED_GPIOS[]= {GPIO_GREEN1, GPIO_GREEN2, GPIO_YELLOW1, GPIO_YELLOW2, GPIO_RED1, GPIO_RED2};
// Array of LEDs names.
static char *led_desc[]= {"GPIO_GREEN1","GPIO_GREEN2","GPIO_YELLOW1","GPIO_YELLOW2","GPIO_RED1","GPIO_RED2"};


/****************************************************************************/
/* Driver write/read using gpio kernel API                                    */
/****************************************************************************/

/**************** LEDs ****************/
static void byte2leds(char ch){
    int val = (int)ch;
    int aux = (int)ch;
    // Here in 'aux' I keep the bits number 6 y number 7.
    aux = (aux >> 6);
    // Bits 6 and 7 to zero.
    if(aux == 0){
		int i;
		for(i = 0; i < 6; i++){
			gpio_set_value(LED_GPIOS[i], (val >> i) & 1);
		}
	// Bit 6 to one and bit 7 to zero.
    } else if(aux == 1){
		int i;
		for(i = 0; i < 6; i++){
			if(((val >> i) & 1) == 1){
				gpio_set_value(LED_GPIOS[i], (val >> i) & 1);
			}
		}
	// Bit 6 to zero and bit 7 to one.
	} else if(aux == 2){
		int i;
		for(i = 0; i < 6; i++){
			if(((val >> i) & 1) == 1){
				gpio_set_value(LED_GPIOS[i], (val >> i) & 0);
			}
		}
	// Bits 6 and 7 to one.
	} else {
		int i;
			
		printk(KERN_INFO "It is not define. I turn on the green LEDs. Viva er Beti'! \n");
		
		// Turn on the green LEDs.
		for(i = 0; i < 2; i++){
			gpio_set_value(LED_GPIOS[i], (val >> i) & 1);
		}
		// Turn off the rest of the LEDs.	
		for(i = 2; i < 6; i++){
			gpio_set_value(LED_GPIOS[i], (val >> i) & 0);
		}
	}
}

static char leds2byte(void){
    int val;
    char ch = 0;

	int i;
    for(i = 0; i < 6; i++){
        val = gpio_get_value(LED_GPIOS[i]);
        ch = ch | (val << i);
    }
    return ch;
}

/****************************************************************************/
/* Driver device file operations                                              */
/****************************************************************************/

static ssize_t leds_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos){
    char ch;

    if(copy_from_user(&ch, buf, 1)){
        return -EFAULT;
    }

    printk(KERN_INFO " (write) valor recibido: %d\n", (int)ch);

    byte2leds(ch);

    return 1;
}

static ssize_t leds_read(struct file *file, char __user *buf, size_t count, loff_t *ppos){
    char ch;

    if(*ppos == 0){
		*ppos += 1;
	}
    else return 0;

    ch = leds2byte();

    printk(KERN_INFO " (read) valor entregado: %d\n", (int)ch);

    if(copy_to_user(buf, &ch, 1)){
		return -EFAULT;
	}
    return 1;
}

/**************** Speaker ****************/
static void byte2speaker(char ch){
    if(ch == '0'){
		gpio_set_value(GPIO_SPEAKER, 0);
	} else {
		gpio_set_value(GPIO_SPEAKER, 1);
	}
}

static ssize_t speaker_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos){
    char ch;

    if(copy_from_user(&ch, buf, 1)){
        return -EFAULT;
    }

    printk(KERN_INFO " (write) valor recibido: %d\n", (int)ch);

    byte2speaker(ch);

    return 1;
}

/**************** Buttons ****************/

// Buttons device file operation.
static ssize_t buttons_read(struct file *file, char __user *buf, size_t count, loff_t *ppos){
	unsigned int copy;
	unsigned int length;
    
    if(down_interruptible(&semaphore)){
		return -ERESTARTSYS;
    }
    
    while(bloqueo){
		// We are block.
		up(&semaphore); // Release the lock.
		printk(KERN_INFO " (read) block starts.\n");
		if(wait_event_interruptible(wait, !bloqueo))  return -ERESTARTSYS;
		if(down_interruptible(&semaphore)) return -ERESTARTSYS;
	}
	
	printk(KERN_INFO " (read) block ends.\n");
	// We alreay consume the data and we block it again.
	// Change 'bloqueo' to 1 again.
	bloqueo = 1;
	up(&semaphore);

	// Comparation to know the exact length to show.
	if(kfifo_len(&buffer) <= count){
		length = kfifo_len(&buffer);
	} else {
		length = count;
	}
	
    // Show the buffer.	
	printk(KERN_INFO "Buttons Pressed: \n");
    if(kfifo_to_user(&buffer, buf, length, &copy)){
		return -EFAULT;
    }
	return length;
}

// Tasklets functions.
static void tasklet_func1(struct tasklet_struct *ptr){
	unsigned char *data = "1";
	
	kfifo_put(&buffer, *data);
	
	// Semaphore catch.
    down(&semaphore);
    // Change bloqueo to 0.
    bloqueo = 0;
    up(&semaphore);
    wake_up(&wait);
}
static void tasklet_func2(struct tasklet_struct *ptr){
	unsigned char *data = "2";
	
	kfifo_put(&buffer, *data);

	// Semaphore catch.
    down(&semaphore);
    // Change bloqueo to 0.
    bloqueo = 0;
    up(&semaphore);
    wake_up(&wait);
}

// Timers functions.
static void timer_func1(struct timer_list *ptr){
    printk(KERN_NOTICE "%s : Timer run at %u\n", KBUILD_MODNAME, (unsigned) jiffies);
    enable_irq(irq_BUTTON1);
    
}
static void timer_func2(struct timer_list *ptr){
    printk(KERN_NOTICE "%s : Timer run at %u\n", KBUILD_MODNAME, (unsigned) jiffies);
    enable_irq(irq_BUTTON2);
}

// IRQ handler1 - fired on interrupt. 
static irqreturn_t r_irq_handler1(int irq, void *dev_id, struct pt_regs *regs) {
    disable_irq_nosync(irq_BUTTON1);
    
    tasklet_schedule(&taskletButton1);    
    
    // To calculte how many ticks are a second.
    ticks_unsegundo = msecs_to_jiffies(velocity);
    // Programed for after a second.
    mod_timer(&timerButton1, jiffies + ticks_unsegundo);
	
    return IRQ_HANDLED;
}
// IRQ handler2 - fired on interrupt.  
static irqreturn_t r_irq_handler2(int irq, void *dev_id, struct pt_regs *regs) {
	disable_irq_nosync(irq_BUTTON2);
	
    tasklet_schedule(&taskletButton2);	
	
	// To calculte how many ticks are a second.
    ticks_unsegundo = msecs_to_jiffies(velocity);
    // Programed for after a second.
    mod_timer(&timerButton2, jiffies + ticks_unsegundo); 
	
    return IRQ_HANDLED;
}

/***************************** File Operations *****************************/

static const struct file_operations leds_fops = {
    .owner	= THIS_MODULE,
    .write	= leds_write,
    .read	= leds_read,
};

static const struct file_operations speaker_fops = {
    .owner	= THIS_MODULE,
    .write	= speaker_write,
};

static const struct file_operations buttons_fops = {
    .owner	= THIS_MODULE,
    .read	= buttons_read,
};

/* Driver device struct */

static struct miscdevice leds_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "leds",
    .fops = &leds_fops,
    .mode = S_IRUGO | S_IWUGO,
};

static struct miscdevice speaker_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "speaker",
    .fops = &speaker_fops,
    .mode = S_IRUGO | S_IWUGO,
};

static struct miscdevice buttons_miscdev = {
    .minor	= MISC_DYNAMIC_MINOR,
    .name	= "buttons",
    .fops	= &buttons_fops,
    .mode   = S_IRUGO | S_IWUGO,
};

/*****************************************************************************/
/* This functions registers devices, requests GPIOs and configures interrupts */
/*****************************************************************************/

/*******************************
 *  register device for driver
 *******************************/

static int r_dev_config_leds(void){
    int ret = misc_register(&leds_miscdev);
    if(ret < 0) {
        printk(KERN_ERR "misc_register failed\n");
    }
	else {
		printk(KERN_NOTICE "misc_register OK... leds_miscdev.minor=%d\n", leds_miscdev.minor);
	}
	return ret;
}

static int r_dev_config_speaker(void){
    int ret = misc_register(&speaker_miscdev);
    if(ret < 0) {
        printk(KERN_ERR "misc_register failed\n");
    }
	else {
		printk(KERN_NOTICE "misc_register OK... speaker_miscdev.minor=%d\n", speaker_miscdev.minor);
	}
	return ret;
}

static int r_dev_config_buttons(void) {
	int ret;
	ret = misc_register(&buttons_miscdev);
    if(ret < 0) {
        printk(KERN_ERR "misc_register failed.\n");
    }
	else {
		printk(KERN_NOTICE "misc_register OK... buttons_miscdev.minor = %d.\n", buttons_miscdev.minor);
	}
	return ret;
}

/*******************************
 *  request and init gpios for leds, speaker and buttons.
 *******************************/

static int r_GPIO_config_leds(void){
    int res = 0;
    int i;
    for(i = 0; i < 6; i++){
        if((res = gpio_request_one(LED_GPIOS[i], GPIOF_INIT_LOW, led_desc[i]))){
            printk(KERN_ERR "GPIO request faiure: led GPIO %d %s\n", LED_GPIOS[i], led_desc[i]);
            return res;
        }
        gpio_direction_output(LED_GPIOS[i], 0);
	}
	return res;
}

static int r_GPIO_config_speaker(void){
	int res = 0;
    if((res = gpio_request_one(GPIO_SPEAKER, GPIOF_INIT_LOW, 0))){
		printk(KERN_ERR "GPIO request faiure: speaker %d \n", GPIO_SPEAKER);
		return res;
	}
	gpio_direction_output(GPIO_SPEAKER, 0);
	return res;
}

static int r_int_config_buttons(void){
    int res = 0;

    // BUTTON 1
    if((res = gpio_request(GPIO_BUTTON1, GPIO_BUTTON1_DESC))) {
        printk(KERN_ERR "GPIO request faiure: %s.\n", GPIO_BUTTON1_DESC);
        return res;
    }

    if((irq_BUTTON1 = gpio_to_irq(GPIO_BUTTON1)) < 0 ) {
        printk(KERN_ERR "GPIO to IRQ mapping faiure %s.\n", GPIO_BUTTON1_DESC);
        return irq_BUTTON1;
    }

    printk(KERN_NOTICE "Mapped int %d for button1 in gpio %d.\n", irq_BUTTON1, GPIO_BUTTON1);

    if((res = request_irq(irq_BUTTON1, (irq_handler_t ) r_irq_handler1, IRQF_TRIGGER_FALLING,
    GPIO_BUTTON1_DESC, GPIO_BUTTON1_DEVICE_DESC))) {
        printk(KERN_ERR "Irq Request failure.\n");
        return res;
    }

    // BUTTON 2
    if((res = gpio_request(GPIO_BUTTON2, GPIO_BUTTON2_DESC))) {
        printk(KERN_ERR "GPIO request faiure: %s.\n", GPIO_BUTTON1_DESC);
        return res;
    }

    if((irq_BUTTON2 = gpio_to_irq(GPIO_BUTTON2)) < 0 ) {
        printk(KERN_ERR "GPIO to IRQ mapping faiure %s.\n", GPIO_BUTTON2_DESC);
        return irq_BUTTON2;
    }

    printk(KERN_NOTICE "Mapped int %d for button2 in gpio %d.\n", irq_BUTTON2, GPIO_BUTTON2);

    if((res=request_irq(irq_BUTTON2,(irq_handler_t ) r_irq_handler2, IRQF_TRIGGER_FALLING,
    GPIO_BUTTON2_DESC, GPIO_BUTTON2_DEVICE_DESC))) {
        printk(KERN_ERR "Irq Request failure.\n");
        return res;
    }
    
	return res;
}

/****************************************************************************/
/* Module init / cleanup block. */
/****************************************************************************/

static void r_cleanup(void){
	if(cntInit == 7){
		int i;
		// Clean timers.
		del_timer(&timerButton1);
		del_timer(&timerButton2);
		printk(KERN_NOTICE "%s Timers cleaned.\n", KBUILD_MODNAME);
		
		// Clean tasklets.
		tasklet_kill(&taskletButton1);
		tasklet_kill(&taskletButton2);
		printk(KERN_NOTICE "%s Tasklets cleaned.\n", KBUILD_MODNAME);
		
		// Clean GPIO button 2.
		if(irq_BUTTON2){
			free_irq(irq_BUTTON2, GPIO_BUTTON2_DEVICE_DESC);
		} 
		gpio_free(GPIO_BUTTON2);
		printk(KERN_NOTICE "%s Button1 cleaned.\n", KBUILD_MODNAME);
		
		// Clean GPIO button 1.
		if(irq_BUTTON1){
			free_irq(irq_BUTTON1, GPIO_BUTTON1_DEVICE_DESC);
		}
		gpio_free(GPIO_BUTTON1);
		printk(KERN_NOTICE "%s Button2 cleaned.\n", KBUILD_MODNAME);
		
		// Clean buttons device config.
		if(buttons_miscdev.this_device){
			misc_deregister(&buttons_miscdev);
		}
		printk(KERN_NOTICE "%s fpos buttons cleaned.\n", KBUILD_MODNAME);
		
		// Clean speaker device config.
		if(speaker_miscdev.this_device){
			misc_deregister(&speaker_miscdev);
		}
		printk(KERN_NOTICE "%s fpos speaker cleaned.\n", KBUILD_MODNAME);

		// Clean leds device config.
		if(leds_miscdev.this_device){
			misc_deregister(&leds_miscdev);
		}
		printk(KERN_NOTICE "%s fpos leds clean.\n", KBUILD_MODNAME);
		
		// Clean GPIOs.
		printk(KERN_NOTICE "%s LEDs and Speaker cleaned.\n", KBUILD_MODNAME);		
		printk(KERN_NOTICE "%s Module cleaning up...\n", KBUILD_MODNAME);
		
		gpio_free(GPIO_SPEAKER);
		
		for(i = 0; i < 6; i++){
			gpio_free(LED_GPIOS[i]);
		}
		
		// Clean Kfifo.
		printk(KERN_NOTICE "%s Deleting kiffo.\n", KBUILD_MODNAME);
		kfifo_free(&buffer);
		
	} else if(cntInit == 6){
		int i;
		// Clean buttons device config.
		if(buttons_miscdev.this_device){
			misc_deregister(&buttons_miscdev);
		}
		printk(KERN_NOTICE "%s fpos buttons cleaned.\n", KBUILD_MODNAME);
		
		// Clean speaker device config.
		if(speaker_miscdev.this_device){
			misc_deregister(&speaker_miscdev);
		}
		printk(KERN_NOTICE "%s fpos speaker cleaned.\n", KBUILD_MODNAME);

		// Clean leds device config.
		if(leds_miscdev.this_device){
			misc_deregister(&leds_miscdev);
		}
		printk(KERN_NOTICE "%s fpos leds clean.\n", KBUILD_MODNAME);
		
		// Clean GPIOs.
		printk(KERN_NOTICE "%s LEDs and Speaker cleaned.\n", KBUILD_MODNAME);		
		printk(KERN_NOTICE "%s Module cleaning up...\n", KBUILD_MODNAME);
		
		gpio_free(GPIO_SPEAKER);
		
		for(i = 0; i < 6; i++){
			gpio_free(LED_GPIOS[i]);
		}
		
		// Clean Kfifo.
		printk(KERN_NOTICE "%s Deleting kiffo.\n", KBUILD_MODNAME);
		kfifo_free(&buffer);
		
	} else if(cntInit == 5){
		int i;
		// Clean speaker device config.
		if(speaker_miscdev.this_device){
			misc_deregister(&speaker_miscdev);
		}
		printk(KERN_NOTICE "%s fpos speaker cleaned.\n", KBUILD_MODNAME);

		// Clean leds device config.
		if(leds_miscdev.this_device){
			misc_deregister(&leds_miscdev);
		}
		printk(KERN_NOTICE "%s fpos leds clean.\n", KBUILD_MODNAME);
		
		// Clean GPIOs.
		printk(KERN_NOTICE "%s LEDs and Speaker cleaned.\n", KBUILD_MODNAME);		
		printk(KERN_NOTICE "%s Module cleaning up...\n", KBUILD_MODNAME);
		
		gpio_free(GPIO_SPEAKER);
		
		// Cleaning backwards.
		for(i = 0; i < 6; i++){
			gpio_free(LED_GPIOS[i]);
		}
		
		// Clean Kfifo.
		printk(KERN_NOTICE "%s Deleting kiffo.\n", KBUILD_MODNAME);
		kfifo_free(&buffer);
		
	} else if(cntInit == 4){
		int i;
		// Clean leds device config.
		if(leds_miscdev.this_device){
			misc_deregister(&leds_miscdev);
		}
		printk(KERN_NOTICE "%s fpos leds clean.\n", KBUILD_MODNAME);
		
		// Clean GPIOs.
		printk(KERN_NOTICE "%s LEDs and Speaker cleaned.\n", KBUILD_MODNAME);		
		printk(KERN_NOTICE "%s Module cleaning up...\n", KBUILD_MODNAME);
		
		gpio_free(GPIO_SPEAKER);
		
		for(i = 0; i < 6; i++){
			gpio_free(LED_GPIOS[i]);
		}
		
		// Clean Kfifo.
		printk(KERN_NOTICE "%s Deleting kiffo.\n", KBUILD_MODNAME);
		kfifo_free(&buffer);
		
	} else if(cntInit == 3){
		int i;
		// Clean GPIOs.
		printk(KERN_NOTICE "%s LEDs and Speaker cleaned.\n", KBUILD_MODNAME);		
		printk(KERN_NOTICE "%s Module cleaning up...\n", KBUILD_MODNAME);
		
		gpio_free(GPIO_SPEAKER);
		
		for(i = 0; i < 6; i++){
			gpio_free(LED_GPIOS[i]);
		}
		
		// Clean Kfifo.
		printk(KERN_NOTICE "%s Deleting kiffo.\n", KBUILD_MODNAME);
		kfifo_free(&buffer);
		
	} else if(cntInit == 2){
		// Clean GPIOs.
		int i;
		printk(KERN_NOTICE "%s LEDs cleaned.\n", KBUILD_MODNAME);		
		printk(KERN_NOTICE "%s Module cleaning up...\n", KBUILD_MODNAME);
		
		for(i = 0; i < 6; i++){
			gpio_free(LED_GPIOS[i]);
		}
		
		// Clean Kfifo.
		printk(KERN_NOTICE "%s Deleting kiffo.\n", KBUILD_MODNAME);
		kfifo_free(&buffer);
		
	} else if(cntInit == 1){
		// Clean Kfifo.
		printk(KERN_NOTICE "%s Deleting kiffo.\n", KBUILD_MODNAME);
		kfifo_free(&buffer);
		
	} else {  // (cntInit == 0)
		printk(KERN_NOTICE "%s Impossible clean anything, something was wrong.\n", KBUILD_MODNAME);	
	}
    
    // End.
    printk(KERN_NOTICE "Done. Bye from %s module.\n", KBUILD_MODNAME);
    return;
}

static int r_init(void){
	int res = 0;
    printk(KERN_NOTICE "Hello, loading %s module!\n", KBUILD_MODNAME);
    
    // Call for GPIOs config.
    printk(KERN_NOTICE "%s - GPIO config...\n", KBUILD_MODNAME);
    
	// LEDs GPIOs.
    if((res = r_GPIO_config_leds())){
		r_cleanup();
		return res;
	} else {
		cntInit++;  // (cntInit = 1)	
	}
	
	// Speaker GPIO.
	if((res = r_GPIO_config_speaker())){
		r_cleanup();
		return res;
	} else {
		cntInit++;  // (cntInit = 2)	
	}
	
	// Call for leds device config.	
	printk(KERN_NOTICE "%s - devices config...\n", KBUILD_MODNAME);
    if((res = r_dev_config_leds())){
		r_cleanup();
		return res;
	} else {
		cntInit++;  // (cntInit = 3)	
	}	
	
	// Call for speaker device config.
	if((res = r_dev_config_speaker())){
		r_cleanup();
		return res;
	} else {
		cntInit++;  // (cntInit = 4)	
	}
	
	// Call for buttons device config.
	if((res = r_dev_config_buttons())){
		r_cleanup();
		return res;
	} else {
		cntInit++;  // (cntInit = 5)		
	}	
	
	// Call for buttons int config.
    if((res = r_int_config_buttons())){
		r_cleanup();
		return res;
	} else {
		cntInit++;  // (cntInit = 6)		
	}	

	// Call for Kfifo config.
	printk(KERN_NOTICE "%s Start KFIFO.\n", KBUILD_MODNAME);
    INIT_KFIFO(buffer);
    cntInit++;  // (cntInit = 7)
    
    // Reset Kfifo.
    kfifo_reset(&buffer);
    
    return res;
}

module_init(r_init);
module_exit(r_cleanup);

/****************************************************************************/
/* Module licensing/description block.                                      */
/****************************************************************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
