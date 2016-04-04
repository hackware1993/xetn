/*
 * Linux and FreeBSD compatible Coroutine implementation.
 * Not ready for Windows currently.
 */
#ifndef _COROUTINE_H_
#define _COROUTINE_H_

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define STK_DEFAULT_SIZE (64 * 1024)

#if defined(__i386__)
	#define RLEN 6
#elif defined(__amd64__) || defined(__x86_64__)
	#define RLEN 8
#endif

#define CORO_PROMPT_RESET -1
#define CORO_PROMPT_INIT   1

struct coroutine;

typedef void* (*coro_cb_t)(struct coroutine*, void*);

typedef enum coro_stat {
	CORO_INIT,
	CORO_PEND,
	CORO_RUN,
	CORO_END
} coro_stat_t;

typedef void* regbuf_t[RLEN];

typedef struct coroutine {
	coro_stat_t state;
	regbuf_t env;
#ifdef _WIN32
	HANDLE page;
#endif
	void*  stk;
	size_t len;
	void*  top;
	void*  bot;
	void*  sp;
	void*  data;
	coro_cb_t main;
} coroutine_t, *Coroutine;

/* get or judge the status of coroutine */
#define Coroutine_status(coro) (coro)->state
#define Coroutine_isInit(coro) ((coro)->state == CORO_INIT)
#define Coroutine_isPend(coro) ((coro)->state == CORO_PEND)
#define Coroutine_isRun(coro)  ((coro)->state == CORO_RUN)
#define Coroutine_isEnd(coro)  ((coro)->state == CORO_END)

Coroutine Coroutine_new(coro_cb_t, size_t);
void Coroutine_close(Coroutine);

void* Coroutine_yield(Coroutine, void*);
void* Coroutine_resume(Coroutine, void*);
void  Coroutine_reset(Coroutine, coro_cb_t);

/* functions for Debug */
#if defined(DEBUG)
void Coroutine_dumpRegs(Coroutine);
void Coroutine_dumpStack(Coroutine);
#endif // DEBUG

#endif // _COROUTINE_H_
