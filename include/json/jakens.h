#ifndef _JAKENS_H_
#define _JAKENS_H_

#include <stdint.h>
#include "util/list.h"
#include "memblock.h"
#include "json/jpath.h"

typedef enum json_type {
	JSON_STRING,
	JSON_NUMBER,
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_BOOLEAN,
	JSON_NULL,
} JsonType_t;

struct json_element;
struct json_pair;

typedef struct json_array {
	uint32_t             len;
	uint32_t             cap;
	struct json_element* elements;
} JsonArray_t, *JsonArray;

typedef struct json_object {
	uint32_t          len;
	uint32_t          cap;
	struct json_pair* pairs;
} JsonObject_t, *JsonObject;

typedef struct json_element {
	JsonType_t type;
	union {
		uint8_t       bol;
		const char*   str;
		double        num;
		JsonArray_t   arr;
		JsonObject_t  obj;
	} val;
} JsonElement_t, *JsonElement;

typedef struct json_pair {
	const char* key;
	JsonElement_t val;
} JsonPair_t, *JsonPair;

typedef struct json_document {
	JsonElement_t root;
} JsonDocument_t, *JsonDocument;

typedef struct json_parser {
	ArrayList_t stack;

	struct {
		uint8_t* stack;
		uint32_t len;
		uint32_t cap;
	} token;
	MemBlock_t buf;
	/* used to store true, false and null */
	char         sbuf[5];
	uint8_t      sbufLen;
	JsonElement  curEle;
	/* used to record the function which is pended */
	uint8_t    (*curOpt)
		(struct json_parser*, const char*, uint32_t*, uint32_t);
	uint8_t      errnum;
	union {
		uint8_t     bol;
		double      real;
		const char* str;
	} tempVal;

	uint32_t     loc;
} JsonParser_t, *JsonParser;

JsonParser JsonParser_init(JsonParser);
void       JsonParser_close(JsonParser);
const char* JsonParser_getErrorMsg(JsonParser);

JsonDocument Json_parseFromString(JsonParser, const char*, size_t, JsonDocument);
JsonDocument Json_parseFromFile(JsonParser, const char*, JsonDocument);

void        JsonDocument_free(JsonDocument);
JsonElement JsonDocument_getRoot(JsonDocument);
JsonElement JsonDocument_putRoot(JsonDocument, JsonType_t);
JsonElement JsonDocument_findElement(JsonDocument, JPath);

#define JsonElement_getType(e)    ((e)->type)
#define JsonElement_setType(e, t) (e)->type = (t)

#define JsonElement_isNull(e)  ((e)->type == JSON_NULL)
#define JsonElement_getStr(e)  (e)->val.str
#define JsonElement_getBool(e) (e)->val.bol
#define JsonElement_getNum(e)  (e)->val.num
#define JsonElement_setStr (e, v) (e)->val.str = (v)
#define JsonElement_setBool(e, v) (e)->val.bol = (v)
#define JsonElement_setNum (e, v) (e)->val.num = (v)

JsonElement JsonObject_putElement(JsonElement, const char*, JsonType_t);
JsonElement JsonObject_getElement(JsonElement, const char*);

JsonElement JsonArray_putElement(JsonElement, JsonType_t);
JsonElement JsonArray_getElement(JsonElement, uint32_t);

#endif // _JAKENS_H_
