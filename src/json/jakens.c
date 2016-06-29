#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "json/jakens.h"

#define DEFAULT_DEPTH 16

/* character map for hex */
static uint8_t HEX_MAP[128] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

#define JAKENS_ERROR_MSG(XX)                            \
	XX(ERR_NONE,  "INFO: Success")                      \
	XX(ERR_PEND,  "INFO: Pend")                         \
    XX(ERR_BOOL,  "TOKEN: Invalid boolean value")       \
	XX(ERR_NIL,   "TOKEN: Invalid null value")          \
	XX(ERR_NUM,   "TOKEN: Invalid number value")        \
	XX(ERR_STR,   "TOKEN: Invalid string value")        \
	XX(ERR_UNI,   "TOKEN: Invalid unicode")             \
	XX(ERR_ESCP,  "TOKEN: Invalid escaped character")   \
	XX(ERR_TOK,   "TOKEN: Invalid token")               \
	XX(ERR_FIN,   "MATCH: Already finished")            \
	XX(ERR_STAR,  "MATCH: Token unaccepted by START")   \
	XX(ERR_EOBJ,  "MATCH: Token unaccepted by OBJECT")  \
	XX(ERR_EARR,  "MATCH: Token unaccepted by ARRAY")   \
	XX(ERR_EPAIR, "MATCH: Token unaccepted by PAIR")    \
	XX(ERR_EMEM,  "MATCH: Token unaccepted by MEMBER")  \
	XX(ERR_EELE,  "MATCH: Token unaccepted by ELEMENT") \
	XX(ERR_EVAL,  "MATCH: Token unaccepted by VALUE")   \
	XX(ERR_ETOK,  "MATCH: Token unmatched")             \
	XX(ERR_FOPEN, "FILE: File open error")              \
	XX(ERR_FREAD, "FILE: File read error")

#define XX(tk, msg) tk,
enum {
	JAKENS_ERROR_MSG(XX)
};
#undef XX

#define XX(tk, msg) msg,
static char* ERROR_MAP[] = {
	JAKENS_ERROR_MSG(XX)
};
#undef XX

#define JAKENS_TOKEN_MAP(XX) \
	XX(TOKEN_NONE)           \
	XX(TOKEN_PEND)           \
    XX(TOKEN_INVALID)        \
	XX(TOKEN_START)          \
	XX(TOKEN_END)            \
	XX(TOKEN_OBJ)            \
	XX(TOKEN_MEMBERS)        \
	XX(TOKEN_PAIR)           \
	XX(TOKEN_ARR)            \
	XX(TOKEN_ELEMENTS)       \
	XX(TOKEN_VAL)            \
	XX(TOKEN_OBJ_BEGIN)      \
	XX(TOKEN_ARR_BEGIN)      \
	XX(TOKEN_OBJ_END)        \
	XX(TOKEN_ARR_END)        \
	XX(TOKEN_STR)            \
	XX(TOKEN_BOOL)           \
	XX(TOKEN_NUM)            \
	XX(TOKEN_NULL)           \
	XX(TOKEN_COL)            \
	XX(TOKEN_CMA)

#define XX(tk) #tk,
static char* TK_STR[] = {
	JAKENS_TOKEN_MAP(XX)
};
#undef XX

#define XX(tk) tk,
typedef enum token {
	JAKENS_TOKEN_MAP(XX)
} Token_t;
#undef XX

static inline void PushToken(JsonParser parser, Token_t tk) {
	uint32_t cap = parser->token.len;
	uint32_t len = parser->token.cap;
	if(len >= cap) {
		while(len >= cap) {
			cap <<= 1;
		}
		parser->token.cap = cap;
		parser->token.stack = (uint8_t*)realloc(
				parser->token.stack, sizeof(uint8_t) * cap);
	}
	parser->token.stack[len] = tk;
	parser->token.len = len + 1;
}

