#include "connection.h"
#include "datamap.h"
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <stdio.h>

#define PRIVATE static

extern http_header_t HttpHeader_find(uint32_t, uint32_t);

HttpConnection HttpConnection_init(HttpConnection conn, http_type_t type) {
	conn->type = type;
	conn->ver  = 0;
	conn->code = 0;
	conn->str  = 0;
	conn->data = NULL;
	uint32_t* pfield = conn->fields;
	uint32_t* pend   = pfield + HEADER_MAX;
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
	const char* str = name;
	while(*str) {
		hash = (hash << 7) + (hash << 1) + hash + *str++;
	}
	uint32_t index = hash % MAGNUM;
	index = HttpHeader_find(index, hash);
	const char* res = NULL;
	if(index != HH_INVALID) {
		res = conn->data + conn->fields[index];
	} else {
		uint32_t* pf = conn->fields + HTTP_HEADER_NUM;
		uint32_t* pend = conn->fields + HEADER_MAX;
		while(pf < pend && *pf) {
			ExtraField ef = conn->data + *pf++;
			if(ef->hash == hash && strcmp(conn->data + ef->key, name) == 0) {
				res = conn->data + ef->value;
				break;
			}
		}
	}
	return res;
}

