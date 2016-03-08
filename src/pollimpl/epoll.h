#ifndef _POLLIMPL_H_
#define _POLLIMPL_H_

#include <sys/epoll.h>

/* operation functions for internal poll handler */
PRIVATE int poll_create(Handler poll) {
	int epfd = epoll_create1(0);
	if(epfd == -1) {
		return errno;
	}
	Handler_init(poll, epfd, H_POLL);
	return 0;
}

PRIVATE int poll_ctl(Handler poll, pollopt_t opt, Handler handler) {
	Watcher wt = (Watcher)handler;
	int32_t stat, op;
	uint32_t events = EPOLLET;
	struct epoll_event* pev = NULL;
	fd_t fd = handler->fileno;
	switch(opt) {
		case P_ADD: op = EPOLL_CTL_ADD; break;
		case P_MOD: op = EPOLL_CTL_MOD; break;
		case P_DEL: op = EPOLL_CTL_DEL; goto ENTRY_DEL;
	}
	switch(wt->event) {
		case EV_READ:  events |= EPOLLIN;  break;
		case EV_WRITE: events |= EPOLLOUT; break;
	}
	struct epoll_event ev;
	ev.events = events;
	//ev.data.fd = fd;
	/* use the pointer to Watcher directly */
	ev.data.ptr = wt;
	pev = &ev;
ENTRY_DEL:
	stat = epoll_ctl(poll->fileno, op, fd, pev);
	if(stat == -1) {
		return errno;
	}
	return 0;
}

PRIVATE int poll_wait(Handler poll, int32_t to, 
		Watcher* ios, uint8_t* ios_len, Watcher* errs, uint8_t* err_len) {
	struct epoll_event events[MAXEVENT];
	int32_t count = epoll_wait(poll->fileno, events, MAXEVENT, to);
	if(count == -1) {
		return errno;
	}
	uint8_t io = 0, ie = 0;
	struct epoll_event* pc;
	struct epoll_event* pe = events + count;
	for(pc = events; pc < pe; ++pc) {
		if(pc->events & (EPOLLERR | EPOLLHUP)) {
			errs[ie++] = (Watcher)pc->data.ptr;
		} else if(pc->events & (EPOLLIN | EPOLLOUT)) {
			ios[io++] = (Watcher)pc->data.ptr;
		}
	}
	*ios_len = io;
	*err_len = ie;
	return 0;
}

#endif // _POLLIMPL_H_
