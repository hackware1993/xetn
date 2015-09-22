#include "reactor.h"

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

/* operation functions for struct fdlist */
PRIVATE void __fdlist_put(fdlist_t* li, fd_t fd) {
	if(*li != NULL) {
		fdlist_t new = (fdlist_t)malloc(sizeof(struct fdlist));
		new->fd = fd;
		/* switch to new tail */
		new->next = (*li)->next;
		(*li)->next = new;
		(*li) = new;
		return;
	}
	*li = (fdlist_t)malloc(sizeof(struct fdlist));
	(*li)->fd = fd;
	(*li)->next = *li;
}

PRIVATE fd_t __fdlist_get(fdlist_t* li) {
	fdlist_t p = (*li)->next;
	fd_t res;
	if(p != *li) {
		(*li)->next = p->next;
		res = p->fd;
		free(p);
		return res;
	}
	res = p->fd;
	free(p);
	*li = NULL;
	return res;
}

PRIVATE void __fdlist_free(fd_list li) {
	/* skip empty list */
	if(li == NULL) {
		return;
	}
	fdlist_t temp = NULL;
	fdlist_t p = li;
	do {
		temp = p;
		p = p->next;
		free(temp);
	} while(p != li);
}

/* operation functions for internal poll handler */
PRIVATE void __poll_create(handler_t poll) {
	int epfd = epoll_create1(0);
	if(epfd == -1) {
		perror("poll_create");
		exit(-1);
	}
	*poll = handler_create(epfd, H_POLL);
}

PRIVATE void __poll_ctl(handler_t poll, pollopt_t opt, handler_t handler) {
	watcher_t wt = (watcher_t)handler;
	int op;
	switch(opt) {
		case P_ADD:
			op = EPOLL_CTL_ADD;
			goto process_add;
		case P_MOD:
			op = EPOLL_CTL_MOD;
			goto process_mod;
		case P_DEL:
			op = EPOLL_CTL_DEL;
			goto process_del;
		default:
			return;
	}
process_add:
process_mod:
	unsigned events = EPOLLET;
	switch(wt->event) {
		case E_READ:
			events |= EPOLLIN;
			break;
		case E_WRITE:
			events |= EPOLLOUT;
			break;
		default:
			return;
	}
	int fd = handler->fileno;
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	int s = epoll_ctl(poll->fileno, op, fd, &ev);
	if(s == -1) {
		perror("poll_ctl");
		exit(-1);
	}
	return;
process_del:
	int s = epoll_ctl(poll->fileno, op, fd, NULL);
	if(s == -1) {
		perror("poll_ctl::dell");
		exit(-1);
	}
	return;
}

PRIVATE void __poll_wait(handler_t poll, int32_t to, fdlist_t* ios, fdlist_t* errs) {
	struct epoll_event events[MAXEVENT];
	int count = epoll_wait(poll->fileno, events, MAXEVENT, to);
	if(s == -1) {
		perror("poll_wait");
		exit(-1);
	}
	struct epoll_event* pc;
	int i;
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

PRIVATE void __auto_append(reactor_t re, int fd) {
	unsigned len = re->wl;
	while(len <= fd) {
		len *= 2;
	}

	len = (len > WATCHER_MAX) ? WATCHER_MAX : len;

	re->wl = len;
	re->ws = (watcher_t*)realloc(re->ws, len * sizeof(watcher_t));
}

/* public function for reactor & watcher */
void reactor_create(reactor_t re) {
	re->wl = WATCHER_INIT;
	re->ws = (watcher_t*)calloc(WATCHER_INIT, sizeof(watcher_t));
	/* create internal poll handler */
	__poll_create(&re->poll);
}

void reactor_close(reactor_t re) {
	/* destroy internal fd list */
	__fdlist_free(re->io_list);
	__fdlist_free(re->er_list);
	re->io_list = NULL;
	re->er_list = NULL;
	/* destroy watcher list*/
	free(re->watchers);
	/* close internal poll */
	handler_close(&re->poll);
	/* reactor won't free re itself */
}

void reactor_register(reactor_t re, watcher_t wt) {
	int fd = wt->handler.fileno;
	re->listeners[fd] = wt;
	wt->host = re;
	__poll_ctl(&re->poll, P_ADD, (handler_t)wt);
	//TODO change the event of poll
}

void reactor_unregister(reactor_t re, watcher_t wt) {
	int fd = wt->handler.fileno;
	re->listeners[fd] = NULL;
	wt->host = NULL:
	__poll_ctl(&re->poll, P_DEL, (handler_t)wt);
	//TODO change the event of poll
}

void reactor_loop_once(reactor_t re, int32_t to) {
	__poll_wait(&re->poll, to, &re->io_list, &re->er_list);
	// TODO process io_list & er_list
}

void watcher_mod_event(watcher_t wt, event_t ev) {
	if(watcher_get_event == ev) {
		return;
	}
	handler_t poll = &wt->host->poll;
	__poll_ctl(poll, P_MOD, &wt->handler);
}

