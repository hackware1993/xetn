#ifndef _DATAMAP_H_
#define _DATAMAP_H_

extern uint8_t     HEADER_LENS[];
extern uint16_t    STATUS_NUM[];
extern const char* METHOD_NAME[];
extern const char* VERSION_NAME[];
extern const char* HEADER_NAMES[];
extern const char* HEADER_LABELS[];
extern const char* STATUS_DESC[];

extern uint8_t URL_TOKEN_MAP[128];
extern uint8_t KEY_TOKEN_MAP[128];
extern uint8_t VAL_TOKEN_MAP[128];

extern uint64_t HEADER_HASHS[256];
extern uint64_t VERSION_HASHS[4];
extern uint64_t METHOD_HASHS[16];

#endif //_DATAMAP_H_
