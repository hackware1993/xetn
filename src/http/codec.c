#include <stdio.h>

#include <x86intrin.h>
#include <assert.h>
#include <string.h>
#include "../optimize.h"
#include "codec.h"
#include "datamap.h"

#define PRIVATE static

#define SP   ' '
#define COL  ':'
#define CR   '\r'
#define LF   '\n'
#define CRLF "\r\n"
#define SEP  ": "

#define HASH(hash, ch) \
	(hash) = ((hash) << 7) + ((hash) << 1) + (hash) + (ch)

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

PRIVATE inline void MemBlock_putStrN(MemBlock m, const char* str, size_t l) {
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

http_header_t HttpHeader_find(uint32_t index, uint32_t hash) {
	uint32_t i = HEADER_HASH[index];
	if((i ^ hash) == 0) {
		return HEADER_INDEX[index];
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
	REQL_METHOD, REQL_PATH, REQL_VER, REQL_EOL,
};

enum {
	RESL_VER, RESL_ST, RESL_DESC, RESL_EOL,
};

enum {
	FLD_INIT,
	FLD_KEY, FLD_SEP, FLD_VAL, FLD_EOL,
	FLD_DONE,
};

HttpCodec HttpCodec_init(HttpCodec codec, HttpConnection conn) {
	codec->state = STATE_INIT;
	codec->phaseHandler = NULL;
	codec->conn  = conn;
	codec->hash  = 0;
	codec->is_ext = 0;
	MemBlock_init(&codec->temp, 1024);
	/* keep 0 as the default index which represents NULL */
	codec->temp.len = 1;
	codec->temp.last = 1;
	return codec;
}

void HttpCodec_close(HttpCodec codec) {
	MemBlock_free(&codec->temp);
}

PRIVATE inline int8_t find_method(int32_t hash) {
	HashPair p = (HashPair)(METHOD_HASH + hash % 14);
	if((p->hash ^ hash) == 0) {
		return p->id;
	}
	return -1;
}
PRIVATE inline int8_t find_version(int32_t hash) {
	HashPair p = (HashPair)(VERSION_HASH + hash % 4);
	if((p->hash ^ hash) == 0) {
		return p->id;
	}
	return -1;
}
PRIVATE inline http_header_t find_header(uint32_t hash) {
	uint32_t index = hash % MAGNUM;
	if((HEADER_HASH[index] ^ hash) == 0) {
		return HEADER_INDEX[index];
	}
	return HH_INVALID;
}

int8_t decodeInitPhase     (HttpCodec, char*, uint32_t*, uint32_t);
int8_t decodeReqStatusPhase(HttpCodec, char*, uint32_t*, uint32_t);
int8_t decodeResStatusPhase(HttpCodec, char*, uint32_t*, uint32_t);
int8_t decodeFieldPhase    (HttpCodec, char*, uint32_t*, uint32_t);

int8_t encodeInitPhase     (HttpCodec, char*, uint32_t*, uint32_t);
int8_t encodeReqStatusPhase(HttpCodec, char*, uint32_t*, uint32_t);
int8_t encodeResStatusPhase(HttpCodec, char*, uint32_t*, uint32_t);
int8_t encodeFieldPhase    (HttpCodec, char*, uint32_t*, uint32_t);

int8_t encodeInitPhase(HttpCodec encoder,
		char* buf, uint32_t* pos, uint32_t len) {
	encoder->cursor = 0;
	encoder->step = 0;
	if(encoder->conn->type ^ HTTP_RES) {
		encoder->phaseHandler = encodeReqStatusPhase;
	} else {
		encoder->phaseHandler = encodeResStatusPhase;
	}
}
int8_t encodeReqStatusPhase(HttpCodec encoder,
		char* buf, uint32_t* pos, uint32_t len) {
	HttpConnection conn = encoder->conn;
	MemBlock       temp = &encoder->temp;
	uint32_t pindex = *pos;
	uint32_t nleft;
	uint8_t  step = encoder->step;
	uint32_t ret;
	const char* str;
	uint32_t    slen;

	switch(step) {
		case REQL_METHOD: goto ENTRY_METHOD;
		case REQL_PATH:   goto ENTRY_PATH;
		case REQL_VER:    goto ENTRY_VER;
		case REQL_EOL:    goto ENTRY_EOL;
	}
ENTRY_METHOD:
	str = METHOD_NAME[conn->code];
	slen = strlen(str);
	nleft = (len - pindex >= slen) ? slen : len - pindex;
	strncpy(buf + pindex, str, nleft);
	pindex += nleft;
	//ret = Buffer_putArr(buf, (void*)str, 0, slen);
	if(nleft ^ slen) {
		MemBlock_putStrN(temp, str + nleft, slen - nleft);
		//step = REQL_PATH;
		//goto PEND;
	}
	if(LIKELY(pindex ^ len)) {
		buf[pindex++] = SP;
		//Buffer_put(buf, SP);
	} else {
		MemBlock_putChar(temp, SP);
		step = REQL_PATH;
		goto EXIT_PEND;
	}
ENTRY_PATH:
	str = (const char*)(conn->data + conn->str);
	slen = strlen(str);
	nleft = (len - pindex >= slen) ? slen : len - pindex;
	strncpy(buf + pindex, str, nleft);
	pindex += nleft;
	//ret = Buffer_putArr(buf, (void*)str, 0, slen);
	if(nleft ^ slen) {
		MemBlock_putStrN(temp, str + nleft, slen - nleft);
		//step = REQL_VER;
		//goto PEND;
	}
	if(LIKELY(pindex ^ len)) {
		buf[pindex++] = SP;
		//Buffer_put(buf, SP);
	} else {
		MemBlock_putChar(temp, SP);
		step = REQL_VER;
		goto EXIT_PEND;
	}
ENTRY_VER:
	str = VERSION_NAME[conn->ver];
	//ret = Buffer_putArr(buf, (void*)str, 0, 8);
	nleft = (len - pindex >= 8) ? 8 : len - pindex;
	strncpy(buf + pindex, str, nleft);
	pindex += nleft;
	if(nleft ^ 8) {
		MemBlock_putStrN(temp, str + nleft, 8 - nleft);
		step = REQL_EOL;
		goto EXIT_PEND;
	}
ENTRY_EOL:
	//ret = Buffer_putArr(buf, CRLF, 0, 2);
	nleft = (len - pindex >= 2) ? 2 : len - pindex;
	strncpy(buf + pindex, CRLF, nleft);
	pindex += nleft;
	if(nleft ^ 2) {
		MemBlock_putStrN(temp, CRLF + nleft, 2 - nleft);
	}
EXIT_DONE:
	encoder->step = 0;
	*pos = pindex;
	encoder->phaseHandler = encodeFieldPhase;
	if(temp->len) {
		return EXIT_PEND;
	}
	return EXIT_DONE;
EXIT_PEND:
	encoder->step = step;
	*pos = pindex;
	return EXIT_PEND;
EXIT_ERROR:
	return EXIT_ERROR;
}

int8_t encodeResStatusPhase(HttpCodec encoder,
		char* buf, uint32_t* pos, uint32_t len) {
	HttpConnection conn = encoder->conn;
	MemBlock temp = &encoder->temp;
	uint32_t pindex = *pos;
	uint32_t nleft;
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
	//ret = Buffer_putArr(dest, (void*)str, 0, 8);
	nleft = (len - pindex >= 8) ? 8 : len - pindex;
	strncpy(buf + pindex, str, nleft);
	pindex += nleft;
	if(nleft ^ 8) {
		MemBlock_putStrN(temp, str + nleft, 8 - nleft);
		//step = RESL_ST;
		//goto PEND;
	}
	if(LIKELY(pindex ^ len)) {
		buf[pindex++] = SP;
		//Buffer_put(dest, SP);
	} else {
		MemBlock_putChar(temp, SP);
		step = RESL_ST;
		goto EXIT_PEND;
	}
ENTRY_ST:
	str = STATUS_DESC[conn->code];
	slen = strlen(str);
	//ret = Buffer_putArr(dest, (void*)str, 0, slen);
	nleft = (len - pindex >= slen) ? slen : len - pindex;
	strncpy(buf + pindex, str, nleft);
	pindex += nleft;
	if(nleft ^ slen) {
		MemBlock_putStrN(temp, str + nleft, slen - nleft);
		step = RESL_EOL;
		goto EXIT_PEND;
	}
ENTRY_EOL:
	nleft = (len - pindex >= 2) ? 2 : len - pindex;
	strncpy(buf + pindex, str, nleft);
	pindex += nleft;
	//ret = Buffer_putArr(dest, CRLF, 0, 2);
	if(nleft ^ 2) {
		MemBlock_putStrN(temp, CRLF + nleft, 2 - nleft);
	}
EXIT_DONE:
	encoder->step = 0;
	*pos = pindex;
	encoder->phaseHandler = encodeFieldPhase;
	if(temp->len) {
		return EXIT_PEND;
	}
	return EXIT_DONE;
EXIT_PEND:
	encoder->step = step;
	*pos = pindex;
	return EXIT_PEND;
EXIT_ERROR:
	return EXIT_ERROR;
}

int8_t encodeFieldPhase(HttpCodec encoder,
		char* buf, uint32_t* pos, uint32_t len) {
	HttpConnection conn = encoder->conn;
	MemBlock temp = &encoder->temp;
	uint32_t pindex = *pos;
	uint8_t step = encoder->step;
	uint8_t cursor = encoder->cursor;
	uint8_t is_ext = encoder->is_ext;
	/* temporary variable */
	uint32_t nleft;
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
	// TODO simplify here
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
		pkey = HEADER_NAME[cursor];
		klen = HEADER_LEN[cursor];
	} else {
		ExtraField ef = (ExtraField)(data + fields[cursor]);
		pkey = (const char*)(data + ef->key);
		klen = strlen(pkey);
	}
	nleft = (len - pindex >= klen) ? klen : len - pindex;
	strncpy(buf + pindex, pkey, nleft);
	pindex += nleft;
	//ret = Buffer_putArr(dest, (void*)pkey, 0, klen);
	if(nleft ^ klen) {
		MemBlock_putStrN(temp, pkey + nleft, klen - nleft);
		step = FLD_SEP;
		goto EXIT_PEND;
	}
ENTRY_SEP:
	nleft = (len - pindex >= 2) ? 2 : len - pindex;
	strncpy(buf + pindex, SEP, nleft);
	pindex += nleft;
	//ret = Buffer_putArr(dest, SEP, 0, 2);
	if(nleft ^ 2) {
		MemBlock_putStrN(temp, SEP + nleft, 2 - nleft);
		step = FLD_VAL;
		goto EXIT_PEND;
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
	nleft = (len - pindex >= vlen) ? vlen : len - pindex;
	strncpy(buf + pindex, pval, nleft);
	pindex += nleft;
	//ret = Buffer_putArr(dest, (void*)pval, 0, vlen);
	if(nleft ^ vlen) {
		MemBlock_putStrN(temp, pval + nleft, vlen - nleft);
		step = FLD_EOL;
		goto EXIT_PEND;
	}
ENTRY_EOL:
	++cursor;
	nleft = (len - pindex >= 2) ? 2 : len - pindex;
	strncpy(buf + pindex, CRLF, nleft);
	pindex += nleft;
	//ret = Buffer_putArr(dest, CRLF, 0, 2);
	if(nleft ^ 2) {
		MemBlock_putStrN(temp, CRLF + nleft, 2 - nleft);
		step = FLD_INIT;
		goto EXIT_PEND;
	}
	goto ENTRY_INIT;
ENTRY_DONE:
	nleft = (len - pindex >= 2) ? 2 : len - pindex;
	strncpy(buf + pindex, CRLF, nleft);
	pindex += nleft;
	//ret = Buffer_putArr(dest, CRLF, 0, 2);
	if(nleft ^ 2) {
		MemBlock_putStrN(temp, CRLF + nleft, 2 - nleft);
		goto EXIT_DONE;
	}
EXIT_DONE:
	encoder->step = 0;
	*pos = pindex;
	encoder->phaseHandler = NULL;
	if(temp->len) {
		return EXIT_PEND;
	}
	return EXIT_DONE;
EXIT_PEND:
	encoder->step = step;
	encoder->cursor = cursor;
	encoder->is_ext = is_ext;
	*pos = pindex;
	return EXIT_PEND;
EXIT_ERROR:
	return EXIT_ERROR;
}

int8_t HttpCodec_encode(HttpCodec encoder, char* buf, uint32_t* len) {
	uint32_t pos = 0;
	int8_t   ret;

	switch(encoder->state) {
		case STATE_INIT:
			encoder->phaseHandler = encodeInitPhase;
			encoder->state = STATE_DOING;
		case STATE_DOING:
			if(encoder->temp.len) {
				MemBlock temp = &encoder->temp;
				uint32_t off = temp->last;
				uint32_t nleft = temp->len - off;
				if(*len >= nleft) {
					strncpy(buf, temp->ptr + off, nleft);
					temp->len = temp->last = 0;
					pos = nleft;
				} else {
					strncpy(buf, temp->ptr + off, *len);
					temp->last += *len;
					return EXIT_PEND;
				}
			}
			while(encoder->phaseHandler) {
				switch(ret = encoder->phaseHandler(encoder, buf, &pos, *len)) {
					case EXIT_ERROR:
						encoder->state = STATE_ERROR;
					case EXIT_PEND:
						return ret;
				}
			}
			*len = pos;
			encoder->state = STATE_DONE;
			return EXIT_DONE;
		case STATE_DONE:  return EXIT_DONE;
		case STATE_ERROR: return EXIT_ERROR;
	}
	return EXIT_ERROR;
}
int8_t decodeInitPhase(HttpCodec decoder,
		char* buf, uint32_t* pos, uint32_t len) {
	decoder->cursor = HTTP_HEADER_NUM;
	decoder->step = 0;
	if(decoder->conn->type ^ HTTP_RES) {
		decoder->phaseHandler = decodeReqStatusPhase;
	} else {
		decoder->phaseHandler = decodeResStatusPhase;
	}
	return EXIT_DONE;
}

int8_t decodeReqStatusPhase(HttpCodec decoder,
		char* buf, uint32_t* pos, uint32_t len) {
	/* local variables for data inside HttpEncoder */
	HttpConnection req = decoder->conn;
	MemBlock temp = &decoder->temp;
	uint32_t pindex = *pos;
	/* should be written when PEND */
	int32_t  hash = decoder->hash;
	uint8_t  step = decoder->step;
	/* temporary variables */
	int8_t   ret;
	char     ch;
	
	/* state entry navigation */
	switch(step) {
		case REQL_METHOD: goto ENTRY_METHOD;
		case REQL_PATH:   goto ENTRY_PATH;
		case REQL_VER:    goto ENTRY_VER;
		case REQL_EOL:    goto ENTRY_EOL;
	}
ENTRY_METHOD:
	while(pindex ^ len) {
		ch = buf[pindex++];
		if(LIKELY(ch != SP)) {
			HASH(hash, ch);
		} else {
			step = REQL_PATH;
			hash &= 0x7FFFFFFF;
			ret = find_method(hash);
			if(ret == -1) {
				assert(0);
				goto EXIT_ERROR;
			}
			req->code = ret;
			hash = 0;
			goto ENTRY_PATH;
		}
	}
	goto EXIT_PEND;
ENTRY_PATH:
	while(pindex ^ len) {
		ch = buf[pindex++];
		if(LIKELY(URL_TOKEN_MAP[ch])) {
			MemBlock_putChar(temp, ch);
		} else if(ch == SP) {
			step = REQL_VER;
			req->str = MemBlock_getStr(temp);
			goto ENTRY_VER;
		} else {
			assert(0);
			goto EXIT_ERROR;
		}
	}
	goto EXIT_PEND;
ENTRY_VER:
	while(pindex ^ len) {
		ch = buf[pindex++];
		if(LIKELY(ch != CR)) {
			HASH(hash, ch);
		} else {
			step = REQL_EOL;
			hash &= 0x7FFFFFFF;
			ret = find_version(hash);
			if(ret == -1) {
				assert(0);
				goto EXIT_ERROR;
			}
			req->ver = ret;
			hash = 0;
			goto ENTRY_EOL;
		}
	}
	goto EXIT_PEND;
ENTRY_EOL:
	if(UNLIKELY(!(pindex ^ len))) {
		goto EXIT_PEND;
	}
	ch = buf[pindex++];
	if(ch ^ LF) {
		assert(0);
		goto EXIT_ERROR;
	}
EXIT_DONE:
	decoder->step = 0;
	decoder->hash = 0;
	decoder->phaseHandler = decodeFieldPhase;
	*pos = pindex;
	return EXIT_DONE;
EXIT_PEND:
	decoder->step = step;
	decoder->hash = hash;
	*pos = pindex;
	return EXIT_PEND;
EXIT_ERROR:
	return EXIT_ERROR;
}

int8_t decodeResStatusPhase(HttpCodec decoder,
		char* buf, uint32_t* pos, uint32_t len) {
	/* local variables for data inside HttpEncoder */
	HttpConnection res = decoder->conn;
	MemBlock temp = &decoder->temp;
	uint32_t pindex = *pos;
	/* should be written when PEND */
	int32_t  hash = decoder->hash;
	uint8_t  step = decoder->step;
	/* temporary variables */
	int16_t  ret;
	char     ch;
	uint32_t begin, end;
	
	/* state entry navigation */
	switch(step) {
		case RESL_VER:  goto ENTRY_VER;
		case RESL_ST:   goto ENTRY_ST;
		case RESL_DESC: goto ENTRY_DESC;
		case RESL_EOL:  goto ENTRY_EOL;
	}
ENTRY_VER:
	while(pindex ^ len) {
		ch = buf[pindex++];
		if(LIKELY(ch ^ SP)) {
			HASH(hash, ch);
		} else {
			hash &= 0x7FFFFFFF;
			ret = find_version(hash);
			if(ret == -1) {
				assert(0);
				goto EXIT_ERROR;
			}
			res->ver = ret;
			step = RESL_ST;
			goto ENTRY_ST;
		}
	}
	goto EXIT_PEND;
ENTRY_ST:
	/* let atoi check the validity of status code */
	while(pindex ^ len) {
		ch = buf[pindex++];
		if(ch ^ SP) {
			MemBlock_putChar(temp, ch);
		} else {
			MemBlock_putChar(temp, '\0');
			ret = atoi(temp->ptr + temp->last);
			ret = HttpStatus_find(ret);
			if(ret == ST_INVALID) {
				assert(0);
				goto EXIT_ERROR;
			}
			MemBlock_rollback(temp);
			res->code = ret;
			step = RESL_DESC;
			goto ENTRY_DESC;
		}
	}
	goto EXIT_PEND;
ENTRY_DESC:
	/* skip all description of status code */
	while(pindex ^ len) {
		ch = buf[pindex++];
		if(ch == CR) {
			step = RESL_EOL;
			goto ENTRY_EOL;
		}
	}
	goto EXIT_PEND;
ENTRY_EOL:
	if(UNLIKELY(!(pindex ^ len))) {
		goto EXIT_PEND;
	}
	ch = buf[pindex++];
	if(ch ^ LF) {
		assert(0);
		goto EXIT_ERROR;
	}
EXIT_DONE:
	decoder->step = 0;
	decoder->hash = 0;
	decoder->phaseHandler = decodeFieldPhase;
	*pos = pindex;
	return EXIT_DONE;
EXIT_PEND:
	decoder->step = step;
	decoder->hash = hash;
	*pos = pindex;
	return EXIT_PEND;
EXIT_ERROR:
	return EXIT_ERROR;
}

int8_t decodeFieldPhase(HttpCodec decoder,
		char* buf, uint32_t* pos, uint32_t len) {
	/* local variable for data inside HttpDecoder */
	HttpConnection conn = decoder->conn;
	MemBlock temp = &decoder->temp;
	uint32_t pindex = *pos;
	/* variables below should be write when PEND */
	uint32_t hash = decoder->hash;
	uint8_t  step = decoder->step;
	uint32_t fld  = decoder->fld;
	uint8_t  is_ext = decoder->is_ext;
	uint8_t  cursor = decoder->cursor;
	/* temporary variable */
	char     ch;
	uint32_t begin, end;

	/* state entry navigation */
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
	if(UNLIKELY(!(pindex ^ len))) {
		goto EXIT_PEND;
	}
	ch = buf[pindex++];
	if(LIKELY(KEY_TOKEN_MAP[ch])) {
		HASH(hash, ch);
		MemBlock_putChar(temp, ch);
		step = FLD_KEY;
		goto ENTRY_KEY;
	} else if(ch == CR) {
		step = FLD_DONE;
		goto ENTRY_DONE;
	} else {
		assert(0);
		goto EXIT_ERROR;
	}
ENTRY_KEY:
	begin = pindex;
	while(pindex ^ len) {
		ch = buf[pindex++];
		if(LIKELY(KEY_TOKEN_MAP[ch])) {
			HASH(hash, ch);
		} else if(ch == COL) {
			end = pindex - 1;
			MemBlock_putStrN(temp, buf + begin, end - begin);
			step = FLD_SEP;
			goto ENTRY_SEP;
		} else {
			assert(0);
			goto EXIT_ERROR;
		}
	}
	end = pindex;
	MemBlock_putStrN(temp, buf + begin, end - begin);
	goto EXIT_PEND;
ENTRY_SEP:
	if(UNLIKELY(!(pindex ^ len))) {
		goto EXIT_PEND;
	}
	ch = buf[pindex++];
	if(LIKELY(ch == SP)) {
		fld = find_header(hash);
		if(LIKELY(fld ^ HH_INVALID)) {
			MemBlock_rollback(temp);
		} else {
			fld = MemBlock_getStr(temp);
			/* the header belongs to extra header */
			is_ext = 1;
		}
		step = FLD_VAL;
		goto ENTRY_VAL;
	} else {
		assert(0);
		goto EXIT_ERROR;
	}
ENTRY_VAL:
	begin = pindex;
	while(pindex ^ len) {
		ch = buf[pindex++];
		if(LIKELY(VAL_TOKEN_MAP[ch])) {
			//MemBlock_putChar(temp, ch);
		} else if(ch == CR) {
			end = pindex - 1;
			MemBlock_putStrN(temp, buf + begin, end - begin);
			step = FLD_EOL;
			goto ENTRY_EOL;
		} else {
			assert(0);
			goto EXIT_ERROR;
		}
	}
	end = pindex;
	MemBlock_putStrN(temp, buf + begin, end - begin);
	goto EXIT_PEND;

ENTRY_EOL:
	if(UNLIKELY(!(pindex ^ len))) {
		goto EXIT_PEND;
	}
	ch = buf[pindex++];
	if(LIKELY(ch == LF)) {
		step = FLD_INIT;
		uint32_t v = MemBlock_getStr(temp);
		/* save the current line of http header field */
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
		goto EXIT_ERROR;
	}
ENTRY_DONE:
	if(UNLIKELY(!(pindex ^ len))) {
		goto EXIT_PEND;
	}
	ch = buf[pindex++];
	if(ch ^ LF) {
		assert(0);
		goto EXIT_ERROR;
	}
EXIT_DONE:
	decoder->step = 0;
	decoder->hash = 0;
	decoder->phaseHandler = NULL;
	*pos = pindex;
	return EXIT_DONE;
EXIT_PEND:
	decoder->is_ext = is_ext;
	decoder->cursor = cursor;
	decoder->fld = fld;
	decoder->step = step;
	decoder->hash = hash;
	*pos = pindex;
	return EXIT_PEND;
EXIT_ERROR:
	return EXIT_ERROR;
}

int8_t HttpCodec_decode(HttpCodec decoder, char* buf, uint32_t len) {
	uint32_t pos = 0;
	int8_t   ret;
	switch(decoder->state) {
		case STATE_INIT:
			decoder->phaseHandler = decodeInitPhase;
			decoder->state = STATE_DOING;
		case STATE_DOING:
			while(decoder->phaseHandler) {
				switch(ret = decoder->phaseHandler(decoder, buf, &pos, len)) {
					case EXIT_ERROR:
						decoder->state = STATE_ERROR;
					case EXIT_PEND:
						return ret;
				}
			}
			decoder->conn->data = decoder->temp.ptr;
			decoder->state = STATE_DONE;
			return EXIT_DONE;
		case STATE_DONE:  return EXIT_DONE;
		case STATE_ERROR: return EXIT_ERROR;
	}
	return EXIT_ERROR;
}

