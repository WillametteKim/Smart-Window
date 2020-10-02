/* Device should start after close the window. kernel driver for Servo Motor. When User opens the device file, window will be opened and closed If user sent 
message "OPEN", servo motor will open the window. IF user sent message "CLOSE", servo motor will close the window. */

#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define SERVOPIN_NUM 27          //Physical Pin Number for Servo Motor
#define SERVOPIN_NAME "GPIO. 2"  //GPIO NAME for Servo Motor
#define DEV_NAME "servotest_dev" //Defice File Name for Servo Motor
#define DEV_NUM 240              //Device Major Number for Servo Motor

static char msg[10]; //message from user
static int i;
static int ret;

MODULE_LICENSE("GPL"); //Module License

//Operation Device File Opened
int servotest_open(struct inode *pinode, struct file *pfile) {
  printk(KERN_ALERT "OPEN servotest_dev\n"); //Kernel Message for FILE OPEN
  gpio_request(SERVOPIN_NUM, SERVOPIN_NAME); //GPIO Request for Servo Motor

  return 0;
}
//Operation Device File Closed
int servotest_close(struct inode *pinode, struct file *pfile) {
  printk(KERN_ALERT "RELEASE servotest_dev\n"); //Kernel Message for FILE CLOSE

  return 0;
}

//Operation Device File Read
ssize_t servotest_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) {
  printk(KERN_ALERT "READ servotest_dev\n"); //Kernel Message for FILE READ
  return 0;
}

//Operation Device File Write
ssize_t servotest_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) {

  printk("WRITE SERVOTEST\n"); //Kernel Message for FILE WRITE
  ret = copy_from_user(msg, buffer, length); //msg take message from User
  printk("INPUT: %s\n", msg);


  if(!strcmp(msg, "OPEN")) { //if user sent message "OPEN", Servo Motor will open the window
    printk(KERN_ALERT "OPENING\n");
    for(i = 0 ; i < 45 ; i++) {
      gpio_direction_output(SERVOPIN_NUM, 1);
      udelay(2000);
      gpio_direction_output(SERVOPIN_NUM, 0); // POSITION 180 FULL SPEED FORWARD
      udelay(2000);
   }
  mdelay(100);
}

  else if(!strcmp(msg, "CLOS")) { //if user sent message "CLOSE", Servo Motor will close the window
    printk(KERN_ALERT "CLOSING\n");
    for(i = 0 ; i < 90; i++) {
      gpio_direction_output(SERVOPIN_NUM, 1);
      udelay(1000);
      gpio_direction_output(SERVOPIN_NUM, 0); //position 0 FULL SPEED BACKWARD
      udelay(1000);
    }
  mdelay(100);
  }

  else {
    printk(KERN_ALERT "UNKNOWN VALUE DETECTED. NO OPERATION\n"); //if user sent UNKNOWN VALUE, Servo Motor do nothing
  }

  return length;
}

struct file_operations fop = {
  .owner = THIS_MODULE,
  .open = servotest_open,
  .read = servotest_read,
  .write = servotest_write,
  .release = servotest_close,
};

int __init servotest_init(void) {
  printk(KERN_ALERT "sudo mknod -m 666 /dev/servotest_dev c %d 0\n", DEV_NUM); //kernel message notice creating device file command
  printk(KERN_ALERT "INIT servotest\n"); //initialize success
  register_chrdev(DEV_NUM, DEV_NAME, &fop);

  return 0;
}

void __exit servotest_exit(void) {
  printk(KERN_ALERT "EXIT servotest\n");
  unregister_chrdev(DEV_NUM, DEV_NAME);
}

module_init(servotest_init);
module_exit(servotest_exit);


