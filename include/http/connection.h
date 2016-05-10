#ifndef _HTTP_CONNECTION_
#define _HTTP_CONNECTION_

#include "memblock.h"
#include "http/header.h"
#include "http/status.h"
#include "http/method.h"
#include "http/version.h"

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
	uint32_t    flags;
	uint32_t    path;
	uint32_t    pathinfo;
	uint32_t    query;
	uint32_t    fields[HEADER_MAX];
	mem_block_t data;
} http_connection_t, *HttpConnection;


HttpConnection HttpConnection_init(HttpConnection, http_type_t);
void           HttpConnection_clear(HttpConnection);
#define        HttpConnection_close(conn)   MemBlock_free(&(conn)->data)
#define        HttpConnection_getType(conn) (conn)->type      

const char*    HttpConnection_decodePath(HttpConnection);
void           HttpConnection_setPath(HttpConnection, const char*);

const char*    HttpConnection_getPath(HttpConnection);
const char*    HttpConnection_getPathInfo(HttpConnection);
const char*    HttpConnection_getQuery(HttpConnection);

#define        HttpConnection_getMethod(conn)    (conn)->code
#define        HttpConnection_setMethod(conn, m) (conn)->code = (m)
const char*    HttpConnection_getMethodStr(HttpConnection);

#define        HttpConnection_getVersion(conn)    (conn)->ver
#define        HttpConnection_setVersion(conn, v) (conn)->ver = (v)
const char*    HttpConnection_getVersionStr(HttpConnection);

#define        HttpConnection_getStatus(conn)    (conn)->code
#define        HttpConnection_setStatus(conn, s) (conn)->code = (s)
const char*    HttpConnection_getStatusStr(HttpConnection);

const char*    HttpConnection_getHeader(HttpConnection, const char*);
void           HttpConnection_putHeader(HttpConnection, const char*, const char*);

#endif // _HTTP_CONNECTION_
