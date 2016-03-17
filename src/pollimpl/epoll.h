#ifndef _POLLIMPL_H_
#define _POLLIMPL_H_

#include <sys/epoll.h>

/* operation functions for internal poll handler */
PRIVATE int poll_create(Handler poll) {
	int epfd = epoll_create1(0);
	if(epfd == -1) {
		return -1;
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
		case XETN_READ:  events |= EPOLLIN;  break;
		case XETN_WRITE: events |= EPOLLOUT; break;
	}
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	pev = &ev;
ENTRY_DEL:
	stat = epoll_ctl(poll->fileno, op, fd, pev);
	if(stat == -1) {
		return -1;
	}
	return 0;
}

PRIVATE int poll_wait(Handler poll, int32_t to, fd_t* ios, uint8_t* ios_index, uint8_t* err_index) {
	struct epoll_event events[MAXEVENT];
	int32_t count = epoll_wait(poll->fileno, events, MAXEVENT, to);
	if(count == -1) {
		return -1;
	}
	uint8_t io = 0, ie = MAXEVENT - 1;
	struct epoll_event* pc;
	struct epoll_event* pe = events + count;
	for(pc = events; pc < pe; ++pc) {
		if(pc->events & (EPOLLERR | EPOLLHUP)) {
			ios[ie--] = pc->data.fd;
		} else if(pc->events & (EPOLLIN | EPOLLOUT)) {
			ios[io++]  = pc->data.fd;
		}
	}
	*ios_index = io;
	*err_index = ie + 1;
	return 0;
}

#endif // _POLLIMPL_H_
