#include "helper.h"
#include <assert.h>

int8_t HttpHelper_decode(HttpCodec decoder, NetStream ns, stream_state_t* state) {
	int8_t   ret;
	uint32_t len;
	char*    off;

	Buffer buf = NetStream_getBuf(ns);
	while(1) {
		*state = NetStream_readToBuf(ns);
		switch(*state) {
			case STREAM_HUP: case STREAM_ERROR:
				return EXIT_ERROR;
			case STREAM_PEND:
				return EXIT_PEND;
		}
		len = Buffer_getLen(buf) - Buffer_getPos(buf);
		off = Buffer_getPtr(buf) + Buffer_getPos(buf);
		ret = HttpCodec_decode(decoder, off, &len);
		if(ret == EXIT_DONE) {
			Buffer_setPos(buf, Buffer_getPos(buf) + len);
			if(Buffer_isEnd(buf)) {
				Buffer_setPos(buf, 0);
				Buffer_setLen(buf, 0);
			}
			break;
			// TODO process error pos
		} else if(ret == EXIT_ERROR) {
			return EXIT_ERROR;
		}
		/* EXIT_PEND indicates there is no content left inside buffer */
		Buffer_setPos(buf, Buffer_getPos(buf) + len);
		assert(Buffer_isEnd(buf));
		Buffer_setLen(buf, 0);
		Buffer_setPos(buf, 0);
	}
	return EXIT_DONE;
}

int8_t HttpHelper_encode(HttpCodec encoder, NetStream ns, stream_state_t* state) {
	int8_t   ret;
	uint32_t len;
	char*    off;
	
	Buffer buf = NetStream_getBuf(ns);
	while(1) {
		*state = NetStream_writeFromBuf(ns);
		switch(*state) {
			case STREAM_ERROR:
				return EXIT_ERROR;
			case STREAM_PEND:
				return EXIT_PEND;
		}
		len = Buffer_getLim(buf) - Buffer_getLen(buf);
		off = Buffer_getPtr(buf) + Buffer_getLen(buf);
		ret = HttpCodec_encode(encoder, off, &len);
		if(ret == EXIT_DONE) {
			Buffer_setLen(buf, Buffer_getLen(buf) + len);
			break;
		} else if(ret == EXIT_ERROR) {
			return EXIT_ERROR;
		}
		Buffer_setLen(buf, Buffer_getLen(buf) + len);
	}
	return EXIT_DONE;
}
