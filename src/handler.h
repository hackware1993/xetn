#ifndef _HANDLER_H_
#define _HAHDLER_H_

#include <unistd.h>

typedef int fd_t;

typedef enum htype {
	H_NONE,
	H_FILE,
	H_SOCK,
	H_POLL
} htype_t;

typedef struct handler {
	fd_t    fileno;
	htype_t type;
} *handler_t;

#define handler_create(f, t) {(f), (t)}
#define handler_close(h) close((h)->fileno)

#endif // _HANDLER_H_