static inline void PushTokens(JsonParser parser, uint8_t n, ...) {
	uint8_t* stk = parser->token.stack;
	uint32_t len = parser->token.len;
	uint32_t cap = parser->token.cap;
	if(len >= cap) {
		while(len >= cap) {
			cap <<= 1;
		}
		parser->token.cap = cap;
		parser->token.stack = (uint8_t*)realloc(stk, sizeof(uint8_t) * cap);
		stk = parser->token.stack;
	}
	va_list vtks;
	va_start(vtks, n);
	for(uint8_t i = 0; i < n; ++i) {
		stk[len++] = va_arg(vtks, Token_t);
	}
	va_end(vtks);
	parser->token.len = len;
}

static inline void PopToken(JsonParser parser) {
	uint32_t len = parser->token.len;
	if(len == 0) {
		return;
	}
	parser->token.len = len - 1;
}
static inline void ModTopToken(JsonParser parser, Token_t tk) {
	uint32_t len = parser->token.len;
	if(len != 0) {
		parser->token.stack[len - 1] = tk;
	}
}

static inline Token_t TopToken(JsonParser parser) {
	uint32_t len = parser->token.len;
	if(len == 0) {
		return TOKEN_NONE;
	}
	return parser->token.stack[len - 1];
}

static uint8_t EscapeToChar(JsonParser, const char*, uint32_t*, uint32_t);
static uint8_t EscapeToUtf8(JsonParser, const char*, uint32_t*, uint32_t);
static uint8_t GetStr(JsonParser, const char*, uint32_t*, uint32_t);
static uint8_t GetNum(JsonParser, const char*, uint32_t*, uint32_t);
static uint8_t GetTrue(JsonParser, const char*, uint32_t*, uint32_t);
static uint8_t GetFalse(JsonParser, const char*, uint32_t*, uint32_t);
static uint8_t GetNull(JsonParser, const char*, uint32_t*, uint32_t);

static uint8_t EscapeToUtf8(JsonParser parser, const char* str, uint32_t* off, uint32_t len) {
	uint8_t slen = parser->sbufLen;
	char*    buf = parser->sbuf;
	uint32_t loff = *off;
	while(loff < len && slen < 4) {
		/* validate the hex number */
		if(HEX_MAP[(uint8_t)str[loff]] == 0xFF) {
			parser->errnum = ERR_UNI;
			return TOKEN_INVALID;
		}
		buf[slen++] = str[loff++];
	}
	*off = loff;
	if(slen == 4) {
		parser->sbufLen = 0;
		uint16_t val = HEX_MAP[(uint8_t)buf[0]];
		/* platform independent transform */
		for(uint8_t i = 1; i < 4; ++i) {
			val <<= 4;
			val += HEX_MAP[(uint8_t)buf[i]];
		}
		uint8_t ch;
		MemBlock mblock = &parser->buf;
		if(val <= 0x7F) {
			ch = (char)val;
			MemBlock_putChar(mblock, ch);
		} else if(val <= 0x7FF) {
			ch = (char)(val >> 6) | 0xC0;
			MemBlock_putChar(mblock, ch);
			ch = ((char)val & 0x3F) | 0x80;
			MemBlock_putChar(mblock, ch);
		} else if(val <= 0xFFFF) {
			ch = (char)(val >> 12) | 0xE0;
			MemBlock_putChar(mblock, ch);
			ch = ((char)(val >> 6) & 0x3F) | 0x80;
			MemBlock_putChar(mblock, ch);
			ch = ((char)val & 0x3F) | 0x80;
			MemBlock_putChar(mblock, ch);
		} else if(val <= 0x10FFFF) {
			ch = (char)(val >> 18) | 0xF0;
			MemBlock_putChar(mblock, ch);
			ch = ((char)(val >> 12) & 0x3F) | 0x80;
			MemBlock_putChar(mblock, ch);
			ch = ((char)(val >> 6) & 0x3F) | 0x80;
			MemBlock_putChar(mblock, ch);
			ch = ((char)val & 0x3F) | 0x80;
			MemBlock_putChar(mblock, ch);
		} else {
			parser->errnum = ERR_UNI;
			return TOKEN_INVALID;
		}
		parser->curOpt = GetStr;
		return TOKEN_NONE;
	}
	parser->sbufLen = slen;
	return TOKEN_PEND;
}

