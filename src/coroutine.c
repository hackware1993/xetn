#include "coroutine.h"

#include <stdio.h>
#include <stdint.h>

#define PRIVATE static

PRIVATE void __bridge() {
	/* get the pointer to the keys */
	void* zone;
	zone = (void*)(((long)&zone + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE);
	zone -= sizeof(void*) * 12;
	void** keys = (void**)zone;
	Coroutine coro = (Coroutine)keys[0];

MAIN_RUN:
	coro->main(coro);
END_AGAIN:
	coro->state = CORO_END;
	int ret = regsw(coro->env, 0);
	switch(ret) {
		case -1:
			Coroutine_yield(coro);
			goto MAIN_RUN;
		default:
	/* can not resume coroutine whose state is C_END */
	/* this situation can be seen as a kind of error */
			goto END_AGAIN;
	}
}

Coroutine Coroutine_new(size_t size) {
	if(size <= STK_DEFAULT_SIZE) {
		size = STK_DEFAULT_SIZE;
	} else {
		size = (size + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
	}
	void* stk = aligned_alloc(PAGE_SIZE, size);
	Coroutine coro = (Coroutine)stk;
	coro->state = CORO_INIT;
	coro->stk = stk;
	coro->bot = stk + GREEN_ZONE;
	coro->top = stk + (STK_DEFAULT_SIZE - RED_ZONE);
	return coro;
}

void Coroutine_bind(Coroutine coro, coro_cb_t main) {
	if(Coroutine_isInit(coro) == 0) {
		coro->main = main;
		return;
	}
	if(setreg(coro->env)) {
		__bridge();
		/* NOTICE: pc NEVER point to this position */
	}
	coro->state = CORO_PEND;
	//coro->ctx = ctx;
	coro->main = main;
	coro->sp = coro->env[0];
	coro->env[0] = coro->top;
	void* keys = coro->stk + (STK_DEFAULT_SIZE - sizeof(void*) * 12);
	((void**)keys)[0] = coro;
}

/* the following functions is used for DEBUG */
/* formation is only ready for UNIX-like platform */
void Coroutine_dumpRegs(Coroutine coro) {
#if defined(__i386__)
	fprintf(stderr, "[\e[1;32mSP\e[0m] %p\n", coro->env[0]);
	fprintf(stderr, "[\e[1;32mBX\e[0m] %p\n", coro->env[1]);
	fprintf(stderr, "[\e[1;32mSI\e[0m] %p\n", coro->env[2]);
	fprintf(stderr, "[\e[1;32mDI\e[0m] %p\n", coro->env[3]);
	fprintf(stderr, "[\e[1;32mBP\e[0m] %p\n", coro->env[4]);
	fprintf(stderr, "[\e[1;32mPC\e[0m] %p\n", coro->env[5]);
#elif defined(__amd64__) || defined(__x86_64__)
	fprintf(stderr, "[\e[1;32mRSP\e[0m] %p\n", coro->env[0]);
	fprintf(stderr, "[\e[1;32mRBX\e[0m] %p\n", coro->env[1]);
	fprintf(stderr, "[\e[1;32mRBP\e[0m] %p\n", coro->env[2]);
	fprintf(stderr, "[\e[1;32mR12\e[0m] %p\n", coro->env[3]);
	fprintf(stderr, "[\e[1;32mR13\e[0m] %p\n", coro->env[4]);
	fprintf(stderr, "[\e[1;32mR14\e[0m] %p\n", coro->env[5]);
	fprintf(stderr, "[\e[1;32mR15\e[0m] %p\n", coro->env[6]);
	fprintf(stderr, "[\e[1;32mRPC\e[0m] %p\n", coro->env[7]);
#endif
}

#define STRIDE 32

void Coroutine_dumpStack(Coroutine coro) {
	void* p = coro->top - STRIDE;
	int i;
	while(p >= coro->bot) {
		fprintf(stderr, "[\e[1;32m%p\e[0m]", p);
		for(i = 1; i <= STRIDE; ++i) {
			if(i % 4 == 1) {
				fprintf(stderr, " %02hhX", *((unsigned char*)p + i));
			} else {
				fprintf(stderr, "%02hhX", *((unsigned char*)p + i));
			}
		}
		fprintf(stderr, "\n");
		p -= STRIDE;
	}
}
