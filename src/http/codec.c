#include <stdio.h>

//#include <x86intrin.h>
#include <assert.h>
#include <string.h>
#include "../common.h"
#include "codec.h"
#include "datamap.h"

/* main state of http codec */
enum {
	STATE_INIT,
	STATE_DOING,
	STATE_DONE,
	STATE_ERROR,
};

#define CRLF "\r\n"
#define SEP  ": "

#define SP   ' '
#define COL  ':'
#define CR   '\r'
#define LF   '\n'
#define DOT  '.'
#define QS   '?'
#define SL   '/'

#define VER_LEN  8
#define CRLF_LEN 2
#define SEP_LEN  2
#define SP_LEN   1
#define ST_LEN   3

#define HASH(hash, ch) (hash) = ((hash) << 7) + ((hash) << 1) + (hash) + (ch)

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

/* Http Codec Flags */
enum {
	IS_EXT   = 0x02,
	IS_TRANS = 0x04,
};

/* internal phases */
enum {
	REQL_METHOD, REQL_SP1,
	REQL_PATH, REQL_PQ, REQL_QUERY, REQL_SP2,
	REQL_VER, REQL_EOL,
};
enum {
	RESL_VER, RESL_SP1,
	RESL_ST, RESL_SP2,
	RESL_DESC, RESL_EOL,
};
enum {
	FLD_INIT,
	FLD_KEY, FLD_SEP, FLD_VAL, FLD_EOL,
	FLD_DONE,
};

HttpCodec HttpCodec_init(HttpCodec codec, HttpConnection conn) {
	codec->state = STATE_INIT;
	codec->conn  = conn;
	codec->hash  = 0;
	codec->flag  = 0;
	return codec;
}

PRIVATE INLINE int8_t FindMethod(int32_t hash) {
	HashPair p = (HashPair)(METHOD_HASH + hash % 14);
	if((p->hash ^ hash) == 0) {
		return p->id;
	}
	return -1;
}
PRIVATE INLINE int8_t FindVersion(int32_t hash) {
	HashPair p = (HashPair)(VERSION_HASH + hash % 4);
	if((p->hash ^ hash) == 0) {
		return p->id;
	}
	return -1;
}
PRIVATE INLINE http_header_t FindHeader(uint32_t hash) {
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

int8_t encodeInitPhase(HttpCodec encoder, char* buf, uint32_t* pos, uint32_t len) {
	encoder->str = NULL;
	encoder->sindex = 0;
	encoder->cursor = 0;
	encoder->step = 0;
	if(encoder->conn->type ^ HTTP_RES) {
		encoder->phaseHandler = encodeReqStatusPhase;
	} else {
		encoder->phaseHandler = encodeResStatusPhase;
	}
	return EXIT_DONE;
}
int8_t encodeReqStatusPhase(HttpCodec encoder, char* buf, uint32_t* pos, uint32_t len) {
	HttpConnection conn = encoder->conn;
	uint32_t pindex = *pos;
	uint32_t nleft;
	uint8_t  step = encoder->step;
	uint32_t ret;
	const char* str = encoder->str;
	uint32_t    sindex = encoder->sindex;
	uint32_t    slen;

	switch(step) {
		case REQL_METHOD: goto ENTRY_METHOD;
		case REQL_SP1:    goto ENTRY_SP1;
		case REQL_PATH:   goto ENTRY_PATH;
		case REQL_PQ:     goto ENTRY_PQ;
		case REQL_QUERY:  goto ENTRY_QUERY;
		case REQL_SP2:    goto ENTRY_SP2;
		case REQL_VER:    goto ENTRY_VER;
		case REQL_EOL:    goto ENTRY_EOL;
	}
ENTRY_METHOD:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = METHOD_NAME[conn->code];
	}
	slen = strlen(str);
	goto ENTRY_ENC;
	/* @ASSERTION: str != NULL && slen == VER_LEN */
ENTRY_SP1:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = " ";
	}
	slen = SP_LEN;
	goto ENTRY_ENC;
	/* @ASSERTION: str != NULL && slen == VER_LEN */
ENTRY_PATH:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = (const char*)(conn->data.ptr + conn->path);
	}
	slen = strlen(str);
	goto ENTRY_ENC;
	/* @ASSERTION: str != NULL && slen == VER_LEN */
ENTRY_PQ:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = "?";
	}
	slen = strlen(str);
	goto ENTRY_ENC;
	/* @ASSERTION: str != NULL && slen == VER_LEN */
