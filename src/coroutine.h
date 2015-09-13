#ifndef _COROUTINE_H_
#define _COROUTINE_H_

#include <setjmp.h>
#include <stdlib.h>

#define RED_ZONE 128
#define GREEN_ZONE 512
#define PAGE_SIZE 4096
#define STK_DEFAULT_SIZE (64 * 1024)

struct coroutine;

typedef void (*coro_cb_t)(struct coroutine*);

typedef struct coroutine {
	jmp_buf env;
	jmp_buf buf;
	void* stk;
	void* top;
	void* bot;
	void* sp;
	coro_cb_t main;
	void* res;
	void* args;
} *coroutine_t;

coroutine_t coroutine_new(void);

#define coroutine_free(coro) free((coro))
//void coroutine_free(coroutine_t);

void coroutine_init(coroutine_t, coro_cb_t);

extern int setreg(jmp_buf);

extern void regjmp(jmp_buf, int);

#define coroutine_yield(coro)    \
	if(setreg((coro)->buf) == 0) \
		regjmp((coro)->env, 1)

#define coroutine_call(coro)      \
	if(setreg((coro)->env) == 0) \
		regjmp((coro)->buf, 1)

#endif // _COROUTINE_H_
