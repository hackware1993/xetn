#ifndef _REACTOR_H_
#define _REACTOR_H_

#include <stdint.h>
#include "handler.h"

/* WARNING: only epoll is supported now */

typedef enum wtype {
	ACCEPTOR,
	CHANNEL
} wtype_t;

/* only E_TIME can be set with others */
typedef enum event {
	E_NONE = 0x00,
	E_TIME = 0x01,
	E_READ = 0x02,
	E_WRITE = 0x04,
	E_ERROR = 0x08
} event_t;

typedef struct fdlist {
	fd_t fd;
	struct fdlist* next;
} *fdlist_t;

typedef struct watcher {
	struct handler handler;
	wtype_t  type;
	event_t  event;
	reactor_t host;
	// timer_t timer;
	// TODO add callback for event
} *watcher_t;

/* prepare for the latter timeout machanism */
//typedef struct timer {
//
//} *timer_t;
//
typedef struct reactor {
	struct handler  poll;
	/* watcher array */
	watcher_t* ws;
	/* length of watcher array */
	uint32_t  wl;
	fdlist_t  io_list;
	fdlist_t  er_list;
} *reactor_t;

#define watcher_create(w, t, h, e) \
	(w)->type = (t);               \
	(w)->handler = *(h);           \
	(w)->event = (e)

#define watcher_get_event(w) (w)->event

#define watcher_get_type(w) (w)->type

#define watcher_has_host(w) (w)->host == NULL

void watcher_mod_event(watcher_t, event_t);

void reactor_create(reactor_t);

void reactor_close(reactor_t);

void reactor_register(reactor_t, watcher_t);

void reactor_unregister(reactor_t, watcher_t);

void reactor_loop_once(reactor_t, int32_t);



#endif // _REACTOR_H_