ENTRY_QUERY:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = (const char*)(conn->data.ptr + conn->query);
	}
	slen = strlen(str);
	goto ENTRY_ENC;
	/* @ASSERTION: str != NULL && slen == VER_LEN */
ENTRY_SP2:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = " ";
	}
	slen = SP_LEN;
	goto ENTRY_ENC;
	/* @ASSERTION: str != NULL && slen == VER_LEN */
ENTRY_VER:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = VERSION_NAME[conn->ver];
	}
	slen = VER_LEN;
	goto ENTRY_ENC;
	/* @ASSERTION: str != NULL && slen == VER_LEN */
ENTRY_EOL:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = CRLF;
	}
	slen = CRLF_LEN;
	goto ENTRY_ENC;
	/* @ASSERTION: str != NULL && slen == VER_LEN */
ENTRY_ENC:
	/* @ASSERTION: str != NULL && slen > 0 */
	// TODO consider if we need this
	if(pindex == len) {
		goto EXIT_PEND;
	}
	if((nleft = len - pindex) > slen - sindex) {
		nleft = slen - sindex;
	}
	/* @ASSERTION: nleft == MIN(len - pindex, slen - index) */
	strncpy(buf + pindex, str + sindex, nleft);
	pindex += nleft;
	sindex  += nleft;
	if(sindex ^ slen) {
		goto EXIT_PEND;
	}
	str = NULL;
	sindex = 0;
	/* @ASSERTION: slen == index && str = NULL */
ENTRY_AF_ENC:
	switch(step) {
		case REQL_METHOD: step = REQL_SP1;  goto ENTRY_SP1;
		case REQL_SP1:    step = REQL_PATH; goto ENTRY_PATH;
		case REQL_PATH:
			if(conn->query) {
				step = REQL_PQ;
				goto ENTRY_PQ;
			}
			step = REQL_SP2;
			goto ENTRY_SP2;
		case REQL_PQ:    step = REQL_QUERY; goto ENTRY_QUERY;
		case REQL_QUERY:  step = REQL_SP2;   goto ENTRY_SP2;
		case REQL_SP2:    step = REQL_VER;   goto ENTRY_VER;
		case REQL_VER:    step = REQL_EOL;   goto ENTRY_EOL;
		case REQL_EOL:    goto EXIT_DONE;
	}
EXIT_DONE:
	/* @ASSERTION: step == REQL_EOL */
	encoder->str = NULL;
	encoder->sindex = 0;
	encoder->step = 0;
	*pos = pindex;
	encoder->phaseHandler = encodeFieldPhase;
	return EXIT_DONE;
	/* @ASSERTION: pos != NULL && *pos == pindex && pindex == len */
EXIT_PEND:
	/* @ASSERTION: sindex != slen && str != NULL */
	encoder->str = str;
	encoder->sindex = sindex;
	encoder->step = step;
	*pos = pindex;
	/* @ASSERTION: encoder != NULL && pos != NULL && *pos == pindex && pindex == len && */
	/*             str != NULL && encoder->str == str && encoder->sindex == sindex */
	return EXIT_PEND;
EXIT_ERROR:
	return EXIT_ERROR;
}

int8_t encodeResStatusPhase(HttpCodec encoder, char* buf, uint32_t* pos, uint32_t len) {
	HttpConnection conn = encoder->conn;
	uint8_t        step = encoder->step;
	const char*    str = encoder->str;
	uint32_t       sindex = encoder->sindex;
	uint32_t pindex = *pos;
	uint32_t nleft;
	uint32_t ret;
	uint32_t slen;

	switch(step) {
		case RESL_VER:  goto ENTRY_VER;
		case RESL_SP1:  goto ENTRY_SP1;
		case RESL_ST:   goto ENTRY_ST;
		case RESL_SP2:  goto ENTRY_SP2;
		case RESL_DESC: goto ENTRY_DESC;
		case RESL_EOL:  goto ENTRY_EOL;
	}
ENTRY_VER:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = VERSION_NAME[conn->ver];
	}
	slen = VER_LEN;
	/* @ASSERTION: str != NULL && slen == VER_LEN */
	goto ENTRY_ENC;
ENTRY_SP1:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = " ";
	}
	slen = SP_LEN;
	/* @ASSERTION: str != NULL && slen == SP_LEN */
	goto ENTRY_ENC;
ENTRY_ST:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = STATUS_CODE[conn->code];
	}
	slen = ST_LEN;
	/* @ASSERTION: str != NULL && slen == ST_LEN */
	goto ENTRY_ENC;
