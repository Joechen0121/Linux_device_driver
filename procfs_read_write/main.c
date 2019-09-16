#include <module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_LICENSE("Dual BSD/GPL");

#define PROCNAME "driver/sample"

#define BUFFERSIZE 4096


int sample_read_proc(char *page,char **start, off_t off,int count,int *eof,void data){

	int len;

	printk("page=%p *start=%p off=%d count=%d *eof=%d data=%p\n",page,*start,(int)off,count,*eof,data);



}