static uint8_t EscapeToChar(JsonParser parser, const char* str, uint32_t* off, uint32_t len) {
	if(*off >= len) {
		return TOKEN_PEND;
	}
	MemBlock mblock = &parser->buf;

	char ch = str[(*off)++];
	switch(ch) {
		case '"': case '/': case '\\': break;
		case 'b':  ch = '\b'; break;
		case 'f':  ch = '\f'; break;
		case 'n':  ch = '\n'; break;
		case 'r':  ch = '\r'; break;
		case 't':  ch = '\t'; break;
		case 'u':
			parser->curOpt = EscapeToUtf8;
			return EscapeToUtf8(parser, str, off, len);
		default:
			parser->errnum = ERR_ESCP;
			return TOKEN_INVALID;
	}
	MemBlock_putChar(mblock, ch);
	parser->curOpt = GetStr;
	return TOKEN_NONE;
}

static uint8_t GetStr(JsonParser parser, const char* str, uint32_t* off, uint32_t len) {
	MemBlock mblock = &parser->buf;
	uint32_t loff = *off;
	Token_t res;
	while(loff < len) {
		switch(str[loff++]) {
			case '"':
				/* skip the last '"' */
				MemBlock_putStrN(mblock, str + *off, loff - *off - 1);
				parser->tempVal.str = MemBlock_newStr(mblock);
printf("STR>> %s\n", parser->tempVal.str);
				MemBlock_clear(mblock);
				*off = loff;
				parser->curOpt = NULL;
				return TOKEN_STR;
			case '\\':
				/* skip '\' */
				MemBlock_putStrN(mblock, str + *off, loff - *off - 1);
				parser->curOpt = EscapeToChar;
				res = EscapeToChar(parser, str, &loff, len);
				*off = loff;
				if(res != TOKEN_NONE) {
					return res;
				}
		}
	}
	/* thought it is pend, we still need to store the data */
	MemBlock_putStrN(mblock, str + *off, loff - *off);
	*off = loff;
	return TOKEN_PEND;
}
static uint8_t GetNum(JsonParser parser, const char* str, uint32_t* off, uint32_t len) {
	MemBlock mblock = &parser->buf;
	uint32_t   loff = *off;
	while(loff < len) {
		switch(str[loff]) {
			case '-': case '+': case '.':
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case 'e': case 'E':
				++loff;
				break;
			//case ',': case '}': case ']':
			default:
				MemBlock_putStrN(mblock, str + *off, loff - *off);
				*off = loff;
				{
					char* start = MemBlock_getPtr(mblock) + MemBlock_getStr(mblock);
					char* end;
					parser->tempVal.real = strtod(start, &end);
					if(end - start != strlen(start)) {
						parser->errnum = ERR_NUM;
						return TOKEN_INVALID;
					}
				}
				MemBlock_clear(mblock);
				parser->curOpt = NULL;
printf("NUM>> %lf\n", parser->tempVal.real);
				return TOKEN_NUM;
			//default:
			//	*off = loff;
			//	parser->errnum = ERR_NUM;
			//	return TOKEN_INVALID;
		}
	}
	MemBlock_putStrN(mblock, str + *off, loff - *off);
	*off = loff;
	return TOKEN_PEND;
}

static uint8_t GetTrue(JsonParser parser, const char* str, uint32_t* off, uint32_t len) {
	uint8_t  slen = parser->sbufLen;
	char*     buf = parser->sbuf;
	uint32_t loff = *off;
	while(loff < len && slen < 4) {
		buf[slen++] = str[loff++];
	}
	*off = loff;
	if(slen == 4) {
		parser->sbufLen = 0;
		if(strncmp(buf, "true", 4) == 0) {
			parser->curOpt = NULL;
			parser->tempVal.bol = 1;
printf("BOL: true\n");
			return TOKEN_BOOL;
		}
		parser->errnum = ERR_BOOL;
		return TOKEN_INVALID;
	}
	parser->sbufLen = slen;
	return TOKEN_PEND;
}

