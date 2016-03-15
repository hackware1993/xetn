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

#define IO_write(H, buf, len) write((H)->fileno, buf, len)
#define IO_read(H, buf, len)  read((H)->fileno, buf, len)

static inline ssize_t IO_writeSpec(Handler h, void* buf, size_t len) {
	int n = 0;
	size_t nleft = len;
	while(nleft) {
		n = IO_write(h, buf, nleft);
		if(n == -1) {
			if(errno == EAGAIN) {
				return len - nleft;
			}
			return -1;
		}
		buf   += n;
		nleft -= n;
	}
	return len;
}

static inline ssize_t IO_readSpec(Handler h, void* buf, size_t len) {
	int n = 0;
	size_t nleft = len;
	while(nleft) {
		n = IO_read(h, buf, nleft);
		if(n == -1) {
			if(errno == EAGAIN) {
				return len - nleft;
			}
			return -1;
		}
		buf   += n;
		nleft -= n;
	}
	return len;
}

static inline int IO_waitEvent(Handler h, uint32_t to) {
	struct timeval* timeout = NULL;
	struct timeval tv;
	if(to >= 0) {
		int sec  = to / 1000;
		int usec = (to - sec) * 1000;
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

static inline ssize_t IO_timedRead(Handler h, void* buf, size_t len, uint32_t to) {
	switch(IO_waitEvent(h, to)) {
		case 0:  errno = ETIMEDOUT;
		case -1: return -1;
	}
	return IO_read(h, buf, len);
} 
static inline ssize_t IO_timedWrite(Handler h, void* buf, size_t len, uint32_t to) {
	switch(IO_waitEvent(h, to)) {
		case 0:  errno = ETIMEDOUT;
		case -1: return -1;
	}
	return IO_write(h, buf, len);
}

static inline ssize_t IO_timedReadSpec(Handler h, void* buf, size_t len, uint32_t to) {
	switch(IO_waitEvent(h, to)) {
		case 0:  errno = ETIMEDOUT;
		case -1: return -1;
	}
	return IO_readSpec(h, buf, len);
}

static inline ssize_t IO_timedWriteSpec(Handler h, void* buf, size_t len, uint32_t to) {
	switch(IO_waitEvent(h, to)) {
		case 0:  errno = ETIMEDOUT;
		case -1: return -1;
	}
	return IO_writeSpec(h, buf, len);
}

#endif //_IO_H_
