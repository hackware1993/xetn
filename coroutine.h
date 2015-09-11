#ifndef _COROUTINE_H_
#define _COROUTINE_H_

#include <setjmp.h>
#include <stdlib.h>

#define RED_ZONE 128
#define PAGE_SIZE 4096
#define STK_DEFAULT_SIZE (64 * 1024)

struct coroutine;

typedef void (*coro_cb_t)(struct coroutine*);

struct coroutine {
	jmp_buf env;
	jmp_buf buf;
	void* stk;
	void* top;
	void* bot;
	void* sp;
	coro_cb_t main;
	void* res;
	void* args;
};

struct coroutine* coroutine_new(void);

void coroutine_init(struct coroutine*, coro_cb_t);

void coroutine_yield(struct coroutine*);

void coroutine_call(struct coroutine*);

#endif // _COROUTINE_H_
