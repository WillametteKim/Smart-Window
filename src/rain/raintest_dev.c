/*
kernel driver for raindrop sensor
When User read device file to know the value of raindrop sensor,
driver get value from module itself, and give status message to device file.
If there were water on sensor, device file will get "Y".
If there were not water on sensor, device file will get "N".
*/

#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define RAINPIN_NUM 22 //Physical Pin Number for Raindrop Sensor
#define RAINPIN_NAME "GPIO. 3" //GPIO NAME for Raindrop Sensor
#define DEV_NAME "raintest_dev" //Device File Name for Raindrop Sensor
#define DEV_NUM 242 //Device Major Number for Raindrop Sensor

static char status[256];

MODULE_LICENSE("GPL"); //Module License

//Operation Device File Opened
int raintest_open(struct inode *pinode, struct file *pfile) {
  printk(KERN_ALERT "Open raintest_dev\n"); //Kernal Message for FILE OPEN
  gpio_request(RAINPIN_NUM, RAINPIN_NAME); //GPIO Request for Raindrop Sensor
  gpio_direction_input(RAINPIN_NUM); //GPIO

  return 0;
}

//Operation Device File Closed
int raintest_close(struct inode *pinode, struct file *pfile) {
  printk(KERN_ALERT "RELEASE raintest_dev\n"); //Kernel Message for FILE CLOSE
  return 0;
}

//Operation when User reads device file
ssize_t raintest_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) {
  int ret; //integer for copy_to_user()

  printk("Read raintest dev \n"); //Kernel Message for FILE READ
  ret = copy_to_user(buffer, status, length); //Return string Y or N
  if(ret < 0) {
    printk(KERN_ALERT "DEVICE FILE ERROR\n"); //copy_to_user() ERROR
  }  

  return 0;
}

//Operation when User writes something on Device File
ssize_t raintest_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) {
  int tmp;
  printk(KERN_ALERT "Write raintest dev \n"); //Kernel Message for FILE WRITE

  tmp = copy_from_user(status, buffer, length); 
    if(tmp){
    printk(KERN_ALERT "Write Error\n");
    return -1;
  }

   //Read Raindrop sensor value
   tmp = gpio_get_value(RAINPIN_NUM);
   printk("rain gpio_get_value : %d\n", tmp);
   if(tmp == 1){ //Raindrop Sensor detect water
     strcpy(status, "Y"); //Write "Y" on Device File
     printk("RAIN: %s\n", status);
   }

  else if (tmp == 0) { //Raindrop Sensor detect nothing
    strcpy(status, "N");
    printk("RAIN: %s\n", status);
  }

  return length;
}

struct file_operations fop = {
  .owner = THIS_MODULE,
  .open = raintest_open,
  .read = raintest_read,
  .write = raintest_write,
  .release = raintest_close,
};

int __init raintest_init(void) { //initialize
  printk(KERN_ALERT "sudo mknod -m 666 /dev/raintest_dev c %d 0\n", DEV_NUM); //kernel message notice creating device file command
  register_chrdev(DEV_NUM, DEV_NAME, &fop);
  return 0;
}

void __exit raintest_exit(void) { //exit
  printk(KERN_ALERT "EXIT raintest\n"); //exit
  unregister_chrdev(DEV_NUM, DEV_NAME);
}

module_init(raintest_init);
module_exit(raintest_exit);

