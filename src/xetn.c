#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <errno.h>
#include <string.h>

#include "xetn.h"
#include "net.h"
#include "reactor.h"
#include "servwatcher.h"

xetn_context_t xetn_ctx;

int Xetn_workerMain(void* args) {
	printf("Main start\n");
	Watcher wt = (Watcher)args;
	reactor_t re;
	Reactor reactor = Reactor_init(&re);
	Reactor_register(reactor, wt);
	/* only wair for 200ms */
	printf("Loop start\n");
	Reactor_loop(reactor, -1);
	Reactor_close(reactor);
	return 0;
}

pid_t Xetn_createWorker(int (*cb)(void*), void* args) {
	pid_t p;
	if((p = fork()) > 0) {
		return p;
	} else if(p == -1) {
		perror("Xetn_createWork");
		exit(-1);
	}
	int status = 0;
	if(cb) {
		status = cb(args);
	}
	exit(status);
	/* the worker is end */
}

int32_t Xetn_waitWorker(pid_t p) {
	int32_t status;
	waitpid(p, &status, 0);
	return status;
}

int Xetn_initServHandler(Handler hdl) {
	netoption_t op[] = {
		{NET_NODELAY,   1},
		{NET_REUSEADDR, 1},
		{NET_REUSEPORT, 1},
		{NET_NONBLOCK,  1},
		{NET_NULL,      1},
	};
	TcpServer_create(hdl, "tcp://0.0.0.0:8080", op);
	return 0;
}

void Xetn_processParam(int argc, char* argv[]) {
	int ret = -1;
	while((ret = getopt(argc, argv, "hvc:")) != -1) {
		switch(ret) {
			case 'h':
				fprintf(stdout, LOGO"Version: %s\n"USAGE, XETN_VER);
				break;
			case 'v':
				fprintf(stdout, "Version: %s\n", XETN_VER);
				break;
			case 'c':
				if(optarg) {
					Xetn_getContext().conf = optarg;
				} else {
					fprintf(stderr, USAGE);
				}
				break;
			default:
				fprintf(stderr, USAGE);
		}
	}
}

int main(int argc, char* argv[]) {
	int np = 3;
	int fd[2];
	int pid[np];
	int ptest = 1;
	int ret;
	struct rlimit limit;
	ret = getrlimit(RLIMIT_NOFILE, &limit);
	if(ret == -1) {
		perror("getrlimit");
		exit(-1);
	}
	limit.rlim_cur = limit.rlim_max;
	ret = setrlimit(RLIMIT_NOFILE, &limit);
	if(ret == -1) {
		perror("setrlimit");
		exit(-1);
	}
	ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
	handler_t log = {H_SOCK, fd[1]};
	Xetn_getContext().hlog = log;
	Xetn_initServHandler(&Xetn_getContext().hsvr);
	Acceptor acc = Acceptor_new(&Xetn_getContext().hsvr);
	for(int i = 0; i < np; ++i) {
		pid[i] = Xetn_createWorker(Xetn_workerMain, acc);
	}

	for(int i = 0; i < np; ++i) {
		Xetn_waitWorker(pid[i]);
	}
	return 0;
}