static uint8_t GetFalse(JsonParser parser, const char* str, uint32_t* off, uint32_t len) {
	uint8_t  slen = parser->sbufLen;
	char*     buf = parser->sbuf;
	uint32_t loff = *off;
	while(loff < len && slen < 5) {
		buf[slen++] = str[loff++];
	}
	*off = loff;
	if(slen == 5) {
		parser->sbufLen = 0;
		if(strncmp(buf, "false", 5) == 0) {
			parser->curOpt = NULL;
			parser->tempVal.bol = 0;
printf("BOL: false\n");
			return TOKEN_BOOL;
		}
		parser->errnum = ERR_BOOL;
		return TOKEN_INVALID;
	}
	parser->sbufLen = slen;
	return TOKEN_PEND;
}

static uint8_t GetNull(JsonParser parser, const char* str, uint32_t* off, uint32_t len) {
	uint8_t slen = parser->sbufLen;
	char*    buf = parser->sbuf;
	uint32_t loff = *off;
	while(loff < len && slen < 4) {
		buf[slen++] = str[loff++];
	}
	*off = loff;
	if(slen == 4) {
		parser->sbufLen = 0;
		if(strncmp(buf, "null", 4) == 0) {
			parser->curOpt = NULL;
			return TOKEN_NULL;
		}
		parser->errnum = ERR_NIL;
		return TOKEN_INVALID;
	}
	parser->sbufLen = slen;
	return TOKEN_PEND;
}
static Token_t GetNextToken(JsonParser parser, const char* str, uint32_t* off, uint32_t len) {
	Token_t res;
	if(parser->curOpt != NULL) {
		while(parser->curOpt != NULL) {
			res = parser->curOpt(parser, str, off, len);
			if(res == TOKEN_PEND) {
				parser->errnum = ERR_PEND;
				return TOKEN_PEND;
			} else if(res == TOKEN_INVALID) {
				return TOKEN_INVALID;
			}
		}
		return res;
	}
	uint32_t loff = *off;
AGAIN:
	if(loff >= len) {
		return TOKEN_PEND;
	}
	switch(str[loff++]) {
		case '{': res = TOKEN_OBJ_BEGIN; break;
		case '}': res = TOKEN_OBJ_END;   break;
		case '[': res = TOKEN_ARR_BEGIN; break;
		case ']': res = TOKEN_ARR_END;   break;
		case ':': res = TOKEN_COL;       break;
		case ',': res = TOKEN_CMA;       break;
		case '"':
			parser->curOpt = GetStr;
			res = GetStr(parser, str, &loff, len);
			break;
		case '-':
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			--loff;
			parser->curOpt = GetNum;
			res = GetNum(parser, str, &loff, len);
			break;
		case 't':
			--loff;
			parser->curOpt = GetTrue;
			res = GetTrue(parser, str, &loff, len);
			break;
		case 'f':
			--loff;
			parser->curOpt = GetFalse;
			res = GetFalse(parser, str, &loff, len);
			break;
		case 'n':
			--loff;
			parser->curOpt = GetNull;
			res = GetNull(parser, str, &loff, len);
			break;
		/* skip \n \s \t */
		case '\n':
			++parser->loc;
		case ' ': case '\t':
			goto AGAIN;
		default:
			parser->errnum = ERR_TOK;
			return TOKEN_INVALID;
	}

	*off = loff;
	return res;
}

JsonArray JsonArray_init(JsonArray arr) {
	arr->len = 0;
	arr->cap = 8;
	arr->elements = (JsonElement)calloc(8, sizeof(JsonElement_t));
	return arr;
}

JsonObject JsonObject_init(JsonObject obj) {
	obj->len = 0;
	obj->cap = 8;
	obj->pairs = (JsonPair)calloc(8, sizeof(JsonPair_t));
	return obj;
}

JsonElement JsonElement_init(JsonElement ele, JsonType_t type) {
	JsonElement res = ele;
	res->type = type;
	switch(type) {
		case JSON_STRING:
			res->val.str = NULL;
			break;
		case JSON_NUMBER:
			res->val.num = 0.0;
			break;
		case JSON_OBJECT:
			JsonObject_init(&res->val.obj);
			break;
		case JSON_ARRAY:
			JsonArray_init(&res->val.arr);
			break;
		case JSON_BOOLEAN:
			res->val.bol = 0;
			break;
		case JSON_NULL:
			break;
	}
	return res;
}


