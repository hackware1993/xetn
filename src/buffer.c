#include "buffer.h"

#include <string.h>

Buffer Buffer_new(uint32_t size) {
	uint32_t size_total = size + sizeof(buffer_t);
	Buffer res = (Buffer)malloc(size_total);
	//res->ptr = (char*)(res + 1);
	res->pos = 0;
	res->end = 0;
	res->lim = size;
	res->cap = size;
	return res;
}

uint32_t Buffer_putArr(Buffer buf, char* arr, uint32_t off, size_t len) {
	size_t rest = buf->lim - buf->end;
	len = (len > rest) ? rest : len;

	size_t res = len;
	strncpy(buf->ptr + buf->end, arr + off, len);
	buf->end += len;
	return res;
}

uint32_t Buffer_getArr(Buffer buf, char* arr, uint32_t off, size_t len) {
	size_t rest = buf->end - buf->pos;
	len = (len > rest) ? rest : len;

	size_t res = len;
	strncpy(arr + off, buf->ptr + buf->pos, len);
	buf->pos += len;
	return res;
}
