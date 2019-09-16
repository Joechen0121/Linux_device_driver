#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/poll.h>
#include <asm/current.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "devone_ioctl.h"

MODULE_LICENSE("Dual BSD/GPL");


static int devone_devs = 1 ;
static int devone_major = 0;
static int devone_minor = 0;
static struct cdev devone_cdev;
static struct class *devone_class = NULL; 
static dev_t devone_dev; //dev_t: kernel裡代表設備內的類型  	



struct devone_data{
	unsigned char val;
	rwlock_t lock;
};


//struct file是字符設備驅動相關重要結構,代表一個打開的文件描述符. 在kernel內創立
//struct inode 為每個存儲設備或存儲設備的分區,存儲這些數據的信息，這些信息包括文件大小、屬主、歸屬的用戶組、讀寫權限等
//struct ioctl_cmd data 為ioctl須做的事


long devone_ioctl_add( struct file *filp, unsigned int cmd, unsigned long arg){


	struct devone_data *dev = filp -> private_data;
	int retval = 0;
	unsigned char val;
	struct ioctl_cmd data;


	//初始化,將指定內存的前n個字節設置為特定的值
	memset(&data,0,sizeof(data));


	
	switch (cmd){
	
		//傳資料給驅動程式
		case IOCTL_VALSET:
			if(!capable(CAP_SYS_ADMIN)){
				retval = -EPERM;
				goto done;				
			}
			if(!access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd))){
				retval = -EFAULT;
				goto done;
			}
			
			//copy data from userspace to kernel space
			if(copy_from_user(&data, (int __user *)arg, sizeof(data))){
				retval = -EFAULT;
				goto done;
			}
			
			printk("IOCTL_cmd.val %u (%s)\n", data.val,__func__);
			
			//寫鎖寫入,其他process不能申請讀鎖和寫鎖
			write_lock(&dev->lock);
			dev->val = data.val;
			write_unlock(&dev->lock);

			break;

				
		//驅動程式讀回資料
  		case IOCTL_VALGET:
                        if(!access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd))){
                                retval = -EFAULT;
                                goto done;
                    	}
                        read_lock(&dev->lock);
                        val = dev->val;
                        read_unlock(&dev->lock);
			data.val = val;
			
			//copy kernel data to userspace
			if(copy_to_user((int __user *)arg,&data,sizeof(data))){
				retval = -EFAULT;
				goto done;
			}
			break;
		default:
			retval = -ENOTTY;
			break;
	}
done:
	return (retval);

}


ssize_t devone_read(struct file *filp, char __user *buf,size_t count,loff_t *f_pos){
	struct devone_data *dev = filp -> private_data;
	unsigned char val;
	int i;
	int retval;

	read_lock(&dev->lock);
	val = dev->val;
	read_unlock(&dev ->lock);


	for(i = 0; i < count ;i++){
		if(copy_to_user(&buf[i],&val,1)){
			retval = -EFAULT;
			goto out;
		}
	}
	retval = count;
out:
	return (retval);

}


int devone_open(struct inode *inode, struct file *file){

	struct devone_data *dev;

	dev = kmalloc(sizeof(struct devone_data),GFP_KERNEL);
	if(dev == NULL){
		return -ENOMEM;	
	}


	//讀寫鎖
	rwlock_init(&dev->lock);
	dev->val = 0xff;

	file->private_data = dev;
	return 0;

}


int devone_close(struct inode* inode ,struct file *file){
	struct devone_data *dev = file -> private_data;
	kfree(dev);
	return 0;

}


struct file_operations devone_fops = {
	.owner = THIS_MODULE,	
	.unlocked_ioctl = devone_ioctl_add,
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
