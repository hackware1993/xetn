#include "coroutine.h"

#include <stdlib.h>

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
	coro->bot = stk + GREEN_ZONE;
	coro->top = stk + (STK_DEFAULT_SIZE - RED_ZONE);
	return coro;
}
void coroutine_free(struct coroutine* coro) {
	free(coro);
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

