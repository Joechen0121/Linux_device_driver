#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>


#define DEVFILE "/dev/devone0"

int main(){
	unsigned char buf[64];
	int ret;
	int fd;
	int i;
	int sz;

	//open file	
	fd = open(DEVFILE, O_RDWR);  //blocking mode
	if (fd == -1){
		perror("open");
		exit(1);
	}
	
	printf("read()....\n");
	sz = read(fd ,buf,sizeof(buf));
	printf("read()%d \n",sz);
	if(sz > 0){
		for(i = 0; i < sz;i++) printf("%02x ",buf[i]);
	printf("\n");
	}
	else{
		printf("errno &d\n",errno);
		perror("read");
	}
	
	close(fd);
	
	return 0;
}	

