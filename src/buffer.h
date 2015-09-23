#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stdlib.h>

typedef struct buffer {
	unsigned pos;
	unsigned end;
	unsigned lim;
	unsigned cap;
	char     ptr[0];
	// int appendable;
} *buffer_t;

buffer_t buffer_new(unsigned);

#define buffer_free(buf) free((buf))

#define buffer_get_ptr(buf) (buf)->ptr

#define buffer_get_cap(buf) (buf)->cap

#define buffer_get_pos(buf) (buf)->pos

#define buffer_set_pos(buf, p) (buf)->pos = (p)

#define buffer_get_len(buf) (buf)->end

#define buffer_set_len(buf, l) (buf)->end = (l)

#define buffer_get_lim(buf) (buf)->lim

#define buffer_set_lim(buf, l) (buf)->lim = (l)

#define buffer_is_empty(buf) (buf)->end == 0

#define buffer_is_end(buf) (buf)->pos == (buf)->end

#define buffer_is_full(buf) (buf)->end == (buf)->lim

#define buffer_clear(buf) \
	(buf)->pos = 0;       \
	(buf)->end = 0;       \
	(buf)->lim = (buf)->cap

#define buffer_rewind(buf) (buf)->pos = 0

#define buffer_put(buf, c) (buf)->ptr[(buf)->end++] = (c)

#define buffer_get(buf) (buf)->ptr[(buf)->pos++]

#define buffer_get_by_index(buf, i) (buf)->ptr[(i)]

#define buffer_set_by_index(buf, i, v) (buf)->ptr[(i)] = (v)

char* buffer_get_between(buffer_t, unsigned, unsigned);

unsigned buffer_put_arr(buffer_t, char*, unsigned, unsigned);

unsigned buffer_get_arr(buffer_t, char*, unsigned, unsigned);

#endif // _BUFFER_H_
