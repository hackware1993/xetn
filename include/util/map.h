#ifndef _MAP_H_
#define _MAP_H_

#include "../common.h"

#define INIT_BUCK_SIZE 4096

struct hash_node;

typedef struct hash_map {
	 struct hash_node** bucket;
	 struct hash_node* nodes;
	 uint32_t len;
} hash_map_t, *HashMap;

HashMap HashMap_init(HashMap);
void    HashMap_free(HashMap);
void    HashMap_clear(HashMap);
void*   HashMap_put(HashMap, const char*, void*);
void*   HashMap_get(HashMap, const char*);

#endif //_MAP_H_
