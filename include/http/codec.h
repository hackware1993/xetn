#ifndef _HTTP_PARSER_H_
#define _HTTP_PARSER_H_

#include <stdint.h>
#include "memblock.h"
#include "connection.h"

enum {
	EXIT_ERROR = -1,
	EXIT_DONE  = 0,
	EXIT_PEND  = 1,
};

struct http_codec;
typedef int8_t (*phase_cb_t)(struct http_codec*, char*, uint32_t*, uint32_t);

typedef struct http_codec {
	HttpConnection conn;
	phase_cb_t     phaseHandler;
	/* for decoding */
	MemBlock       temp;
	uint32_t       fld;
	uint32_t       hash;
	/* for encoding */
	const char*    str;
	uint32_t       sindex;

	uint8_t        flag;
	uint8_t        cursor;
	/* main phase & sub phase */
	unsigned       state : 4;
	unsigned       step  : 4;
	//////////////
	uint32_t       sep;
} http_codec_t, *HttpCodec;

HttpCodec HttpCodec_init(HttpCodec, HttpConnection);
int8_t    HttpCodec_encode(HttpCodec, char*, uint32_t*);
int8_t    HttpCodec_decode(HttpCodec, char*, uint32_t*);

#endif // _HTTP_PARSER_H_
