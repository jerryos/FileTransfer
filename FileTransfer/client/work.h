
/*************************************************************************
    > File Name: work.h
    > Author: HeJie
    > Mail: sa614415@mail.ustc.edu.cn
    > Created Time: 2015年01月02日 星期五 17时04分28秒
 ************************************************************************/

#ifndef WORK_H__
#define WORK_H__

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <time.h>


#define SERVER_IP   "127.0.0.1"     //server IP
#define PORT 10000                  //Server端口
#define THREAD_NUM  4               //线程池大小
#define FILENAME_MAXLEN   30        //文件名最大长度
#define INT_SIZE    4               //int类型长度

//#define SEND_SIZE    32768       	//32K
#define SEND_SIZE    65536       	//64K
//#define SEND_SIZE	131072			//128K
//#define SEND_SIZE	262144			//256K

//#define BLOCKSIZE   134217728		//128M
//#define BLOCKSIZE   268435456		//256M
#define BLOCKSIZE   536870912		//512M
//#define BLOCKSIZE	1073741824		//1G

/*文件信息*/
struct fileinfo{
    char filename[FILENAME_MAXLEN];     //文件名
    int filesize;                       //文件大小
    int count;                          //分块数量
    int bs;                             //标准分块大小
};

/*分块头部信息*/
struct head{
    char filename[FILENAME_MAXLEN];     //文件名
    int id;                             //分块所属文件的id，gconn[CONN_MAX]数组的下标
    int offset;                         //分块在原文件中偏移
    int bs;                             //本文件块实际大小
};

/*创建大小为size的文件*/
int createfile(char *filename, int size);

/*设置fd非阻塞*/
void set_fd_noblock(int fd);

/*初始化Client*/
int Client_init(char *ip);

/*发送文件信息，初始化分块头部信息*/
/*last_bs==0:所有分块都是标准分块；flag>0:最后一个分块不是标准分块，last_bs即为最后一块大小*/
void send_fileinfo(int sock_fd                  //要发送文件fd
                   , char *fname                //filename
                   , struct stat* p_fstat       //文件属性结构体
                   , struct fileinfo *p_finfo   //返回初始化后的文件信息
                   , int *flag);                 //最后一个分块是否时标准分块，0代表是；1代表不是

/*发送文件数据块*/
void * send_filedata(void *args);

/*生成文件块头部*/
struct head * new_fb_head(char *filename, int freeid, int *offset);

#endif
