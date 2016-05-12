#ifndef _METHOD_H_
#define _METHOD_H_

#define METHOD_MAP(XX) \
	XX(OPTIONS)        \
	XX(GET)            \
	XX(HEAD)           \
	XX(POST)           \
	XX(PUT)            \
	XX(DELETE)         \
	XX(TRACE)          \
	XX(CONNECT)

#define XX(index) HTTP_##index,
typedef enum http_method {
	METHOD_MAP(XX)
} http_method_t;
#undef XX

#endif //_METHOD_H_
