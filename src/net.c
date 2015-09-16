#include "net.h"

#include <stdio.h>
#include <string.h>

#define SA_SIZE sizeof(struct sockaddr)

void sockaddr_get(struct sockaddr* addr, const char* ip, uint16_t port) {
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
const char* sockaddr_get_addr(struct sockaddr* addr) {
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

uint16_t sockaddr_get_port(struct sockaddr* addr) {
	uint16_t port;
	if(addr->sa_family == AF_INET) {
		port = (sockaddr_in*)addr->sin_port;
	} else {
		port = (sockaddr_in6*)addr->sin6_port;
	}
	return ntohs(port);
}

handler_t sock_create() {
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(ret == -1) {
		perror("sock_create");
		exit(EXIT_FAILURE);
	}
	return {fd, H_SOCK};
}

handler_t sock_accept(handler_t sock) {
	int fd = accept(sock.fileno, NULL, NULL);
	if(ret != 0) {
		perror("sock_accept");
		exit(EXIT_FAILURE);
	}
	return {fd, H_SOCK};
}

void sock_listen(handler_t sock, sockaddr_t addr) {
	int fd = sock.fileno;
	int ret = bind(fd, addr, SA_SIZE);
	if(ret != 0) {
		perror("sock_listen");
		exit(EXIT_FAILURE);
	}
	ret = listen(fd, SOMAXCONN);
}

void sock_connect(handler_t sock, sockaddr_t addr) {
	int ret = connect(sock.fileno, addr, SA_SIZE);
	if(ret != 0) {
		perror("sock_connect");
		exit(EXIT_FAILURE);
	}
}

