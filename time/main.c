#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

  

int dev_init(void){

	printk(KERN_ALERT "%s:devie start!!\n",__func__);
	
	
	struct timeval now;
	suseconds_t diff;
	volatile int i;
	do_gettimeofday(&now);
	diff = now.tv_usec;
	printk(KERN_INFO"Current UTC is %lu(%lu)\n",now.tv_sec,now.tv_usec);
	for(i= 0 ;i<9999;i++)
		;  
	do_gettimeofday(&now);
	diff=now.tv_usec - diff;
	printk("Elapsed time: %lu\n",diff);	

	return 0;
}


void dev_exit(void){
	
		

	printk(KERN_ALERT "%s:device exit!!\n",__func__);
	
}

module_init(dev_init);
module_exit(dev_exit);
