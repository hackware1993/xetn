#include "error.h"

#define XX(n, t, s) {"EX_"#t, s},
static struct {
	const char* name;
	const char* desc;
} ex_strtab[] = {
	ERR_MAP(XX)
};
#undef  XX

const char* error_name(error_t err) {
	return ex_strtab[err].name;
}

const char* error_desc(error_t err) {
	return ex_strtab[err].desc;
}
