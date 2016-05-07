#include "service.h"
#include "net.h"
#include "io.h"
#include "helper.h"
#include "datetime.h"
#include "mime.h"
#include "log.h"
#include "xetn.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/sendfile.h>

char* P404 = "HTTP/1.1 404 Not Found\r\n"
"Connection: close\r\n"
"Content-Type: text/html\r\n"
"Date: Tue, 08 Sep 2015 14:03:22 GMT\r\n"
"Server: XETN/1.0\r\n"
"\r\n"
"<html>"
"<head><title>Not Found</title></head>"
"<body>File is not found！</body>"
"</html>\0";

#include <stdio.h>

int32_t file_length(int fd) {
	off_t off = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	return off;
}

int8_t Acceptor_onProcess(Watcher wt) {
	Acceptor acc = (Acceptor)wt;
	/* every incoming connection */
	/* should be set to NONBLOCK mode */
	netoption_t op[] = {
		{NET_NONBLOCK, 1},
		{NET_NULL, 1},
	};
	handler_t h;
	Processor pro = NULL;
	while(TcpServer_accept(&wt->handler, &h, op)) {
		Processor pro = Processor_new(&h);
		Reactor_register(wt->host, &pro->base);
	}
	return 1;
	/* just pend it instead of stoping */
}

int8_t Acceptor_onClose(Watcher wt) {
	Acceptor acc = (Acceptor)wt;
	free(wt);
	return 0;
}

Acceptor Acceptor_new(Handler h) {
	Acceptor acc = (Acceptor)malloc(sizeof(acceptor_t));
	Watcher_init(&acc->base, h, TRIG_EV_READ, Acceptor_onClose);
	acc->pList[0] = Acceptor_onProcess;
	Watcher_bindProcess(&acc->base, acc->pList, 1);
	return acc;
}

int8_t Processor_onPreProcess(Watcher wt) {
	Processor pro = (Processor)wt;
	int8_t   ret;
	stream_state_t state;
	while((ret = HttpHelper_decode(&pro->codec, &pro->ns, &state)) != EXIT_PEND) {
		if(ret == EXIT_DONE) {
			Watcher_changeEvent(wt, TRIG_EV_WRITE);
			return 0;
		} else {
		    if(state == STREAM_HUP) {
		    } else if(state == STREAM_ERROR) {
		        perror("Process_onRead");
		    }
			return -1;
		}
	}
	printf("after PRE\n");
	return 1;
}
int8_t Processor_onProcess(Watcher wt) {
	Processor pro = (Processor)wt;
	/* save the path of request */
	const char* path = HttpConnection_getPath(&pro->req);
	Log_record(&Xetn_getContext().logger, LOG_INFO, "PATH: %s", path);

	if(access(path, F_OK) == -1) {
		pro->is404 = 1;
		return 0;
	}
	int fd = open(path, O_RDONLY);
	Handler_init(&pro->hf, fd, H_FILE);
	pro->off = 0;

	char len[10];
	int l = file_length(fd);
	pro->len = l;
	/* get the length of content */
	snprintf(len, 10, "%d", l);
	/* get the time stamp of response */
	char tBuf[DATETIME_LEN];
	time_t tm = time(NULL);
	DateTime_formatGmt(&tm, tBuf);
	/* set http version and response status number */
	HttpConnection res = &pro->res;
	HttpConnection_init(res, HTTP_RES);
	HttpConnection_setVersion(res, HTTP1_1);
	HttpConnection_setStatus(res, ST_200);
	/* construct the neccessary http headers */
	HttpConnection_putHeader(res, "Connection", "close");
	HttpConnection_putHeader(res, "Server", "XETN/1.1");
	HttpConnection_putHeader(res, "Content-Type", Mime_lookupType(HttpConnection_getPathInfo(&pro->res)));
	HttpConnection_putHeader(res, "Content-Length", len);
	HttpConnection_putHeader(res, "Date", tBuf);
	return 0;
}
int8_t Processor_onPostProcess(Watcher wt) {
	Processor pro = (Processor)wt;
	size_t nsend;
	int ret;
	if(pro->is404) {
		nsend = strlen(P404 + 1);
		IO_write(&wt->handler, P404, &nsend);
		return 0;
	}
	if(!pro->isHeaderFin) {
		stream_state_t state;
		HttpCodec_init(&pro->codec, &pro->res);
		NetStream_clearBuf(&pro->ns);
		ret = HttpHelper_encode(&pro->codec, &pro->ns, &state);
		if(ret == EXIT_DONE) {
			HttpConnection_close(&pro->res);
			/* the headers of response have been processed */
			pro->isHeaderFin = 1;
			PipeStream_init(&pro->pipe, &pro->hf, &wt->handler);
		} else {
			printf("error closed!\n");
			return -1;
		}
	} 
	if(pro->isHeaderFin) {
		PipeStream_deliver(&pro->pipe, &pro->len);
		if(pro->len == 0) {
			Handler_close(&pro->hf);
			return 0;
		}
		return 1;
	}
}
int8_t Processor_onClose(Watcher wt) {
	Processor pro = (Processor)wt;
	HttpConnection_close(&pro->req);
	NetStream_close(&pro->ns);
	free(wt);
	return 0;
}

Processor Processor_new(Handler h) {
	Processor pro = (Processor)malloc(sizeof(processor_t));
	Watcher_init(&pro->base, h, TRIG_EV_READ, Processor_onClose);

	pro->pList[0] = Processor_onPreProcess;
	pro->pList[1] = Processor_onProcess;
	pro->pList[2] = Processor_onPostProcess;
	Watcher_bindProcess(&pro->base, pro->pList, 3);

	NetStream_init(&pro->ns, h, 1024);
	/* initialize the HTTP codec & conn */
	HttpConnection_init(&pro->req, HTTP_REQ);
	HttpCodec_init(&pro->codec, &pro->req);
	pro->isHeaderFin = 0;
	pro->is404 = 0;
	return pro;
}
