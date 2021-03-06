#ifndef _HANDLER_H_
#define _HANDLER_H_

#include <unistd.h>

typedef int fd_t;

typedef enum htype {
	H_NONE,
	H_FILE,
	H_SOCK,
	H_POLL,
} htype_t;

typedef struct handler {
	fd_t    fileno;
	htype_t type;
} handler_t, *Handler;

#define Handler_init(h, f, t) \
	(h)->fileno = (f); \
	(h)->type = (t)

#define Handler_close(h) close((h)->fileno)

#endif // _HANDLER_H_
