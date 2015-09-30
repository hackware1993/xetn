#include "coroutine.h"

#include <stdio.h>
#include <stdint.h>

#define PRIVATE static

PRIVATE void __bridge() {
	/* get the pointer to the keys */
	void* zone;
	zone = (void*)(((long)&zone + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE);
	zone += -sizeof(void*) * 12;
	void** keys = (void**)zone;
	Coroutine coro = (Coroutine)keys[0];

MAIN_RUN:
	coro->main(coro);
END_AGAIN:
	coro->state = C_END;
	int ret = regsw(coro->env, 0);
	switch(ret) {
		case -1:
			coroutine_yield(coro);
			goto MAIN_RUN;
		default:
	/* can not resume coroutine whose state is C_END */
	/* this situation can be seen as a kind of error */
			goto END_AGAIN;
	}
}

Coroutine coroutine_new() {
	void* stk = aligned_alloc(PAGE_SIZE, STK_DEFAULT_SIZE);
	Coroutine coro = (Coroutine)stk;
	coro->state = C_INIT;
	coro->stk = stk;
	coro->bot = stk + GREEN_ZONE;
	coro->top = stk + (STK_DEFAULT_SIZE - RED_ZONE);
	return coro;
}

void coroutine_init(Coroutine coro, coro_cb_t main) {
	if(setreg(coro->env)) {
		__bridge();
		/* NOTICE: pc NEVER point to this position */
	}
	coro->state = C_PEND;
	//coro->ctx = ctx;
	coro->main = main;
	coro->sp = coro->env[0];
	coro->env[0] = coro->top;
	void* keys = coro->stk + (STK_DEFAULT_SIZE - sizeof(void*) * 12);
	((void**)keys)[0] = coro;
}

/* the following functions is used for DEBUG */
void coroutine_dump_regs(Coroutine coro) {
#if defined(__i386__)
	printf("[\e[1;32mSP\e[0m] %08X\n", (uint32_t)(coro->env[0]));
	printf("[\e[1;32mBX\e[0m] %08X\n", (uint32_t)(coro->env[1]));
	printf("[\e[1;32mSI\e[0m] %08X\n", (uint32_t)(coro->env[2]));
	printf("[\e[1;32mDI\e[0m] %08X\n", (uint32_t)(coro->env[3]));
	printf("[\e[1;32mBP\e[0m] %08X\n", (uint32_t)(coro->env[4]));
	printf("[\e[1;32mPC\e[0m] %08X\n", (uint32_t)(coro->env[5]));
#elif defined(__amd64__) || defined(__x86_64__)
	printf("[\e[1;32mRSP\e[0m] %016lX\n", (uint64_t)(coro->env[0]));
	printf("[\e[1;32mRBX\e[0m] %016lX\n", (uint64_t)(coro->env[1]));
	printf("[\e[1;32mRBP\e[0m] %016lX\n", (uint64_t)(coro->env[2]));
	printf("[\e[1;32mR12\e[0m] %016lX\n", (uint64_t)(coro->env[3]));
	printf("[\e[1;32mR13\e[0m] %016lX\n", (uint64_t)(coro->env[4]));
	printf("[\e[1;32mR14\e[0m] %016lX\n", (uint64_t)(coro->env[5]));
	printf("[\e[1;32mR15\e[0m] %016lX\n", (uint64_t)(coro->env[6]));
	printf("[\e[1;32mRPC\e[0m] %016lX\n", (uint64_t)(coro->env[7]));
#endif
}

#if defined(__i386__)

#define PLEN 4

#elif defined(__amd64__) || defined(__x86_64__)

#define PLEN 8

#endif

void coroutine_dump_stack(Coroutine coro) {
	void* p = coro->top + (-PLEN * PLEN);
	int i;
	while(p >= coro->bot) {
	#if defined(__i386__)
		printf("[\e[1;32m%08X\e[0m]", (uint32_t)p);
	#elif defined(__amd64__) || defined(__x86_64__)
		printf("[\e[1;32m%016lX\e[0m]", (uint64_t)p);
	#endif
		for(i = 0; i < PLEN; ++i) {
		#if defined(__i386__)
			printf(" %08X", *((uint32_t*)p + i));
		#elif defined(__amd64__) || defined(__x86_64__)
			printf(" %016lX", *((uint64_t*)p + i));
		#endif
		}
		printf("\n");
		p += -PLEN * PLEN;
	}
}
