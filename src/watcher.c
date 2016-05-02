#include "watcher.h"
#include "reactor.h"

Watcher Watcher_init(Watcher wt, Handler h, event_t initEvent, process_cb destructor) {
	wt->oldEvent = TRIG_EV_NONE;
	wt->errnum   = 0;
	wt->handler  = *h;
	wt->event    = initEvent;
	wt->onClose  = destructor;
	wt->index    = 0;
	wt->nProcess = 0;
	wt->pList    = NULL;
	return wt;
}

void Watcher_close(Watcher wt) {
	Watcher_unregisterSelf(wt);
	Handler_close(&wt->handler);
	/* user SHOULD bind the onClose destructor */
	if(wt->onClose) {
		wt->onClose(wt);
	}
}

void Watcher_process(Watcher wt) {
	if(wt->pList == NULL) {
		/* empty proecss list, close the watcher immediately */
		Watcher_close(wt);
		return;
	}
	int ret;
	uint8_t index, n  = wt->nProcess;
	process_cb* pList = wt->pList;
	process_cb callback = NULL;
	/* we should read real index every time */
	/* because of the operation of revertProcess */
	while((index = wt->index) < n) {
		callback = pList[index];
		ret = callback(wt);
		switch(ret) {
			case -1:
				/* an error occured */
				/* process should handle the error before return -1 */
				Watcher_close(wt);
				return;
			case 0:
				++wt->index;
				break;
			case 1:
				return;
		}
	}
	Watcher_close(wt);
}
