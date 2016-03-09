#ifndef _STREAM_H_
#define _STREAM_H_

#include "handler.h"
#include "buffer.h"

typedef enum iostate {
	S_NONE,
	S_ERR,
	S_PEND,
	S_FULL,
	S_FIN
} iostate_t;

/* iostream for common handler */
iostate_t Stream_read(Handler, Buffer);
iostate_t Stream_write(Handler, Buffer);
iostate_t Stream_timedRead(Handler, Buffer, int32_t);
iostate_t Stream_timedWrite(Handler, Buffer, int32_t);

#endif // _STREAM_H_
