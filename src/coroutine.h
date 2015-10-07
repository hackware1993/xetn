/*
 * Linux and FreeBSD compatible Coroutine implementation.
 * Not ready for Windows currently.
 */
#ifndef _COROUTINE_H_
#define _COROUTINE_H_

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
	C_INIT,
	C_PEND,
	C_RUN,
	C_END
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
} coroutine_t;

typedef coroutine_t* Coroutine;

Coroutine coroutine_new(size_t);

#define coroutine_free(coro) free((coro))

void coroutine_init(Coroutine, coro_cb_t);

extern int setreg(regbuf_t);

extern void regjmp(regbuf_t, int);

extern int regsw(regbuf_t, int);

//#define coroutine_yield(coro)    \
//	if(setreg((coro)->env) == 0) { \
//		(coro)->state = C_PEND;    \
//		regjmp((coro)->ctx, 1);   \
//	}
#define coroutine_yield(coro)  \
	(coro)->state = C_PEND;    \
	regsw((coro)->env, 0)

//#define coroutine_resume(coro)    \
//	if(setreg((coro)->ctx) == 0) {  \
//		(coro)->state = C_RUN;      \
//		regjmp((coro)->env, 1);     \
//	}
#define coroutine_resume(coro)  \
	(coro)->state = C_RUN;      \
	regsw((coro)->env, 1)

//#define coroutine_reset(coro)    \
//	if(setreg((coro)->ctx) == 0) { \
//		regjmp((coro)->env, -1);   \
//	}
#define coroutine_reset(coro) regsw((coro)->env, -1)

/* the following macros is used to get or judge the status of coroutine */
#define coroutine_status(coro) (coro)->state

#define coroutine_is_init(coro) (coro)->state == C_INIT

#define coroutine_is_pend(coro) (coro)->state == C_PEND

#define coroutine_is_run(coro) (coro)->state == C_RUN

#define coroutine_is_end(coro) (coro)->state == C_END

/* functions for Debug */
#if defined(DEBUG)
void coroutine_dump_regs(Coroutine);

void coroutine_dump_stack(Coroutine);
#endif // DEBUG

#endif // _COROUTINE_H_
