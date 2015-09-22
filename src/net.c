#include "net.h"


#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

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
	int ret = getaddrinfo(ip, serv, &hints, & res);
	if(ret != 0) {
		perror("sock_addr_get");
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
	if(paddr == 0) {
		perror("sock_addr_get_ip");
		exit(EXIT_FAILURE);
	}
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
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(ret == -1) {
		perror("sock_create");
		exit(EXIT_FAILURE);
	}
	sock->fileno = fd;
	sock->type = H_SOCK;
}

/* repeat until getting NULL under nonblocking mode */
handler_t sock_accept(handler_t sock, handler_t target) {
	int fd = accept(sock.fileno, NULL, NULL);
	if(ret == -1) {
		/* EAGAIN MUST be chcked under nonblocking mode */
		if(errno == EAGAIN) {
			return NULL;
		}
		perror("sock_accept");
		exit(EXIT_FAILURE);
	}
	target->fileno = fd;
	target->type = H_SOCK;
	return target;
}

void sock_listen(handler_t sock, sockaddr_t addr) {
	int fd = sock->fileno;
	int ret = bind(fd, addr, sizeof(struct sockaddr));
	if(ret != 0) {
		perror("sock_listen");
		exit(EXIT_FAILURE);
	}
	ret = listen(fd, SOMAXCONN);
}

void sock_connect(handler_t sock, sockaddr_t addr) {
	int ret = connect(sock->fileno, addr, sizeof(struct sockaddr));
	if(ret == -1) {
		perror("sock_connect");
		exit(EXIT_FAILURE);
	}
}

void sock_get_addr(handler_t sock, sockaddr_t addr) {
	int ret = getsockname(sock->fileno, addr, sizeof(struct sockaddr));
	if(ret == -1) {
		perror("sock_get_addr");
		exit(EXIT_FAILURE);
	}
}
