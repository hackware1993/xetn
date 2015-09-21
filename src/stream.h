#ifndef _STREAM_H_
#define _STREAM_H_

#include "handler.h"
#include "buffer.h"

enum iostate {
	S_NONE,
	S_ERR,
	S_PEND,
	S_FULL,
	S_FIN
} iostate_t;

typedef bufstream {
	handler_t handler;
	buffer_t  buf;
} *bufstream_t;

/* iostream for common handler */
iostate_t stream_read(handler_t, buffer_t);

iostate_t stream_write(handler_t, buffer_t);

///* iostream for common handler with internal buffer */
//iostate_t stream_buffered_read(handler_t, buffer_t);
//
//iostate_t stream_buffered_write(handler_t, buffer_t);
//
//iostate_t stream_flush_buffer(handler_t);

#endif // _STREAM_H_
