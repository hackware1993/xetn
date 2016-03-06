#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct buffer {
	unsigned pos;
	unsigned end;
	unsigned lim;
	unsigned cap;
	char     ptr[0];
} buffer_t, *Buffer;


#define Buffer_free(buf) free((buf))

#define Buffer_getPtr(buf) (buf)->ptr

#define Buffer_getCap(buf) (buf)->cap

#define Buffer_getPos(buf)    (buf)->pos
#define Buffer_setPos(buf, p) (buf)->pos = (p)

#define Buffer_getLen(buf)    (buf)->end
#define Buffer_setLen(buf, l) (buf)->end = (l)

#define Buffer_getLim(buf)    (buf)->lim
#define Buffer_setLim(buf, l) (buf)->lim = (l)

#define Buffer_isEmpty(buf) ((buf)->end == 0)

#define Buffer_isEnd(buf) ((buf)->pos == (buf)->end)

#define Buffer_isFull(buf) ((buf)->end == (buf)->lim)

#define Buffer_clear(buf) \
	(buf)->pos = 0;       \
	(buf)->end = 0;       \
	(buf)->lim = (buf)->cap

#define Buffer_rewind(buf) (buf)->pos = 0

#define Buffer_put(buf, c) (buf)->ptr[(buf)->end++] = (c)
#define Buffer_get(buf)    (buf)->ptr[(buf)->pos++]

#define Buffer_getByIndex(buf, i)    (buf)->ptr[(i)]
#define Buffer_setByIndex(buf, i, v) (buf)->ptr[(i)] = (v)

static inline Buffer Buffer_new(uint32_t size) {
	uint32_t size_total = size + sizeof(buffer_t);
	Buffer res = (Buffer)malloc(size_total);
	res->pos = res->end = 0;
	res->lim = res->cap = size;
	return res;
}

static inline uint32_t Buffer_putArr(Buffer buf, void* arr, uint32_t off, size_t len) {
	size_t rest = buf->lim - buf->end;
	len = (len > rest) ? rest : len;
	size_t res = len;
	memcpy(buf->ptr + buf->end, arr + off, len);
	buf->end += len;
	return res;
}

static inline uint32_t Buffer_getArr(Buffer buf, void* arr, uint32_t off, size_t len) {
	size_t rest = buf->end - buf->pos;
	len = (len > rest) ? rest : len;
	size_t res = len;
	memcpy(arr + off, buf->ptr + buf->pos, len);
	buf->pos += len;
	return res;
}

static inline char* Buffer_getStr(Buffer buf) {
	int len = buf->end - buf->pos;
	if(len == 0) {
		return NULL;
	}
	char* res = (char*)malloc(len + 1);
	memcpy(res, buf->ptr, len);
	res[len] = '\0';
	return res;
}

#endif // _BUFFER_H_
