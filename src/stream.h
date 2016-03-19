#ifndef _STREAM_H_
#define _STREAM_H_

#include "handler.h"
#include "buffer.h"

typedef enum stream_state {
	STREAM_NONE,
	STREAM_ERROR,
	STREAM_PEND,
	STREAM_HUP,
	STREAM_DONE,
} stream_state_t;

typedef enum stream_type {
	INPUT_STREAM,
	OUTPUT_STREAM,
} stream_type_t;

typedef struct net_stream {
	handler_t handler;
	Buffer    buf;
	int       errnum;
} net_stream_t, *NetStream;

NetStream NetStream_init(NetStream, Handler, uint32_t);
void      NetStream_close(NetStream);

stream_state_t NetStream_read(NetStream, void*, size_t*);
stream_state_t NetStream_write(NetStream, void*, size_t*);
stream_state_t NetStream_flush(NetStream);

/* WARNING: following functions are used internally */
#define        NetStream_getBuf(ns) (ns)->buf
stream_state_t NetStream_readToBuf(NetStream);
stream_state_t NetStream_writeFromBuf(NetStream);

#endif // _STREAM_H_
