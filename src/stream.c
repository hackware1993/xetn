#include "stream.h"

#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>

iostate_t Stream_timedRead(Handler handler, Buffer buf, int32_t to) {
	struct timeval* timeout = NULL;
	struct timeval tv;
	if(to >= 0) {
		int sec  = to / 1000;
		int usec = (to - sec) * 1000;
		tv.tv_sec  = sec;
		tv.tv_usec = usec;
		timeout = &tv;
	}
	int fd = handler->fileno;
	int ret;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(fd, &set);
	ret = select(fd + 1, &set, NULL, NULL, timeout);
	if(ret == 0) {
		errno = ETIMEDOUT;
		return S_ERR;
	} else if(ret == -1) {
		return S_ERR;
	}
	return Stream_read(handler, buf);
}

iostate_t Stream_timedWrite(Handler handler, Buffer buf, int32_t to) {
	struct timeval* timeout = NULL;
	struct timeval tv;
	if(to >= 0) {
		int sec  = to / 1000;
		int usec = (to - sec) * 1000;
		tv.tv_sec  = sec;
		tv.tv_usec = usec;
		timeout = &tv;
	}
	int fd = handler->fileno;
	int ret;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(fd, &set);
	ret = select(fd + 1, NULL, &set, NULL, timeout);
	if(ret == 0) {
		errno = ETIMEDOUT;
		return S_ERR;
	} else if(ret == -1) {
		return S_ERR;
	}
	return Stream_write(handler, buf);
}

iostate_t Stream_read(Handler handler, Buffer buf) {
	fd_t fd = handler->fileno;
	char* ptr = Buffer_getPtr(buf) + Buffer_getLen(buf);
	size_t nleft = Buffer_getLim(buf) - Buffer_getLen(buf);
	ssize_t nread = 0;
	while(nleft > 0) {
		if((nread = read(fd, ptr, nleft)) < 0) {
			switch(errno) {
				case EINTR:
					nread = 0;
					break;
				case EAGAIN:
					Buffer_setLen(buf, Buffer_getLim(buf) - nleft);
					return S_PEND;
				default:
					return S_ERR;
			}
		} else if(nread == 0) {
			Buffer_setLen(buf, Buffer_getLim(buf) - nleft);
			return S_FIN;
		}
		ptr += nread;
		nleft -= nread;
	}
	Buffer_setLen(buf, Buffer_getLim(buf));
	return S_FULL;
}

iostate_t Stream_write(Handler handler, Buffer buf) {
	fd_t fd = handler->fileno;
	char* ptr = Buffer_getPtr(buf) + Buffer_getPos(buf);
	size_t nleft = Buffer_getLen(buf) - Buffer_getPos(buf);
	ssize_t nwritten = 0;
	while(nleft > 0) {
		if((nwritten = write(fd, ptr, nleft)) < 0) {
			switch(errno) {
				case EINTR:
					nwritten = 0;
					break;
				case EAGAIN:
					Buffer_setPos(buf, Buffer_getLen(buf) - nleft);
					return S_PEND;
				default:
					return S_ERR;
			}
		}
		ptr += nwritten;
		nleft -= nwritten;
	}
	Buffer_setPos(buf, Buffer_getLen(buf));
	return S_FIN;
}
