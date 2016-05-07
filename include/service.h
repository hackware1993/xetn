#ifndef _SERVWATCHER_H_
#define _SERVWATCHER_H_

#include "reactor.h"
#include "buffer.h"
#include "http/codec.h"
#include "http/connection.h"
#include "stream.h"

typedef struct acceptor {
	watcher_t base;
	process_cb pList[1];
} acceptor_t, *Acceptor;

typedef struct processor {
	watcher_t base;
	process_cb pList[3];
	net_stream_t ns;
	pipe_stream_t pipe;

	handler_t hf;
	http_codec_t codec;
	http_connection_t req;
	http_connection_t res;
	uint8_t isHeaderFin;
	uint8_t is404;
	size_t len;
	off_t off;
} processor_t, *Processor;

Acceptor  Acceptor_new(Handler);
Processor Processor_new(Handler);

#endif //_SERVWATCHER_H_
