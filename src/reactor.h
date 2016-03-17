#ifndef _REACTOR_H_
#define _REACTOR_H_

#include <stdint.h>
#include "handler.h"

#define MAXEVENT 64

/* INFOMATION: only epoll is supported now */

typedef enum wtype {
	ACCEPTOR,
	PROCESSOR,
} wtype_t;

/* only E_TIME can be set with others */
typedef enum event {
	XETN_NONE  = 0x00,
	XETN_TIME  = 0x01,
	XETN_READ  = 0x02,
	XETN_WRITE = 0x04,
	XETN_ERROR = 0x08,
} event_t;

struct reactor;
struct watcher;

/* developers should process every event inside processor */
typedef void (*watcher_cb)(struct watcher*);

typedef struct watcher {
	handler_t handler;
	wtype_t   type;
	event_t   old_event;
	event_t   event;
	struct reactor* host;
	// timer_t timer;
	watcher_cb onRead;
	watcher_cb onWrite;
	watcher_cb onError;
	watcher_cb onClose;
} watcher_t, *Watcher;

typedef struct reactor {
	handler_t  poll;
	/* watcher array */
	Watcher* ws;
	/* length of watcher array */
	uint32_t  wl;
	int32_t   err_code;
} reactor_t, *Reactor;

#define Watcher_init(w, t, h, e) \
	(w)->type = (t);             \
	(w)->handler = *(h);         \
	(w)->event = (e);            \
	(w)->old_event = XETN_NONE

#define Watcher_bindHost(w, h) \
	(w)->host = (h)
#define Watcher_bindOnRead(w, fn) \
	(w)->onRead = (fn)
#define Watcher_bindOnWrite(w, fn) \
	(w)->onWrite = (fn)
#define Watcher_bindOnError(w, fn) \
	(w)->onError = (fn)
#define Watcher_bindOnClose(w, fn) \
	(w)->onClose = (fn)

#define Watcher_getEvent(w) ((w)->event)
#define Watcher_getType(w)  ((w)->type)
#define Watcher_getHost(w)  ((w)->host)
#define Watcher_hasHost(w)  ((w)->host != NULL)
#define Watcher_resigterSelf(wt) \
	Reactor_register((wt)->host, (wt))
#define Watcher_unregisterSelf(wt) \
	Reactor_unregister((wt)->host, (wt))

void Watcher_modEvent(Watcher, event_t);
void Watcher_close(Watcher);

Reactor Reactor_init(Reactor);
void    Reactor_close(Reactor);
void    Reactor_register(Reactor, Watcher);
void    Reactor_unregister(Reactor, Watcher);
// TODO use tstamp_t to replace int32_t
//void    Reactor_loopOnce(Reactor, int32_t);
void    Reactor_loop(Reactor, int32_t);

#endif // _REACTOR_H_
