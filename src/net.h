#ifndef _NET_H_
#define _NET_H_

#include "handler.h"

#include <stdint.h>
/* provide sockaddr symbol */
#include <sys/socket.h>

/* these enum represents the type of NetOption */
typedef enum netop {
	NET_NONBLOCK,
	NET_KEEPALIVE,
	NET_NODELAY,
	NET_REUSEADDR,
	NET_SENDBUF,
	NET_RECVBUF,
	NET_NULL,
} netop_t;

typedef struct netoption {
	netop_t type;
	uint32_t opt;
} netoption_t;

typedef netoption_t* NetOption;

typedef struct sockaddr sockaddr_t;

typedef sockaddr_t* SockAddr;

Handler tcp_server_create(Handler, const char*, NetOption);
Handler tcp_server_accept(Handler, Handler, NetOption);
Handler tcp_client_create(Handler, const char*, NetOption);

Handler sock_create(Handler);

Handler sock_accept(Handler, Handler);

void sock_listen(Handler, SockAddr);

void sock_connect(Handler, SockAddr);

void sock_get_addr(Handler, SockAddr);

int32_t sock_get_errno(Handler);

uint32_t sock_get_nonblock(Handler);
uint32_t sock_get_nodelay(Handler);
uint32_t sock_get_keepalive(Handler);
uint32_t sock_get_reuseaddr(Handler);
uint32_t sock_get_sendbuf(Handler);
uint32_t sock_get_recvbuf(Handler);

int32_t sock_set_nonblock(Handler, uint32_t);
int32_t sock_set_nodelay(Handler, uint32_t);
int32_t sock_set_keepalive(Handler, uint32_t);
int32_t sock_set_reuseaddr(Handler, uint32_t);
int32_t sock_set_sendbuf(Handler, uint32_t);
int32_t sock_set_recvbuf(Handler, uint32_t);

SockAddr sockaddr_get(SockAddr, const char*);

const char* sockaddr_get_addr(SockAddr);

uint16_t sockaddr_get_port(SockAddr);

#endif // _NET_H_
