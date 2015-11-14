#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <stdint.h>
#include <pthread.h>

/**
 * ThreadPool is provide for the following situation:
 *     1. there is a master thread and several worker threads
 *     2. master thread cannot block for a long time
 *     3. worker can be blocked until the task is accomplished
 */

// TODO: add error report mechanism

typedef enum task_result {
	RES_DONE,
	RES_PEND,
} task_result_t;

typedef struct task {
	void (*pre_process)(struct task*);
	void (*post_process)(struct task*);
	task_result_t (*process)(struct task*);
} task_t, *Task;

typedef enum thread_signal {
	TS_INIT,
	TS_STOP,
	TS_WAIT,
} thrsig_t;

struct thread_pool;
typedef struct thread {
	uint32_t   id;
	pthread_t  tid;
	Task       tasks;
	thrsig_t   signal;
	pthread_cond_t cond;
	struct thread_pool*  pool;
} thread_t, *Thread;

struct task_queue;
typedef struct thread_pool {
	uint32_t init_threads;
	uint32_t max_threads;
	uint32_t min_threads;
	uint32_t num_threads;

	Thread threads;
	struct task_queue* taskq;
} thread_pool_t, *ThreadPool;

ThreadPool ThreadPool_init(ThreadPool, uint32_t, uint32_t, uint32_t);
void ThreadPool_close(ThreadPool);

void ThreadPool_start(ThreadPool);
void ThreadPool_wait(ThreadPool);
void ThreadPool_stop(ThreadPool);
void ThreadPool_halt(ThreadPool);

Task ThreadPool_getTask(ThreadPool, Thread);
void ThreadPool_putTask(ThreadPool, Task);

#endif // _THREADPOOL_H_
