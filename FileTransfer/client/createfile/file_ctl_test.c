/*************************************************************************
    > File Name: file_ctl_test.c
    > Author: HeJie
    > Mail: sa614415@mail.ustc.edu.cn
    > Created Time: 2015年01月03日 星期六 16时39分48秒
 ************************************************************************/

#include "file_ctl.h"
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main()
{
	int i, fd, size=1024*1024*512;

	createfile("abc", size);
	if((fd = open("abc", O_RDWR)) == -1 )
	{
		printf("open erro\n");
		exit(-1);
	}
	struct stat filestat;
	fstat(fd ,&filestat);
	printf("file size = %ld\n",filestat.st_size);

	return 0;
}
