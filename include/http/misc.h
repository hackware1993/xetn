#ifndef _MISC_H_
#define _MISC_H_

/* ===================== NOTICE====================== */
/* misc.h    - used internally                        */
/* misc.c    - contains the validator needed char map */
/* header.c  - contains hash map of headers           */
/* version.c - contains hash map of versions          */
/* method.c  - contains hash map of methods           */

#define MAGNUM 458

extern uint8_t     ALPHA_HEX[];
extern uint8_t     URL_TOKEN_MAP[];
extern uint8_t     KEY_TOKEN_MAP[];
extern uint8_t     VAL_TOKEN_MAP[];

extern uint8_t     HEADER_LEN[];
extern uint32_t    HEADER_HASH[];
extern uint8_t     HEADER_INDEX[];
extern const char* HEADER_NAME[];
extern const char* HEADER_LABEL[];

extern uint16_t    STATUS_NUM[];
extern const char* STATUS_CODE[];
extern const char* STATUS_DESC[];

extern const char* VERSION_NAME[];
extern uint64_t    VERSION_HASH[];

extern uint64_t    METHOD_HASH[];
extern const char* METHOD_NAME[];

#endif //_MISC_H_
