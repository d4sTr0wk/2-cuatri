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
#define DRIVER_DESC   "Driver_Botones para plaquita Rpi"

// GPIOS numbers as in BCM RPi.
#define GPIO_BUTTON1 2
#define GPIO_BUTTON2 3

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

// Timers functions.
static void timer_func1(struct timer_list *);
static void timer_func2(struct timer_list *);

static unsigned long ticks_unsegundo;

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
static int velocity = 150;
module_param(velocity, int, S_IRUGO);

// Counter for the total of inits we've done.
unsigned int cntInit = 0;

// Buttons device file operation.
static ssize_t buttons_read(struct file *file, char __user *buf, size_t count, loff_t *ppos){
	unsigned int copy;
	unsigned int longitud;
    
    if(down_interruptible(&semaphore)) return -ERESTARTSYS;
    
    while(bloqueo){
		// We are block.
		up(&semaphore); // Release the lock.
		printk( KERN_INFO " (read) block starts.\n");
		if(wait_event_interruptible(wait, !bloqueo))  return -ERESTARTSYS;
		if(down_interruptible(&semaphore)) return -ERESTARTSYS;
	}
	
	printk(KERN_INFO " (read) block ends.\n");
	// We alreay consume the data and we block it again.
	// Change 'bloqueo' to 1 again.
	bloqueo = 1;
	up(&semaphore);

	// Comparation
	if(kfifo_len(&buffer) < count){
		longitud = kfifo_len(&buffer);
	} else {
		longitud = count;
	}
	
    // Show the buffer.	
	printk(KERN_INFO "Buttons Pressed: \n");
    if(kfifo_to_user(&buffer, buf, longitud, &copy))return -EFAULT;
    
	return longitud;
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
    
    ticks_unsegundo = msecs_to_jiffies(velocity);  // para calcular cuantos ticks son un segundo
    mod_timer(&timerButton1, jiffies + ticks_unsegundo); // programar para despues de 1seg
    

    return IRQ_HANDLED;
}
// IRQ handler2 - fired on interrupt.  
static irqreturn_t r_irq_handler2(int irq, void *dev_id, struct pt_regs *regs) {
	
	disable_irq_nosync(irq_BUTTON2);
    
    tasklet_schedule(&taskletButton2);

    ticks_unsegundo = msecs_to_jiffies(velocity);  // para calcular cuantos ticks son un segundo
    mod_timer(&timerButton2, jiffies + ticks_unsegundo); // programar para despues de 1seg
	
    return IRQ_HANDLED;
}

// Buttons device struct.
static const struct file_operations buttons_fops = {
    .owner	= THIS_MODULE,
    .read	= buttons_read,
};
static struct miscdevice buttons_miscdev = {
    .minor	= MISC_DYNAMIC_MINOR,
    .name	= "buttons",
    .fops	= &buttons_fops,
    .mode   = S_IRUGO | S_IWUGO,
};

/*******************************
 *  register device for leds/speaker/buttons
 *******************************/

static int r_dev_config(void) {
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
 *  request and init gpios for leds
 *******************************/

static int r_int_config(void){
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

    printk(KERN_NOTICE "  Mapped int %d for button1 in gpio %d.\n", irq_BUTTON1, GPIO_BUTTON1);

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

    printk(KERN_NOTICE "  Mapped int %d for button2 in gpio %d.\n", irq_BUTTON2, GPIO_BUTTON2);

    if((res=request_irq(irq_BUTTON2,(irq_handler_t ) r_irq_handler2, IRQF_TRIGGER_FALLING,
    GPIO_BUTTON2_DESC, GPIO_BUTTON2_DEVICE_DESC))) {
        printk(KERN_ERR "Irq Request failure.\n");
        return res;
    }
    
	return res;
}

/****************************************************************************/
/* Module init / cleanup block.                                             */
/****************************************************************************/

static void r_cleanup(void) {	
	// Clean Kfifo.
    printk(KERN_NOTICE "%s deleting kiffo.\n", KBUILD_MODNAME);
    kfifo_free(&buffer);
	
    printk(KERN_NOTICE "%s module cleaning up...\n", KBUILD_MODNAME);

    // Clean GPIO button 2.
    if(irq_BUTTON1) free_irq(irq_BUTTON2, GPIO_BUTTON2_DEVICE_DESC);  
	gpio_free(GPIO_BUTTON2);

    // Clean GPIO button 1.
    if(irq_BUTTON2) free_irq(irq_BUTTON1, GPIO_BUTTON1_DEVICE_DESC); 
	gpio_free(GPIO_BUTTON1);
	
    // Clean device.
    if(buttons_miscdev.this_device) misc_deregister(&buttons_miscdev);
    
    // Clean timers.
    del_timer(&timerButton1);
    del_timer(&timerButton2);
 
    // Clean tasklets.
    tasklet_kill(&taskletButton1);
    tasklet_kill(&taskletButton2);   

    // End.
    printk(KERN_NOTICE "Done. Bye from %s module.\n", KBUILD_MODNAME);
    return;
}

static int r_init(void) {
	int res = 0;
    printk(KERN_NOTICE "Hello, loading %s module!\n", KBUILD_MODNAME);
    printk(KERN_NOTICE "%s - devices config...\n", KBUILD_MODNAME);

    // Call for device config.
    if((res = r_dev_config())){
		r_cleanup();
		return res;
	}
	
    // Call for int config.
    printk(KERN_NOTICE "%s - GPIO config...\n", KBUILD_MODNAME);
    
    if((res = r_int_config())){
		r_cleanup();
		return res;
	}
	
	// Call for Kfifo config.
	printk(KERN_NOTICE "%s inicializamos KFIFO.\n", KBUILD_MODNAME);
    INIT_KFIFO(buffer);
    
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
