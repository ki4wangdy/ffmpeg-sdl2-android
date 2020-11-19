/*
 * This file is part of Player.
 *
 * Player is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have Player a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

#define MAX_THREADS 100
#define MAX_QUEUE 1024

typedef enum {
    THREADPOOL_INVALID        = -1,
    THREADPOOL_LOCK_FAILURE   = -2,
    THREADPOOL_QUEUE_FULL     = -3,
    THREADPOOL_SHUTDOWN       = -4,
    THREADPOOL_THREAD_FAILURE = -5
} ThreadPoolErrorType;

typedef enum {
    IMMEDIATE_SHUTDOWN = 1,
    LEISURELY_SHUTDOWN = 2
} ThreadPoolShutdownType;

typedef void (*Runable)(void *, void *);
/**
 *  @struct ThreadPoolTask
 *  @brief the work struct
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var in_arg Argument to be passed to the function.
 *  @var out_arg Argument to be passed to the call function.
 */

typedef struct ThreadPoolTask {
    Runable function;
    void *in_arg;
    void *out_arg;
} ThreadPoolTask;

/**
 *  @struct ThreadPoolContext
 *  @brief The threadpool context struct
 *
 *  @var notify        Condition variable to notify worker threads.
 *  @var threads       Array containing worker threads ID.
 *  @var thread_count  Number of threads
 *  @var queue         Array containing the task queue.
 *  @var queue_size    Size of the task queue.
 *  @var queue_head    Index of the first element.
 *  @var queue_tail    Index of the next element.
 *  @var pending_count Number of pending tasks
 *  @var shutdown      Flag indicating if the pool is shutting down
 *  @var started       Number of started threads
 */
typedef struct ThreadPoolContext {
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *threads;
    ThreadPoolTask *queue;
    int thread_count;
    int queue_size;
    int queue_head;
    int queue_tail;
    int pending_count;
    int shutdown;
    int started_count;
} ThreadPoolContext;

ThreadPoolContext *threadpool_create(int thread_count, int queue_size, int flags);

int threadpool_add(ThreadPoolContext *ctx, Runable function,
                   void *in_arg, void *out_arg, int flags);

int threadpool_destroy(ThreadPoolContext *ctx, int flags);

#endif
