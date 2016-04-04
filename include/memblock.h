#ifndef _MEMBLOCK_H_
#define _MEMBLOCK_H_

/**
 * A data structure used to build memory block.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct mem_block {
	void*    ptr;
	uint32_t len;
	uint32_t last;
	uint32_t size;
} mem_block_t, *MemBlock;

#define MemBlock_rollback(m) (m)->len = (m)->last

static inline MemBlock MemBlock_init(MemBlock m, uint32_t size) {
	m->ptr = malloc(size);
	m->len = 0;
	m->last = 0;
	m->size = size;
	return m;
}

static inline MemBlock MemBlock_bind(MemBlock m, void* data, uint32_t size) {
	m->ptr = data;
	m->len = 0;
	m->last = 0;
	m->size = size;
	return m;
}

static inline MemBlock MemBlock_clear(MemBlock m) {
	m->len = 0;
	m->last = 0;
	m->size = 0;
	m->ptr = NULL;
	return m;
}

static inline void MemBlock_free(MemBlock m) {
	if(m->ptr) {
		free((m)->ptr);
	}
}
static inline void MemBlock_putChar(MemBlock m, char ch) {
	uint32_t s   = m->size;
	uint32_t len = m->len;
	if(len >= s) {
		s <<= 1;
		m->ptr = realloc(m->ptr, s);
		m->size = s;
	}
	((char*)m->ptr)[len++] = ch;
	m->len = len;
}

static inline void MemBlock_putStrN(MemBlock m, const char* str, size_t l) {
	uint32_t s = m->size;
	uint32_t len = m->len;
	uint32_t n;
	if((n = len + l) >= s) {
		while(n >= s) {
			s <<= 1;
		}
		m->ptr = realloc(m->ptr, s);
		m->size = s;
	}
	memcpy(m->ptr + len, str, l);
	m->len = len + l;
}

static inline void MemBlock_putInt4(MemBlock m, uint32_t n) {
	uint32_t s = m->size;
	uint32_t len = m->len;
	if(len >= s) {
		s <<= 1;
		m->ptr = realloc(m->ptr, s);
		m->size = s;
	}
	*(uint32_t*)(m->ptr + len) = n;
	m->len = len + sizeof(n);
}

static inline uint32_t MemBlock_getStr(MemBlock m) {
	uint32_t res = m->last;
	uint32_t s = m->size;
	uint32_t len = m->len;
	if(len >= s) {
		m->ptr = realloc(m->ptr, s << 1);
	}
	((char*)m->ptr)[len++] = '\0';
	m->len = len;
	m->last = len;
	return res;
}

static inline uint32_t MemBlock_getStruct(MemBlock m) {
	uint32_t res = m->last;
	m->last = m->len;
	return res;
}

#endif //_MEMBLOCK_H_
