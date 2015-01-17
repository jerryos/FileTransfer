/*************************************************************************
    > File Name: file_ctl.c
    > Author: HeJie
    > Mail: sa614415@mail.ustc.edu.cn
    > Created Time: 2015年01月02日 星期五 17时04分28秒
 ************************************************************************/

#include "file_ctl.h"

int createfile(char *filename, int size)
{
	int fd = open(filename, O_RDWR | O_CREAT);
	fchmod(fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	int pos = lseek(fd, size-1, SEEK_SET);
	write(fd, "", 1);
	close(fd);
	return 0;
}
