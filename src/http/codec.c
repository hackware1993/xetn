#include <stdio.h>

#include <x86intrin.h>
#include <assert.h>
#include <string.h>
#include "../optimize.h"
#include "codec.h"
#include "datamap.h"

#define PRIVATE static

#define SP  ' '
#define COL ':'
#define CR  '\r'
#define LF  '\n'
#define CRLF "\r\n"
#define SEP  ": "

#define HASH(hash, ch) \
	(hash) = ((hash) << 7) + ((hash) << 1) + (hash) + (ch)

#define MAGNUM 237

#define MemBlock_free(m) free((m)->ptr)
#define MemBlock_rollback(m) (m)->len = (m)->last

/* memory block is a special data structure */
/* it can expand automatically when needed */
PRIVATE inline MemBlock MemBlock_init(MemBlock m, uint32_t s) {
	m->ptr = malloc(s);
	m->len = 0;
	m->last = 0;
	m->size = s;
}

PRIVATE inline void MemBlock_putChar(MemBlock m, char ch) {
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

PRIVATE inline void MemBlock_putStrn(MemBlock m, const char* str, size_t l) {
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

PRIVATE inline void MemBlock_putInt4(MemBlock m, uint32_t n) {
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

PRIVATE inline uint32_t MemBlock_getStr(MemBlock m) {
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

PRIVATE inline uint32_t MemBlock_getStruct(MemBlock m) {
	uint32_t res = m->last;
	m->last = m->len;
	return res;
}

typedef struct hash_pair {
	int32_t hash;
	int32_t id;
} hash_pair_t, *HashPair;

PRIVATE inline int32_t BKDRHash(const char* str) {
	unsigned hash = 0;
	while(*str) {
		hash = (hash << 7) + (hash << 1) + hash + *str++;
	}
	return hash & 0x7FFFFFFF;
}

http_header_t HttpHeader_find(uint32_t index, uint32_t hash) {
	uint64_t* i = HEADER_HASHS + index;
	while(*i) {
		HashPair p = (HashPair)i;
		if((p->hash ^ hash) == 0) {
			return p->id;
		}
		++i;
	}
	return HH_INVALID;
}

http_status_t HttpStatus_find(int32_t num) {
	http_status_t res = ST_INVALID;
	switch(num) {
		case 100: res = ST_100; break;
		case 101: res = ST_101; break;
		case 200: res = ST_200; break;
		case 201: res = ST_201; break;
		case 202: res = ST_202; break;
		case 203: res = ST_203; break;
		case 204: res = ST_204; break;
		case 205: res = ST_205; break;
		case 206: res = ST_206; break;
		case 300: res = ST_300; break;
		case 301: res = ST_301; break;
		case 302: res = ST_302; break;
		case 303: res = ST_303; break;
		case 304: res = ST_304; break;
		case 305: res = ST_305; break;
		case 306: res = ST_306; break;
		case 307: res = ST_307; break;
		case 400: res = ST_400; break;
		case 401: res = ST_401; break;
		case 402: res = ST_402; break;
		case 403: res = ST_403; break;
		case 404: res = ST_404; break;
		case 405: res = ST_405; break;
		case 406: res = ST_406; break;
		case 407: res = ST_407; break;
		case 408: res = ST_408; break;
		case 409: res = ST_409; break;
		case 410: res = ST_410; break;
		case 411: res = ST_411; break;
		case 412: res = ST_412; break;
		case 413: res = ST_413; break;
		case 414: res = ST_414; break;
		case 415: res = ST_415; break;
		case 416: res = ST_416; break;
		case 417: res = ST_417; break;
		case 500: res = ST_500; break;
		case 501: res = ST_501; break;
		case 502: res = ST_502; break;
		case 503: res = ST_503; break;
		case 504: res = ST_504; break;
		case 505: res = ST_505; break;
	}
	return res;
}
enum {
	LINE_METHOD, LINE_PATH, LINE_VER, LINE_EOL,
};

enum {
	RESL_VER, RESL_ST, RESL_EOL,
};

enum {
	FLD_INIT,
	FLD_KEY, FLD_SEP, FLD_VAL, FLD_EOL,
	FLD_DONE,
};

PRIVATE int8_t parse_req_line(HttpDecoder);
PRIVATE int8_t parse_res_line(HttpDecoder);
PRIVATE int8_t parse_fields(HttpDecoder);

HttpDecoder HttpDecoder_init(HttpDecoder decoder, HttpConnection conn, Buffer src) {
	decoder->phase = PHASE_INIT;
	decoder->conn  = conn;
	decoder->src   = src;
	decoder->hash  = 0;
	decoder->is_ext = 0;
	decoder->cursor = HTTP_HEADER_NUM;
	MemBlock_init(&decoder->temp, 1024);
	return decoder;
}

HttpEncoder HttpEncoder_init(HttpEncoder encoder, HttpConnection conn, Buffer dest) {
	encoder->phase = PHASE_INIT;
	encoder->conn = conn;
	encoder->dest = dest;
	encoder->step = 0;
	encoder->is_ext = 0;
	encoder->cursor = 0;
	MemBlock_init(&encoder->temp, 256);
	return encoder;
}

void HttpDecoder_close(HttpDecoder decoder) {
	MemBlock_free(&decoder->temp);
}


PRIVATE inline int8_t find_method(int32_t hash) {
	HashPair p = (HashPair)(METHOD_HASHS + hash % 14);
	if((p->hash ^ hash) == 0) {
		return p->id;
	}
	return -1;
}
PRIVATE inline int8_t find_version(int32_t hash) {
	HashPair p = (HashPair)(VERSION_HASHS + hash % 4);
	if((p->hash ^ hash) == 0) {
		return p->id;
	}
	return -1;
}
PRIVATE inline http_header_t find_header(int32_t hash) {
	uint64_t* i = HEADER_HASHS + hash % MAGNUM;
	while(*i) {
		HashPair p = (HashPair)i;
		if((p->hash ^ hash) == 0) {
			return p->id;
		}
		++i;
	}
	return HH_INVALID;
}

PRIVATE int8_t decode_req_line(HttpDecoder decoder) {
	HttpConnection req = decoder->conn;
	Buffer   buf  = decoder->src;
	MemBlock temp = &decoder->temp;
	int32_t  hash = decoder->hash;
	uint8_t  step = decoder->step;
	int8_t   ret;
	char     ch;
	
	/* state entry navigation */
	switch(step) {
		case LINE_METHOD: goto ENTRY_METHOD;
		case LINE_PATH:   goto ENTRY_PATH;
		case LINE_VER:    goto ENTRY_VER;
		case LINE_EOL:    goto ENTRY_EOL;
	}
ENTRY_METHOD:
	while(!Buffer_isEnd(buf)) {
		ch = Buffer_get(buf);
		if(LIKELY(ch != SP)) {
			HASH(hash, ch);
		} else {
			step = LINE_PATH;
			hash &= 0x7FFFFFFF;
			ret = find_method(hash);
			if(ret == -1) {
				assert(0);
				goto ERROR;
			}
			req->code = ret;
			hash = 0;
			goto ENTRY_PATH;
		}
	}
	goto PEND;
ENTRY_PATH:
	while(!Buffer_isEnd(buf)) {
		ch = Buffer_get(buf);
		if(LIKELY(URL_TOKEN_MAP[ch])) {
			MemBlock_putChar(temp, ch);
		} else if(ch == SP) {
			step = LINE_VER;
			req->str = MemBlock_getStr(temp);
			goto ENTRY_VER;
		} else {
			assert(0);
			goto ERROR;
		}
	}
	goto PEND;
ENTRY_VER:
	while(!Buffer_isEnd(buf)) {
		ch = Buffer_get(buf);
		if(LIKELY(ch != CR)) {
			HASH(hash, ch);
		} else {
			step = LINE_EOL;
			hash &= 0x7FFFFFFF;
			ret = find_version(hash);
			if(ret == -1) {
				assert(0);
				goto ERROR;
			}
			req->ver = ret;
			hash = 0;
			goto ENTRY_EOL;
		}
	}
	goto PEND;
ENTRY_EOL:
	if(UNLIKELY(Buffer_isEnd(buf))) {
		goto PEND;
	}
	ch = Buffer_get(buf);
	if(LIKELY(ch == LF)) {
		goto FIN;
	} else {
		assert(0);
		goto ERROR;
	}

PEND:
	decoder->step = step;
	decoder->hash = hash;
	return EXIT_PEND;
FIN:
	decoder->step = 0;
	decoder->hash = 0;
	return EXIT_DONE;
ERROR:
	return EXIT_ERROR;
}

PRIVATE int8_t decode_fields(HttpDecoder decoder) {
	HttpConnection conn = decoder->conn;
	Buffer   buf  = decoder->src;
	MemBlock temp = &decoder->temp;
	int32_t  hash = decoder->hash;
	uint8_t  step = decoder->step;
	uint32_t fld  = decoder->fld;
	uint8_t  is_ext = decoder->is_ext;
	uint8_t  cursor = decoder->cursor;
	char     ch;

	//uint32_t start;

	switch(step) {
		case FLD_INIT: goto ENTRY_INIT;
		case FLD_KEY:  goto ENTRY_KEY;
		case FLD_SEP:  goto ENTRY_SEP;
		case FLD_VAL:  goto ENTRY_VAL;
		case FLD_EOL:  goto ENTRY_EOL;
		case FLD_DONE: goto ENTRY_DONE;
	}
ENTRY_INIT:
	hash = 0;
	if(UNLIKELY(Buffer_isEnd(buf))) {
		goto PEND;
	}
	ch = Buffer_get(buf);
	if(ch == CR) {
		step = FLD_DONE;
		goto ENTRY_DONE;
	}
	if(LIKELY(KEY_TOKEN_MAP[ch])) {
		step = FLD_KEY;
		HASH(hash, ch);
		MemBlock_putChar(temp, ch);
		goto ENTRY_KEY;
	} else {
		assert(0);
		goto ERROR;
	}

ENTRY_KEY:
	while(!Buffer_isEnd(buf)) {
		ch = Buffer_get(buf);
		if(LIKELY(KEY_TOKEN_MAP[ch])) {
			HASH(hash, ch);
			MemBlock_putChar(temp, ch);
		} else if(ch == COL) {
			step = FLD_SEP;
			goto ENTRY_SEP;
		} else {
			assert(0);
			goto ERROR;
		}
	}
	goto PEND;

ENTRY_SEP:
	if(UNLIKELY(Buffer_isEnd(buf))) {
		goto PEND;
	}
	ch = Buffer_get(buf);
	if(LIKELY(ch == SP)) {
		step = FLD_VAL;
		hash &= 0x7FFFFFFF;
		fld = find_header(hash);
		if(LIKELY(fld ^ HH_INVALID)) {
			MemBlock_rollback(temp);
		} else {
			fld = MemBlock_getStr(temp);
			is_ext = 1;
		}
		goto ENTRY_VAL;
	} else {
		assert(0);
		goto ERROR;
	}
ENTRY_VAL:
	//start = Buffer_getPos(buf);
	while(!Buffer_isEnd(buf)) {
		ch = Buffer_get(buf);
		if(LIKELY(VAL_TOKEN_MAP[ch])) {
			MemBlock_putChar(temp, ch);
		} else if(ch == CR) {
			//MemBlock_putStrn(temp, Buffer_getPtr(buf) + start, Buffer_getPos(buf) - start - 1);
			step = FLD_EOL;
			goto ENTRY_EOL;
		} else {
			assert(0);
			goto ERROR;
		}
	}
	//MemBlock_putStrn(temp, Buffer_getPtr(buf) + start, Buffer_getPos(buf) - start);
	goto PEND;

ENTRY_EOL:
	if(UNLIKELY(Buffer_isEnd(buf))) {
		goto PEND;
	}
	ch = Buffer_get(buf);
	if(LIKELY(ch == LF)) {
		step = FLD_INIT;
		uint32_t v = MemBlock_getStr(temp);
		if(LIKELY(!is_ext)) {
			conn->fields[fld] = v;
		} else {
			MemBlock_putInt4(temp, hash);
			MemBlock_putInt4(temp, fld);
			MemBlock_putInt4(temp, v);
			conn->fields[cursor++] = MemBlock_getStruct(temp);
			is_ext = 0;
		}
		goto ENTRY_INIT;
	} else {
		assert(0);
		goto ERROR;
	}
ENTRY_DONE:
	if(UNLIKELY(Buffer_isEnd(buf))) {
		goto PEND;
	}
	ch = Buffer_get(buf);
	if(ch == LF) {
		goto FIN;
	} else {
		assert(0);
		goto ERROR;
	}
PEND:
	decoder->is_ext = is_ext;
	decoder->cursor = cursor;
	decoder->fld = fld;
	decoder->step = step;
	decoder->hash = hash;
	return EXIT_PEND;
FIN:
	decoder->step = 0;
	decoder->hash = 0;
	return EXIT_DONE;
ERROR:
	return EXIT_ERROR;
}

int8_t encode_req_line(HttpEncoder encoder) {
	HttpConnection conn = encoder->conn;
	Buffer   dest = encoder->dest;
	MemBlock temp = &encoder->temp;
	uint8_t  step = encoder->step;
	uint32_t ret;
	const char* str;
	uint32_t    slen;

DONE:
	encoder->step = 0;
	return EXIT_DONE;
PEND:
	encoder->step = step;
	return EXIT_PEND;
ERROR:
	return EXIT_ERROR;
}

int8_t encode_res_line(HttpEncoder encoder) {
	HttpConnection conn = encoder->conn;
	Buffer   dest = encoder->dest;
	MemBlock temp = &encoder->temp;
	uint8_t  step = encoder->step;
	uint32_t ret;
	const char* str;
	uint32_t    slen;

	switch(step) {
		case RESL_VER: goto ENTRY_VER;
		case RESL_ST:  goto ENTRY_ST;
		case RESL_EOL: goto ENTRY_EOL;
	}
ENTRY_VER:
	str = VERSION_NAME[conn->ver];
	ret = Buffer_putArr(dest, (void*)str, 0, 8);
	if(ret ^ 8) {
		MemBlock_putStrn(temp, str + ret, 8 - ret);
		step = RESL_ST;
		goto PEND;
	}
	if(!Buffer_isFull(dest)) {
		Buffer_put(dest, SP);
	} else {
		MemBlock_putChar(temp, SP);
		step = RESL_ST;
		goto PEND;
	}
ENTRY_ST:
	str = STATUS_DESC[conn->code];
	slen = strlen(str);
	ret = Buffer_putArr(dest, (void*)str, 0, slen);
	if(ret ^ slen) {
		MemBlock_putStrn(temp, str + ret, slen - ret);
		step = RESL_EOL;
		goto PEND;
	}
ENTRY_EOL:
	ret = Buffer_putArr(dest, CRLF, 0, 2);
	if(ret ^ 2) {
		MemBlock_putStrn(temp, CRLF + ret, 2 - ret);
	}
DONE:
	encoder->step = 0;
	return EXIT_DONE;
PEND:
	encoder->step = step;
	return EXIT_PEND;
ERROR:
	return EXIT_ERROR;
}

int8_t encode_fields(HttpEncoder encoder) {
	HttpConnection conn = encoder->conn;
	Buffer   dest = encoder->dest;
	MemBlock temp = &encoder->temp;

	uint8_t step = encoder->step;
	uint8_t cursor = encoder->cursor;
	uint8_t is_ext = encoder->is_ext;
	/* temporary variable */
	void*       data   = conn->data;
	uint32_t*   fields = conn->fields;
	const char* pkey, * pval;
	uint32_t    klen,   vlen;
	uint32_t    offset;
	uint32_t    ret;

	switch(step) {
		case FLD_INIT: goto ENTRY_INIT;
		case FLD_KEY:  goto ENTRY_KEY;
		case FLD_SEP:  goto ENTRY_SEP;
		case FLD_VAL:  goto ENTRY_VAL;
		case FLD_EOL:  goto ENTRY_EOL;
		case FLD_DONE: goto ENTRY_DONE;
	}
ENTRY_INIT:
	while(cursor < HEADER_MAX) {
		/* check is the is_ext flag should be set */
		if(!is_ext && cursor == HTTP_HEADER_NUM) {
			is_ext = 1;
		}
		if(fields[cursor]) {
			goto ENTRY_KEY;
		} else {
			if(is_ext) {
				break;
			}
			++cursor;
		}
	}
	/* cursor == HEADER_MAX OR fields[cursor] == 0 && is_ext */
	step = FLD_DONE;
	goto ENTRY_DONE;
ENTRY_KEY:
	if(LIKELY(!is_ext)) {
		pkey = HEADER_NAMES[cursor];
		klen = HEADER_LENS[cursor];
	} else {
		ExtraField ef = (ExtraField)(data + fields[cursor]);
		pkey = (const char*)(data + ef->key);
		klen = strlen(pkey);
	}
	ret = Buffer_putArr(dest, (void*)pkey, 0, klen);
	if(ret ^ klen) {
		MemBlock_putStrn(temp, pkey + ret, klen - ret);
		step = FLD_SEP;
		goto PEND;
	}
ENTRY_SEP:
	ret = Buffer_putArr(dest, SEP, 0, 2);
	if(ret ^ 2) {
		MemBlock_putStrn(temp, SEP + ret, 2 - ret);
		step = FLD_VAL;
		goto PEND;
	}
ENTRY_VAL:
	offset = fields[cursor];
	if(LIKELY(!is_ext)) {
		pval = (const char*)(data + offset);
	} else {
		ExtraField ef = (ExtraField)(data + offset);
		pval = (const char*)(data + ef->value);
	}
	vlen = strlen(pval);
	ret = Buffer_putArr(dest, (void*)pval, 0, vlen);
	if(ret ^ vlen) {
		MemBlock_putStrn(temp, pval + ret, vlen - ret);
		step = FLD_EOL;
		goto PEND;
	}
ENTRY_EOL:
	++cursor;
	ret = Buffer_putArr(dest, CRLF, 0, 2);
	if(ret ^ 2) {
		MemBlock_putStrn(temp, CRLF + ret, 2 - ret);
		step = FLD_INIT;
		goto PEND;
	}
	goto ENTRY_INIT;
ENTRY_DONE:
	ret = Buffer_putArr(dest, CRLF, 0, 2);
	if(ret ^ 2) {
		MemBlock_putStrn(temp, CRLF + ret, 2 - ret);
		goto DONE;
	}
DONE:
	encoder->step = 0;
	return EXIT_DONE;
PEND:
	encoder->step = step;
	encoder->cursor = cursor;
	encoder->is_ext = is_ext;
	return EXIT_PEND;
ERROR:
	return EXIT_ERROR;
}

int8_t HttpDecoder_decodeConnection(HttpDecoder decoder, hint_t hint) {
	int8_t ret;
	switch(decoder->phase) {
		case PHASE_INIT:   goto ENTRY_INIT;
		case PHASE_STATUS: goto ENTRY_STATUS;
		case PHASE_FIELD:  goto ENTRY_FIELD;
		case PHASE_DONE:   return EXIT_DONE;
	}
ENTRY_INIT:
	decoder->step = 0;
	decoder->phase = PHASE_STATUS;
ENTRY_STATUS:
	switch(hint) {
		case HTTP_REQ:
			ret = decode_req_line(decoder);
			break;
		case HTTP_RES:
			//ret = decode_res_line(decoder);
			break;
	}
	switch(ret) {
		case EXIT_ERROR: goto ERROR;
		case EXIT_PEND:  goto PEND;
		case EXIT_DONE:
			decoder->phase = PHASE_FIELD;
			goto ENTRY_FIELD;
	}
ENTRY_FIELD:
	ret = decode_fields(decoder);
	switch(ret) {
		case EXIT_ERROR: goto ERROR;
		case EXIT_PEND:  goto PEND;
		case EXIT_DONE:
			decoder->phase = PHASE_DONE;
			goto DONE;
	}
ERROR:
	return EXIT_ERROR;
PEND:
	return EXIT_PEND;
DONE:
	decoder->conn->data = decoder->temp.ptr;
	return EXIT_DONE;
}

int8_t HttpEncoder_encodeConnection(HttpEncoder encoder, hint_t hint) {
	Buffer   dest = encoder->dest;
	MemBlock temp = &encoder->temp;
	uint32_t len = temp->len;
	uint32_t last = temp->last;
	int32_t ret;

	/* check the temp zone every time calling this function */
	/* if there exist data inside temp, move it to dest buffer */
	if(len) {
		last += Buffer_putArr(dest, temp->ptr, last, len - last);
		temp->last = last;
		if(last ^ len) {
			goto PEND;
		} else {
			temp->len = 0;
			temp->last = 0;
		}
	}

	/* navigation */
	switch(encoder->phase) {
		case PHASE_INIT:
			goto ENTRY_INIT;
		case PHASE_STATUS:
			goto ENTRY_STATUS;
		case PHASE_FIELD:
			goto ENTRY_FIELD;
		case PHASE_DONE:
			return EXIT_DONE;
	}

ENTRY_INIT:
	encoder->step = 0;
	encoder->phase = PHASE_STATUS;
ENTRY_STATUS:
	switch(hint) {
		case HTTP_REQ:
			//ret = encode_req_line(encoder);
			break;
		case HTTP_RES:
			ret = encode_res_line(encoder);
			break;
	}
	if(ret == EXIT_DONE){
		encoder->phase = PHASE_FIELD;
		if(temp->len) {
			goto PEND;
		}
	} else {
		goto PEND;
	}
ENTRY_FIELD:
	ret = encode_fields(encoder);
	if(ret == EXIT_DONE){
		encoder->phase = PHASE_DONE;
		if(temp->len) {
			goto PEND;
		}
	} else {
		goto PEND;
	}
DONE:
	return EXIT_DONE;
PEND:
	return EXIT_PEND;
ERROR:
	return EXIT_ERROR;
}
