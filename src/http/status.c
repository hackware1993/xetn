#include "common.h"
#include "http/status.h"

#define XX(a, b, c) b,
uint16_t STATUS_NUM[] = {
	STATUS_MAP(XX) 
};
#undef XX

#define XX(a, b, c) #b,
const char* STATUS_CODE[] = {
	STATUS_MAP(XX)
};
#undef XX

#define XX(a, b, c) c,
const char* STATUS_DESC[] = {
	STATUS_MAP(XX)
};
#undef XX

