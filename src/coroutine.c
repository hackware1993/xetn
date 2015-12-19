#include "coroutine.h"

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#ifndef _WIN32
#include <sys/mman.h>
#endif

#define PAGE_SIZE 4096
#define RED_ZONE 128
#define GREEN_ZONE 512

extern void* setreg(regbuf_t);
extern void* regsw(regbuf_t, void*);

#define PRIVATE static

PRIVATE void __sched(Coroutine coro) {
	long ret;
	void* para = regsw(coro->env, NULL);
MAIN_RUN:
	para = coro->main(coro, para);
	coro->state = CORO_END;
END_AGAIN:
	ret = (long)regsw(coro->env, para);
	switch(ret) {
		case CORO_PROMPT_RESET:
			para = regsw(coro->env, NULL);
			goto MAIN_RUN;
		default:
	/* can not resume coroutine whose state is C_END */
	/* this situation can be seen as a kind of error */
			para = NULL;
			goto END_AGAIN;
	}
}

Coroutine Coroutine_new(coro_cb_t main, size_t size) {
#ifdef _WIN32
	size_t page_size = PAGE_SIZE;
#else
	size_t page_size = sysconf(_SC_PAGESIZE);
#endif
	if(size <= STK_DEFAULT_SIZE) {
		size = STK_DEFAULT_SIZE;
	} else {
		size = (size + page_size - 1) / page_size * page_size;
	}
#ifdef _WIN32
	HANDLE hmap;
	hmap = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			size,
			NULL
	);
	void* stk = MapViewOfFile(
			hmap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			size
	);
#else
	int mmap_flags = MAP_PRIVATE | MAP_ANON;
	void* stk = mmap(NULL, size, PROT_READ | PROT_WRITE, mmap_flags, -1, 0);

	if(stk == MAP_FAILED) {
		return NULL;
	}
#endif

	Coroutine coro = (Coroutine)stk;
#ifdef _WIN32
	coro->page = hmap;
#endif
	coro->len = size;
	coro->main = main;
	coro->stk = stk;
	coro->bot = stk + GREEN_ZONE;
	coro->top = stk + (size - RED_ZONE);
	Coroutine local = NULL;
	if((local = setreg(coro->env))) {
		__sched(local);
		/* NOTICE: PC will NEVER arrive this position */
	}
	coro->sp = coro->env[0];
	coro->env[0] = coro->top;

	regsw(coro->env, coro);
	coro->state = CORO_INIT;
	return coro;
}

void Coroutine_close(Coroutine coro) {
	assert(!Coroutine_isRun(coro));
#ifdef _WIN32
	HANDLE hmap = coro->page;
	UnmapViewOfFile(coro->stk);
	CloseHandle(hmap);
#else
	/* it should not be error here, if the stk * len is not modified */
	int ret = munmap(coro->stk, coro->len);
	// TODO: check -1 here, and remove assert
	assert(ret != -1);
#endif
}

void* Coroutine_yield(Coroutine coro, void* para) {
	assert(Coroutine_isRun(coro));
	coro->state = CORO_PEND;
	return regsw(coro->env, para);
}
void* Coroutine_resume(Coroutine coro, void* para) {
	assert(Coroutine_isPend(coro) || Coroutine_isInit(coro));
	coro->state = CORO_RUN;
	return regsw(coro->env, para);
}

void Coroutine_reset(Coroutine coro, coro_cb_t main) {
	assert(Coroutine_isEnd(coro));
	if(main) {
		coro->main = main;
	}
	coro->state = CORO_INIT;
	regsw(coro->env, (void*)CORO_PROMPT_RESET);
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
