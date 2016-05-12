#include "stream.h"
#include "io.h"

#include <sys/sendfile.h>

NetStream NetStream_init(NetStream ns, Handler h, uint32_t s) {
	ns->handler = *h;
	ns->buf = Buffer_new(s);
	ns->errnum = 0;
	return ns;
}

void NetStream_close(NetStream ns) {
	// TODO consider if there should shutdown the sock
	Buffer_free(ns->buf);
}

stream_state_t NetStream_readToBuf(NetStream ns) {
	Buffer  buf   = ns->buf;
	if(Buffer_isFull(buf)) {
		return STREAM_DONE;
	}
	size_t len = buf->lim - buf->end;
	/* DO NOT use IO_readSpec */
	/* because IO can be blocked */
	int ret = IO_read(&ns->handler, buf->ptr + buf->end, &len);
	buf->end += len;
	if(ret == 0) {
		return STREAM_DONE;
	} else if(ret == 1) {
		return STREAM_HUP;
	}
	/* ret == -1 */
	if(errno == EAGAIN) {
		return STREAM_PEND;
	}
	ns->errnum = errno;
	return STREAM_ERROR;
}

stream_state_t NetStream_writeFromBuf(NetStream ns) {
	Buffer  buf   = ns->buf;
	if(Buffer_isEnd(buf)) {
		return STREAM_DONE;
	}
	int ret;
	size_t len = buf->end - buf->pos;
	/* DO NOT use IO_writeSpec */
	/* because IO can be blocked */
	ret = IO_write(&ns->handler, buf->ptr + buf->pos, &len);
	buf->pos += len;
	// TODO: check this
	if(Buffer_isEnd(buf)) {
		Buffer_setPos(buf, 0);
		Buffer_setLen(buf, 0);
	}
	if(ret == 0) {
		return STREAM_DONE;
	}
	/* ret == -1 */
	if(errno == EAGAIN) {
		return STREAM_PEND;
	}
	ns->errnum = errno;
	return STREAM_ERROR;
}
stream_state_t NetStream_readToBufSpec(NetStream ns) {
	Buffer  buf   = ns->buf;
	if(Buffer_isFull(buf)) {
		return STREAM_DONE;
	}
	size_t len = buf->lim - buf->end;
	/* DO NOT use IO_readSpec */
	/* because IO can be blocked */
	int ret = IO_readSpec(&ns->handler, buf->ptr + buf->end, &len);
	buf->end += len;
	if(ret == 0) {
		return STREAM_DONE;
	} else if(ret == 1) {
		return STREAM_HUP;
	}
	/* ret == -1 */
	if(errno == EAGAIN) {
		return STREAM_PEND;
	}
	ns->errnum = errno;
	return STREAM_ERROR;
}

stream_state_t NetStream_writeFromBufSpec(NetStream ns) {
	Buffer  buf   = ns->buf;
	if(Buffer_isEnd(buf)) {
		return STREAM_DONE;
	}
	size_t len = buf->end - buf->pos;
	/* DO NOT use IO_writeSpec */
	/* because IO can be blocked */
	int ret = IO_writeSpec(&ns->handler, buf->ptr + buf->pos, &len);
	buf->pos += len;
	// TODO: check this
	if(Buffer_isEnd(buf)) {
		Buffer_setPos(buf, 0);
		Buffer_setLen(buf, 0);
	}
	if(ret == 0) {
		return STREAM_DONE;
	}
	/* ret == -1 */
	if(errno == EAGAIN) {
		return STREAM_PEND;
	}
	ns->errnum = errno;
	return STREAM_ERROR;
}

FileStream FileStream_init(FileStream stream, Handler h, uint32_t size) {
	stream->handler = *h;
	stream->buf     = Buffer_new(size);
	stream->errnum  = 0;
	return stream;
}
FileStream FileStream_initWithoutBuf(FileStream stream, Handler h) {
	stream->handler = *h;
	stream->buf     = NULL;
	stream->errnum  = 0;
	return stream;
}

PipeStream PipeStream_init(PipeStream stream, Handler src, Handler dest) {
	stream->src  = *src;
	stream->dest = *dest;
	if(src->type == H_SOCK) {
		stream->buf = Buffer_new(4096);
	}
	return stream;
}

void PipeStream_close(PipeStream stream) {
	if(stream->buf) {
		Buffer_free(stream->buf);
	}
}

stream_state_t PipeStream_deliver(PipeStream stream, size_t* len) {
	ssize_t ret;
	size_t count = *len;
		ret = sendfile(stream->dest.fileno, stream->src.fileno, NULL, count);
		if(ret == -1) {
			if(errno == EAGAIN) {
				return STREAM_PEND;
			}
			stream->errnum = errno;
			return STREAM_ERROR;
		}
		count -= ret;
		*len = count;
		if(count != 0) {
			return STREAM_PEND;
		}
	//} else {
	//	Buffer buf = stream->buf;

	//	
	//}
	return STREAM_DONE;
}

