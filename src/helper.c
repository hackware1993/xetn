#include "helper.h"
#include <assert.h>

enum {
	NONE = -2,
	LOOP =  2,
};
/* possible state composition for decoding: */
/*   EXIT_ERROR + STREAM_*    -> EXIT_ERROR */
/*   EXIT_PEND  + STREAM_PEND -> EXIT_PEND  */
/*   EXIT_DONE  + STREAM_PEND -> EXIT_DONE  */
/*   EXIT_PEND  + STREAM_HUP  -> EXIT_ERROR */
/*   EXIT_DONE  + STREAM_HUP  -> EXIT_DONE  */
/*   EXIT_PEND  + STREAM_DONE -> LOOP       */
/*   EXIT_DONE  + STREAM_DONE -> EXIT_DONE  */
int8_t DECODE_COMPOSE_MAP[2][5] = {
             /* STREAM_NONE, STREAM_ERROR, STREAM_PEND, STREAM_HUP, STREAM_DONE */
/* EXIT_DONE */{       NONE,         NONE,   EXIT_DONE,  EXIT_DONE,   EXIT_DONE},
/* EXIT_PEND */{       NONE,         NONE,   EXIT_PEND, EXIT_ERROR,        LOOP},
};
/* possible state composition for encoding: */
/*   EXIT_ERROR + STREAM_*    -> EXIT_ERROR */
/*   EXIT_PEND  + STREAM_PEND -> EXIT_PEND  */
/*   EXIT_DONE  + STREAM_PEND -> EXIT_PEND  */
/*   EXIT_PEND  + STREAM_DONE -> LOOP       */
/*   EXIT_DONE  + STREAM_DONE -> EXIT_DONE  */
int8_t ENCODE_COMPOSE_MAP[2][5] = {
             /* STREAM_NONE, STREAM_ERROR, STREAM_PEND, STREAM_HUP, STREAM_DONE */
/* EXIT_DONE */{       NONE,         NONE,   EXIT_PEND,       NONE,   EXIT_DONE},
/* EXIT_PEND */{       NONE,         NONE,   EXIT_PEND,       NONE,        LOOP},
};
int8_t HttpHelper_decode(HttpCodec decoder, NetStream ns, stream_state_t* state) {
	int8_t   ret;
	uint32_t len;
	char*    off;
	stream_state_t st = STREAM_NONE;
	Buffer buf = NetStream_getBuf(ns);

	while(1) {
		st = NetStream_readToBufSpec(ns);
		/* rewrite back the state of NetStream */
		*state = st;
		/* an IO error occur, the following process makes no sense */
		if(st == STREAM_ERROR) {
			return EXIT_ERROR;
		}
		/* setup the buf & len */
		len = Buffer_getLen(buf) - Buffer_getPos(buf);
		off = Buffer_getPtr(buf) + Buffer_getPos(buf);
		ret = HttpCodec_decode(decoder, off, &len);
		// TODO determine the position of EXIT_ERROR
		Buffer_setPos(buf, Buffer_getPos(buf) + len);
		if(Buffer_isEnd(buf)) {
			Buffer_setPos(buf, 0);
			Buffer_setLen(buf, 0);
		}
		/* decoding error occur, halt immediately */
		if(ret == EXIT_ERROR) {
			return EXIT_ERROR;
		}
		ret = DECODE_COMPOSE_MAP[ret][st];
		if(ret != LOOP) {
			return ret;
		}
	}
}

int8_t HttpHelper_encode(HttpCodec encoder, NetStream ns, stream_state_t* state) {
	int8_t   ret;
	uint32_t len;
	char*    off;
	stream_state_t st = STREAM_NONE;
	Buffer buf = NetStream_getBuf(ns);

	while(1) {
		/* setup off & len */
		len = Buffer_getLim(buf) - Buffer_getLen(buf);
		off = Buffer_getPtr(buf) + Buffer_getLen(buf);
		ret = HttpCodec_encode(encoder, off, &len);
		Buffer_setLen(buf, Buffer_getLen(buf) + len);
		/* encoding error occur, halt immediately */
		if(ret == EXIT_ERROR) {
			*state = STREAM_NONE;
			return EXIT_ERROR;
		}
		st = NetStream_writeFromBufSpec(ns);
		/* rewrite back the state of NetStream */
		*state = st;
		/* an error occur, the following process makes no sense */
		if(st == STREAM_ERROR) {
			return EXIT_ERROR;
		}
		ret = ENCODE_COMPOSE_MAP[ret][st];
		if(ret != LOOP) {
			return ret;
		}
	}
}
