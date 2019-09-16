#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/poll.h>
#include <asm/current.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/slab.h>


MODULE_LICENSE("Dual BSD/GPL");


static int devone_devs = 1 ;
static int devone_major = 0;
static int devone_minor = 0;
static struct cdev devone_cdev;
static struct class *devone_class = NULL; 
static dev_t devone_dev; //dev_t: kernel裡代表設備內的類型  	



struct devone_data{
	struct timer_list timeout;
	spinlock_t lock;
	wait_queue_head_t read_wait;
	int timeout_done;
	struct semaphore sem;
};


static void devone_timeout(unsigned long arg){
	struct devone_data *dev = (struct devone_data*)arg;
	unsigned long flags;
	
	printk("%s called\n",__func__);

	spin_lock_irqsave(&dev->lock,flags);
	
	dev->timeout_done = 1;
	wake_up_interruptible(&dev->read_wait);

	spin_lock_irqstore(&dev->lock,flags);

}
			

static int devone_poll(struct file *filp, poll_table *wait){
	struct devone_data *dev = filp-> private_data;
	unsigned int mask = POLLOUT | POLLWRNOR;
	
	printk("%s called\n",__func__);

	if(dev == NULL) return -EBADFD;

	down(&dev->sem);
	poll_wait(filp, &dev->read,wait);
	
	if(dev->timeout_done == 1){
		mask |= POLLIN | POLLRDNORM;
	}
	up(&dev->sem);

	printk("%s returned (mask 0x%x)\n"__func__,mask);
	return (mask);



}



ssize_t devone_read(struct file *filp, char __user *buf,size_t count,loff_t *f_pos){
	struct devone_data *dev = filp -> private_data;
	unsigned char val;
	int i;
	int retval;

	read_lock(&dev->lock);
	val = dev->val;
	read_unlock(&dev ->lock);

	if(down_interruptible(&dev->sem)) return -ERESTARTSYS;


	if(dev->timeout_done == 0){
		up(&dev->sem);
		if(filp->f_flags & O_NONBLOCK) return -EAGAIN;

		do{
			retval = wait_event_interruptible_timeout(dev->read,dev->timeout_done==1,1*HZ);
			if(retval == -ERESTARTSYS;	
			return
		}while(retval == 0);

		if(down_interruptible(&dev->sem)) return -ERESTARTSYS;

	}
	
	
	val = 0xff;
	for(i = 0; i < count ;i++){
		if(copy_to_user(&buf[i],&val,1)){
			retval = -EFAULT;
			goto out;
		}
	}
	retval = count;
out:
	dev->timeout_done = 0;

	mod_timer(&dev->timeout,jiffies + timeout_value*HZ);

	up(&dev->sem);

	return (retval);

}


int devone_open(struct inode *inode, struct file *file){

	struct devone_data *dev;

	dev = kmalloc(sizeof(struct devone_data),GFP_KERNEL);
	if(dev == NULL){
		return -ENOMEM;	
	}

	spin_lock_init(&dev->lock);

	init_waitqueue_head(&dev->read_wait);
	
	init_MUTEX(&dev->sem);

	init_timer(&dev->timeout);	
	dev->timeout.function =devone_timeout;
	dev->timeout.data = (unsigned long)dev;

	file->private_data = dev;
		
	dev->timeout_done = 0;
	mod_timer(&dev->timeout , jiffies + timeout_value*HZ);
	return 0;

}


int devone_close(struct inode* inode ,struct file *file){
	struct devone_data *dev = file -> private_data;
	kfree(dev);
	return 0;

}


struct file_operations devone_fops = {
	.owner = THIS_MODULE,
	.write = devone_write,	
	.poll = devone_poll,
	.open = devone_open,
	.release = devone_close,
	.read = devone_read,
};


static int devone_init(void){


	//註冊設備號碼
	dev_t dev = MKDEV(devone_major, 0);  //將major number和minor number 建立成dev_t
	int alloc_ret = 0;
	int major ;
	int cdev_err = 0;
	static struct device *class_dev_func;

	alloc_ret = alloc_chrdev_region(&dev, 0 ,devone_devs, "devone");//動態申請設備編號範圍
	if(alloc_ret)
		goto error;
	


	//初始化,分配,註冊cdev
	cdev_init(&devone_cdev, &devone_fops);    //字符設備驅動的初始
	devone_major = major = MAJOR(dev);        //給予主編號
	devone_cdev.owner = THIS_MODULE;
	devone_cdev.ops = &devone_fops;
	cdev_err = cdev_add(&devone_cdev, MKDEV(devone_major,devone_minor),1); //初始化cdev後,加入到系統中
	if(cdev_err)
		goto error;



	// mknod  (不用手動自己加 sudo mknod -m 666 /dev/name )
	devone_class = class_create(THIS_MODULE, "devone");  //建立設備類
	devone_dev = MKDEV(devone_major,devone_minor);
	class_dev_func = device_create(devone_class, NULL,devone_dev,NULL,"devone%d",devone_minor);//建立設備節點

	printk(KERN_ALERT " driver(major %d) installed\n",major);

	return 0;

error:
	if(cdev_err == 0) cdev_del(&devone_cdev);
	if(alloc_ret == 0) unregister_chrdev_region(dev,devone_devs);

	return -1;

}


static void devone_exit(void){
	dev_t dev = MKDEV(devone_major, 0);
	
	device_destroy(devone_class, devone_dev);
	class_destroy(devone_class);
	cdev_del(&devone_cdev);                      //remove module 時釋放記憶體
	unregister_chrdev_region(dev, devone_devs);  
	
	printk(KERN_ALERT "driver removed\n");

}

module_init(devone_init);
module_exit(devone_exit);
