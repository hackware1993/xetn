#include "stream.h"
#include "io.h"

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
	int ret;
	size_t len = buf->lim - buf->end;
	/* DO NOT use IO_readSpec */
	/* because IO can be blocked */
	ret = IO_read(&ns->handler, buf->ptr + buf->end, &len);
	buf->end += len;
	switch(ret) {
		case -1:
			if(errno == EAGAIN) {
				return STREAM_PEND;
			}
			ns->errnum = errno;
			return STREAM_ERROR;
		case 0:
			return STREAM_DONE;
		case 1:
			return STREAM_HUP;
	}

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
	switch(ret) {
		case -1:
			if(errno = EAGAIN) {
				return STREAM_PEND;
			}
			ns->errnum = errno;
			return STREAM_ERROR;
		case 0:
			return STREAM_DONE;
	}
}

