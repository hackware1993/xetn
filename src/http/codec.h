#ifndef _HTTP_PARSER_H_
#define _HTTP_PARSER_H_

#include <stdint.h>
#include "../memblock.h"
#include "connection.h"

typedef enum codec_state {
	STATE_INIT,
	STATE_DOING,
	STATE_DONE,
	STATE_ERROR,
} codec_state_t;

enum {
	EXIT_ERROR = -1,
	EXIT_PEND  = 0,
	EXIT_DONE  = 1,
};

struct http_codec;
typedef int8_t (*phase_cb_t)(struct http_codec*, char*, uint32_t*, uint32_t);
typedef struct http_codec {
	HttpConnection conn;
	codec_state_t  state;
	mem_block_t    temp;
	phase_cb_t     phaseHandler;
	uint8_t        step;
	uint8_t        is_ext;
	uint8_t        cursor;
	uint32_t       fld;
	uint32_t       hash;
} http_codec_t, *HttpCodec;

HttpCodec HttpCodec_init(HttpCodec, HttpConnection);
void HttpCodec_reset(HttpCodec);
void HttpCodec_close(HttpCodec);

/**
 * Return Value:
 *   EXIT_ERROR: -1
 *	 EXIT_PEND : 0
 *	 EXIT_DONE : 1
 */
int8_t HttpCodec_encode(HttpCodec, char*, uint32_t*);
int8_t HttpCodec_decode(HttpCodec, char*, uint32_t*);

#endif // _HTTP_PARSER_H_
