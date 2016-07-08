#ifndef _XETN_H_
#define _XETN_H_

#include "handler.h"
#include "log.h"
#include "reactor.h"

#define XETN_VER_MAJ "0"
#define XETN_VER_MIN "1"

#define XETN_VER \
	"XETN/"XETN_VER_MAJ"."XETN_VER_MIN

#define LOGO \
"\e[1;32m"                        \
"  __  _______ _____ _   _ \n"    \
"  \\ \\/ / ____|_   _| \\ | |\n" \
"   \\  /|  _|   | | |  \\| |\n"  \
"   /  \\| |___  | | | |\\  |\n"  \
"  /_/\\_\\_____| |_| |_| \\_|\n" \
"\e[0m\n"

#define USAGE \
"Usage: xetn [-hv] [-c filename]\n\n"      \
"Options:\n"                               \
"  -h          : show help and exit\n"     \
"  -v          : show version and exit\n"  \
"  -c filename : set configuration file\n"

typedef struct xetn_conf {
	unsigned    nProcess;
	const char* listen;
	const char* logFile;
} XetnConf_t, *XetnConf;

typedef struct xetn_context {
	Logger_t    logger;
	handler_t   hsvr;
	XetnConf_t  conf;
	Reactor_t   loop;
	uint8_t     terminate;
} XetnContext_t, *XetnContext;

extern XetnContext_t xetnCtx;

#define Xetn_getContext() (xetnCtx)

#endif // _XETN_H_
