#include "reactor.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

#define WATCHER_MAX  65536
#define WATCHER_INIT 4096
#define MAXEVENT 64

#define PRIVATE static

typedef enum pollopt {
	P_ADD,
	P_MOD,
	P_DEL
} pollopt_t;

typedef struct fdlist {
	fd_t fd;
	struct fdlist* next;
} fdlist_t, *FdList;

/* operation functions for struct fdlist */
PRIVATE void __fdlist_put(FdList* li, fd_t fd) {
	FdList new = (FdList)malloc(sizeof(fdlist_t));
	new->fd = fd;
	if(*li != NULL) {
		new->next = (*li)->next;
		(*li)->next = new;
	} else {
		new->next = new;
	}
	*li = new;
}

PRIVATE fd_t __fdlist_get(FdList* li) {
	if(*li == NULL) {
		return -1;
	}
	FdList p = (*li)->next;
	fd_t res = p->fd;
	if(p != *li) {
		(*li)->next = p->next;
	} else {
		*li = NULL;
	}
	free(p);
	return res;
}

PRIVATE void __fdlist_free(FdList li) {
	/* skip empty list */
	if(li == NULL) {
		return;
	}
	FdList temp = NULL;
	FdList p = li;
	do {
		temp = p;
		p = p->next;
		free(temp);
	} while(p != li);
}

/* operation functions for internal poll handler */
PRIVATE void __poll_create(Handler poll) {
	int epfd = epoll_create1(0);
	if(epfd == -1) {
		perror("poll_create");
		exit(EXIT_FAILURE);
	}
	Handler_init(poll, epfd, H_POLL);
}

PRIVATE void __poll_ctl(Handler poll, pollopt_t opt, Handler handler) {
	Watcher wt = (Watcher)handler;
	int32_t stat, op;
	uint32_t events = EPOLLET;
	struct epoll_event* pev = NULL;
	fd_t fd = handler->fileno;
	switch(opt) {
		case P_ADD:
			op = EPOLL_CTL_ADD;
			goto OP_MOD;
		case P_MOD:
			op = EPOLL_CTL_MOD;
			goto OP_MOD;
		case P_DEL:
			op = EPOLL_CTL_DEL;
			goto OP_DEL;
		default:
			// TODO: add exception here
			return;
	}
OP_MOD:
	switch(wt->event) {
		case EV_READ:
			events |= EPOLLIN;
			break;
		case EV_WRITE:
			events |= EPOLLOUT;
			break;
		default:
			// TODO: add exception here
			return;
	}
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	pev = &ev;
OP_DEL:
	stat = epoll_ctl(poll->fileno, op, fd, pev);
	if(stat == -1) {
		perror("poll_ctl");
		exit(-1);
	}
	return;
}

PRIVATE void __poll_wait(Handler poll, int32_t to, FdList* ios, FdList* errs) {
	struct epoll_event events[MAXEVENT];
	int32_t count = epoll_wait(poll->fileno, events, MAXEVENT, to);
	if(count == -1) {
		perror("poll_wait");
		exit(-1);
	}
	struct epoll_event* pc;
	uint32_t i;
	for(i = 0; i < MAXEVENT; ++i) {
		pc = events + i;
		if(pc->events & (EPOLLERR | EPOLLHUP)) {
			__fdlist_put(errs, pc->data.fd);
		} else if(pc->events & (EPOLLIN | EPOLLOUT)) {
			__fdlist_put(ios, pc->data.fd);
		}
	}
	// TODO check here
}

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
	re->io_list = NULL;
	re->er_list = NULL;
	/* create internal poll handler */
	__poll_create(&re->poll);
	return re;
}

void Reactor_close(Reactor re) {
	/* destroy internal fd list */
	__fdlist_free(re->io_list);
	__fdlist_free(re->er_list);
	/* destroy watcher list*/
	free(re->ws);
	/* close internal poll */
	Handler_close(&re->poll);
	/* reactor won't free re itself */
}

void Reactor_register(Reactor re, Watcher wt) {
	int fd = wt->handler.fileno;
	re->ws[fd] = wt;
	wt->host = re;
	__poll_ctl(&re->poll, P_ADD, (Handler)wt);
	//TODO change the event of poll
}

void Reactor_unregister(Reactor re, Watcher wt) {
	int fd = wt->handler.fileno;
	re->ws[fd] = NULL;
	wt->host = NULL;
	__poll_ctl(&re->poll, P_DEL, (Handler)wt);
	//TODO change the event of poll
}

void Reactor_loopOnce(Reactor re, int32_t to) {
	__poll_wait(&re->poll, to, &re->io_list, &re->er_list);
	fd_t fd;
	Watcher wt;
	/* error watcher has the highest priority */
	while((fd = __fdlist_get(&re->er_list)) != -1) {
		wt = re->ws[fd];
		wt->event = EV_ERROR;
		wt->processor(wt);
	}

	while((fd = __fdlist_get(&re->io_list)) != -1) {
		wt = re->ws[fd];
		wt->processor(wt);
	}
	// TODO process io_list & er_list
}

void Watcher_modEvent(Watcher wt, event_t ev) {
	if(Watcher_getEvent(wt) == ev) {
		return;
	}
	handler_t poll = wt->host->poll;
	wt->event = ev;
	__poll_ctl(&poll, P_MOD, &wt->handler);
}

