#ifndef _NET_H_
#define _NET_H_

#include "handler.h"

#include <stdint.h>
/* provide sockaddr symbol */
#include <sys/socket.h>

typedef sockaddr *sockaddr_t;

void sock_create(handler_t);

handler_t sock_accept(handler_t, handler_t);

void sock_listen(handler_t, sockaddr_t);

void sock_connect(handler_t, sockaddr_t);

void sock_get_addr(handler_t, sockaddr_t);

int32_t sock_get_errno(handler_t);

uint32_t sock_get_nonblock(handler_t);

void sock_set_nonblock(handler_t, uint32_t);

uint32_t sock_get_keepalive(handler_t);

void sock_set_keepalive(handler_t, uint32_t);

uint32_t sock_get_reuseaddr(handler_t);

void sock_set_reuseaddr(handler_t, uint32_t);

uint32_t sock_get_nodelay(handler_t);

void sock_set_nodelay(handler_t, uint32_t);

uint32_t sock_get_sendbuf(handler_t);

void sock_set_sendbuf(handler_t, uint32_t);

uint32_t sock_get_recvbuf(handler_t);

void sock_set_recvbuf(handler_t, uint32_t);

void sockaddr_get(sockaddr_t, const char*, uint16_t);

const char* sockaddr_get_addr(sockaddr_t);

uint16_t sockaddr_get_port(sockaddr_t);

#endif // _NET_H_
