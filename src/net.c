#include "net.h"
#include "error.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#define PRIVATE static
#define TCP 0x3a706374
#define UDP 0x3a706475
#define SLSL 0x2f2f

typedef struct __addr {
	unsigned type;
	char*    host;
	char*    serv;
} __addr_t;

#define __addr_free(a) free((a)->host)

PRIVATE __addr_t* __addr_get(__addr_t* res, const char* addr) {
	const char* p = addr;
	int type = *(int*)p;
	switch(type) {
		case TCP:
			res->type = 0;
			break;
		case UDP:
			res->type = 1;
			break;
		default:
			return NULL;
	}
	p += 4;
	short ss = *(short*)p;
	if(ss != SLSL) {
		return NULL;
	}
	p += 2;
	int len = strlen(p);
	int i = len - 1;
	while(i > -1 && p[i] != ':') {
		--i;
	}
	if(i == -1) {
		return NULL;
	}
	char* temp = (char*)malloc(len + 1);
	strcpy(temp, p);
	temp[i] = '\0';
	res->host = temp;
	res->serv = temp + i + 1;
	return res;
}

SockAddr sockaddr_get(SockAddr addr, const char* hs) {
	__addr_t taddr;
	__addr_t* ret = __addr_get(&taddr, hs);
	if(ret == NULL) {
		fprintf(stderr, "sockaddr_get: %s\n", "invalid addr");
		exit(EXIT_FAILURE);
	}

	struct addrinfo hints;
	struct addrinfo* res;
	hints.ai_flags = AI_PASSIVE
	               | AI_NUMERICHOST
				   | AI_NUMERICSERV;
	hints.ai_family = AF_UNSPEC;
	if(ret->type == 0) {
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
	} else {
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
	}
	//char serv[6]; // 65535 upmost 5B + 1B(\0)
	//sprintf(serv, "%hu", port);
	int32_t stat = getaddrinfo(ret->host, ret->serv, &hints, &res);
	if(stat != 0) {
		fprintf(stderr, "sockaddr_get: %s\n", gai_strerror(stat));
		exit(EXIT_FAILURE);
	}
	*addr = *res->ai_addr;
	freeaddrinfo(res);
	__addr_free(ret);
	return addr;
}

/* the return value MUST be free */
const char* sockaddr_get_addr(SockAddr addr) {
	const char* paddr = NULL;
	if(addr->sa_family == AF_INET) {
		struct sockaddr_in* taddr = (struct sockaddr_in*)addr;
		char caddr[INET_ADDRSTRLEN];
		paddr = inet_ntop(AF_INET, &taddr->sin_addr, caddr, INET_ADDRSTRLEN);
	} else {
		struct sockaddr_in6* taddr = (struct sockaddr_in6*)addr;
		char caddr[INET6_ADDRSTRLEN];
		paddr = inet_ntop(AF_INET6, &taddr->sin6_addr, caddr, INET6_ADDRSTRLEN);
	}
	error_exit(paddr == NULL, sockaddr_get_addr);
	return strdup(paddr);
}

uint16_t sockaddr_get_port(SockAddr addr) {
	uint16_t port;
	if(addr->sa_family == AF_INET) {
		struct sockaddr_in* taddr = (struct sockaddr_in*)addr;
		port = taddr->sin_port;
	} else {
		struct sockaddr_in6* taddr = (struct sockaddr_in6*)addr;
		port = taddr->sin6_port;
	}
	return ntohs(port);
}

Handler sock_create(Handler target) {
	int32_t fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	error_exit(fd == -1, sock_create);
	target->fileno = fd;
	target->type = H_SOCK;
	return target;
}

/* repeat until getting NULL under nonblocking mode */
Handler sock_accept(Handler sock, Handler target) {
	fd_t fd = accept(sock->fileno, NULL, NULL);
	if(fd == -1) {
		/* EAGAIN MUST be chcked under nonblocking mode */
		if(errno == EAGAIN) {
			return NULL;
		}
                              
		error_exit(1, sock_accept);
	}
	target->fileno = fd;
	target->type = H_SOCK;
	return target;
}

void sock_listen(Handler sock, SockAddr addr) {
	fd_t fd = sock->fileno;
	int32_t ret = bind(fd, addr, sizeof(struct sockaddr));
	error_exit(ret == -1, sock_listen);
	ret = listen(fd, SOMAXCONN);
}

