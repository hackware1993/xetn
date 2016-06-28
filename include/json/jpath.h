#ifndef _JPATH_H_
#define _JPATH_H_

#include <stdint.h>

typedef enum {
	NAV_ROOT,
	NAV_INDEX,
	NAV_POINT,
} Nav_t;

typedef struct jpath_nav {
	Nav_t type;
	union {
		const char* str;
		uint32_t    num;
	} key;
} JPathNav_t, *JPathNav;

/**
 * NOTICE:
 *   JPath only support key name consists of alphabet, '-', '_'
 * */
typedef struct jpath {
	JPathNav navs;
	uint32_t len;
	uint32_t cap;
} JPath_t, *JPath;

JPath JPath_init(JPath);
JPath JPath_compile(JPath, const char*);
void  JPath_free(JPath);

#endif // _JPATH_H_
