/*---------------------------------------------
 * Diseño de Sistemas Operativos (DAC) *
    para compilar el módulo:
    $ MODULE=media_mod_v2 make
    para instalar el módulo:
    $ sudo insmod media_mod_v2.ko
    para comprobar si el módulo fue cargado:
    $ sudo lsmod
    $ dmesg | tail
    para desinstalar el módulo:
    $ sudo rmmod media_mod_v2
----------------------------------------------*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h> 
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#define MAX_COOKIE_LENGTH       PAGE_SIZE

static struct proc_dir_entry *proc_entry;
static int sumaTotal;  // Index to write next mean
static int cntNums;  // Index to archive 

ssize_t mean_write(struct file *filp, const char *buf, size_t count, loff_t *off){
  /*
  int space_available = (MAX_COOKIE_LENGTH-cookie_index)+1;
  if(count > space_available) {
    printk(KERN_INFO "mean: cookie pot is full!\n");
    return -ENOSPC;
  }
  if(copy_from_user( &cookie_pot[cookie_index], buf, count )) {
    return -EFAULT;
  }
  cookie_index += count;
  *off+=count;  // avanzo el offset del fichero
  cookie_pot[cookie_index-1] = 0; // marco el fin de la frase
  return count;
  */
  char copia_buffer[512];
  int space_avaiable = 512;
  int variable;
  printk(KERN_INFO "media: write\n");
  if(count > space_avaiable){
    count = space_avaiable;
  }
  if(*off>0) return 0;
  if(copy_from_user(copia_buffer, buf, (count>512)?512:count)) {
    return -EFAULT;
  }
  printk(KERN_INFO "media: write %d\n",count);
  if (strncmp(copia_buffer, "CLEAR", 5) == 0){
    cntNums = 0;
    sumaTotal = 0;
  } else if(sscanf(copia_buffer, " %d ", &variable) == 1){
    sumaTotal += variable;
    cntNums++;
  } else {
    return -EINVAL;
  }
  *off += count;
  return count;
}

ssize_t mean_read(struct file *filp, char __user *buf, size_t count, loff_t *off ){
  char respuesta [1024];
  int entera, fraccion, len;
  if(*off > 0){
    return 0;
  }
  entera = sumaTotal/cntNums;
  fraccion = ((sumaTotal % cntNums))*100/cntNums;
  if(cntNums > 0){
    len = sprintf(respuesta, "La media es: %d.%d (de %d numeros)\n",
    entera, fraccion, cntNums);
  }else{
    len = sprintf(respuesta, "No hay respuesta\n");
  }
  if(copy_to_user(buf, respuesta, len)){
    return -EFAULT;
  }
  *off += len;
  return len;
}

static struct proc_ops my_fops = {
	.proc_read = mean_read,
	.proc_write = mean_write
};

int init(void) {
  int ret = 0;
  proc_entry = proc_create("media", 0666, NULL, &proc_fops);
  if (proc_entry == NULL) {
    ret = -ENOMEM;
    printk(KERN_INFO "media: Couldn't create proc entry\n");
  } else {
    printk(KERN_INFO "media: Module loaded.\n");
  }
  return ret;
}

void clean(void){
	remove_proc_entry("mean", NULL);
	printk(KERN_INFO "mean: Module unloaded.\n");
}

module_init(init);

module_exit(clean);