void sock_connect(Handler sock, SockAddr addr) {
	int32_t ret = connect(sock->fileno, addr, sizeof(struct sockaddr));
	error_exit(ret == -1, sock_connect);
}

void sock_get_addr(Handler sock, SockAddr addr) {
	socklen_t len = sizeof(sockaddr_t);
	int32_t ret = getsockname(sock->fileno, addr, &len);
	error_exit(ret == -1, sock_get_addr);
}

int32_t sock_get_errno(Handler sock) {
	fd_t fd = sock->fileno;
	int32_t res;
	socklen_t len = sizeof(res);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_ERROR,
			&res, &len);
	error_exit(ret == -1, sock_get_errno);
	return res;
}

uint32_t sock_get_nonblock(Handler sock) {
	fd_t fd = sock->fileno;
	int flags = fcntl(fd, F_GETFL, 0);
	error_exit(flags < 0,  sock_get_nonblock);
	return flags & O_NONBLOCK;
}

void sock_set_nonblock(Handler sock, uint32_t opt) {
	fd_t fd = sock->fileno;
	int flags = fcntl(fd, F_GETFL, 0);
	error_exit(flags < 0,  sock_set_nonblock);

	int ret;
	if(opt != 0) {
		ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	} else {
		// TODO check here
		ret = fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
	}
	error_exit(ret == -1,  sock_set_nonblock);
}

uint32_t sock_get_keepalive(Handler sock) {
	fd_t fd = sock->fileno;
	uint32_t opt;
	socklen_t len = sizeof(opt);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
			&opt, &len);
	error_exit(ret == -1, sock_get_keepalive);
	return opt;
}

void sock_set_keepalive(Handler sock, uint32_t opt) {
	fd_t fd = sock->fileno;
	int32_t ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
			&opt, (socklen_t)sizeof(opt));
	error_exit(ret == -1, sock_set_keepalive);
}

uint32_t sock_get_sendbuf(Handler sock) {
	fd_t fd = sock->fileno;
	uint32_t size;
	socklen_t len = sizeof(size);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_SNDBUF,
			&size, &len);
	error_exit(ret == -1, sock_get_sendbuf);
	return size;
}

void sock_set_sendbuf(Handler sock, uint32_t size) {
	fd_t fd = sock->fileno;
	socklen_t len = sizeof(size);
	int32_t ret = setsockopt(fd, SOL_SOCKET, SO_SNDBUF,
			&size, len);
	error_exit(ret == -1, sock_set_sendbuf);
}

uint32_t sock_get_recvbuf(Handler sock) {
	fd_t fd = sock->fileno;
	uint32_t size;
	socklen_t len = sizeof(size);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_RCVBUF,
			&size, &len);
	error_exit(ret == -1, sock_get_recvbuf);
	return size;
}

void sock_set_recvbuf(Handler sock, uint32_t size) {
	fd_t fd = sock->fileno;
	socklen_t len = sizeof(size);
	int32_t ret = setsockopt(fd, SOL_SOCKET, SO_RCVBUF,
			&size, len);
	error_exit(ret == -1, sock_set_recvbuf);
}

void sock_set_reuseaddr(Handler sock, uint32_t opt) {
	fd_t fd = sock->fileno;
	int32_t ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
			&opt, (socklen_t)sizeof(opt));
	error_exit(ret == -1, sock_set_reuseaddr);
}

uint32_t sock_get_reuseaddr(Handler sock) {
	fd_t fd = sock->fileno;
	uint32_t opt;
	socklen_t len = sizeof(opt);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
			&opt, &len);
	error_exit(ret == -1, sock_get_reuseaddr);
	return opt;
}

void sock_set_nodelay(Handler sock, uint32_t opt) {
	fd_t fd = sock->fileno;
	int32_t ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
			&opt, (socklen_t)sizeof(opt));
	error_exit(ret == -1, sock_set_nodelay);
}

uint32_t sock_get_nodelay(Handler sock) {
	fd_t fd = sock->fileno;
	uint32_t opt;
	socklen_t len = sizeof(opt);
	int32_t ret = getsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
			&opt, &len);
	error_exit(ret == -1, sock_get_nodelay);
	return opt;
}
