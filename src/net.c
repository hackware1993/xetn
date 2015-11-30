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

uint32_t SocketOption_getNonBlock(Handler sock) {
	fd_t fd = sock->fileno;
	int flags = fcntl(fd, F_GETFL, 0);
	error_exit(flags < 0,  SocketOption_getNonBlock);
	return flags & O_NONBLOCK;
}

uint32_t SockOption_getNoDelay(Handler sock) {
	fd_t fd = sock->fileno;
	uint32_t opt;
	socklen_t len = sizeof(opt);
	int32_t ret = getsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
			&opt, &len);
	error_exit(ret == -1, SocketOption_getNoDelay);
	return opt;
}

uint32_t SocketOption_getKeepAlive(Handler sock) {
	fd_t fd = sock->fileno;
	uint32_t opt;
	socklen_t len = sizeof(opt);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
			&opt, &len);
	error_exit(ret == -1, SocketOption_getKeepAlive);
	return opt;
}

uint32_t SocketOption_getReuseAddr(Handler sock) {
	fd_t fd = sock->fileno;
	uint32_t opt;
	socklen_t len = sizeof(opt);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
			&opt, &len);
	error_exit(ret == -1, SocketOption_getReuseAddr);
	return opt;
}

uint32_t SocketOption_getReusePort(Handler sock) {
	fd_t fd = sock->fileno;
	uint32_t opt;
	socklen_t len = sizeof(opt);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_REUSEPORT,
			&opt, &len);
	error_exit(ret == -1, SocketOption_getReusePort);
	return opt;
}

uint32_t SockOption_getSendBuf(Handler sock) {
	fd_t fd = sock->fileno;
	uint32_t size;
	socklen_t len = sizeof(size);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_SNDBUF,
			&size, &len);
	error_exit(ret == -1, SocketOption_getSendBuf);
	return size;
}

uint32_t SocketOption_getRecvBuf(Handler sock) {
	fd_t fd = sock->fileno;
	uint32_t size;
	socklen_t len = sizeof(size);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_RCVBUF,
			&size, &len);
	error_exit(ret == -1, SocketOption_getRecvBuf);
	return size;
}

int32_t SocketOption_setNonBlock(Handler sock, uint32_t opt) {
	fd_t fd = sock->fileno;
	int32_t flags = fcntl(fd, F_GETFL, 0);
	if(flags < 0) {
		return -1;
	}

	int32_t ret;
	if(opt != 0) {
		ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	} else {
		// TODO check here
		ret = fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
	}
	return ret;
}

int32_t SocketOption_setNoDelay(Handler sock, uint32_t opt) {
	fd_t fd = sock->fileno;
	int32_t ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
			&opt, (socklen_t)sizeof(opt));
	return ret;
}

int32_t SocketOption_setKeepAlive(Handler sock, uint32_t opt) {
	fd_t fd = sock->fileno;
	int32_t ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
			&opt, (socklen_t)sizeof(opt));
	return ret;
}

int32_t SocketOption_setReuseAddr(Handler sock, uint32_t opt) {
	fd_t fd = sock->fileno;
	int32_t ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
			&opt, (socklen_t)sizeof(opt));
	return ret;
}

int32_t SocketOption_setReusePort(Handler sock, uint32_t opt) {
	fd_t fd = sock->fileno;
	int32_t ret = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT,
			&opt, (socklen_t)sizeof(opt));
	return ret;
}

int32_t SocketOption_setSendBuf(Handler sock, uint32_t size) {
	fd_t fd = sock->fileno;
	socklen_t len = sizeof(size);
	int32_t ret = setsockopt(fd, SOL_SOCKET, SO_SNDBUF,
			&size, len);
	return ret;
}

int32_t SocketOption_setRecvBuf(Handler sock, uint32_t size) {
	fd_t fd = sock->fileno;
	socklen_t len = sizeof(size);
	int32_t ret = setsockopt(fd, SOL_SOCKET, SO_RCVBUF,
			&size, len);
	return ret;
}

#define SOCKOPT_MAP(XX) \
	XX(NonBlock)  \
	XX(KeepAlive) \
	XX(NoDelay)   \
	XX(ReuseAddr) \
	XX(ReusePort) \
	XX(SendBuf)   \
	XX(RecvBuf) 

typedef int32_t (*sockopt_cb)(Handler, uint32_t);

#define XX(x) { SocketOption_set##x },
PRIVATE struct {
	sockopt_cb callback;
} netopt_cb[] = {
	SOCKOPT_MAP(XX)
};
#undef XX

PRIVATE int32_t sock_option_handle(Handler sock, NetOption oplist) {
	NetOption p = oplist;
	int ret;
	while(p->type != NET_NULL) {
		ret = netopt_cb[p->type].callback(sock, p->opt);
		if(ret == -1) {
			return -1;
		}
		++p;
	}
	return 0;
}

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

/* internal sockaddr getting function */
PRIVATE SockAddr __sockaddr_get(SockAddr addr, __addr_t* url) {
	struct addrinfo hints;
	struct addrinfo* res;
	hints.ai_flags = AI_PASSIVE
	               | AI_NUMERICHOST
				   | AI_NUMERICSERV;
	hints.ai_family = AF_UNSPEC;
	if(url->type == 0) {
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
	} else {
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
	}
	int32_t stat = getaddrinfo(url->host, url->serv, &hints, &res);
	if(stat != 0) {
		fprintf(stderr, "sockaddr_get: %s\n", gai_strerror(stat));
		exit(EXIT_FAILURE);
	}
	*addr = *res->ai_addr;
	freeaddrinfo(res);
	return addr;
}

