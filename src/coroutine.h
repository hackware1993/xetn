/*
 * Linux and FreeBSD compatible Coroutine implementation.
 * Not ready for Windows currently.
 */
#ifndef _COROUTINE_H_
#define _COROUTINE_H_

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

#define RED_ZONE 128
#define GREEN_ZONE 512
#define PAGE_SIZE 4096
#define STK_DEFAULT_SIZE (64 * 1024)

#if defined(__i386__)

#define RLEN 6

#elif defined(__amd64__) || defined(__x86_64__)

#define RLEN 8

#endif

struct coroutine;

typedef void (*coro_cb_t)(struct coroutine*);

typedef enum cstat {
	CORO_INIT,
	CORO_PEND,
	CORO_RUN,
	CORO_END
} cstat_t;

typedef void* regbuf_t[RLEN];

typedef struct coroutine {
	cstat_t state;
	//void* ctx;
	regbuf_t env;
	void* stk;
	void* top;
	void* bot;
	void* sp;
	coro_cb_t main;
	void* res;
} coroutine_t, *Coroutine;

/* get or judge the status of coroutine */
#define Coroutine_status(coro) (coro)->state
#define Coroutine_isInit(coro) (coro)->state == CORO_INIT
#define Coroutine_isPend(coro) (coro)->state == CORO_PEND
#define Coroutine_isRun(coro) (coro)->state == CORO_RUN
#define Coroutine_isEnd(coro) (coro)->state == CORO_END

Coroutine Coroutine_new(size_t);

/* coroutine cannot be closed when it is running */
#define Coroutine_close(coro) \
	assert(Coroutine_status(coro) != CORO_RUN); \
	free((coro))

/* bind the main function for coroutine */
void Coroutine_bind(Coroutine, coro_cb_t);

/* internal function of coroutine, do not use them */
extern int  setreg(regbuf_t);
extern void regjmp(regbuf_t, int);
extern int  regsw(regbuf_t, int);

//#define coroutine_yield(coro)    \
//	if(setreg((coro)->env) == 0) { \
//		(coro)->state = C_PEND;    \
//		regjmp((coro)->ctx, 1);   \
//	}
#define Coroutine_yield(coro)  \
	(coro)->state = CORO_PEND;    \
	regsw((coro)->env, 0)

//#define coroutine_resume(coro)    \
//	if(setreg((coro)->ctx) == 0) {  \
//		(coro)->state = C_RUN;      \
//		regjmp((coro)->env, 1);     \
//	}
#define Coroutine_resume(coro)  \
	(coro)->state = CORO_RUN;      \
	regsw((coro)->env, 1)

//#define coroutine_reset(coro)    \
//	if(setreg((coro)->ctx) == 0) { \
//		regjmp((coro)->env, -1);   \
//	}
#define Coroutine_reset(coro) regsw((coro)->env, -1)

/* functions for Debug */
#if defined(DEBUG)
void Coroutine_dumpRegs(Coroutine);

void Coroutine_dumpStack(Coroutine);
#endif // DEBUG

#endif // _COROUTINE_H_
