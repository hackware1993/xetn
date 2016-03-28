#include "watcher.h"
#include "reactor.h"

Watcher Watcher_init(Watcher wt, wtype_t t, Handler h, event_t e) {
	wt->type = t;
	wt->handler = *h;
	wt->event = e;
	wt->old_event = XETN_NONE;
	wt->errnum = 0;
	return wt;
}

void Watcher_close(Watcher wt) {
	Watcher_unregisterSelf(wt);
	Handler_close(&wt->handler);
	wt->onClose(wt);
}

