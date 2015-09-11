#include "coroutine.h"

extern int setreg(jmp_buf);
extern void regjmp(jmp_buf, int);

static void** __keys() {
	void* zone;
	zone = (void*)(((long)&zone + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE);
	zone += -sizeof(void*) * 12;
	return (void**)zone;
}

static void __bridge() {
	struct coroutine* coro = (struct coroutine*)__keys()[0];
	coro->main(coro);
}

struct coroutine* coroutine_new() {
	void* stk = aligned_alloc(PAGE_SIZE, STK_DEFAULT_SIZE);
	struct coroutine* coro = (struct coroutine*)stk;
	coro->stk = stk;
	coro->bot = stk + RED_ZONE;
	coro->top = stk + (STK_DEFAULT_SIZE - RED_ZONE);
	return coro;
}

void coroutine_init(struct coroutine* coro, coro_cb_t main) {
	if(setreg(coro->buf)) {
		__bridge();
	}
	coro->main = main;
	coro->sp = (void*)coro->buf->__jmpbuf[0];
	coro->buf->__jmpbuf[0] = (long)coro->top;
	void* keys = coro->stk + (STK_DEFAULT_SIZE - sizeof(void*) * 12);
	((void**)keys)[0] = coro;
}

void coroutine_yield(struct coroutine* coro) {
	if(setreg(coro->buf) == 0) {
		regjmp(coro->env, 1);
	}
}

void coroutine_call(struct coroutine* coro) {
	if(setreg(coro->env) == 0) {
		regjmp(coro->buf, 1);
	}
}

