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
#include "common.h"

#define LOG_BUF_LEN 4096
#define LOG_MSG_LEN 2000

#define XX(a, b, c) b,
const char* LOG_STR[] = {
	LOG_LV(XX)
};
#undef XX

#define XX(a, b, c) c,
const uint8_t LOG_STR_LEN[] = {
	LOG_LV(XX)
};
#undef XX

//PRIVATE inline int Log_persist(Log logger) {
//	return 0;
//}
//
//Log Log_init(Log logger) {
//	int fd[2];
//	if(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1) {
//		perror("Log_init");
//		exit(-1);
//	}
//	logger->server.fileno = fd[0];
//	logger->client.fileno = fd[1];
//	logger->server.type   = H_LOG;
//	logger->client.type   = H_LOG;
//	return logger;
//}
//
//Log Log_getClient(Log logger) {
//	Handler_close(&logger->server);
//	logger->server.fileno = -1;
//	return logger;
//}
//
//Log Log_getServer(Log logger, const char* filename) {
//	/* log server is also a client */
//	//Handler_close(&logger->client);
//	logger->data = (char*)calloc(1, LOG_BUF_LEN);
//	int fd = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0644);
//	if(fd == -1) {
//		perror("Log_getServer");
//		exit(-1);
//	}
//	logger->logfile.fileno = fd;
//	logger->logfile.type   = H_FILE;
//	return logger;
//}
//
//void Log_close(Log logger) {
//	if(logger->server.fileno != -1) {
//		Handler_close(&logger->client);
//		/* terminate the daemon thread */
//		pthread_join(logger->tid, NULL);
//		Handler_close(&logger->server);
//		Handler_close(&logger->logfile);
//		free(logger->data);
//	} else {
//		Handler_close(&logger->client);
//	}
//}
//
//void Log_record(Log logger, log_level_t level, const char* fmt, ...) {
//	char buf[2048];
//	char* p = buf;
//	uint8_t ls = LOG_STR_LEN[level];
//	strncpy(p, LOG_STR[level], ls);
//	p += ls;
//	*p++ = ' ';
//	time_t tnow = time(NULL);
//	DateTime_formatTimeStamp(&tnow, p);
//	p += TIMESTAMP_LEN - 1;
//	/* separator "%s - %s" */
//	*p++ = ' '; *p++ = '-'; *p++ = ' ';
//	va_list varg;
//	va_start(varg, fmt);
//	vsnprintf(p, 2000, fmt, varg);
//	va_end(varg);
//	size_t len = strlen(buf);
//	buf[len++] = '\n';
//	int8_t ret = IO_writeSpec(&logger->client, buf, &len);
//	assert(ret == 0);
//}
//
//void* Log_main(void* arg) {
//	Log logger   = (Log)arg;
//	char* buf    = logger->data;
//	size_t len   = LOG_BUF_LEN;
//	size_t nread = LOG_BUF_LEN;
//	size_t off   = 0;
//	int8_t ret;
//	while(ret = IO_read(&logger->server, buf + off, &nread) == 0) {
//		off += nread;
//		len -= nread;
//		nread = len;
//		if(len == 0) {
//			ret = IO_writeSpec(&logger->logfile, buf, &off);
//			if(ret == -1) {
//				perror("Log_main::write");
//				exit(-1);
//			}
//			off   = 0;
//			len   = LOG_BUF_LEN;
//			nread = LOG_BUF_LEN;
//		}
//	}
//	if(ret == -1) {
//		perror("Log_main::write");
//		exit(-1);
//	}
//	/* peer shutdown */
//	/* @ASSERTION: ret == 1 */
//	if(off) {
//		ret = IO_writeSpec(&logger->logfile, buf, &off);
//		assert(ret == 0);
//	}
//	pthread_exit(0);
//}
//
//void Log_runServer(Log logger) {
//	pthread_create(&logger->tid, NULL, Log_main, logger);
//}

