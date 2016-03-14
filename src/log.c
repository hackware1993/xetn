#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include "datetime.h"
#include "stream.h"
#include "log.h"
#include "io.h"

Log Log_init(Log logger) {
	int fd[2];
	if(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1) {
		perror("Log_init");
		exit(-1);
	}
	logger->server.fileno = fd[0];
	logger->client.fileno = fd[1];
	logger->server.type   = H_LOG;
	logger->client.type   = H_LOG;
	return logger;
}

Log Log_getClient(Log logger) {
	Handler_close(&logger->server);
	logger->server.fileno = -1;
	return logger;
}

Log Log_getServer(Log logger) {
	/* log server is also a client */
	//Handler_close(&logger->client);
	logger->data = (char*)calloc(1, 4096);
	int fd = open("./log/log.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
	if(fd == -1) {
		perror("Log_getServer");
		exit(-1);
	}
	logger->logfile.fileno = fd;
	logger->logfile.type   = H_FILE;
	return logger;
}

void Log_close(Log logger) {
	if(logger->server.fileno != -1) {
		Handler_close(&logger->client);
		/* terminate the daemon thread */
		pthread_join(logger->tid, NULL);
		Handler_close(&logger->server);
		Handler_close(&logger->logfile);
		free(logger->data);
	} else {
		Handler_close(&logger->client);
	}
}

void Log_record(Log logger, log_level_t level, const char* fmt, ...) {
	char buf[1024];
	va_list varg;
	va_start(varg, fmt);
	vsnprintf(buf, 1024, fmt, varg);
	va_end(varg);
	char item[2048];
	char dt[20];
	time_t tnow = time(NULL);
	DateTime_formatTimeStamp(&tnow, dt);
	dt[19] = '\0';
	char* lvstr;
	switch(level) {
		case LOG_INFO:  lvstr = "INFO";    break;
		case LOG_WARN:  lvstr = "WARNING"; break;
		case LOG_ERROR: lvstr = "ERROR";   break;
	}
	uint32_t len = snprintf(item, 2048, "[%s] %s - %s\n", lvstr, dt, buf);
	int n = 0;
	while(n < len) {
		n = IO_writeSpec(&logger->client, item, len);
		assert(n != -1);
	}
}

void* Log_main(void* arg) {
	Log logger = (Log)arg;
	char* buf = logger->data;
	uint32_t len   = 4096;
	uint32_t off   = 0;
	uint32_t nread = len;
	int n = 0;
	while(1) {
		n = IO_read(&logger->server, buf + off, nread);
		off   += n;
		nread -= n;
		if(nread == 0) {
			IO_writeSpec(&logger->logfile, buf, len);
			off   = 0;
			nread = len;
		}
		if(n == 0) {
			if(off) {
				IO_writeSpec(&logger->logfile, buf, off);
			}
			break;
		}
	}
	pthread_exit(0);
}

void Log_runServer(Log logger) {
	pthread_create(&logger->tid, NULL, Log_main, logger);
}

/*
 * Benmark: recording 1 million log items costs 3.33 seconds
#include <wait.h>

int main() {
	log_t logger;
	Log_init(&logger);
	int pid;
	if((pid = fork()) == 0) {
		Log client = Log_getClient(&logger);
		sleep(1);
		int c = 1000000;
		while(c--) {
			Log_record(client, LOG_INFO, "log message from %d", getpid());
		}
		Log_close(client);
		exit(0);
	}
	Log serv = Log_getServer(&logger);
	Log_record(serv, LOG_WARN, "log start");
	Log_runServer(serv);
	wait(NULL);
	Log_record(serv, LOG_WARN, "log end");
	Log_close(serv);
	return 0;
}
*/
