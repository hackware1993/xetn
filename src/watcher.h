#ifndef _WATCHER_H_
#define _WATCHER_H_

#include "handler.h"

/* declarations */
struct reactor;
struct watcher;

typedef enum wtype {
	ACCEPTOR,
	PROCESSOR,
} wtype_t;

typedef enum event {
	XETN_NONE  = 0x00,
	XETN_TIME  = 0x01,
	XETN_READ  = 0x02,
	XETN_WRITE = 0x04,
} event_t;

/* developers should process every event inside processor */
typedef void (*watcher_cb)(struct watcher*);
typedef struct watcher {
	handler_t handler;
	struct reactor* host;
	wtype_t   type;
	event_t   old_event;
	event_t   event;
	int       errnum;
	// timer_t timer;
	watcher_cb onRead;
	watcher_cb onWrite;
	watcher_cb onError;
	watcher_cb onClose;
} watcher_t, *Watcher;

#define Watcher_bindHost(w, h)     (w)->host = (h)
#define Watcher_hasHost(w)         ((w)->host != NULL)
#define Watcher_bindOnRead(w, fn)  (w)->onRead  = (fn)
#define Watcher_bindOnWrite(w, fn) (w)->onWrite = (fn)
#define Watcher_bindOnError(w, fn) (w)->onError = (fn)
#define Watcher_bindOnClose(w, fn) (w)->onClose = (fn)
#define Watcher_getEvent(w)        ((w)->event)
#define Watcher_getType(w)         ((w)->type)
#define Watcher_getHost(w)         ((w)->host)
#define Watcher_resigterSelf(wt)   Reactor_register((wt)->host, (wt))
#define Watcher_unregisterSelf(wt) Reactor_unregister((wt)->host, (wt))
#define Watcher_swapEvent(wt, ev) \
{ \
	(wt)->old_event = (wt)->event; \
	(wt)->event = (ev); \
}
#define Watcher_modEvent(w, ev)   Reactor_modEvent((wt)->host, wt, ev)

Watcher Watcher_init(Watcher, wtype_t, Handler, event_t);
void    Watcher_close(Watcher);

#endif //_WATCHER_H_
