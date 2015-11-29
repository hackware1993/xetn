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
} netoption_t, *NetOption;

typedef struct sockaddr sockaddr_t;

typedef sockaddr_t* SockAddr;

Handler TcpServer_create(Handler, const char*, NetOption);
Handler TcpServer_accept(Handler, Handler, NetOption);
Handler TcpClient_create(Handler, const char*, NetOption);

Handler Socket_create(Handler);
Handler Socket_accept(Handler, Handler);

void    Socket_listen(Handler, SockAddr);
void    Socket_connect(Handler, SockAddr);
void    Socket_getAddr(Handler, SockAddr);
int32_t Socket_getErrno(Handler);

uint32_t SocketOption_getNonBlock(Handler);
uint32_t SocketOption_getNoDelay(Handler);
uint32_t SocketOption_getKeepAlive(Handler);
uint32_t SocketOption_getReuseAddr(Handler);
uint32_t SocketOption_getSendBuf(Handler);
uint32_t SocketOption_getRecvBuf(Handler);

int32_t SocketOption_setNonBlock(Handler, uint32_t);
int32_t SocketOption_setNoDelay(Handler, uint32_t);
int32_t SocketOption_setKeepAlive(Handler, uint32_t);
int32_t SocketOption_setReuseAddr(Handler, uint32_t);
int32_t SocketOption_setSendBuf(Handler, uint32_t);
int32_t SocketOption_setRecvBuf(Handler, uint32_t);

SockAddr    SockAddr_get(SockAddr, const char*);
const char* SockAddr_getAddr(SockAddr);
uint16_t    SockAddr_getPort(SockAddr);

#endif // _NET_H_
