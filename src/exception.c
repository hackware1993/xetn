#include "exception.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern int setreg(regbuf_t);
extern int regjmp(regbuf_t, int);

ExceptionInfo ExceptionInfo_create(int32_t eid, char* msg, void* other) {
	int len = strlen(msg);
	ExceptionInfo ei = (ExceptionInfo)malloc(
		sizeof(exception_info_t) + len + 1
	);
	ei->id = eid;
	strncpy(ei->msg, msg, len);
	ei->msg[len] = '\0';
	ei->other = other;
	return ei;
}

void Exception_handle(Exception ex, int32_t eid, exception_cb_t cb) {
	/* skip if there is no exception */
	if(ex->info == NULL) {
		return;
	}
	//assert(ex->st == EX_ST_ERROR);
	if(ex->info->id == eid) {
		cb(ex->info);
		ex->st = EX_ST_INIT;
	}
	free(ex->info);
	ex->info = NULL;
}
/*
Exception Exception_init(Exception ex) {
	ex->st = EX_ST_INIT;
	return ex;
}
*/
/*
int8_t Exception_try(Exception exc) {
	assert(exc->st == EX_ST_INIT);
	if(setreg(exc->regs)) {
		return 0;
	}
	exc->st = EX_ST_NORMAL;
	return 1;
}
*/
/*
void Exception_throw(Exception ex, ExceptionInfo info) {
	assert(ex->st == EX_ST_NORMAL);
	ex->info = info;
	ex->st = EX_ST_ERROR;
	regjmp(ex->regs, 1);
}
*/

