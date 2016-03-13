#ifndef _DATAMAP_H_
#define _DATAMAP_H_

#define MAGNUM 458

extern uint16_t    STATUS_NUM[];
extern const char* STATUS_DESC[];

extern uint64_t    METHOD_HASH[];
extern const char* METHOD_NAME[];

extern const char* VERSION_NAME[];
extern uint64_t    VERSION_HASH[];

extern uint8_t     HEADER_LEN[];
extern uint32_t    HEADER_HASH[];
extern uint8_t     HEADER_INDEX[];
extern const char* HEADER_NAME[];
extern const char* HEADER_LABEL[];

extern uint8_t URL_TOKEN_MAP[];
extern uint8_t KEY_TOKEN_MAP[];
extern uint8_t VAL_TOKEN_MAP[];


#endif //_DATAMAP_H_
