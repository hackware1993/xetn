#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <stdint.h>

#if defined(__i386__)
	#define RLEN 6
#elif defined(__amd64__) || defined(__x86_64__)
	#define RLEN 8
#endif

typedef void* regbuf_t[RLEN];
extern int setreg(regbuf_t);
extern int regjmp(regbuf_t, int);

typedef enum exception_state {
	EX_ST_INIT,
	EX_ST_NORMAL,
	EX_ST_ERROR,
} exception_state_t;

typedef struct exception_info {
	int32_t id;
	void*   other;
	char    msg[];
} exception_info_t, *ExceptionInfo;

typedef void (*exception_cb_t)(ExceptionInfo);

typedef struct exception {
	exception_state_t st;
	regbuf_t regs;
	ExceptionInfo info;
} exception_t, *Exception;

//int8_t Exception_try(Exception);
//Exception Exception_init(Exception);
//void Exception_close(Exception);
//void Exception_throw(Exception, ExceptionInfo);

#define Exception_throw(ex, in) \
{ \
	(ex)->info = in; \
	regjmp((ex)->regs, 1); \
}
#define Exception_try(ex) !setreg((ex)->regs)
#define Exception_clean(ex) \
{ \
	if((ex)->info != NULL) free((ex)->info); \
}

void Exception_handle(Exception, int32_t, exception_cb_t);
ExceptionInfo ExceptionInfo_create(int32_t, char*, void*);

#define TRY(ex) if(Exception_try(ex))
#define CATCH else

#endif // _EXCEPTION_H_
