/*************************************************************************
    > File Name: server.c
    > Author: HeJie
    > Mail: sa614415@mail.ustc.edu.cn
    > Created Time: 2015年01月03日 星期五 01时01分39秒
 ************************************************************************/
#include "work.h"
#include "tpool.h"

int main(int argc, char **argv)
{
    printf("##################### Server #####################\n");
    int port = PORT;
    if (argc>1)
        port = atoi(argv[1]);

    /*创建线程池*/
    if (tpool_create(THREAD_NUM) != 0) {
        printf("tpool_create failed\n");
        exit(-1);
    }
    printf("--- Thread Pool Strat ---\n");

    /*初始化server，监听请求*/
    int listenfd = Server_init(port);
    socklen_t sockaddr_len = sizeof(struct sockaddr);

    /*epoll*/
    static struct epoll_event ev, events[EPOLL_SIZE];
	int epfd = epoll_create(EPOLL_SIZE);
	ev.events = EPOLLIN;
	ev.data.fd = listenfd;
	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    while(1){
        int events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
        int i=0;

        /*接受连接，添加work到work-Queue*/
        for(; i<events_count; i++){
            if(events[i].data.fd == listenfd)
            {
                int connfd;
                struct sockaddr_in  clientaddr;
                while( ( connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &sockaddr_len) ) > 0 )
				{
				    printf("EPOLL: Received New Connection Request---connfd= %d\n",connfd);
					struct args *p_args = (struct args *)malloc(sizeof(struct args));
                    p_args->fd = connfd;
                    p_args->recv_finfo = recv_fileinfo;
                    p_args->recv_fdata = recv_filedata;

                    /*添加work到work-Queue*/
                    tpool_add_work(worker, (void*)p_args);
				}
            }
        }
    }

    return 0;
}
