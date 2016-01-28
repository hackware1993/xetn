#include "threadpool.h"

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include "list.h"

#define PRIVATE static

typedef struct task_queue {
	link_list_t     list;
	pthread_mutex_t lock;
} task_queue_t, *TaskQueue;

#define TaskQueue_trylock(tq) \
	pthread_mutex_trylock(&(tq)->lock)

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

PRIVATE inline TaskQueue TaskQueue_init(TaskQueue tskq) {
	LinkList_init(&tskq->list);
	pthread_mutex_init(&tskq->lock, NULL);
	return tskq;
}

PRIVATE inline void TaskQueue_close(TaskQueue tskq) {
	LinkList_free(&tskq->list);
	pthread_mutex_destroy(&tskq->lock);
}

#define GET_TIMEOUT(ts, to)                            \
{                                                      \
	 struct timeval now;                               \
	 gettimeofday(&now, NULL);                         \
	 (ts)->tv_sec = now.tv_sec;                        \
	 (ts)->tv_nsec = (now.tv_usec + to * 1000) * 1000; \
}

#define PUSH_BACK_TASK(thr, tsk)                    \
{                                                   \
	TaskQueue tsq = (thr)->pool->taskq + (thr)->id; \
	TaskQueue_put(tsq, (tsk));                      \
}

PRIVATE void Thread_getTask(Thread thr, LinkList list) {
	TaskQueue tsq = thr->pool->taskq + thr->id;
	size_t ilen = LinkList_length(list);
	size_t nget = 0;
	size_t olen = 0;
	
	TaskQueue_lock(tsq);
AGAIN:
	olen = TaskQueue_length(tsq);
	if(olen == 0) {
		struct timespec to;
		/* only wait for 200ms */
		GET_TIMEOUT(&to, 200);
		// TODO: wait only when the ntasks is equal to 0
		int ret = pthread_cond_timedwait(&thr->cond, &tsq->lock, &to);
		if(ret == ETIMEDOUT) {
			TaskQueue_unlock(tsq);
			return;
		} else {
			/* thread is waked by the master */
			/* check the available tasks again */
			goto AGAIN;
		}
	} else if(olen > 2 * ilen) {
		nget = olen;
	} else if(ilen > 2 * olen) {
		nget = 1;
	} else {
		nget = (ilen + olen) / 3;
		if(nget > olen) {
			nget = olen;
		}
	}
	uint32_t i;
	for(i = 0; i < nget; ++i) {
		LinkList_put(list, TaskQueue_get(tsq));
	}

	TaskQueue_unlock(tsq);
}

PRIVATE void Thread_getAllTasks(Thread thr, LinkList list) {
	TaskQueue tsq = thr->pool->taskq + thr->id;
	size_t nget = 0;
	
	TaskQueue_lock(tsq);
	nget = TaskQueue_length(tsq);
	uint32_t i;
	for(i = 0; i < nget; ++i) {
		LinkList_put(list, TaskQueue_get(tsq));
	}
	TaskQueue_unlock(tsq);
}
PRIVATE void* thread_handle(void* arg) {
	Thread targ = (Thread)arg;
	/* internal task queue */
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
	/* a simple algorithm to keep the balance of outter & inner task queue */
	Thread_getTask(targ, tlist);
	/* iterate the internal task queue & process them */
	uint32_t l = LinkList_length(tlist);
	while(l > 0) {
		tsk = (Task)LinkList_get(tlist);
		if(tsk->pre_process) {
			tsk->pre_process(tsk);
		}
		res = tsk->process(tsk);
		if(tsk->post_process) {
			tsk->post_process(tsk);
		}
		if(res == RES_PEND) {
			LinkList_put(tlist, tsk);
		}
		--l;
	}
	goto START;
	/* will not arrive here */
WAIT:
	/* consume all tasks inside the outter task queue */
	Thread_getAllTasks(targ, tlist);
	while(LinkList_length(tlist) != 0) {
		tsk = (Task)LinkList_get(tlist);
		if(tsk->pre_process) {
			tsk->pre_process(tsk);
		}
		res = tsk->process(tsk);
		if(tsk->post_process) {
			tsk->post_process(tsk);
		}
		if(res == RES_PEND) {
			LinkList_put(tlist, tsk);
		}
	}
	pthread_exit(NULL);
	//return NULL;
FINAL:
	/* pushback the incomplete tasks */
	while(LinkList_length(tlist) != 0) {
		tsk = (Task)LinkList_get(tlist);
		PUSH_BACK_TASK(targ, tsk);
	}
	pthread_exit(NULL);
	/* will not arrive here */
	//return NULL;
}

ThreadPool ThreadPool_init(ThreadPool pool, uint32_t init) {
	pool->round = 0;
	pool->nthreads = init;
	pool->threads = (Thread)malloc(init * sizeof(thread_t));
	pool->taskq   = (TaskQueue)malloc(init * sizeof(task_queue_t));
	uint32_t i;
	for(i = 0; i < init; ++i) {
		TaskQueue_init(pool->taskq + i);
	}
	return pool;
}

void ThreadPool_close(ThreadPool pool) {
	uint32_t i;
	for(i = 0; i < pool->nthreads; ++i) {
		TaskQueue_close(pool->taskq + i);
	}
	free(pool->taskq);
	free(pool->threads);
}

void ThreadPool_start(ThreadPool pool, size_t stk) {
	uint32_t i;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	/* set the size of stack for every worker in bytes */
	pthread_attr_setstacksize(&attr, stk);
	for(i = 0; i < pool->nthreads; ++i) {
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
	for(i = 0; i < pool->nthreads; ++i) {
		pool->threads[i].signal = TS_STOP;
	}
	for(i = 0; i < pool->nthreads; ++i) {
		pthread_join(pool->threads[i].tid, NULL);
	}
}

/* stop all threads, but will wait until all internal tasks are finished */
void ThreadPool_wait(ThreadPool pool) {
	uint32_t i;
	for(i = 0; i < pool->nthreads; ++i) {
		pool->threads[i].signal = TS_WAIT;
	}
	for(i = 0; i < pool->nthreads; ++i) {
		pthread_join(pool->threads[i].tid, NULL);
	}
}

/* stop all threads immediately without wait or pushback internal tasks */
void ThreadPool_halt(ThreadPool pool) {
	uint32_t i;
	for(i = 0; i < pool->nthreads; ++i) {
		pthread_t tid = pool->threads[i].tid;
		pthread_detach(tid);
		pthread_cancel(tid);
	}
}

void ThreadPool_putTask(ThreadPool pool, Task tsk) {
	//TODO add lock here if you want to make this function used by multithread
	uint32_t round = pool->round++;
	round %= pool->nthreads;
	TaskQueue tsq = pool->taskq + round;

	/* put the task without blocking */
	/* if the task queue is busy, use the next one */
	while(TaskQueue_trylock(tsq) == EBUSY) {
		round = ++round % pool->nthreads;
		tsq = pool->taskq + round;
	}
	uint32_t len = TaskQueue_length(tsq);
	TaskQueue_put(tsq, tsk);

	TaskQueue_unlock(tsq);

	if(len == 0) {
		pthread_cond_signal(&pool->threads[round].cond);
	}
}

