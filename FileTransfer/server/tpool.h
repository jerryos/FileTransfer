/*************************************************************************
    > File Name: tpool.h
    > Author: HeJie
    > Mail: sa614415@mail.ustc.edu.cn
    > Created Time: 2015年01月02日 星期五 17时04分28秒
 ************************************************************************/

#ifndef THREAD_POOL_H__
#define THREAD_POOL_H__

#include <pthread.h>

/* 任务结点 */
typedef struct tpool_work {
    void*               (*routine)(void*);       /* 任务函数 */
    void                *arg;                    /* 传入任务函数的参数 */
    struct tpool_work   *next;
}tpool_work_t;

/* 线程池 */
typedef struct tpool {
    int             shutdown;                    /* 线程池是否销毁 */
    int             max_thr_num;                 /* 最大线程数 */
    pthread_t       *thr_id;                     /* 线程ID数组首地址 */
    tpool_work_t    *queue_head;                 /* 任务链表队首 */
	tpool_work_t    *queue_tail; 				 /* 任务链表队尾 */
    pthread_mutex_t queue_lock;
    pthread_cond_t  queue_ready;
}tpool_t;

/* 创建线程池 */
int tpool_create(int max_thr_num);

/* 销毁线程池 */
void tpool_destroy();

/* 向线程池中添加任务 */
int tpool_add_work(void*(*routine)(void*), void *arg);

#endif
