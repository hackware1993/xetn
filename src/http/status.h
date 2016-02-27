#ifndef _STATUS_H_
#define _STATUS_H_

#include <stdint.h>

#define STATUS_MAP(XX)                                \
	XX(ST_100, 100, "Continue")                        \
	XX(ST_101, 101, "Switching Protocols")             \
	XX(ST_200, 200, "OK")                              \
	XX(ST_201, 201, "Created")                         \
	XX(ST_202, 202, "Accepted")                        \
	XX(ST_203, 203, "Non-Authoritative Information")   \
	XX(ST_204, 204, "No Content")                      \
	XX(ST_205, 205, "Reset Content")                   \
	XX(ST_206, 206, "Partial Content")                 \
	XX(ST_300, 300, "Multiple Choices")                \
	XX(ST_301, 301, "Moved Permanently")               \
	XX(ST_302, 302, "Found")                           \
	XX(ST_303, 303, "See Other")                       \
	XX(ST_304, 304, "Not Modified")                    \
	XX(ST_305, 305, "Use Proxy")                       \
	XX(ST_306, 306, "Not Used")                        \
	XX(ST_307, 307, "Temporary Redirect")              \
	XX(ST_400, 400, "Bad Request")                     \
	XX(ST_401, 401, "Unauthorized")                    \
	XX(ST_402, 402, "Payment Required")                \
	XX(ST_403, 403, "Forbidden")                       \
	XX(ST_404, 404, "Not Found")                       \
	XX(ST_405, 405, "Method Not Allowed")              \
	XX(ST_406, 406, "Not Acceptable")                  \
	XX(ST_407, 407, "Proxy Authentication Required")   \
	XX(ST_408, 408, "Request Timeout")                 \
	XX(ST_409, 409, "Conflict")                        \
	XX(ST_410, 410, "Gone")                            \
	XX(ST_411, 411, "Length Required")                 \
	XX(ST_412, 412, "Precondition Failed")             \
	XX(ST_413, 413, "Request Entity Too Large")        \
	XX(ST_414, 414, "Request-URI Too Long")            \
	XX(ST_415, 415, "Unsupported Media Type")          \
	XX(ST_416, 416, "Requested Range Not Satisfiable") \
	XX(ST_417, 417, "Expectation Failed")              \
	XX(ST_500, 500, "Internal Server Error")           \
	XX(ST_501, 501, "Not Implemented")                 \
	XX(ST_502, 502, "Bad Gateway")                     \
	XX(ST_503, 503, "Service Unavailable")             \
	XX(ST_504, 504, "Gateway Timeout")                 \
	XX(ST_505, 505, "HTTP Version Not Supported")

#define XX(a, b, c) a,
typedef enum http_status {
	STATUS_MAP(XX)
} http_status_t;
#undef XX

http_status_t HttpStatus_getNumber(http_status_t);
const char*   HttpStatus_getDescription(http_status_t);

#endif // _STATUS_H_
