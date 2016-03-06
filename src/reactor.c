#include "reactor.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define PRIVATE static

typedef enum pollopt {
	P_ADD,
	P_MOD,
	P_DEL
} pollopt_t;

#define WATCHER_MAX  65536
#define WATCHER_INIT 4096
#define MAXEVENT     64

#include "pollimpl/epoll.h"

PRIVATE void __auto_append(Reactor re, fd_t fd) {
	uint32_t len = re->wl;
	while(len <= fd) {
		len *= 2;
	}

	len = (len > WATCHER_MAX) ? WATCHER_MAX : len;

	re->wl = len;
	re->ws = (Watcher*)realloc(re->ws, len * sizeof(Watcher));
}

/* public function for reactor & watcher */
Reactor Reactor_init(Reactor re) {
	re->wl = WATCHER_INIT;
	re->ws = (Watcher*)calloc(WATCHER_INIT, sizeof(Watcher));
	/* create internal poll handler */
	poll_create(&re->poll);
	return re;
}

void Reactor_close(Reactor re) {
	uint32_t i;
	Watcher wt;
	for(i = 0; i < re->wl; ++i) {
		if(wt = re->ws[i]) {
			/* if the watcher doesn't close before the reactor is ended */
			/* the destructor will close them automatically */
			wt->onClose(wt);
		}
	}
	/* destroy watcher list*/
	free(re->ws);
	/* close internal poll */
	Handler_close(&re->poll);
	/* reactor won't free re itself */
}

void Reactor_register(Reactor re, Watcher wt) {
	int fd = wt->handler.fileno;
	wt->host = re;
	re->ws[fd] = wt;
	poll_ctl(&re->poll, P_ADD, (Handler)wt);
}

void Reactor_unregister(Reactor re, Watcher wt) {
	int fd = wt->handler.fileno;
	wt->host = NULL;
	re->ws[fd] = NULL;
	poll_ctl(&re->poll, P_DEL, (Handler)wt);
}

void Reactor_loopOnce(Reactor re, int32_t to) {
	Watcher wt;
	fd_t* p;
	poll_wait(&re->poll, to, re->io_list, re->er_list);
	/* error watcher has the highest priority */
	p = re->er_list;
	while(*p != -1) {
		wt = re->ws[*p];
		wt->event = EV_ERROR;
		wt->onError(wt);
		*p = -1;
	}

	p = re->io_list;
	while(*p != -1) {
		wt = re->ws[*p];
		wt->onProcess(wt);
		*p = -1;
	}
}

void Reactor_loop(Reactor re, int32_t to) {
	Watcher wt;
	fd_t* p;
	while(1) {
		poll_wait(&re->poll, to, re->io_list, re->er_list);
		/* error watcher has the highest priority */
		p = re->er_list;
		while(*p != -1) {
			wt = re->ws[*p];
			wt->event = EV_ERROR;
			wt->onError(wt);
			*p = -1;
		}

		p = re->io_list;
		while(*p != -1) {
			wt = re->ws[*p];
			wt->onProcess(wt);
			*p = -1;
		}
	}
}

void Watcher_modEvent(Watcher wt, event_t ev) {
	if(Watcher_getEvent(wt) == ev) {
		return;
	}
	handler_t poll = wt->host->poll;
	wt->event = ev;
	poll_ctl(&poll, P_MOD, &wt->handler);
}

