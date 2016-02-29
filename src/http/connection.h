#ifndef _HTTP_CONNECTION_
#define _HTTP_CONNECTION_

#include "header.h"
#include "status.h"
#include "../list.h"

#define METHOD_MAP(XX) \
	XX(OPTIONS)        \
	XX(GET)            \
	XX(HEAD)           \
	XX(POST)           \
	XX(PUT)            \
	XX(DELETE)         \
	XX(TRACE)          \
	XX(CONNECT)

#define VERSION_MAP(XX)   \
	XX(HTTP1_0, HTTP/1.0) \
	XX(HTTP1_1, HTTP/1.1) \
	XX(HTTP2_0, HTTP/2)

#define XX(tag, name) tag,
typedef enum http_version {
	VERSION_MAP(XX)
} http_version_t;
#undef XX

#define XX(index) HTTP_##index,
typedef enum http_method {
	METHOD_MAP(XX)
} http_method_t;
#undef XX

#define HEADER_MAX 128

typedef struct extra_field {
	uint32_t hash;
	uint32_t key;
	uint32_t value;
} extra_field_t, *ExtraField;

typedef struct http_connection {
	unsigned    ver  : 2;
	unsigned    code : 14;
	uint32_t    str;
	uint32_t    fields[128];
	void*       data;
	link_list_t ext;
} http_connection_t, *HttpConnection;

#define        HttpConnection_getMethodCode(req)  (req)->code
#define        HttpConnection_getVersionCode(res) (res)->ver
#define        HttpConnection_getStatusCode(res)  (res)->code
HttpConnection HttpConnection_init(HttpConnection);
const char*    HttpConnection_getMethod(HttpConnection);
const char*    HttpConnection_getVersion(HttpConnection);
const char*    HttpConnection_getHeader(HttpConnection, const char*);

#endif // _HTTP_CONNECTION_
