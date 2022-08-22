#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>



#define LEN 100
#define DEVICE "/dev/char_module"

static char read_buff[LEN];


int main(){
	
	int ret,fd;
	char write_buff[LEN];
	char ch;
	//load device
	printf("load the device.....\n");
	fd = open(DEVICE,O_RDWR);
	if(fd < 0 )
		printf("failed to load !!\n");


	printf("Enter the word:\n");
	scanf("%s",write_buff);
	
	printf("writing to the device....\n");
	ret = write(fd, write_buff, LEN);		
	if(ret < 0){
		printf("failed to write to the device !!!\n");
		return errno;
	}


	ret = read(fd, read_buff, LEN);
   	if (ret < 0){
      		perror("Failed to read the message from the device.\n");
      		return errno;
   	}
   	printf("The device received message include: [%s]\n", read_buff);

	close(fd);	

	printf("The  End\n");
	return 0;
}
