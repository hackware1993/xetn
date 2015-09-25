#include "net.h"
#include "error.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

void sockaddr_get(sockaddr_t addr, const char* ip, uint16_t port) {
	struct addrinfo hints;
	struct addrinfo* res;
	hints.ai_flags = AI_PASSIVE
	               | AI_NUMERICHOST
				   | AI_NUMERICSERV;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	char serv[6]; // 65535 upmost 5B + 1B(\0)
	sprintf(serv, "%hu", port);
	int32_t ret = getaddrinfo(ip, serv, &hints, & res);
	if(ret != 0) {
		fprintf(strerr, "sockaddr_get: %s\n", gai_strerror(ret));
		exit(EXIT_FAILURE);
	}
	*addr = *res.ai_addr;
	freeaddrinfo(result);
}

/* the return value MUST be free */
const char* sockaddr_get_addr(sockaddr_t addr) {
	const char* paddr = NULL;
	if(addr->sa_family == AF_INET) {
		struct sockaddr_in* taddr = (struct sockaddr_in*)addr;
		char caddr[INET_ADDRSTRLEN];
		paddr = inet_ntop(AF_INET, &addr->sin_addr, caddr, INET_ADDRSTRLEN);
	} else {
		struct sockaddr_in6* taddr = (struct sockaddr_in6*)addr;
		paddr = inet_ntop(AF_INET6, &addr->sin6_addr, caddr, INET6_ADDRSTRLEN);
	}
	error_exit(paddr == NULL, sockaddr_get_addr);
	return strdup(paddr);
}

uint16_t sockaddr_get_port(sockaddr_t addr) {
	uint16_t port;
	if(addr->sa_family == AF_INET) {
		port = (sockaddr_in*)addr->sin_port;
	} else {
		port = (sockaddr_in6*)addr->sin6_port;
	}
	return ntohs(port);
}

void sock_create(handler_t sock) {
	int32_t fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	error_exit(fd == -1, sock_create);
	*sock = handler_create(fd, H_SOCK);
}

/* repeat until getting NULL under nonblocking mode */
handler_t sock_accept(handler_t sock, handler_t target) {
	fd_t fd = accept(sock->fileno, NULL, NULL);
	if(fd == -1) {
		/* EAGAIN MUST be chcked under nonblocking mode */
		if(errno == EAGAIN) {
			return NULL;
		}
		error_exit(true, sock_accept);
	}
	*target = handler_create(fd, H_SOCK);
	return target;
}

void sock_listen(handler_t sock, sockaddr_t addr) {
	fd_t fd = sock->fileno;
	int32_t ret = bind(fd, addr, sizeof(struct sockaddr));
	error_exit(ret == -1, sock_listen);
	ret = listen(fd, SOMAXCONN);
}

void sock_connect(handler_t sock, sockaddr_t addr) {
	int32_t ret = connect(sock->fileno, addr, sizeof(struct sockaddr));
	error_exit(ret == -1, sock_connect);
}

void sock_get_addr(handler_t sock, sockaddr_t addr) {
	int32_t ret = getsockname(sock->fileno, addr, sizeof(struct sockaddr));
	error_exit(ret == -1, sock_get_addr);
}

int32_t sock_get_errno(handler_t sock) {
	fd_t fd = sock->fileno;
	int32_t res;
	socklen_t len = sizeof(res);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_ERROR,
			&res, &len);
	error_exit(ret == -1, sock_get_errno);
	return res;
}

uint32_t sock_get_nonblock(handler_t sock) {
	fd_t fd = sock->fileno;
	int flags = fcntl(fd, F_GETFL, 0);
	error_exit(ret < 0,  sock_get_nonblock);
	return flags & O_NONBLOCK;
}

void sock_set_nonblock(handler_t sock, uint32_t opt) {
	fd_t fd = sock->fileno;
	int flags = fcntl(fd, F_GETFL, 0);
	error_exit(ret < 0,  sock_set_nonblock);

	int ret;
	if(opt != 0) {
		ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	} else {
		// TODO check here
		ret = fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
	}
	error_exit(ret == -1,  sock_set_nonblock);
}

uint32_t sock_get_keepalive(handler_t sock) {
	fd_t fd = sock->fileno;
	uint32_t opt;
	socklen_t len = sizeof(opt);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
			&opt, &len);
	error_exit(ret == -1, sock_get_keepalive);
	return opt;
}

void sock_set_keepalive(handler_t sock, uint32_t opt) {
	fd_t fd = sock->fileno;
	int32_t ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
			&opt, (socklen_t)sizeof(opt));
	error_exit(ret == -1, sock_set_keepalive);
}

uint32_t sock_get_sendbuf(handler_t sock) {
	fd_t fd = sock->fileno;
	uint32_t size;
	socklen_t len = sizeof(size);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_SNDBUF,
			&size, &len);
	error_exit(ret == -1, sock_get_sendbuf);
	return size;
}

void sock_set_sendbuf(handler_t sock, uint32_t size) {
	fd_t fd = sock->fileno;
	uint32_t size;
	socklen_t len = sizeof(size);
	int32_t ret = setsockopt(fd, SOL_SOCKET, SO_SNDVBUF,
			&size, len);
	error_exit(ret == -1, sock_set_sendbuf);
}

uint32_t sock_get_recvbuf(handler_t sock) {
	fd_t fd = sock->fileno;
	uint32_t size;
	socklen_t len = sizeof(size);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_RCVBUF,
			&size, &len);
	error_exit(ret == -1, sock_get_recvbuf);
	return size;
}

void sock_set_recvbuf(handler_t sock, uint32_t size) {
	fd_t fd = sock->fileno;
	uint32_t size;
	socklen_t len = sizeof(size);
	int32_t ret = setsockopt(fd, SOL_SOCKET, SO_RCVBUF,
			&size, len);
	error_exit(ret == -1, sock_set_recvbuf);
}

void sock_set_reuseaddr(handler_t sock, uint32_t opt) {
	fd_t fd = sock->fileno;
	int32_t ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
			&opt, (socklen_t)sizeof(opt));
	error_exit(ret == -1, sock_set_reuseaddr);
}

uint32_t sock_get_reuseaddr(handler_t sock) {
	fd_t fd = sock->fileno;
	uint32_t opt;
	socklen_t len = sizeof(opt);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
			&opt, &len);
	error_exit(ret == -1, sock_get_reuseaddr);
	return opt;
}

void sock_set_nodelay(handler_t sock, uint32_t opt) {
	fd_t fd = sock->fileno;
	int32_t ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
			&opt, (socklen_t)sizeof(opt));
	error_exit(ret == -1, sock_set_nodelay);
}

uint32_t sock_get_nodelay(handler_t sock) {
	fd_t fd = sock->fileno;
	uint32_t opt;
	socklen_t len = sizeof(opt);
	int32_t ret = getsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
			&opt, &len);
	error_exit(ret == -1, sock_get_nodelay);
	return opt;
}