ENTRY_SP2:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = " ";
	}
	slen = SP_LEN;
	/* @ASSERTION: str != NULL && slen == SP_LEN */
	goto ENTRY_ENC;
ENTRY_DESC:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = STATUS_DESC[conn->code];
	}
	slen = strlen(str);
	/* @ASSERTION: str != NULL && slen == strlen(str) */
	goto ENTRY_ENC;
ENTRY_EOL:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = CRLF;
	}
	slen = CRLF_LEN;
	/* @ASSERTION: str != NULL && slen == CRLF_LEN */
	goto ENTRY_ENC;
ENTRY_ENC:
	/* @ASSERTION: str != NULL && slen > 0 */
	// TODO consider if we need this
	if(pindex == len) {
		goto EXIT_PEND;
	}
	if((nleft = len - pindex) > slen - sindex) {
		nleft = slen - sindex;
	}
	/* @ASSERTION: nleft == MIN(len - pindex, slen - index) */
	strncpy(buf + pindex, str + sindex, nleft);
	pindex += nleft;
	sindex  += nleft;
	if(sindex ^ slen) {
		goto EXIT_PEND;
	}
	str = NULL;
	sindex = 0;
	/* @ASSERTION: slen == index && str = NULL */
ENTRY_AF_ENC:
	switch(step) {
		case RESL_VER:  step = RESL_SP1;  goto ENTRY_SP1;
		case RESL_SP1:  step = RESL_ST;   goto ENTRY_ST;
		case RESL_ST:   step = RESL_SP2;  goto ENTRY_SP2;
		case RESL_SP2:  step = RESL_DESC; goto ENTRY_DESC;
		case RESL_DESC: step = RESL_EOL;  goto ENTRY_EOL;
		case RESL_EOL:  goto EXIT_DONE;
	}
EXIT_DONE:
	/* @ASSERTION: step == RESL_EOL */
	encoder->str = NULL;
	encoder->sindex = 0;
	encoder->step = 0;
	*pos = pindex;
	encoder->phaseHandler = encodeFieldPhase;
	/* @ASSERTION: pos != NULL && *pos == pindex */
	return EXIT_DONE;
EXIT_PEND:
	/* @ASSERTION: sindex != slen && str != NULL */
	encoder->step = step;
	*pos = pindex;
	encoder->sindex = sindex;
	encoder->str = str;
	/* @ASSERTION: encoder != NULL && pos != NULL && *pos == pindex && pindex == len && */
	/*             str != NULL && encoder->str == str && encoder->sindex == sindex      */
	return EXIT_PEND;
EXIT_ERROR:
	/* It seems no error will occur, right? */
	return EXIT_ERROR;
}

int8_t encodeFieldPhase(HttpCodec encoder, char* buf, uint32_t* pos, uint32_t len) {
	HttpConnection conn = encoder->conn;
	uint32_t pindex = *pos;
	uint8_t step   = encoder->step;
	uint8_t cursor = encoder->cursor;
	uint8_t flag   = encoder->flag;
	const char* str = encoder->str;
	uint32_t    sindex = encoder->sindex;
	/* temporary variable */
	void*       data   = conn->data.ptr;
	uint32_t*   fields = conn->fields;
	uint32_t    slen;
	uint32_t    offset;
	uint32_t    ret;
	uint32_t    nleft;

	switch(step) {
		case FLD_INIT: goto ENTRY_INIT;
		case FLD_KEY:  goto ENTRY_KEY;
		case FLD_SEP:  goto ENTRY_SEP;
		case FLD_VAL:  goto ENTRY_VAL;
		case FLD_EOL:  goto ENTRY_EOL;
		case FLD_DONE: goto ENTRY_DONE;
	}
ENTRY_INIT:
	for(; cursor < HEADER_MAX; ++cursor) {
		if(fields[cursor]) {
			if(cursor >= HTTP_HEADER_NUM) {
				flag |= IS_EXT;
			}
			step = FLD_KEY;
			goto ENTRY_KEY;
		} else if(cursor >= HTTP_HEADER_NUM) {
			break;
		}
	}
	/* cursor == HEADER_MAX OR fields[cursor] == 0 && is_ext */
	step = FLD_DONE;
	goto ENTRY_DONE;
ENTRY_KEY:
	if(str == NULL) {
		if(LIKELY(!(flag & IS_EXT))) {
			str = HEADER_NAME[cursor];
		} else {
			ExtraField ef = (ExtraField)(data + fields[cursor]);
			str = (const char*)(data + ef->key);
		}
	}
	if(LIKELY(!(flag & IS_EXT))) {
		slen = HEADER_LEN[cursor];
	} else {
		slen = strlen(str);
	}
	goto ENTRY_ENC;
ENTRY_SEP:
	if(str == NULL) {
		str = SEP;
	}
	slen = SEP_LEN;
	goto ENTRY_ENC;
ENTRY_VAL:
	if(str == NULL) {
		offset = fields[cursor];
		if(LIKELY(!(flag & IS_EXT))) {
			str = (const char*)(data + offset);
		} else {
			ExtraField ef = (ExtraField)(data + offset);
			str = (const char*)(data + ef->value);
		}
	}
	slen = strlen(str);
	goto ENTRY_ENC;
ENTRY_EOL:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = CRLF;
	}
	slen = CRLF_LEN;
	/* @ASSERTION: str != NULL && slen > 0 */
	goto ENTRY_ENC;
