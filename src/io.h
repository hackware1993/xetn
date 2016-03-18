#ifndef _IO_H_
#define _IO_H_

/**
 * Basic wrapper of low level io api
 */

#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>

static inline int IO_write(Handler h, void* buf, size_t* len) {
	int n = write(h->fileno, buf, *len);
	switch(n) {
		case -1: return -1;
		default:
			*len = n;
	}
	return 0;
}

static inline int IO_read(Handler h, void* buf, size_t* len) {
	int n = read(h->fileno, buf, *len);
	switch(n) {
		/* END OF FILE */
		case 0:  return 1;
		case -1: return -1;
		default:
			*len = n;
	}
	return 0;
}

static inline int IO_writeSpec(Handler h, void* buf, size_t* len) {
	int    n      = 0;
	size_t total  = *len;
	size_t nwrite = total;
	while(total) {
		n = IO_write(h, buf, &nwrite);
		if(n == -1) {
			*len = *len - total;
			return -1;
		}
		buf   += nwrite;
		total -= nwrite;
		nwrite = total;
	}
	return 0;
}

static inline int IO_readSpec(Handler h, void* buf, size_t* len) {
	int    n     = 0;
	size_t total = *len;
	size_t nread = total;
	while(total) {
		n = IO_read(h, buf, &nread);
		if(n == -1) {
			*len = *len - total;
			return -1;
		} else if(n == 1) {
			*len = *len - total;
			return 1;
		}
		buf   += nread;
		total -= nread;
		nread  = total;
	}
	return 0;
}

static inline int IO_waitEvent(Handler h, int32_t to) {
	struct timeval* timeout = NULL;
	struct timeval tv;
	if(to >= 0) {
		int sec  = to / 1000;
		int usec = (to - sec * 1000) * 1000;
		tv.tv_sec  = sec;
		tv.tv_usec = usec;
		timeout = &tv;
	}
	int fd = h->fileno;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(fd, &set);
	return select(fd + 1, &set, NULL, NULL, timeout);
}

static inline ssize_t IO_timedRead(Handler h, void* buf, size_t* len, int32_t to) {
	switch(IO_waitEvent(h, to)) {
		case 0:  errno = ETIMEDOUT;
		case -1: return -1;
	}
	return IO_read(h, buf, len);
} 
static inline ssize_t IO_timedWrite(Handler h, void* buf, size_t* len, int32_t to) {
	switch(IO_waitEvent(h, to)) {
		case 0:  errno = ETIMEDOUT;
		case -1: return -1;
	}
	return IO_write(h, buf, len);
}

static inline ssize_t IO_timedReadSpec(Handler h, void* buf, size_t* len, int32_t to) {
	switch(IO_waitEvent(h, to)) {
		case 0:  errno = ETIMEDOUT;
		case -1: return -1;
	}
	return IO_readSpec(h, buf, len);
}

static inline ssize_t IO_timedWriteSpec(Handler h, void* buf, size_t* len, int32_t to) {
	switch(IO_waitEvent(h, to)) {
		case 0:  errno = ETIMEDOUT;
		case -1: return -1;
	}
	return IO_writeSpec(h, buf, len);
}

#endif //_IO_H_
