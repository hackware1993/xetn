#ifndef _HELPER_H_
#define _HELPER_H_

/**
 * Several wrap functions for Http and Wsgi
 */

#include "http/codec.h"
#include "stream.h"

int8_t HttpHelper_decode(HttpCodec, NetStream, stream_state_t*);
int8_t HttpHelper_encode(HttpCodec, NetStream, stream_state_t*);

#endif //_HELPER_H_