JsonParser JsonParser_init(JsonParser parser) {
	ArrayList_init(&parser->stack, DEFAULT_DEPTH);

	MemBlock_init(&parser->buf, 1024);

	/* initialize the stack for token stack */
	parser->token.cap = DEFAULT_DEPTH;
	parser->token.len = 0;
	parser->token.stack = (uint8_t*)malloc(sizeof(uint8_t) * DEFAULT_DEPTH);

	/* the line number of parser */
	parser->loc = 1;

	parser->curOpt = NULL;
	parser->curEle = NULL;

	parser->sbufLen = 0;
	parser->errnum = ERR_NONE;

	PushTokens(parser, 2, TOKEN_END, TOKEN_START);
	return parser;
}

void JsonParser_close(JsonParser parser) {
	ArrayList_free(&parser->stack, NULL);
	MemBlock_free(&parser->buf);
	free(parser->token.stack);
}

JsonElement JsonDocument_putRoot(JsonDocument doc, JsonType_t type) {
	if(type != JSON_OBJECT && type != JSON_ARRAY) {
		return NULL;
	}
	JsonElement res = &doc->root;
	JsonElement_init(res, type);
	return res;
}

JsonElement JsonDocument_getRoot(JsonDocument doc) {
	return &doc->root;
}

static void JsonElement_free(JsonElement ele) {
	JsonArray arr;
	JsonObject obj;
	JsonPair pair;
	switch(ele->type) {
		case JSON_NUMBER:
		case JSON_NULL:
		case JSON_BOOLEAN:
			break;
		case JSON_STRING:
			free((void*)ele->val.str);
			break;
		case JSON_ARRAY:
			arr = &ele->val.arr;
			for(uint32_t i = 0; i < arr->len; ++i) {
				JsonElement_free(arr->elements + i);
			}
			free(arr->elements);
			break;
		case JSON_OBJECT:
			obj = &ele->val.obj;
			for(uint32_t i = 0; i < obj->len; ++i) {
				pair = obj->pairs + i;
				free((void*)pair->key);
				JsonElement_free(&pair->val);
			}
			free(obj->pairs);
			break;
	}
	return;
}

void JsonDocument_free(JsonDocument doc) {
	JsonElement root = &doc->root;
	JsonElement_free(root);
}

JsonElement JsonArray_newElement(JsonElement ele) {
	JsonArray arr = &ele->val.arr;
	uint32_t cap = arr->cap;
	uint32_t len = arr->len;
	if(len >= cap) {
		while(len >= cap) {
			cap <<= 1;
		}
		arr->elements = (JsonElement)realloc(arr->elements, sizeof(JsonElement_t) * cap);
		arr->cap = cap;
	}
	JsonElement res = arr->elements + len;
	arr->len = len + 1;
	return res;
}

JsonElement JsonArray_putElement(JsonElement ele, JsonType_t type) {
	JsonElement res = JsonArray_newElement(ele);
	JsonElement_init(res, type);
	return res;
}

JsonElement JsonArray_getElement(JsonElement ele, uint32_t index) {
	JsonArray arr = &ele->val.arr;
	if(index >= arr->len) {
		return NULL;
	}
	return arr->elements + index;
}
JsonElement JsonObject_newElement(JsonElement ele, const char* key) {
	JsonObject obj = &ele->val.obj;
	uint32_t cap = obj->cap;
	uint32_t len = obj->len;
	if(len >= cap) {
		while(len >= cap) {
			cap <<= 1;
		}
		obj->pairs = (JsonPair)realloc(obj->pairs, sizeof(JsonPair_t) * cap);
		obj->cap = cap;
	}
	JsonPair pair = obj->pairs + len;
	pair->key = key;
	obj->len = len + 1;
	return &pair->val;
}
JsonElement JsonObject_putElement(JsonElement ele, const char* key, JsonType_t type) {
	JsonElement res = JsonObject_newElement(ele, key);
	JsonElement_init(res, type);
	return res;
}

