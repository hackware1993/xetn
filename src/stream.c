#include "stream.h"

#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

iostate_t sockstream_read(handler_t sock, buffer_t buf) {
	fd_t fd = sock->fileno;
	char* ptr = buffer_get_ptr(buf) + buffer_get_len(buf);
	size_t nleft = buffer_get_lim(buf) - buffer_get_len(buf);
	ssize_t nread = 0;
	while(nleft > 0) {
		if((nread = read(fd, ptr, nleft)) < 0) {
			switch(errno) {
				case EINTR:
					nread = 0;
					break;
				case EAGAIN:
					buffer_set_len(buf, buf_get_lim(buf) - nleft);
					return S_PEND;
				default:
					return S_ERR;
			}
		} else if(nread == 0) {
			buffer_set_len(buf, buffer_get_lim(buf) - nleft);
			return S_FIN;
		}
		ptr += nread;
		nleft -= nread;
	}
	buffer_set_len(buf, buffer_get_lim(buf));
	return S_FULL;
}

iostate_t sockstream_write(handler_t sock, buffer_t buf) {
	fd_t fd = sock->fileno;
	char* ptr = buffer_get_ptr(buf) + buffer_get_pos(buf);
	size_t nleft = buffer_get_len(buf) - buffer_get_pos(buf);
	ssize_t nwritten = 0;
	while(nleft > 0) {
		if((nwritten = write(fd, ptr, nleft)) < 0) {
			switch(errno) {
				case EINTR:
					nwritten = 0;
					break;
				case EAGAIN:
					buffer_set_pos(buf, buffer_get_len(buf) - nleft);
					return S_PEND;
				default:
					return S_ERR;
			}
		}
		ptr += nwritten;
		nleft -= nwritten;
	}
	buffer_set_pos(buf, buffer_get_len(buf));
	return S_FIN;
}
