#include "connection.h"
#include "datamap.h"
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <stdio.h>

#define PRIVATE static

extern http_header_t HttpHeader_find(uint32_t, uint32_t);

HttpConnection HttpConnection_init(HttpConnection conn, http_type_t type) {
	/* set all content of connection to 0 as default */
	memset(conn, 0, sizeof(http_connection_t));
	conn->type = type;
	MemBlock_init(&conn->data, INIT_DATA_SIZE);
	/* NOTICE 0 is used to indicate there is no value */
	conn->data.len  = 1;
	conn->data.last = 1;
}

const char* HttpConnection_getMethodStr(HttpConnection conn) {
	return METHOD_NAME[conn->code];
}

const char* HttpConnection_getVersionStr(HttpConnection conn) {
	return VERSION_NAME[conn->ver];
}

const char* HttpConnection_getStatusStr(HttpConnection conn) {
	return STATUS_DESC[conn->code];
}

const char* HttpConnection_decodePath(HttpConnection conn) {
	char*   path = (char*)(conn->data.ptr + conn->str);
	char    temp;
	uint8_t ch;
	int8_t  mode = 0;
	char*   pstr = path;
	char*   p    = path;
	while((ch = *p++) != '\0') {
		if(mode == 0) {
			if(ch ^ '%') {
				*pstr++ = ch;
			} else {
				mode = 2;
			}
		} else {
			if((ch = ALPHA_HEX[ch]) == 0xFF) {
				return NULL;
			}
			switch(mode) {
				case 1:
					temp |= ch;
					*pstr++ = temp;
					mode = 0;
					break;
				case 2:
					temp = ch << 4;
					mode = 1;
					break;
			}
		}
	}
	if(mode == 0) {
		*pstr = '\0';
	} else {
		return NULL;
	}
	return path;
}

const char* HttpConnection_getPath(HttpConnection conn) {
	return (const char*)(conn->data.ptr + conn->str);
}

void HttpConnection_setPath(HttpConnection conn, const char* path) {
	MemBlock data = &conn->data;
	MemBlock_putStrN(data, path, strlen(path));
	conn->str = MemBlock_getStr(data);
}

const char* HttpConnection_getHeader(HttpConnection conn, const char* name) {
	const char* data = (const char*)conn->data.ptr;
	uint32_t hash = 0;
	const char* str = name;
	while(*str) {
		hash = (hash << 7) + (hash << 1) + hash + *str++;
	}
	uint32_t index = hash % MAGNUM;
	index = HttpHeader_find(index, hash);
	const char* res = NULL;
	if(index != HH_INVALID) {
		res = data + conn->fields[index];
	} else {
		uint32_t* pf = conn->fields + HTTP_HEADER_NUM;
		uint32_t* pend = conn->fields + HEADER_MAX;
		while(pf < pend && *pf) {
			ExtraField ef = (ExtraField)(data + *pf++);
			if(ef->hash == hash && strcmp(data + ef->key, name) == 0) {
				res = data + ef->value;
				break;
			}
		}
	}
	return res;
}

void HttpConnection_putHeader(HttpConnection conn, const char* name, const char* val) {
	MemBlock data = &conn->data;
	uint32_t hash = 0;
	const char* str = name;
	while(*str) {
		hash = (hash << 7) + (hash << 1) + hash + *str++;
	}
	uint32_t index = hash % MAGNUM;
	index = HttpHeader_find(index, hash);
	if(index != HH_INVALID) {
		MemBlock_putStrN(data, val, strlen(val));
		conn->fields[index] = MemBlock_getStr(data);
	} else {
		uint32_t* pf   = conn->fields + HTTP_HEADER_NUM;
		uint32_t* pend = conn->fields + HEADER_MAX;
		/* skip all fields already have contents */
		while(pf < pend && *pf) { ++pf; }
		assert(pf != pend);
		MemBlock_putStrN(data, name, strlen(name));
		uint32_t fld = MemBlock_getStr(data);
		MemBlock_putStrN(data, val, strlen(val));
		uint32_t v   = MemBlock_getStr(data);
		MemBlock_putInt4(data, hash);
		MemBlock_putInt4(data, fld);
		MemBlock_putInt4(data, v);
		*pf = MemBlock_getStruct(data);
	}
}
