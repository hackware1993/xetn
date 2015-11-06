#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct buffer {
	unsigned pos;
	unsigned end;
	unsigned lim;
	unsigned cap;
	char     ptr[0];
	// int appendable;
} buffer_t, *Buffer;

Buffer Buffer_new(uint32_t);

#define Buffer_free(buf) free((buf))

#define Buffer_getPtr(buf) (buf)->ptr

#define Buffer_getCap(buf) (buf)->cap

#define Buffer_getPos(buf) (buf)->pos

#define Buffer_setPos(buf, p) (buf)->pos = (p)

#define Buffer_getLen(buf) (buf)->end

#define Buffer_setLen(buf, l) (buf)->end = (l)

#define Buffer_getLim(buf) (buf)->lim

#define Buffer_setLim(buf, l) (buf)->lim = (l)

#define Buffer_isEmpty(buf) (buf)->end == 0

#define Buffer_isEnd(buf) (buf)->pos == (buf)->end

#define Buffer_isFull(buf) (buf)->end == (buf)->lim

#define Buffer_clear(buf) \
	(buf)->pos = 0;       \
	(buf)->end = 0;       \
	(buf)->lim = (buf)->cap

#define Buffer_rewind(buf) (buf)->pos = 0

#define Buffer_put(buf, c) (buf)->ptr[(buf)->end++] = (c)

#define Buffer_get(buf) (buf)->ptr[(buf)->pos++]

#define Buffer_getByIndex(buf, i) (buf)->ptr[(i)]

#define Buffer_setByIndex(buf, i, v) (buf)->ptr[(i)] = (v)

uint32_t Buffer_putArr(Buffer, char*, uint32_t, size_t);

uint32_t Buffer_getArr(Buffer, char*, uint32_t, size_t);

#endif // _BUFFER_H_
