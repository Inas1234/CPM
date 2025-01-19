#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <stdbool.h>
#include <pthread.h>

typedef struct Task {
    void (*function)(void *); 
    void *arg;                
    struct Task *next;        
} Task;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_cond_t all_done; 
    pthread_t *threads;
    Task *task_queue_head;
    Task *task_queue_tail;
    bool stop;
    int num_threads;
    int active_tasks;
} ThreadPool;

static ThreadPool pool = {0};

bool thread_pool_init(int num_threads);

void thread_pool_add_task(void (*task)(void *), void *arg);

void thread_pool_wait(void);

void thread_pool_destroy(void);

#endif 
