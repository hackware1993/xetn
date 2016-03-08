#ifndef _SERVWATCHER_H_
#define _SERVWATCHER_H_

#include "reactor.h"
#include "buffer.h"
#include "http/codec.h"
#include "http/connection.h"

typedef struct acceptor {
	watcher_t base;
} acceptor_t, *Acceptor;

typedef struct processor {
	watcher_t base;
	Buffer    buf;

	http_codec_t codec;
	http_connection_t conn;
} processor_t, *Processor;

Acceptor  Acceptor_new(Handler);
Processor Processor_new(Handler);

#endif //_SERVWATCHER_H_
