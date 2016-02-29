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

typedef struct http_decoder {
	Buffer       src;
	mem_block_t  temp;
	HttpConnection conn;
	phase_t      phase;
	uint8_t      step;
	uint8_t      is_ext;
	uint8_t      cursor;
	uint32_t     fld;
	int32_t      hash;
} http_decoder_t, *HttpDecoder;

typedef struct http_encoder {
	Buffer       dest;
	mem_block_t  temp;
	HttpConnection conn;
	phase_t      phase;
	uint8_t      step;
	uint8_t      is_ext;
	uint8_t      cursor;

} http_encoder_t, *HttpEncoder;

HttpEncoder HttpEncoder_init(HttpEncoder, HttpConnection, Buffer);
HttpDecoder HttpDecoder_init(HttpDecoder, HttpConnection, Buffer);
void HttpEncoder_reset(HttpEncoder);
void HttpDecoder_reset(HttpDecoder);
void HttpEncoder_close(HttpEncoder);
void HttpDecoder_close(HttpDecoder);

typedef enum hint {
	HTTP_REQ, HTTP_RES,
} hint_t;

/**
 * Return Value:
 *   EXIT_ERROR: -1
 *	 EXIT_PEND : 0
 *	 EXIT_DONE : 1
 */
int8_t HttpDecoder_decodeConnection(HttpDecoder, hint_t);
int8_t HttpEncoder_encodeConnection(HttpEncoder, hint_t);

#endif // _HTTP_PARSER_H_