SockAddr SockAddr_get(SockAddr addr, const char* hs) {
	__addr_t taddr;
	__addr_t* ret = __addr_get(&taddr, hs);
	if(ret == NULL) {
		fprintf(stderr, "sockaddr_get: %s\n", "invalid addr");
		exit(EXIT_FAILURE);
	}

	SockAddr res = __sockaddr_get(addr, ret);
	__addr_free(ret);
	return res;
}

/* the return value MUST be free */
const char* SockAddr_getAddr(SockAddr addr) {
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

uint16_t SockAddr_getPort(SockAddr addr) {
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

Handler TcpServer_create(Handler sock, const char* url, NetOption oplist) {
	__addr_t addr;
	__addr_t* ret = __addr_get(&addr, url);
	if(ret == NULL) {
		fprintf(stderr, "TcpServer_create::url: %s\n", "invalid server url");
		exit(EXIT_FAILURE);
	}

	sockaddr_t sa;
	SockAddr psa = __sockaddr_get(&sa, ret);
	
	int32_t fd = socket(psa->sa_family, 
			(ret->type == 0) ? SOCK_STREAM : SOCK_DGRAM,
			(ret->type == 0) ? IPPROTO_TCP : IPPROTO_UDP);
	error_exit(fd == -1, TcpServer_create::socket);
	__addr_free(ret);

	sock->fileno = fd;
	sock->type = H_SOCK;
	int32_t stat;
	if(oplist != NULL) {
		stat = sock_option_handle(sock, oplist);
		error_exit(stat == -1, TcpServer_create::netoption);
	}

	stat = bind(fd, psa, sizeof(struct sockaddr));
	error_exit(stat == -1, TcpServer_create::bind);
	stat = listen(fd, SOMAXCONN);
	error_exit(stat == -1, TcpServer_create::listen);

	return sock;
}

Handler TcpClient_create(Handler sock, const char* url, NetOption oplist) {
	__addr_t addr;
	__addr_t* ret = __addr_get(&addr, url);
	if(ret == NULL) {
		fprintf(stderr, "TcpClient_create::url: %s\n", "invalid server url");
		exit(EXIT_FAILURE);
	}

	sockaddr_t sa;
	SockAddr psa = __sockaddr_get(&sa, ret);
	
	int32_t fd = socket(psa->sa_family, 
			(ret->type == 0) ? SOCK_STREAM : SOCK_DGRAM,
			(ret->type == 0) ? IPPROTO_TCP : IPPROTO_UDP);
	error_exit(fd == -1, tcp_client_create::socket);
	__addr_free(ret);

	sock->fileno = fd;
	sock->type = H_SOCK;
	int32_t stat;
	if(oplist != NULL) {
		stat = sock_option_handle(sock, oplist);
		error_exit(stat == -1, TcpClient_create:netoption);
	}

	stat = connect(fd, psa, sizeof(sockaddr_t));
	error_exit(stat == -1, TcpClient_create::connect);

	return sock;
}

Handler TcpServer_accept(Handler sock, Handler target, NetOption oplist) {
	fd_t fd = accept(sock->fileno, NULL, NULL);
	if(fd == -1) {
		/* EAGAIN MUST be chcked under nonblocking mode */
		if(errno == EAGAIN) {
			return NULL;
		}
                              
		error_exit(1, TcpServer_accept::accept);
	}
	target->fileno = fd;
	target->type = H_SOCK;
	if(oplist != NULL) {
		int stat = sock_option_handle(target, oplist);
		error_exit(stat == -1, TcpServer_accept:netoption);
	}
	return target;
}

Handler Socket_create(Handler target) {
	int32_t fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	error_exit(fd == -1, sock_create);
	target->fileno = fd;
	target->type = H_SOCK;
	return target;
}

/* repeat until getting NULL under nonblocking mode */
Handler Socket_accept(Handler sock, Handler target) {
	fd_t fd = accept(sock->fileno, NULL, NULL);
	if(fd == -1) {
		/* EAGAIN MUST be chcked under nonblocking mode */
		if(errno == EAGAIN) {
			return NULL;
		}
                              
		error_exit(1, Socket_accept);
	}
	target->fileno = fd;
	target->type = H_SOCK;
	return target;
}

void Socket_listen(Handler sock, SockAddr addr) {
	fd_t fd = sock->fileno;
	int32_t ret = bind(fd, addr, sizeof(struct sockaddr));
	error_exit(ret == -1, sock_listen);
	ret = listen(fd, SOMAXCONN);
	error_exit(ret == -1, Socket_listen);
}

void Socket_connect(Handler sock, SockAddr addr) {
	int32_t ret = connect(sock->fileno, addr, sizeof(struct sockaddr));
	error_exit(ret == -1, Socket_connect);
}

void Socket_getAddr(Handler sock, SockAddr addr) {
	socklen_t len = sizeof(sockaddr_t);
	int32_t ret = getsockname(sock->fileno, addr, &len);
	error_exit(ret == -1, Socket_getAddr);
}

int32_t Socket_getErrno(Handler sock) {
	fd_t fd = sock->fileno;
	int32_t res;
	socklen_t len = sizeof(res);
	int32_t ret = getsockopt(fd, SOL_SOCKET, SO_ERROR,
			&res, &len);
	error_exit(ret == -1, Socket_getErrno);
	return res;
}

