#include "connection.h"
#include <stdlib.h>
#include <string.h>

#define PRIVATE static

#define XX(name) #name,
PRIVATE char* METHOD_NAME[] = { METHOD_MAP(XX) };
#undef XX
#define XX(tag, name) #name,
PRIVATE char* VERSION_NAME[] = { VERSION_MAP(XX) };
#undef XX
#define XX(a, b, c) c,
PRIVATE const char* HEADER_NAMES[] = { HEADER_MAP(XX) };
#undef XX
#define XX(a, b, c) b,
PRIVATE uint8_t HEADER_LENS[] = { HEADER_MAP(XX) };
#undef XX
#define XX(a, b, c) #a,
PRIVATE const char* HEADER_LABELS[] = { HEADER_MAP(XX) };
#undef XX
#define XX(a, b, c) b,
PRIVATE uint16_t STATUS_NUM[] = { STATUS_MAP(XX) };
#undef XX
#define XX(a, b, c) c,
PRIVATE char* STATUS_DESC[] = { STATUS_MAP(XX) };
#undef XX

extern http_header_t HttpHeader_find(uint32_t, uint32_t);

HttpConnection HttpConnection_init(HttpConnection conn) {
	conn->ver = 0;
	conn->code = 0;
	conn->str = 0;
	uint32_t* pfield = conn->fields;
	uint32_t* pend   = pfield + 128;
	while(pfield < pend) {
		*pfield = 0;
		++pfield;
	}
}

const char* HttpConnection_getMethod(HttpConnection conn) {
	return METHOD_NAME[conn->code];
}

const char* HttpConnection_getVersion(HttpConnection conn) {
	return VERSION_NAME[conn->ver];
}

const char* HttpConnection_getHeader(HttpConnection conn, const char* name) {
	uint32_t hash = 0;
	while(*name) {
		hash = (hash << 7) + (hash << 1) + hash + *name++;
	}
	hash &= 0x7FFFFFFF;
	uint32_t index = hash % 237;
	index = HttpHeader_find(index, hash);
	const char* res = NULL;
	if(index != HH_INVALID) {
		res = conn->data + conn->fields[index];
	} else {
		uint32_t* pf = conn->fields + HTTP_HEADER_NUM;
		uint32_t* pend = conn->fields + 128;
		while(pf < pend && *pf) {
			ExtraField ef = conn->data + *pf++;
			if(ef->hash && strcmp(conn->data + ef->key, name) == 0) {
				res = conn->data + ef->value;
				break;
			}
		}
	}
	return res;
}

