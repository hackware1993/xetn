#ifndef _NET_H_
#define _NET_H_

#include "handler.h"

#include <stdint.h>
/* provide sockaddr symbol */
#include <sys/socket.h>

typedef struct sockaddr sockaddr_t;

typedef sockaddr_t* SockAddr;

Handler sock_create(Handler);

Handler sock_accept(Handler, Handler);

void sock_listen(Handler, SockAddr);

void sock_connect(Handler, SockAddr);

void sock_get_addr(Handler, SockAddr);

int32_t sock_get_errno(Handler);

uint32_t sock_get_nonblock(Handler);

void sock_set_nonblock(Handler, uint32_t);

uint32_t sock_get_keepalive(Handler);

void sock_set_keepalive(Handler, uint32_t);

uint32_t sock_get_reuseaddr(Handler);

void sock_set_reuseaddr(Handler, uint32_t);

uint32_t sock_get_nodelay(Handler);

void sock_set_nodelay(Handler, uint32_t);

uint32_t sock_get_sendbuf(Handler);

void sock_set_sendbuf(Handler, uint32_t);

uint32_t sock_get_recvbuf(Handler);

void sock_set_recvbuf(Handler, uint32_t);

SockAddr sockaddr_get(SockAddr, const char*, uint16_t);

const char* sockaddr_get_addr(SockAddr);

uint16_t sockaddr_get_port(SockAddr);

#endif // _NET_H_
