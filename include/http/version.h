#ifndef _VERSION_H_
#define _VERSION_H_

#define VERSION_MAP(XX)   \
	XX(HTTP1_0, HTTP/1.0) \
	XX(HTTP1_1, HTTP/1.1) \
	XX(HTTP2_0, HTTP/2)

#define XX(tag, name) tag,
typedef enum http_version {
	VERSION_MAP(XX)
} http_version_t;
#undef XX

#endif //_VERSION_H_
