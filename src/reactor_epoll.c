#include "reactor.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <sys/epoll.h>

#define MAX_WATCHER_NUM 65536
#define INIT_WATCHER_NUM 4096

Reactor Reactor_init(Reactor self) {
	self->signal = 0;
	self->wl = INIT_WATCHER_NUM;
	self->signal = 0;
	//re->ws = (Watcher*)calloc(INIT_WATCHER_NUM, sizeof(Watcher));
	fd_t epfd = epoll_create1(0);
	if(epfd == -1) {
		//TODO add error processing
	}
	Handler_init(&self->poll, epfd, H_POLL);
	return self;
}

void Reactor_close(Reactor re) {
	//Watcher wt;
	/*
	for(uint32_t i = 0; i < re->wl; ++i) {
		if((wt = re->ws[i]) != NULL) {
			wt->onClose(wt);
		}
	}
	free(re->ws);
	*/
	Handler_close(&re->poll);
}

void Reactor_register(Reactor re, Watcher wt) {
	fd_t fd = wt->handler.fileno;
	/*
	uint32_t len = re->wl;
	while(len <= fd) {
		len <<= 1;
	}
	if(len != re->wl) {
		re->wl = len;
		re->ws = (Watcher*)realloc(re->ws, len * sizeof(Watcher));
	}
	*/
	wt->host = re;
	//re->ws[fd] = wt;

	/* epoll processing */
	uint32_t events = EPOLLET;
	switch(wt->event) {
		case TRIG_EV_READ:  events |= EPOLLIN;  break;
		case TRIG_EV_WRITE: events |= EPOLLOUT; break;
		default: break;// should not reach here
	}
	struct epoll_event epev;
	epev.events = events;
	//epev.data.fd = fd;
	epev.data.ptr = wt;
	int ret = epoll_ctl(re->poll.fileno, EPOLL_CTL_ADD, fd, &epev);
	if(ret == -1) {
		// TODO error processing
	}
}

void Reactor_unregister(Reactor re, Watcher wt) {
	fd_t fd = wt->handler.fileno;
	wt->host = NULL;
	//re->ws[fd] = NULL;
	/* epoll processing */
	int ret = epoll_ctl(re->poll.fileno, EPOLL_CTL_DEL, fd, NULL);
	if(ret == -1) {
		// TODO error processing
	}
}

void Reactor_modWatcherEvent(Reactor re, Watcher wt, event_t ev) {
	fd_t fd = wt->handler.fileno;
	if(Watcher_getEvent(wt) == ev) {
		return;
	}
	Watcher_swapEvent(wt, ev);
	/* epoll processing */
	uint32_t events = EPOLLET;
	switch(wt->event) {
		case TRIG_EV_READ:  events |= EPOLLIN;  break;
		case TRIG_EV_WRITE: events |= EPOLLOUT; break;
		default: break;// should not reach here
	}
	struct epoll_event epev;
	epev.events = events;
	//epev.data.fd = fd;
	epev.data.ptr = wt;
	int ret = epoll_ctl(re->poll.fileno, EPOLL_CTL_MOD, fd, &epev);
	if(ret == -1) {
		// TODO error processing
	}
}

#define EPOLL_READ  (EPOLLERR | EPOLLHUP | EPOLLIN)
#define EPOLL_WRITE (EPOLLERR | EPOLLHUP | EPOLLOUT)

void Reactor_loop(Reactor re, int32_t to) {
	fd_t    epfd = re->poll.fileno;
	//Watcher* ws = re->ws;
	struct epoll_event events[MAXEVENT];
	void* fds[MAXEVENT];
	uint8_t wr_end, rd_begin;
	struct epoll_event* ev_end;
	int32_t count;
	Watcher wt;
	uint32_t* sig = &re->signal;
	while(*sig == 0) {
		wr_end = 0;
		rd_begin = MAXEVENT;
		count = epoll_wait(epfd, events, MAXEVENT, to);
		if(count == -1) {
			if(errno == EINTR) {
				// TODO: check here
				return;
			}
			perror("Reactor_loop");
			exit(-1);
			// TODO error processing
		}
		ev_end = events + count;
		for(struct epoll_event* p = events; p < ev_end; ++p) {
			if(p->events & EPOLL_WRITE) {
				fds[wr_end++] = p->data.ptr;
			} else if(p->events & EPOLL_READ) {
				fds[--rd_begin] = p->data.ptr;
			}
		}
		/* process event fd queue */
		for(uint8_t i = 0; i < wr_end; ++i) {
			//wt = ws[fds[i]];
			wt = (Watcher)fds[i];
			Watcher_process(wt);
		}
		for(uint8_t i = rd_begin; i < MAXEVENT; ++i) {
			//wt = ws[fds[i]];
			wt = (Watcher)fds[i];
			Watcher_process(wt);
		}
	}
}
