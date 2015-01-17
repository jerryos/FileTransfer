/*************************************************************************
    > File Name: file_ctl.h
    > Author: HeJie
    > Mail: sa614415@mail.ustc.edu.cn
    > Created Time: 2015年01月02日 星期五 17时04分28秒
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
/*
 *  @brief     创建指定大小的文件
 *  @para      filename	文件名；size 文件大小，单位字节
 *  @return    0: 成功 其他: 失败  
 */
int createfile(char *filename, int size);
