#include "thread_pool.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


// Worker thread function
static void *thread_worker(void *arg) {
    while (true) {
        Task *task;

        pthread_mutex_lock(&pool.lock);
        while (!pool.stop && !pool.task_queue_head) {
            pthread_cond_wait(&pool.notify, &pool.lock);
        }

        if (pool.stop) {
            pthread_mutex_unlock(&pool.lock);
            pthread_exit(NULL);
        }

        task = pool.task_queue_head;
        pool.task_queue_head = task->next;
        if (!pool.task_queue_head) pool.task_queue_tail = NULL;

        //pool.active_tasks++;
        pthread_mutex_unlock(&pool.lock);

        task->function(task->arg);
        free(task);

        pthread_mutex_lock(&pool.lock);
        pool.active_tasks--;
        if (pool.active_tasks == 0 && !pool.task_queue_head) {
            pthread_cond_signal(&pool.all_done);
        }
        pthread_mutex_unlock(&pool.lock);
    }
}


bool thread_pool_init(int num_threads) {
    if (num_threads <= 0) return false;

    pool.num_threads = num_threads;
    pool.threads = malloc(num_threads * sizeof(pthread_t));
    if (!pool.threads) return false;

    pool.task_queue_head = NULL;
    pool.task_queue_tail = NULL;
    pool.stop = false;
    pool.active_tasks = 0;

    pthread_mutex_init(&pool.lock, NULL);
    pthread_cond_init(&pool.notify, NULL);
    pthread_cond_init(&pool.all_done, NULL);

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&pool.threads[i], NULL, thread_worker, NULL) != 0) {
            thread_pool_destroy();
            return false;
        }
    }

    return true;
}

void thread_pool_add_task(void (*task)(void *), void *arg) {
    Task *new_task = malloc(sizeof(Task));
    if (!new_task) {
        fprintf(stderr, "Failed to allocate task.\n");
        return;
    }

    new_task->function = task;
    new_task->arg = arg;
    new_task->next = NULL;

    pthread_mutex_lock(&pool.lock);

    // Increment active tasks for the new task
    pool.active_tasks++;

    if (pool.task_queue_tail) {
        pool.task_queue_tail->next = new_task;
    } else {
        pool.task_queue_head = new_task;
    }
    pool.task_queue_tail = new_task;

    pthread_cond_signal(&pool.notify);
    pthread_mutex_unlock(&pool.lock);
}

void thread_pool_wait(void) {
    pthread_mutex_lock(&pool.lock);

    // Wait until no tasks are active and the queue is empty
    while (pool.active_tasks > 0 || pool.task_queue_head != NULL) {
        pthread_cond_wait(&pool.all_done, &pool.lock);
    }

    pthread_mutex_unlock(&pool.lock);
}

void thread_pool_destroy(void) {
    pthread_mutex_lock(&pool.lock);
    pool.stop = true;

    // Wake up all worker threads
    pthread_cond_broadcast(&pool.notify);
    pthread_mutex_unlock(&pool.lock);

    // Join all worker threads
    for (int i = 0; i < pool.num_threads; i++) {
        pthread_join(pool.threads[i], NULL);
    }

    // Clean up remaining tasks in the queue
    while (pool.task_queue_head) {
        Task *task = pool.task_queue_head;
        pool.task_queue_head = task->next;
        free(task);
    }

    free(pool.threads);

    pthread_mutex_destroy(&pool.lock);
    pthread_cond_destroy(&pool.notify);
    pthread_cond_destroy(&pool.all_done);
}

