#ifndef _REACTOR_H_
#define _REACTOR_H_

#include <stdint.h>
#include "handler.h"

/* INFOMATION: only epoll is supported now */

typedef enum wtype {
	ACCEPTOR,
	PROCESSOR,
} wtype_t;

/* only E_TIME can be set with others */
typedef enum event {
	EV_NONE  = 0x00,
	EV_TIME  = 0x01,
	EV_READ  = 0x02,
	EV_WRITE = 0x04,
	EV_ERROR = 0x08
} event_t;

struct reactor;
struct watcher;

/* developers should process every event inside processor */
typedef void (*watcher_cb)(struct watcher*);

typedef struct watcher {
	handler_t handler;
	wtype_t   type;
	event_t   event;
	struct reactor* host;
	// timer_t timer;
	watcher_cb onProcess;
	watcher_cb onError;
	watcher_cb onClose;
} watcher_t, *Watcher;

typedef struct reactor {
	handler_t  poll;
	/* watcher array */
	Watcher* ws;
	/* length of watcher array */
	uint32_t  wl;
	int32_t   errno;
	fd_t      io_list[64];
	fd_t      er_list[64];
} reactor_t, *Reactor;

#define Watcher_init(w, t, h, e) \
	(w)->type = (t);             \
	(w)->handler = *(h);         \
	(w)->event = (e)

#define Watcher_getEvent(w) ((w)->event)
#define Watcher_getType(w)  ((w)->type)
#define Watcher_getHost(w)  ((w)->host)
#define Watcher_hasHost(w)  ((w)->host ^= NULL)

void Watcher_modEvent(Watcher, event_t);

Reactor Reactor_init(Reactor);
void    Reactor_close(Reactor);
void    Reactor_register(Reactor, Watcher);
void    Reactor_unregister(Reactor, Watcher);
// TODO use tstamp_t to replace int32_t
void    Reactor_loopOnce(Reactor, int32_t);
void    Reactor_loop(Reactor, int32_t);

#endif // _REACTOR_H_
