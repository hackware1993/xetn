#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "xetn.h"
#include "net.h"
#include "service.h"
#include "mime.h"
#include "json/jakens.h"

XetnContext_t xetnCtx;

void Xetn_sigMaster(int signo) {
	Xetn_getContext().terminate = 1;
	printf("in Xetn_sigMaster\n");
}

void Xetn_sigWorker(int signo) {
	Xetn_getContext().loop.signal = 1;
	printf("in Xetn_sigWorker\n");
}

int Xetn_workerMain(void* args) {
	struct sigaction act;
	act.sa_handler = Xetn_sigWorker;
	act.sa_flags = SA_INTERRUPT;
	sigemptyset(&act.sa_mask);
	sigaction(SIGUSR1, &act, NULL);

	//close(0); close(1); close(2);
	Logger log = &Xetn_getContext().logger;
	Logger_record(log, LOG_INFO, "Work start");
	printf("Main start\n");
	Watcher wt = (Watcher)args;
	Reactor reactor = Reactor_init(&Xetn_getContext().loop);
	Reactor_register(reactor, wt);
	printf("Loop start\n");
	Logger_record(log, LOG_INFO, "Loop start");
	Reactor_loop(reactor, -1);
	Reactor_close(reactor);
	Logger_record(log, LOG_INFO, "Loop end");
	printf("Loop end\n");
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
	TcpServer_create(hdl, Xetn_getContext().conf.listen, op);
	
	return 0;
}

void Xetn_initConf(XetnConf conf, const char* file) {
	JsonParser_t parser;
	JsonParser_init(&parser);
	JsonDocument_t dom;
	JsonDocument doc = Json_parseFromFile(&parser, file, &dom);
	if(doc == NULL) {
		fprintf(stderr, "ERROR: Invalid config file!\n");
		exit(-1);
	}
	JsonParser_close(&parser);

	JsonElement global = JsonObject_getElement(JsonDocument_getRoot(doc), "global");
	if(global != NULL && JsonElement_getType(global) != JSON_OBJECT) {
		fprintf(stderr, "ERROR: [global] should be an OBJECT!\n");
		exit(-1);
	}
	JsonElement nprocess = JsonObject_getElement(global, "nprocess");
	if(nprocess != NULL && JsonElement_getType(nprocess) != JSON_NUMBER) {
		fprintf(stderr, "ERROR: [global.nprocess] should be a NUMBER!\n");
		exit(-1);
	}
	conf->nProcess = (unsigned)JsonElement_getNum(nprocess);
	JsonElement listen = JsonObject_getElement(global, "listen");
	if(listen != NULL && JsonElement_getType(listen) != JSON_STRING) {
		fprintf(stderr, "ERROR: [global.listen] should be a STRING!\n");
		exit(-1);
	}
	conf->listen = strdup(JsonElement_getStr(listen));
	JsonElement logfile = JsonObject_getElement(global, "log");
	if(logfile != NULL && JsonElement_getType(logfile) != JSON_STRING) {
		fprintf(stderr, "ERROR: [global.log] should be a STRING!\n");
		exit(-1);
	}
	conf->logFile = strdup(JsonElement_getStr(logfile));
	JsonDocument_free(doc);
}

void Xetn_processParam(int argc, char* argv[]) {
	int ret = -1;
	if(argc <= 1) {
		fprintf(stderr, USAGE);
		exit(-1);
	}
	while((ret = getopt(argc, argv, "hvc:"/*n:l:"*/)) != -1) {
		switch(ret) {
			case 'h':
				fprintf(stdout, LOGO"Version: %s\n"USAGE, XETN_VER);
				exit(0);
				break;
			case 'v':
				fprintf(stdout, "Version: %s\n", XETN_VER);
				exit(0);
				break;
			case 'c':
				if(optarg) {
					//Xetn_getContext().conf = optarg;
					Xetn_initConf(&Xetn_getContext().conf, (const char*)optarg);
				} else {
					fprintf(stderr, USAGE);
					exit(-1);
				}
				break;
			/*
			case 'l':
				if(optarg) {
					Xetn_getContext().laddr = optarg;
				} else {
					fprintf(stderr, USAGE);
					exit(-1);
				}
				break;
			case 'n':
				if(optarg) {
					Xetn_getContext().np = atoi(optarg);
				} else {
					fprintf(stderr, USAGE);
					exit(-1);
				}
				break;
			*/
			default:
				fprintf(stderr, USAGE);
				exit(-1);
		}
	}
}


/* add modules that should be initialized */
void Xetn_initModule() {
	Mime_initModule();
}

void Xetn_initContext() {
	Logger_init(&Xetn_getContext().logger, Xetn_getContext().conf.logFile);
	Xetn_getContext().terminate = 0;
}


int main(int argc, char* argv[]) {
	Xetn_processParam(argc, argv);
	int np = Xetn_getContext().conf.nProcess;
	int pid[np];
	int ret;
	struct rlimit limit;
	/* enlarge the current limitition of file descriptors */
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

	Xetn_initModule();
	Xetn_initContext();

	sigset_t set, old;
	sigemptyset(&set);
	sigaddset(&set, SIGQUIT);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGINT);
	sigprocmask(SIG_SETMASK, &set, &old);
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_INTERRUPT;
	act.sa_handler = Xetn_sigMaster;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);

	// TODO change this to support multiple server socket
	Xetn_initServHandler(&Xetn_getContext().hsvr);
	Acceptor acc = Acceptor_new(&Xetn_getContext().hsvr);
	for(int i = 0; i < np; ++i) {
		pid[i] = Xetn_createWorker(Xetn_workerMain, acc);
	}
	sigprocmask(SIG_SETMASK, &old, NULL);

	/* wait for signal */
	pause();
	for(int i = 0; i < np; ++i) {
		kill(pid[i], SIGUSR1);
	}
	for(int i = 0; i < np; ++i) {
		Xetn_waitWorker(pid[i]);
	}
	return 0;
}
