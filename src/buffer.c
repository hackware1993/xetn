#include "buffer.h"

#include <string.h>

buffer_t buffer_new(unsigned size) {
	unsigned size_total = size + sizeof(struct buffer);
	buffer_t res = (buffer_t)malloc(size_total);
	//res->ptr = (char*)(res + 1);
	res->pos = 0;
	res->end = 0;
	res->lim = size;
	res->cap = size;
	return res;
}

char* buffer_get_between(buffer_t buf, unsigned begin, unsigned len) {
	char* pstart = buf->ptr + begin;
	char* res = (char*)malloc(len);
	strncpy(res, pstart, len);
	return res;
}

unsigned buffer_put_arr(buffer_t buf, char* arr, unsigned off, unsigned len) {
	unsigned rest = buf->lim - buf->end;
	len = (len > rest) ? rest : len;

	unsigned res = len;
	strncpy(buf->ptr + buf->end, arr + off, len);
	buf->end += len;
	return res;
}

unsigned buffer_get_arr(buffer_t buf, char* arr, unsigned off, unsigned len) {
	unsigned rest = buf->end - buf->pos;
	len = (len > rest) ? rest : len;

	unsigned res = len;
	strncpy(arr + off, buf->ptr + buf->pos, len);
	buf->pos += len;
	return res;
}