JsonElement JsonObject_getElement(JsonElement ele, const char* key) {
	JsonObject obj = &ele->val.obj;
	JsonPair pairs = obj->pairs;
	uint32_t len = obj->len;
	for(uint32_t i = 0; i < len; ++i) {
		if(strcmp(key, pairs[i].key) == 0) {
			return &pairs[i].val;
		}
	}
	return NULL;
}


JsonDocument Json_parseFromFile(JsonParser parser, const char* filename, JsonDocument doc) {
	/* use mode r instead of rb */
	/* to avoid process the different newline CR on different platforms */
	FILE* file = fopen(filename, "r");
	if(file == NULL) {
		parser->errnum = ERR_FOPEN;
		fclose(file);
		return NULL;
	}
	
	JsonDocument res;
	char buf[4096];
	size_t len;
	uint8_t isFinish = 0;
	while(!isFinish) {
		len = fread(buf, 1, 4096, file);
		if(len < 4096) {
			if(ferror(file)) {
				parser->errnum = ERR_FREAD;
				return NULL;
			}
			//buf[len++] = '\0';
			isFinish = 1;
		}
		res = Json_parseFromString(parser, buf, len, doc);
		/* if ERR except ERR_NONE and ERR_PEND happen, return immediately */
		if(parser->errnum > ERR_PEND) {
			fclose(file);
			return NULL;
		}
	}
	/* all content is passed to parser */
	if(TopToken(parser) != TOKEN_END) {
		fclose(file);
		return NULL;
	}
	/* top == TOKEN_END && errnum == ERR_PEND */
	parser->errnum = ERR_NONE;
	fclose(file);
	return res;
}

