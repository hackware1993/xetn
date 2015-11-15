#include "threadpool.h"

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include "list.h"

#define PRIVATE static

typedef struct task_queue {
	link_list_t list;
	pthread_mutex_t lock;
} task_queue_t, *TaskQueue;

#define TaskQueue_lock(tq) \
	pthread_mutex_lock(&(tq)->lock)

#define TaskQueue_unlock(tq) \
	pthread_mutex_unlock(&(tq)->lock)

#define TaskQueue_length(tq) \
	LinkList_length(&(tq)->list)

#define TaskQueue_get(tq) \
	(Task)LinkList_get(&(tq)->list)

#define TaskQueue_put(tq, t) \
	LinkList_put(&(tq)->list, t)

PRIVATE TaskQueue TaskQueue_init(TaskQueue tskq) {
	LinkList_init(&tskq->list);
	pthread_mutex_init(&tskq->lock, NULL);
	return tskq;
}

PRIVATE void TaskQueue_close(TaskQueue tskq) {
	LinkList_free(&tskq->list);
	pthread_mutex_destroy(&tskq->lock);
}

#define GET_TIMEOUT(ts, to)                     \
{                                               \
	 struct timeval now;                        \
	 gettimeofday(&now, NULL);                  \
	 (ts)->tv_sec = now.tv_sec;                 \
	 (ts)->tv_nsec = (now.tv_usec + to) * 1000; \
}

#define PUSH_BACK_TASK(thr, tsk)                    \
{                                                   \
	TaskQueue tsq = (thr)->pool->taskq + (thr)->id; \
	TaskQueue_put(tsq, (tsk));                      \
}

PRIVATE void* thread_handle(void* arg) {
	Thread targ = (Thread)arg;
	link_list_t list;
	LinkList tlist = LinkList_init(&list);
	task_result_t res;
	Task tsk = NULL;
START:
	switch(targ->signal) {
		case TS_STOP: goto FINAL;
		case TS_WAIT: goto WAIT;
		case TS_INIT: goto INIT;
	}
INIT:
	tsk = ThreadPool_getTask(targ->pool, targ);
	if(tsk != NULL) {
		LinkList_put(tlist, tsk);
	}
	/* iterate the internal task queue & process them */
	uint32_t l = LinkList_length(tlist);
	while(l > 0) {
		tsk = (Task)LinkList_get(tlist);
		res = tsk->process(tsk);
		if(res == RES_PEND) {
			LinkList_put(tlist, tsk);
		}
		--l;
	}
	goto START;
	/* will not arrive here */
WAIT:
	while(LinkList_length(tlist) != 0) {
		tsk = (Task)LinkList_get(tlist);
		res = tsk->process(tsk);
		if(res == RES_PEND) {
			LinkList_put(tlist, tsk);
		}
	}
FINAL:
	/* pushback the incomplete tasks */
	while(LinkList_length(tlist) != 0) {
		tsk = (Task)LinkList_get(tlist);
		PUSH_BACK_TASK(targ, tsk);
	}
	pthread_exit(NULL);
	/* will not arrive here */
	return NULL;
}

ThreadPool ThreadPool_init(ThreadPool pool, uint32_t init) {
	pool->num_threads = init;
	pool->threads = malloc(init * sizeof(thread_t));
	pool->taskq = malloc(init * sizeof(task_queue_t));
	uint32_t i;
	for(i = 0; i < init; ++i) {
		TaskQueue_init(pool->taskq + i);
	}
	return pool;
}

void ThreadPool_close(ThreadPool pool) {
	uint32_t i;
	for(i = 0; i < pool->num_threads; ++i) {
		TaskQueue_close(pool->taskq + i);
	}
	free(pool->taskq);
	free(pool->threads);
}

void ThreadPool_start(ThreadPool pool, size_t stk) {
	uint32_t i;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, stk * 1024);
	for(i = 0; i < pool->num_threads; ++i) {
		// TODO add error check
		// NOTICE don't use global errno
		Thread pthr = pool->threads + i;
		pthr->id   = i;
		pthr->pool = pool;
		pthread_cond_init(&pthr->cond, NULL);
		pthr->signal = TS_INIT;
		pthread_create(&pthr->tid, &attr, &thread_handle, pthr);
		// TODO consider if we should setup attr of pthread
	}
	pthread_attr_destroy(&attr);
}

/* stop all threads, and pushback all internal tasks */
void ThreadPool_stop(ThreadPool pool) {
	uint32_t i;
	for(i = 0; i < pool->num_threads; ++i) {
		pool->threads[i].signal = TS_STOP;
	}
	for(i = 0; i < pool->num_threads; ++i) {
		pthread_join(pool->threads[i].tid, NULL);
	}
}

/* stop all threads, but will wait until all internal tasks are finished */
void ThreadPool_wait(ThreadPool pool) {
	uint32_t i;
	for(i = 0; i < pool->num_threads; ++i) {
		pool->threads[i].signal = TS_WAIT;
	}
	for(i = 0; i < pool->num_threads; ++i) {
		pthread_join(pool->threads[i].tid, NULL);
	}
}

/* stop all threads immediately without wait or pushback internal tasks */
void ThreadPool_halt(ThreadPool pool) {
	uint32_t i;
	for(i = 0; i < pool->num_threads; ++i) {
		pthread_t tid = pool->threads[i].tid;
		pthread_detach(tid);
		pthread_cancel(tid);
	}
}

Task ThreadPool_getTask(ThreadPool pool, Thread thr) {
	TaskQueue tsq = pool->taskq + thr->id;
	
	TaskQueue_lock(tsq);
	if(TaskQueue_length(tsq) == 0) {
		struct timespec to;
		/* only wait for 200ms */
		GET_TIMEOUT(&to, 200);
		int ret = pthread_cond_timedwait(&thr->cond, &tsq->lock, &to);
		if(ret == ETIMEDOUT) {
			TaskQueue_unlock(tsq);
			return NULL;
		}
	}
	Task res = TaskQueue_get(tsq);

	TaskQueue_unlock(tsq);
	return res;
}

void ThreadPool_putTask(ThreadPool pool, Task tsk) {
	//TODO add lock here if you want to make this function used by multithread
	static uint32_t id = 0;
	id %= pool->num_threads;
	TaskQueue tsq = pool->taskq + id;

	TaskQueue_lock(tsq);
	uint32_t len = TaskQueue_length(tsq);
	TaskQueue_put(tsq, tsk);

	TaskQueue_unlock(tsq);
	if(len == 0) {
		pthread_cond_signal(&pool->threads[id].cond);
	}
	++id;
}

