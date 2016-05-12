#ifndef _REACTOR_H_
#define _REACTOR_H_

#include <stdint.h>
#include "handler.h"
#include "watcher.h"

#define MAXEVENT 64

typedef struct reactor {
	handler_t  poll;
	/* watcher array */
	//Watcher*   ws;
	/* length of watcher array */
	uint32_t  wl;
	int32_t   errnum;
} reactor_t, *Reactor;

Reactor Reactor_init(Reactor);
void    Reactor_close(Reactor);
void    Reactor_register(Reactor, Watcher);
void    Reactor_unregister(Reactor, Watcher);
void    Reactor_modWatcherEvent(Reactor, Watcher, event_t);
// TODO use tstamp_t to replace int32_t
//void    Reactor_loopOnce(Reactor, int32_t);
void    Reactor_loop(Reactor, int32_t);

#endif // _REACTOR_H_
