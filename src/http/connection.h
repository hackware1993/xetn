#ifndef _HTTP_CONNECTION_
#define _HTTP_CONNECTION_

#include "../memblock.h"
#include "header.h"
#include "status.h"

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
#define INIT_DATA_SIZE 1024

typedef enum http_type {
	HTTP_REQ,
	HTTP_RES,
} http_type_t;

typedef struct extra_field {
	uint32_t hash;
	uint32_t key;
	uint32_t value;
} extra_field_t, *ExtraField;

typedef struct http_connection {
	unsigned    type :2;
	unsigned    ver  :2;
	unsigned    code :12;
	uint32_t    str;
	uint32_t    fields[HEADER_MAX];
	mem_block_t data;
} http_connection_t, *HttpConnection;


HttpConnection HttpConnection_init(HttpConnection, http_type_t);
#define        HttpConnection_close(conn)   MemBlock_free(&(conn)->data)
#define        HttpConnection_getType(conn) (conn)->type      

const char*    HttpConnection_getPath(HttpConnection);
void           HttpConnection_setPath(HttpConnection, const char*);

#define        HttpConnection_getMethod(conn)    (conn)->code
#define        HttpConnection_setMethod(conn, m) (conn)->code = (m)
const char*    HttpConnection_getMethodStr(HttpConnection);

#define        HttpConnection_getVersion(conn)    (conn)->ver
#define        HttpConnection_setVersion(conn, v) (conn)->ver = (v)
const char*    HttpConnection_getVersionStr(HttpConnection);

#define        HttpConnection_getStatus(conn)    (conn)->code
#define        HttpConnection_setStatus(conn, s) (conn)->code = (s)

const char*    HttpConnection_getHeader(HttpConnection, const char*);
void           HttpConnection_putHeader(HttpConnection, const char*, const char*);

#endif // _HTTP_CONNECTION_
