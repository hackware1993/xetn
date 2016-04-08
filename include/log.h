#ifndef _LOG_H_
#define _LOG_H_

#include <pthread.h>

#include "handler.h"

#define LOG_LV(XX) \
	XX(LOG_INFO , "[INFO]"   , 6) \
	XX(LOG_WARN , "[WARNING]", 9) \
	XX(LOG_ERROR, "[ERROR]"  , 7)

#define XX(a, b, c) a,
typedef enum log_level {
	LOG_LV(XX)
} log_level_t;
#undef XX

typedef struct log {
	handler_t  server;
	handler_t  client;
	handler_t  logfile;
	pthread_t  tid;
	char*      data;
	unsigned   len;
} log_t, *Log;

Log Log_init(Log);

Log Log_getClient(Log);
Log Log_getServer(Log, const char*);

void Log_close(Log);

void Log_record(Log, log_level_t, const char*, ...);

void Log_runServer(Log);

#endif // _LOG_H_
