#include <string.h>
#include <stdlib.h>
#include "json/jpath.h"

static uint8_t CHAR_MAP[128] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0xEE, 0x00, 0x00,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
	0xEE, 0xEE, 0xEE, 0x00, 0x00, 0x00, 0x00, 0xEE,
	0x00, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
	0xEE, 0xEE, 0xEE, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static void AddPoint(JPath path, const char* str) {
	uint32_t len = path->len;
	uint32_t cap = path->cap;
	if(len >= cap) {
		while(len >= cap) {
			cap <<= 1;
		}
		path->cap = cap;
		path->navs = (JPathNav)realloc(path->navs, cap * sizeof(JPathNav_t));
	}
	path->len = len + 1;
	JPathNav p = path->navs + len;
	if(str == NULL) {
		p->type = NAV_ROOT;
		p->key.str = NULL;
		return;
	}
	p->type = NAV_POINT;
	p->key.str = str;
}

static void AddIndex(JPath path, uint32_t num) {
	uint32_t len = path->len;
	uint32_t cap = path->cap;
	if(len >= cap) {
		while(len >= cap) {
			cap <<= 1;
		}
		path->cap = cap;
		path->navs = (JPathNav)realloc(path->navs, cap * sizeof(JPathNav_t));
	}
	path->len = len + 1;
	JPathNav p = path->navs + len;
	p->type = NAV_INDEX;
	p->key.num = num;
}

static uint8_t GetIndex(JPath path, const char* str, uint32_t* off, uint32_t len) {
	uint32_t val = 0;
	uint32_t loff = *off;
	if(CHAR_MAP[str[loff]] != 0xAA) {
		return 0;
	}

	char ch;
	while(loff < len) {
		ch = str[loff];
		if(CHAR_MAP[ch] == 0xAA) {
			val = val * 10 + (ch - '0');
			++loff;
		} else if(ch != ']'){
			/* an error happen */
			return 0;
		} else {
			++loff;
			break;
		}
	}
	AddIndex(path, val);
	*off = loff;
	return 1;
}

static uint8_t GetPoint(JPath path, const char* str, uint32_t* off, uint32_t len) {
	uint32_t begin = *off;
	uint32_t loff  = begin;
	if(CHAR_MAP[str[loff]] == 0x00) {
		return 0;
	}

	char ch;
	while(loff < len) {
		ch = str[loff];
		if(CHAR_MAP[ch] >= 0xAA) {
			++loff;
		} else {
			break;
		}
	}
	uint32_t l = loff - begin;
	char* res = (char*)malloc(l + 1);
	strncpy(res, str + begin, l);
	res[l] = '\0';
	AddPoint(path, res);
	*off = loff;
	return 1;
}

JPath JPath_init(JPath path) {
	/* init the jpath */
	path->len = 0;
	path->cap = 8;
	path->navs = (JPathNav)malloc(sizeof(JPathNav_t) * 8);
	return path;
}

JPath JPath_compile(JPath path, const char* pstr) {
	path->len = 0;
	uint32_t len = strlen(pstr);
	uint32_t off = 0;

	uint8_t ret;
	char ch;

	/* the first character must be $ */
	if(pstr[off++] != '$' || len == 0) {
		return NULL;
	}
	AddPoint(path, NULL);
	while(off < len) {
		ch = pstr[off++];
		switch(ch) {
			/* BLOCK1: \[\d+\] */
			case '[':
				ret = GetIndex(path, pstr, &off, len);
				if(ret == 0) {
					return NULL;
				}
				break;
			/* BLOCK2: \.[-_0-9a-zA-Z]+*/
			case '.':
				ret = GetPoint(path, pstr, &off, len);
				if(ret == 0) {
					return NULL;
				}
				break;
			default:
				return NULL;
		}
	}
	return path;
}

void JPath_free(JPath path) {
	JPathNav p;

	JPathNav navs = path->navs;
	uint32_t len = path->len;
	for(uint32_t i = 0; i < len; ++i) {
		p = navs + i;
		if(p->type == NAV_POINT) {
			free((void*)p->key.str);
		}
	}
	free(navs);
}

