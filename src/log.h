#ifndef _LOG_H_
#define _LOG_H_

#include <pthread.h>

#include "handler.h"

typedef enum log_level {
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
} log_level_t;

typedef struct log {
	handler_t  server;
	handler_t  client;
	handler_t  logfile;
	pthread_t  tid;
	char*      data;
} log_t, *Log;

Log Log_init(Log);

Log Log_getClient(Log);
Log Log_getServer(Log);

void Log_close(Log);

void Log_record(Log, log_level_t, const char*, ...);

void Log_runServer(Log);

#endif // _LOG_H_
