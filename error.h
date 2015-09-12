#ifndef _ERROR_H_
#define _ERROR_H_

#define ERR_MAP(XX) \
	XX(0, NONE, "OK")     \
	XX(1, TYPE, "INVALID TYPE")     

typedef enum error {
#define XX(num, type, s) EX_##type = num,
	ERR_MAP(XX)
#undef  XX
} error_t;

const char* error_name(error_t);

const char* error_desc(error_t);

#endif // _ERROR_H_