ENTRY_DONE:
	/* @ASSERTION: str == NULL || str != NULL */
	if(str == NULL) {
		str = CRLF;
	}
	slen = CRLF_LEN;
	/* @ASSERTION: str != NULL && slen > 0 */
	goto ENTRY_ENC;
ENTRY_ENC:
	/* @ASSERTION: str != NULL && slen > 0 */
	// TODO consider if we need this
	if(pindex == len) {
		goto EXIT_PEND;
	}
	if((nleft = len - pindex) > slen - sindex) {
		nleft = slen - sindex;
	}
	/* @ASSERTION: nleft == MIN(len - pindex, slen - index) */
	strncpy(buf + pindex, str + sindex, nleft);
	pindex += nleft;
	sindex  += nleft;
	if(sindex ^ slen) {
		goto EXIT_PEND;
	}
	str = NULL;
	sindex = 0;
	/* @ASSERTION: slen == index && str = NULL */
ENTRY_AF_ENC:
	switch(step) {
		case FLD_KEY:  step = FLD_SEP; goto ENTRY_SEP;
		case FLD_SEP:  step = FLD_VAL; goto ENTRY_VAL;
		case FLD_VAL:  step = FLD_EOL; goto ENTRY_EOL;
		case FLD_EOL:  ++cursor;       goto ENTRY_INIT;
		case FLD_DONE: goto EXIT_DONE;
	}
EXIT_DONE:
	encoder->str = NULL;
	encoder->sindex = 0;
	encoder->step = 0;
	*pos = pindex;
	encoder->phaseHandler = NULL;
	return EXIT_DONE;
EXIT_PEND:
	encoder->str    = str;
	encoder->sindex = sindex;
	encoder->step = step;
	encoder->cursor = cursor;
	encoder->flag = flag;
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
int8_t decodeInitPhase(HttpCodec decoder, char* buf, uint32_t* pos, uint32_t len) {
	decoder->temp = &decoder->conn->data;
	/* skip the first 1 Byte to avoid bug */
	decoder->cursor    = HTTP_HEADER_NUM;
	decoder->step      = 0;
	////////////
	decoder->sep = 0;
	if(decoder->conn->type ^ HTTP_RES) {
		decoder->phaseHandler = decodeReqStatusPhase;
	} else {
		decoder->phaseHandler = decodeResStatusPhase;
	}
	return EXIT_DONE;
}

