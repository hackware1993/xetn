#include "common.h"
#include "mime.h"

#include "util/map.h"

#include <stdlib.h>
#include <string.h>

typedef struct mime {
	const char* suffix;
	const char* type;
} mime_t, *Mime;

typedef struct mime_module {
	hash_map_t s2t_map;
	hash_map_t t2s_map;
} mime_module_t, *MimeModule;

static mime_module_t module;

#define XX(a, b) {a, b},
static mime_t mime_arr[] = {
	MIME_MAP(XX)
};
#undef XX

void Mime_initModule() {
	HashMap_init(&module.s2t_map);
	HashMap_init(&module.t2s_map);
	Mime p;
	for(uint32_t i = 0; i < MIME_NUM; ++i) {
		p = mime_arr + i;
		HashMap_put(&module.s2t_map, p->suffix, p->type);
		HashMap_put(&module.t2s_map, p->type,   p->suffix);
	}
}

void Mime_closeModule() {
	HashMap_free(&module.s2t_map);
	HashMap_free(&module.t2s_map);
}

PRIVATE INLINE const char* GetSuffix(const char* path) {
	size_t len = strlen(path);
	if(len == 0) {
		return NULL;
	}
	const char* p = path + (len - 1);
	size_t i;
	for(i = 0; i < len; ++i) {
		if(*p == '.') {
			break;
		}
		--p;
	}
	if(i == len) {
		/* case like "txt" */
		/* There MUST exist a dot */
		return NULL;
	} else if(len == 1) {
		/* case like "." */
		return NULL;
	}
	/* skip the first dot */
	return p + 1;
}

const char* Mime_lookupType(const char* path) {
	if(path == NULL) {
		return NULL;
	}
	const char* suf = GetSuffix(path);
	if(suf == NULL) {
		return NULL;
	}
	return HashMap_get(&module.s2t_map, suf);
}

//const char* Mime_lookupSuffix(const char* type) {
//	if(type == NULL) {
//		return NULL;
//	}
//	return HashMap_get(&module.t2s_map, type);
//}

/*
#include <assert.h>
#include <stdio.h>
int main() {
	Mime_initModule();
	const char* p = "/home/codesun/test.html";
	const char* t = "text/html";
	printf("%s ", GetSuffix(p));
	printf("%s ", Mime_lookupType(p));
	Mime_closeModule();
	return 0;
}
*/
