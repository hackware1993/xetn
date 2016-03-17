#ifndef _POLLIMPL_H_
#define _POLLIMPL_H_

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

PRIVATE int poll_create(Handler poll) {
	int fd = kqueue();
	if(fd == -1) {
		return -1;
	}
	Handler_init(poll, fd, H_POLL);
	return 0;
}

PRIVATE int poll_ctl(Handler poll, pollopt_t opt, Handler handler) {
	Watcher wt = (Watcher)handler;
	struct kevent event[2];
	int nev = 1;
	short filter, ofilter;
	switch(wt->event) {
		case XETN_READ:  filter = EVFILT_READ;  break;
		case XETN_WRITE: filter = EVFILT_WRITE; break;
		case XETN_ERROR:
			/* for event deleting of EV_ERROR */
			switch(wt->old_event) {
				case XETN_READ:  filter = EVFILT_READ;  break;
				case XETN_WRITE: filter = EVFILT_WRITE; break;
			}
			break;
	}
	switch(opt) {
		case P_ADD:
			EV_SET(event, handler->fileno, filter, EV_ADD | EV_CLEAR, 0, 0, NULL);
			break;
		case P_MOD:
			nev = 2;
			switch(wt->old_event) {
				case XETN_READ:  ofilter = EVFILT_READ;  break;
				case XETN_WRITE: ofilter = EVFILT_WRITE; break;
			}
			EV_SET(event, handler->fileno, ofilter,  EV_DELETE, 0, 0, NULL);
			EV_SET(event + 1, handler->fileno, filter, EV_ADD | EV_CLEAR, 0, 0, NULL);
			break;
		case P_DEL:
			EV_SET(event, handler->fileno, filter, EV_DELETE, 0, 0, NULL);
			break;
	}
	int ret = kevent(poll->fileno, event, nev, NULL, 0, NULL);
	if(ret == -1) {
		return -1;
	}
	return 0;
}

PRIVATE int poll_wait(Handler poll, int32_t to, fd_t* ios, uint8_t* ios_index, uint8_t* err_index) {
	struct kevent events[MAXEVENT];
	struct timespec* pto = NULL;
	struct timespec ts;
	if(to != -1) {
		time_t sec = to / 1000;
		long nsec  = (to - sec * 1000) * 1000000;
		ts.tv_sec  = sec;
		ts.tv_nsec = nsec;
		pto = &ts;
	}
	int32_t count = kevent(poll->fileno, NULL, 0, events, MAXEVENT, pto);
	if(count == -1) {
		return -1;
	}
	uint8_t io = 0, ie = MAXEVENT - 1;
	struct kevent* pc;
	struct kevent* pe = events + count;
	for(pc = events; pc < pe; ++pc) {
		if(pc->flags & (EV_ERROR | EV_EOF)) {
			ios[ie--] = pc->ident;
		} else if(pc->filter & (EVFILT_READ | EVFILT_WRITE)) {
			ios[io++] = pc->ident;
		}
	}
	*ios_index = io;
	*err_index = ie + 1;
	return 0;
}

#endif //_POLLIMPL_H_
