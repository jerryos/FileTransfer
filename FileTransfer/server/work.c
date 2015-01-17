/*************************************************************************
    > File Name: work.c
    > Author: HeJie
    > Mail: sa614415@mail.ustc.edu.cn
    > Created Time: 2015年01月02日 星期五 17时04分28秒
 ************************************************************************/

#include "work.h"

/*gconn[]数组存放连接信息，带互斥锁*/
int freeid = 0;
struct conn gconn[CONN_MAX];
pthread_mutex_t conn_lock = PTHREAD_MUTEX_INITIALIZER;

/*结构体长度*/
int fileinfo_len = sizeof(struct fileinfo);
socklen_t sockaddr_len = sizeof(struct sockaddr);
int head_len = sizeof(struct head);
int conn_len = sizeof(struct conn);

int createfile(char *filename, int size)
{
	int fd = open(filename, O_RDWR | O_CREAT);
	fchmod(fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	lseek(fd, size-1, SEEK_SET);
	write(fd, "", 1);
	close(fd);
	return 0;
}

/*工作线程，分析type，选择工种*/
void * worker(void *argc)
{
    struct args *pw = (struct args *)argc;
    int conn_fd = pw->fd;

    char type_buf[INT_SIZE] = {0};
    char *p=type_buf;
    int recv_size=0;
    while(1){
        if( recv(conn_fd, p, 1, 0) == 1 ){
            ++recv_size;
            if(recv_size == INT_SIZE)
                break;
            ++p;
        }
    }

    int type=*((int*)type_buf);
    switch (type){
        /*接收文件信息*/
        case 0:
            printf("## worker ##\nCase %d: the work is recv file-info\n", type);
            pw->recv_finfo(conn_fd);
            break;
        /*接收文件块*/
        case 255:
            printf("## worker ##\nCase %d: the work is recv file-data\n", type);
            pw->recv_fdata(conn_fd);
          break;
        default:
            printf("unknown type!");
            return NULL;
    }

    return NULL;
}

/*接收文件信息，添加连接到gobalconn[]数组，创建填充文件，map到内存*/
void recv_fileinfo(int sockfd)
{
     /*接收文件信息*/
    char fileinfo_buf[100] = {0};
    bzero(fileinfo_buf, fileinfo_len);
    int n=0;
    for(n=0; n<fileinfo_len; n++){
        recv(sockfd, &fileinfo_buf[n], 1, 0);
    }

    struct fileinfo finfo;
    memcpy(&finfo, fileinfo_buf, fileinfo_len);

    printf("------- fileinfo -------\n");
    printf("filename = %s\nfilesize = %d\ncount = %d\nbs = %d\n", finfo.filename, finfo.filesize, finfo.count, finfo.bs);
    printf("------------------------\n");

    /*创建填充文件，map到虚存*/
    char filepath[100] = {0};
    strcpy(filepath, finfo.filename);
    createfile(filepath, finfo.filesize);
    int fd=0;
    if((fd = open(filepath, O_RDWR)) == -1 )
	{
		printf("open file erro\n");
		exit(-1);
	}
 //   printf("fd = %d\n", fd);
    char *map = (char *)mmap(NULL, finfo.filesize, PROT_WRITE|PROT_READ, MAP_SHARED, fd , 0);
 //   printf("mbegin = %p\n", map);
    close(fd);

    /*向gconn[]中添加连接*/
    pthread_mutex_lock(&conn_lock);

    printf("recv_fileinfo(): Lock conn_lock, enter gconn[]\n");
    while( gconn[freeid].used ){
        ++freeid;
        freeid = freeid%CONN_MAX;
    }

    bzero(&gconn[freeid].filename, FILENAME_MAXLEN);
    gconn[freeid].info_fd = sockfd;
    strcpy(gconn[freeid].filename, finfo.filename);
    gconn[freeid].filesize = finfo.filesize;
    gconn[freeid].count = finfo.count;
    gconn[freeid].bs = finfo.bs;
    gconn[freeid].mbegin = map;
    gconn[freeid].recvcount = 0;
    gconn[freeid].used = 1;

    pthread_mutex_unlock(&conn_lock);

    printf("recv_fileinfo(): Unock conn_lock, exit gconn[]\n");

    /*向client发送分配的freeid（gconn[]数组下标），作为确认，每个分块都将携带id*/
    char freeid_buf[INT_SIZE]={0};
    memcpy(freeid_buf, &freeid, INT_SIZE);
    send(sockfd, freeid_buf, INT_SIZE, 0);
    printf("freeid = %d\n", *(int *)freeid_buf);

    return;
}

/*接收文件块*/
void recv_filedata(int sockfd)
{
   // set_fd_noblock(sockfd);

    /*读取分块头部信息*/
    int recv_size=0;
    char head_buf[100] = {0};
    char *p=head_buf;
    while(1){
        if( recv(sockfd, p, 1, 0) == 1 ){
            ++recv_size;
            if(recv_size == head_len)
                break;
            ++p;
        }
    }

    struct head fhead;
    memcpy(&fhead, head_buf, head_len);
    int recv_id = fhead.id;

     /*计算本块在map中起始地址fp*/
    int recv_offset = fhead.offset;
    char *fp = gconn[recv_id].mbegin+recv_offset;

    printf("------- blockhead -------\n");
    printf("filename = %s\nThe filedata id = %d\noffset=%d\nbs = %d\nstart addr= %p\n", fhead.filename, fhead.id, fhead.offset, fhead.bs, fp);
    printf("-------------------------\n");

    /*接受数据，往map内存写*/
     int remain_size = fhead.bs;     //数据块中待接收数据大小
    int size = 0;                   //一次recv接受数据大小
    while(remain_size > 0){
         if((size = recv(sockfd, fp, RECVBUF_SIZE, 0)) >0){
                fp+=size;
                remain_size-=size;
//                printf("recv size = %d      ",size);
//                printf("remain size = %d\n",remain_size);
         }
    }

    printf("----------------- Recv a fileblock ----------------- \n");

    /*增加recv_count，判断是否是最后一个分块，如果是最后一个分块，同步map与文件，释放gconn*/
    pthread_mutex_lock(&conn_lock);
    gconn[recv_id].recvcount++;
    if(gconn[recv_id].recvcount == gconn[recv_id].count){
        munmap((void *)gconn[recv_id].mbegin, gconn[recv_id].filesize);

        printf("-----------------  Recv a File ----------------- \n ");

        int fd = gconn[recv_id].info_fd;
        close(fd);
		bzero(&gconn[recv_id], conn_len);
    }
    pthread_mutex_unlock(&conn_lock);

    close(sockfd);
    return;
}

/*初始化Server，监听Client*/
int Server_init(int port)
{
    int listen_fd;
    struct sockaddr_in server_addr;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0))== -1)
    {
        fprintf(stderr, "Creating server socket failed.");
        exit(-1);
    }
    set_fd_noblock(listen_fd);

    bzero(&server_addr, sockaddr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listen_fd, (struct sockaddr *) &server_addr, sockaddr_len) == -1)
    {
        fprintf(stderr, "Server bind failed.");
        exit(-1);
    }

    if(listen(listen_fd, LISTEN_QUEUE_LEN) == -1)
    {
        fprintf(stderr, "Server listen failed.");
        exit(-1);
    }
    return listen_fd;
}

void set_fd_noblock(int fd)
{
    int flag=fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag | O_NONBLOCK);
	return;
}