#include <unistd.h>
#include <sys/mman.h>

#define LOCK_LOGGER(l) \
	pthread_spin_lock((l)->mutex)
#define UNLOCK_LOGGER(l) \
	pthread_spin_unlock((l)->mutex)

PRIVATE inline void FlushLogFile(Logger logger) {
	size_t len = *logger->len;
	int8_t ret = IO_writeSpec(&logger->file, logger->buf, &len);
	assert(ret == 0 && len == *logger->len);
	*logger->len = 0;
}

Logger Logger_init(Logger logger, const char* filename) {
	int fd = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0644);
	if(fd == -1) {
		perror("Logger_initModule");
		exit(-1);
	}
	logger->file.fileno = fd;
	logger->file.type   = H_FILE;
	void* area = mmap(0, LOG_BUF_LEN << 1, PROT_READ | PROT_WRITE,
			MAP_ANON | MAP_SHARED, -1, 0);
	if(area == MAP_FAILED) {
		perror("Logger_initModule");
		exit(-1);
	}
	logger->addr  = area;
	logger->mutex = area;
	logger->len   = area + sizeof(pthread_spinlock_t);
	*logger->len  = 0;
	logger->buf   = area + sizeof(pthread_spinlock_t) + sizeof(uint16_t);

	int ret;
	ret = pthread_spin_init(logger->mutex, PTHREAD_PROCESS_SHARED);
	if(ret != 0) {
		errno = ret;
		perror("Logger_initModule");
		exit(-1);
	}
	return logger;
}

void Logger_close(Logger logger) {
	int ret;
	if(*logger->len > 0) {
		LOCK_LOGGER(logger);
		FlushLogFile(logger);
		UNLOCK_LOGGER(logger);
	}
	ret = pthread_spin_destroy(logger->mutex);
	if(ret != 0) {
		errno = ret;
		perror("Logger_close");
		exit(-1);
	}
	ret = munmap(logger->addr, LOG_BUF_LEN << 1);
	if(ret != 0) {
		perror("Logger_close");
		exit(-1);
	}
	Handler_close(&logger->file);
}

#define LOG_MSG_SEP " - "
void Logger_record(Logger logger, LogLevel_t level, const char* fmt, ...) {
	LOCK_LOGGER(logger);
	if((LOG_BUF_LEN << 1) - *logger->len < LOG_MSG_LEN) {
		FlushLogFile(logger);
	}
	char* buf = logger->buf;
	char* p   = buf + *logger->len;
	uint8_t ls = LOG_STR_LEN[level];
	strncpy(p, LOG_STR[level], ls);
	p += ls;
	*p++ = ' ';
	time_t tnow = time(NULL);
	DateTime_formatTimeStamp(&tnow, p);
	p += TIMESTAMP_LEN - 1;
	/* separator "%s - %s" */
	strncpy(p, LOG_MSG_SEP, 3);
	p += 3;
	va_list varg;
	va_start(varg, fmt);
	uint32_t infolen = vsnprintf(p, LOG_MSG_LEN, fmt, varg);
	va_end(varg);
	p += infolen;
	*p++ = '\n';
	*logger->len = p - buf;
	UNLOCK_LOGGER(logger);
}
/*
 * Benmark: recording 1 million log items costs 3.33 seconds
#include <wait.h>

int main() {
	Logger_t logger;
	Logger_init(&logger, "/tmp/test.log");
	Logger_record(&logger, LOG_WARN, "log start");
	int pid;
	if((pid = fork()) == 0) {
		//Log client = Log_getClient(&logger);
		//sleep(1);
		int c = 1000000;
		while(c--) {
			Logger_record(&logger, LOG_INFO, "log message from %d", getpid());
		}
		exit(0);
	}
	//Log serv = Log_getServer(&logger, "./log.txt");
	//Log_runServer(serv);
	wait(NULL);
	Logger_record(&logger, LOG_WARN, "log end");
	Logger_close(&logger);
	return 0;
}
*/
