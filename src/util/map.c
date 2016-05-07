#include "util/map.h"

#include <stdlib.h>
#include <string.h>

typedef struct hash_node {
	struct hash_node* next;
	struct hash_node* last;
	uint32_t hash;
	void*    val;
} hash_node_t, *HashNode;

PRIVATE INLINE HashNode HashNode_new(HashMap map, uint32_t hash, void* val) {
	HashNode node = (HashNode)malloc(sizeof(hash_node_t));
	node->last = map->nodes;
	map->nodes = node;
	node->next = NULL;
	node->hash = hash;
	node->val  = val;
	return node;
}

PRIVATE INLINE uint32_t BKDRHash(const char* str) {
	unsigned hash = 0;
	while(*str) {
		hash = (hash << 7) + (hash << 1) + hash + *str++;
	}
	return hash;
}

void HashMap_clear(HashMap map) {
	if(map == NULL) {
		return;
	}
	HashNode p = map->nodes;
	HashNode temp = NULL;

	while(p != NULL) {
		temp = p;
		p = p->last;
		free(temp);
	}
	map->nodes = NULL;
	memset(map->bucket, 0, sizeof(HashNode) * map->len);
}

HashMap HashMap_init(HashMap map) {
	if(map == NULL) {
		return NULL;
	}
	map->len = INIT_BUCK_SIZE;
	map->bucket = (HashNode*)calloc(map->len, sizeof(HashNode));
	map->nodes = NULL;
	return map;
}

void HashMap_free(HashMap map) {
	if(map == NULL) {
		return;
	}
	HashNode p = map->nodes;
	HashNode temp = NULL;

	while(p != NULL) {
		temp = p;
		p = p->last;
		free(temp);
	}
	map->nodes = NULL;
}

void* HashMap_put(HashMap map, const char* key, void* val) {
	if(key == NULL) {
		return NULL;
	}
	uint32_t hash = BKDRHash(key);
	uint32_t index = hash % map->len;
	HashNode* bucket = map->bucket;
	HashNode  node = HashNode_new(map, hash, val);
	HashNode  p = bucket[index];
	if(p) {
		node->next = p->next;
		p->next = node;
		bucket[index] = node;
	} else {
		bucket[index] = node;
		node->next = node;
	}
	return val;
}

void* HashMap_get(HashMap map, const char* key) {
	if(key == NULL) {
		return NULL;
	}
	void* res = NULL;
	uint32_t hash = BKDRHash(key);
	uint32_t index = hash % map->len;
	HashNode p = map->bucket[index];
	if(p) {
		HashNode end = p;
		do {
			if(p->hash == hash) {
				res = p->val;
				break;
			}
			p = p->next;
		} while(p != end);
	}
	return res;
}

