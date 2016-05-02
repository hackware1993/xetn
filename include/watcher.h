#ifndef _WATCHER_H_
#define _WATCHER_H_

#include <stdint.h>
#include "handler.h"

/* declarations */
struct reactor;
struct watcher;

typedef enum event {
	TRIG_EV_NONE  = 0x00,
	TRIG_EV_TIME  = 0x01,
	TRIG_EV_READ  = 0x02,
	TRIG_EV_WRITE = 0x04,
} event_t;

typedef int8_t (*process_cb)(struct watcher*);
/* developers should process every event inside processor */
typedef struct watcher {
	handler_t       handler;
	struct reactor* host;
	event_t         oldEvent;
	event_t         event;
	int             errnum;
	/* the destructor of service */
	process_cb      onClose;
	uint8_t         nProcess;
	uint8_t         index;
	process_cb*     pList;
} watcher_t, *Watcher;


#define Watcher_revertProcess(w)     (w)->index = 0
#define Watcher_bindProcess(w, list, n) \
{ \
	(w)->pList    = (list); \
	(w)->nProcess = (n);    \
	(w)->index    = 0;      \
}
#define Watcher_bindHost(w, h)           (w)->host = (h)
#define Watcher_hasHost(w)               ((w)->host != NULL)
#define Watcher_getEvent(w)              ((w)->event)
#define Watcher_getType(w)               ((w)->type)
#define Watcher_getHost(w)               ((w)->host)
#define Watcher_resigterSelf(wt)         Reactor_register((wt)->host, (wt))
#define Watcher_unregisterSelf(wt)       Reactor_unregister((wt)->host, (wt))
#define Watcher_swapEvent(wt, ev) \
{ \
	(wt)->oldEvent = (wt)->event; \
	(wt)->event = (ev); \
}
#define Watcher_changeEvent(w, ev)   Reactor_modWatcherEvent((wt)->host, wt, ev)

Watcher Watcher_init(Watcher, Handler h, event_t, process_cb);
void    Watcher_close(Watcher);
void    Watcher_process(Watcher);

#endif //_WATCHER_H_
