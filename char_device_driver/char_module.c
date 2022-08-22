#include <linux/init.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/cdev.h>  //字元裝置驅動模型
#include <asm/uaccess.h> //copy_to_user , copy_from_user
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

#define DRIVER_NAME "jccl"
#define LEN 100

struct md_data {
	unsigned char data[LEN];	
} virtual_data;

struct cdev *my_dev;
int major_number;
int ret;                    //check error   
dev_t dev_num;              //hold major number



ssize_t dev_read(struct file *filep, char __user *buffer, size_t length, loff_t *offset){

	printk(KERN_ALERT "Reading from device\n ");
	ret = copy_to_user(buffer,virtual_data.data,length);
	return ret;
} 


ssize_t dev_write(struct file *filep, const char __user *buffer, size_t length, loff_t *offset){

	printk(KERN_ALERT "Writing to device\n");
	ret = copy_from_user(virtual_data.data,buffer,length);
	return ret;

} 


int dev_open(struct inode *inodep, struct file *filep){
	
	printk(KERN_ALERT "module open!!\n"); 
		
	return 0;

}   


int dev_close(struct inode *inodep, struct file *filep){


	printk(KERN_ALERT "module close!!\n");
	
	return 0;

}

struct file_operations fops ={
	.open     =   dev_open,
	.read     =   dev_read,
	.write    =   dev_write,
	.release  =   dev_close,
};





int dev_init(void){

	printk(KERN_ALERT "%s:%s start!!\n",__func__,DRIVER_NAME);

	
	//動態申請 major number
	ret = alloc_chrdev_region(&dev_num , 0 , 1 , DRIVER_NAME);

	if(ret < 0){

		printk(KERN_ALERT "%s failed to allocate\n", DRIVER_NAME );

		return ret;
	}

	
	major_number = MAJOR(dev_num);
	printk(KERN_ALERT "%s major number is %d",DRIVER_NAME,major_number);
	
	//init
	my_dev = cdev_alloc();	
	my_dev->ops   = &fops;
	my_dev->owner = THIS_MODULE;

	ret = cdev_add(my_dev,dev_num,1); 


	return 0;
}


void dev_exit(void){
	
	cdev_del(my_dev);
	unregister_chrdev(dev_num,DRIVER_NAME);	

	printk(KERN_ALERT "%s:%s exit!!\n",__func__,DRIVER_NAME);
	
}

module_init(dev_init);
module_exit(dev_exit);
