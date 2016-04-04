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

/* it is not recommend to manipulate */
#define Stream_getBuf(stm) (stm)->buf
#define Stream_clearBuf(stm) Buffer_clear((stm)->buf)

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
#define        NetStream_clearBuf(ns) Buffer_clear((ns)->buf)
#define        NetStream_getBuf(ns) (ns)->buf

/* WARNING: following functions are used internally */
stream_state_t NetStream_readToBuf(NetStream);
stream_state_t NetStream_writeFromBuf(NetStream);
stream_state_t NetStream_readToBufSpec(NetStream);
stream_state_t NetStream_writeFromBufSpec(NetStream);

typedef struct file_stream {
	handler_t handler;
	Buffer    buf;
	int       errnum;
} file_stream_t, *FileStream;

FileStream FileStream_init(FileStream, Handler, uint32_t);
void       FileStream_close(FileStream);

// currently src can only be file handler
typedef struct pipe_stream {
	handler_t src;
	handler_t dest;
	Buffer  buf;
	int     errnum;
} pipe_stream_t, *PipeStream;

PipeStream PipeStream_init(PipeStream, Handler, Handler);
//void       PipeStream_close(PipeStream);

stream_state_t PipeStream_deliver(PipeStream, size_t*);

#endif // _STREAM_H_