JsonDocument Json_parseFromString(JsonParser parser, const char* str, size_t len, JsonDocument doc) {
	uint32_t off = 0;
	ArrayList stack = &parser->stack;

	JsonElement nextEle;

	Token_t tk, top;
	tk = GetNextToken(parser, str, &off, len);
	if(tk == TOKEN_PEND) {
		parser->errnum = ERR_PEND;
		return NULL;
	} else if(tk == TOKEN_INVALID) {
		return NULL;
	}
	while((top = TopToken(parser)) != TOKEN_NONE) {
		printf("TOP: %s | TK: %s | SZ: %d\n", TK_STR[TopToken(parser)], TK_STR[tk], parser->token.len);
		switch(top) {
			case TOKEN_START:
				nextEle = &doc->root;
				if(tk == TOKEN_OBJ_BEGIN) {
					JsonElement_init(nextEle, JSON_OBJECT);
					ModTopToken(parser, TOKEN_OBJ);
				} else if(tk == TOKEN_ARR_BEGIN) {
					JsonElement_init(nextEle, JSON_ARRAY);
					ModTopToken(parser, TOKEN_ARR);
				} else {
					parser->errnum = ERR_STAR;
					return NULL;
				}
				ArrayList_push(stack, nextEle);
				break;
			case TOKEN_OBJ:
				if(tk == TOKEN_OBJ_BEGIN) {
					PopToken(parser);
					PushTokens(parser, 3,
							TOKEN_OBJ_END,
							TOKEN_MEMBERS,
							TOKEN_OBJ_BEGIN);
				} else {
					parser->errnum = ERR_EOBJ;
					return NULL;
				}
				break;
			case TOKEN_MEMBERS:
				if(tk == TOKEN_STR) {
					PushTokens(parser, 2,
							TOKEN_CMA,
							TOKEN_PAIR);
				} else if(tk == TOKEN_OBJ_END) {
					PopToken(parser);
				} else {
					parser->errnum = ERR_EMEM;
					return NULL;
				}
				break;
			case TOKEN_PAIR:
				if(tk == TOKEN_STR) {
					parser->curEle = JsonObject_newElement((JsonElement)ArrayList_top(stack), parser->tempVal.str);
					PopToken(parser);
					PushTokens(parser, 3,
							TOKEN_VAL,
							TOKEN_COL,
							TOKEN_STR);
				} else {
					parser->errnum = ERR_EPAIR;
					return NULL;
				}
				break;
			case TOKEN_ARR:
				if(tk == TOKEN_ARR_BEGIN) {
					PopToken(parser);
					PushTokens(parser, 3,
							TOKEN_ARR_END,
							TOKEN_ELEMENTS,
							TOKEN_ARR_BEGIN);
				} else {
					parser->errnum = ERR_EARR;
					return NULL;
				}
				break;
			case TOKEN_ELEMENTS:
				switch(tk) {
					case TOKEN_OBJ_BEGIN:
					case TOKEN_ARR_BEGIN:
					case TOKEN_STR:  case TOKEN_NUM:
					case TOKEN_BOOL: case TOKEN_NULL:
						parser->curEle = JsonArray_newElement((JsonElement)ArrayList_top(stack));
						PushTokens(parser, 2, 
								TOKEN_CMA,
								TOKEN_VAL);
						break;
					case TOKEN_ARR_END:
						PopToken(parser);
						break;
					default:
						parser->errnum = ERR_EELE;
						return NULL;
				}
				break;
			case TOKEN_VAL:
				/* check which element the value belongs to */
				nextEle = parser->curEle;
				parser->curEle = NULL;
				switch(tk) {
					case TOKEN_OBJ_BEGIN:
						JsonElement_init(nextEle, JSON_OBJECT);
						ArrayList_push(stack, nextEle);
						ModTopToken(parser, TOKEN_OBJ);
						break;
					case TOKEN_ARR_BEGIN:
						JsonElement_init(nextEle, JSON_ARRAY);
						ArrayList_push(stack, nextEle);
						ModTopToken(parser, TOKEN_ARR);
						break;
					case TOKEN_STR:
						nextEle->type = JSON_STRING;
						nextEle->val.str = parser->tempVal.str;
						ModTopToken(parser, TOKEN_STR);
						break;
					case TOKEN_NUM:
						nextEle->type = JSON_NUMBER;
						nextEle->val.num = parser->tempVal.real;
						ModTopToken(parser, TOKEN_NUM);
						break;
					case TOKEN_BOOL:
						nextEle->type = JSON_BOOLEAN;
						nextEle->val.bol = parser->tempVal.bol;
						ModTopToken(parser, TOKEN_BOOL);
						break;
					case TOKEN_NULL:
						nextEle->type = JSON_NULL;
						ModTopToken(parser, TOKEN_NULL);
						break;
					default:
						parser->errnum = ERR_EVAL;
						return NULL;
				}
				break;
			case TOKEN_END:
				if(tk != TOKEN_PEND) {
					parser->errnum = ERR_FIN;
					return NULL;
				} 
				/* do not pop TOKEN_END */
				/* used to prevent continue parsing */
				parser->errnum = 0;
				return doc;
			case TOKEN_CMA:
				/* comma can be skip */
				if(tk == TOKEN_OBJ_END || tk == TOKEN_ARR_END) {
					PopToken(parser);
					break;
				}
				/* break is not necessary here */
			default:
				if(top == tk) {
					if(tk == TOKEN_OBJ_END || tk == TOKEN_ARR_END) {
						ArrayList_pop(stack);
					}
					PopToken(parser);
					tk = GetNextToken(parser, str, &off, len);
					if(tk == TOKEN_PEND) {
						parser->errnum = ERR_PEND;
						return NULL;
					} else if(tk == TOKEN_INVALID) {
						return NULL;
					}
				} else {
					parser->errnum = ERR_ETOK;
					return NULL;
				}
		}
	}
	// TODO : add error check
	return doc;
}

JsonElement JsonDocument_findElement(JsonDocument doc, JPath path) {
	uint32_t len = path->len;
	JsonElement ele = NULL;
	JPathNav navs = path->navs;
	JPathNav nav;
	for(uint32_t i = 0; i < len; ++i) {
		nav = navs + i;
		switch(nav->type) {
			case NAV_ROOT:
				ele = &doc->root;
				break;
			case NAV_INDEX:
				if(JsonElement_getType(ele) != JSON_ARRAY) {
					return NULL;
				}
				ele = JsonArray_getElement(ele, nav->key.num);
				break;
			case NAV_POINT:
				if(JsonElement_getType(ele) != JSON_OBJECT) {
					return NULL;
				}
				ele = JsonObject_getElement(ele, nav->key.str);
				break;
		}
		if(ele == NULL) {
			return NULL;
		}
	}
	return ele;
}

