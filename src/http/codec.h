#ifndef _HTTP_PARSER_H_
#define _HTTP_PARSER_H_

#include <stdint.h>
#include "connection.h"
#include "../buffer.h"

enum {
	EXIT_ERROR = -1,
	EXIT_PEND  = 0,
	EXIT_DONE  = 1,
};

typedef enum phase {
	PHASE_INIT,
	PHASE_STATUS,
	PHASE_FIELD,
	PHASE_DONE,
} phase_t;

typedef struct mem_block {
	void*    ptr;
	uint32_t len;
	uint32_t last;
	uint32_t size;
} mem_block_t, *MemBlock;

typedef struct http_codec {
	HttpConnection conn;
	Buffer      buf;
	mem_block_t temp;
	phase_t     phase;
	uint8_t     step;
	uint8_t     is_ext;
	uint8_t     cursor;
	uint32_t    fld;
	int32_t     hash;
} http_codec_t, *HttpCodec;

HttpCodec HttpCodec_init(HttpCodec, HttpConnection, Buffer);
void HttpCodec_reset(HttpCodec);
void HttpCodec_close(HttpCodec);

typedef enum http_type {
	HTTP_REQ, HTTP_RES,
} http_type_t;

/**
 * Return Value:
 *   EXIT_ERROR: -1
 *	 EXIT_PEND : 0
 *	 EXIT_DONE : 1
 */
int8_t HttpCodec_decode(HttpCodec, http_type_t);
int8_t HttpCodec_encode(HttpCodec, http_type_t);

#endif // _HTTP_PARSER_H_
