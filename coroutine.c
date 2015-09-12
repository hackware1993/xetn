#include "coroutine.h"

static void** __keys() {
	void* zone;
	zone = (void*)(((long)&zone + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE);
	zone += -sizeof(void*) * 12;
	return (void**)zone;
}

static void __bridge() {
	coroutine_t coro = (coroutine_t)__keys()[0];
	coro->main(coro);
}

coroutine_t coroutine_new() {
	void* stk = aligned_alloc(PAGE_SIZE, STK_DEFAULT_SIZE);
	coroutine_t coro = (coroutine_t)stk;
	coro->stk = stk;
	coro->bot = stk + GREEN_ZONE;
	coro->top = stk + (STK_DEFAULT_SIZE - RED_ZONE);
	return coro;
}

void coroutine_init(coroutine_t coro, coro_cb_t main) {
	if(setreg(coro->buf)) {
		__bridge();
	}
	coro->main = main;
	coro->sp = (void*)coro->buf->__jmpbuf[0];
	coro->buf->__jmpbuf[0] = (long)coro->top;
	void* keys = coro->stk + (STK_DEFAULT_SIZE - sizeof(void*) * 12);
	((void**)keys)[0] = coro;
}

