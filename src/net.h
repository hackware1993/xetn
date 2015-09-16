#ifndef _NET_H_
#define _NET_H_

#include "handler.h"

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef sockaddr *sockaddr_t;

handler_t sock_create();

handler_t sock_accept(handler_t);

void sock_listen(handler_t, sockaddr_t);

void sock_connect(handler_t, sockaddr_t);

void sockaddr_get(sockaddr_t, const char*, uint16_t);

const char* sockaddr_get_addr(sockaddr_t);

uint16_t sockaddr_get_port(sockaddr_t);

#endif // _NET_H_
