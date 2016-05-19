#ifndef _MAP_H_
#define _MAP_H_

#include "common.h"

#define INIT_BUCK_SIZE 4096

struct hash_node;

typedef struct pair {
	const char*  key;
	void*        val;
} Pair_t, *Pair;

typedef struct hash_map {
	struct hash_node** bucket;
	//Pair   pairs;
	uint32_t len;
} HashMap_t, *HashMap;

HashMap HashMap_init(HashMap);
void    HashMap_free(HashMap);
void    HashMap_clear(HashMap);
void*   HashMap_put(HashMap, const char*, void*);
void*   HashMap_get(HashMap, const char*);
int8_t  HashMap_contains(HashMap, const char*);

// TODO
void*   HashMap_delete(HashMap, const char*);
//Pair    HashMap_getPair(HashMap, uint32_t);
//Pair    HashMap_putPair(HashMap, uint32_t, Pair);

#endif //_MAP_H_
