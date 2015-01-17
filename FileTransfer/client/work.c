/*************************************************************************
    > File Name: work.c
    > Author: HeJie
    > Mail: sa614415@mail.ustc.edu.cn
    > Created Time: 2015年01月02日 星期五 17时04分28秒
 ************************************************************************/

#include "work.h"

/*信息交换sockfd*/
int info_fd;

extern char *mbegin;
extern int port;

/*结构体长度*/
int fileinfo_len = sizeof(struct fileinfo);
socklen_t sockaddr_len = sizeof(struct sockaddr);
int head_len = sizeof(struct head);

int createfile(char *filename, int size)
{
	int fd = open(filename, O_RDWR | O_CREAT);
	fchmod(fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	lseek(fd, size-1, SEEK_SET);
	write(fd, "", 1);
	close(fd);
	return 0;
}


struct head * new_fb_head(char *filename, int freeid, int *offset)
{
    struct head * p_fhead = (struct head *)malloc(head_len);
    bzero(p_fhead, head_len);
    strcpy(p_fhead->filename, filename);
    p_fhead->id = freeid;
    p_fhead->offset = *offset;
    p_fhead->bs = BLOCKSIZE;
    *offset += BLOCKSIZE;
    return p_fhead;
}


void send_fileinfo(int sock_fd, char *fname, struct stat* p_fstat, struct fileinfo *p_finfo, int *p_last_bs)
{	
	/*初始化fileinfo*/
    bzero(p_finfo, fileinfo_len);
    strcpy(p_finfo->filename, fname);
    p_finfo->filesize = p_fstat->st_size;

    /*最后一个分块可能不足一个标准分块*/
    int count = p_fstat->st_size/BLOCKSIZE;
    if(p_fstat->st_size%BLOCKSIZE == 0){
        p_finfo->count = count;
    }
    else{
        p_finfo->count = count+1;
        *p_last_bs = p_fstat->st_size - BLOCKSIZE*count;
    }
    p_finfo->bs = BLOCKSIZE;

    /*发送type和文件信息*/
    char send_buf[100]= {0};
    int type=0;
    memcpy(send_buf, &type, INT_SIZE);
    memcpy(send_buf+INT_SIZE, p_finfo, fileinfo_len);
    send(sock_fd, send_buf, fileinfo_len+INT_SIZE, 0);

	printf("-------- fileinfo -------\n");
    printf("filename= %s\nfilesize= %d\ncount= %d\nblocksize= %d\n", p_finfo->filename, p_finfo->filesize, p_finfo->count, p_finfo->bs);
	printf("-------------------------\n");
    return;
}

void * send_filedata(void *args)
{
    struct head * p_fhead = (struct head *)args;
    printf("------- blockhead -------\n");
    printf("filename= %s\nThe filedata id= %d\noffset= %d\nbs= %d\n", p_fhead->filename, p_fhead->id, p_fhead->offset, p_fhead->bs);
    printf("-------------------------\n");

    int sock_fd = Client_init(SERVER_IP);
	//set_fd_noblock(sock_fd);

	/*发送type和数据块头部*/
    char send_buf[100]= {0};
    int type=255;
    memcpy(send_buf, &type, INT_SIZE);
	memcpy(send_buf+INT_SIZE, p_fhead, head_len);
	int sendsize=0, len = head_len+INT_SIZE;
    char *p=send_buf;
    while(1){
        if((send(sock_fd, p, 1, 0) >0)){
            ++sendsize;
            if(sendsize == len)
                break;
            ++p;
        }
    }
//	printf("head_len = %d ; send head: sendsize = %d\n",head_len, sendsize);

	/*发送数据块*/
	printf("Thread : send filedata\n");
	int i=0, send_size=0, num=p_fhead->bs/SEND_SIZE;
	char *fp=mbegin+p_fhead->offset;
	for(i=0; i<num; i++){
		if( (send_size = send(sock_fd, fp, SEND_SIZE, 0)) == SEND_SIZE){
			fp+=SEND_SIZE;
//			printf("fp = %p ; a SEND_SIZE ok\n", fp);
		}
		else{
//			printf("send_size = %d ;  a SEND_SIZE erro\n",send_size);
		}
	}

	printf("### send a fileblock ###\n");
    close(sock_fd);
	free(args);
    return NULL;
}


int Client_init(char *ip)
{
    //创建socket
    int sock_fd = socket(AF_INET,SOCK_STREAM, 0);

    //构建地址结构体
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    //连接服务器
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sockaddr_len) < 0)
    {
        perror("connect");
        exit(-1);
    }
    return sock_fd;
}


void set_fd_noblock(int fd)
{
    int flag=fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag | O_NONBLOCK);
	return;
}