int8_t decodeReqStatusPhase(HttpCodec decoder, char* buf, uint32_t* pos, uint32_t len) {
	/* local variables for data inside HttpEncoder */
	HttpConnection req = decoder->conn;
	MemBlock temp = decoder->temp;
	uint32_t pindex = *pos;
	/* should be written when PEND */
	uint32_t hash = decoder->hash;
	uint8_t  step = decoder->step;
	uint32_t sep  = decoder->sep;
	/* temporary variables */
	int8_t   ret;
	char     ch;
	uint32_t begin, end;
	
	/* state entry navigation */
	switch(step) {
		case REQL_METHOD: goto ENTRY_METHOD;
		case REQL_PATH:   goto ENTRY_PATH;
		case REQL_QUERY:  goto ENTRY_QUERY;
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
			ret = FindMethod(hash);
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
	begin = pindex;
	while(pindex ^ len) {
		ch = buf[pindex++];
		if(LIKELY(URL_TOKEN_MAP[ch]) && ch ^ QS) {
			if(ch == SL) {
				sep = 0;
			} else {
				++sep;
			}
		} else if(ch == QS) {
			end = pindex - 1;
			MemBlock_putStrN(temp, buf + begin, end - begin);
			if(sep) {
				req->pathinfo = temp->len - sep;
			}
			req->path = MemBlock_getStr(temp);
			step = REQL_QUERY;
			goto ENTRY_QUERY;
		} else if(ch == SP) {
			/* path process is done */
			end = pindex - 1;
			MemBlock_putStrN(temp, buf + begin, end - begin);
			if(sep) {
				req->pathinfo = temp->len - sep;
			}
			req->path = MemBlock_getStr(temp);
			step = REQL_VER;
			goto ENTRY_VER;
		} else {
			assert(0);
			goto EXIT_ERROR;
		}
	}
	end = pindex;
	MemBlock_putStrN(temp, buf + begin, end - begin);
	goto EXIT_PEND;
ENTRY_QUERY:
	begin = pindex;
	while(pindex ^ len) {
		ch = buf[pindex++];
		if(LIKELY(URL_TOKEN_MAP[ch])) {
		} else if(ch == SP) {
			end = pindex - 1;
			MemBlock_putStrN(temp, buf + begin, end - begin);
			req->query = MemBlock_getStr(temp);
			step = REQL_VER;
			goto ENTRY_VER;
		} else {
			assert(0);
			goto EXIT_ERROR;
		}
	}
	end = pindex;
	MemBlock_putStrN(temp, buf + begin, end - begin);
	goto EXIT_PEND;
ENTRY_VER:
	while(pindex ^ len) {
		ch = buf[pindex++];
		if(LIKELY(ch != CR)) {
			HASH(hash, ch);
		} else {
			step = REQL_EOL;
			hash &= 0x7FFFFFFF;
			ret = FindVersion(hash);
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
	decoder->sep  = sep;
	decoder->step = step;
	decoder->hash = hash;
	*pos = pindex;
	return EXIT_PEND;
EXIT_ERROR:
	return EXIT_ERROR;
}

int8_t decodeResStatusPhase(HttpCodec decoder, char* buf, uint32_t* pos, uint32_t len) {
	/* local variables for data inside HttpEncoder */
	HttpConnection res = decoder->conn;
	MemBlock temp = decoder->temp;
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
			ret = FindVersion(hash);
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

int8_t decodeFieldPhase(HttpCodec decoder, char* buf, uint32_t* pos, uint32_t len) {
	/* local variable for data inside HttpDecoder */
	HttpConnection conn = decoder->conn;
	MemBlock temp = decoder->temp;
	uint32_t pindex = *pos;
	/* variables below should be write when PEND */
	uint32_t hash = decoder->hash;
	uint8_t  step = decoder->step;
	uint32_t fld  = decoder->fld;
	uint8_t  flag = decoder->flag;
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
		fld = FindHeader(hash);
		if(LIKELY(fld ^ HH_INVALID)) {
			MemBlock_rollback(temp);
		} else {
			fld = MemBlock_getStr(temp);
			/* the header belongs to extra header */
			flag |= IS_EXT;
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
		if(LIKELY(!(flag & IS_EXT))) {
			conn->fields[fld] = v;
		} else {
			MemBlock_putInt4(temp, hash);
			MemBlock_putInt4(temp, fld);
			MemBlock_putInt4(temp, v);
			conn->fields[cursor++] = MemBlock_getStruct(temp);
			flag &= ~IS_EXT;
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
	decoder->flag = flag;
	decoder->cursor = cursor;
	decoder->fld = fld;
	decoder->step = step;
	decoder->hash = hash;
	*pos = pindex;
	return EXIT_PEND;
EXIT_ERROR:
	return EXIT_ERROR;
}

int8_t HttpCodec_decode(HttpCodec decoder, char* buf, uint32_t* len) {
	uint32_t pos = 0;
	int8_t   ret;
	switch(decoder->state) {
		case STATE_INIT:
			decoder->phaseHandler = decodeInitPhase;
			decoder->state = STATE_DOING;
		case STATE_DOING:
			while(decoder->phaseHandler) {
				switch(ret = decoder->phaseHandler(decoder, buf, &pos, *len)) {
					case EXIT_ERROR:
						decoder->state = STATE_ERROR;
					case EXIT_PEND:
						return ret;
				}
			}
			decoder->state = STATE_DONE;
			*len = pos;
			return EXIT_DONE;
		case STATE_DONE:  return EXIT_DONE;
		case STATE_ERROR: return EXIT_ERROR;
	}
	return EXIT_ERROR;
}

