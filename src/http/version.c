#include "common.h"
#include "http/version.h"

#define XX(tag, name) #name,
const char* VERSION_NAME[] = {
	VERSION_MAP(XX)
};
#undef XX

uint64_t VERSION_HASH[4] = {
	0x69440D40, 0x169440D41, 0x0, 0x20B0B8CD7,
};

