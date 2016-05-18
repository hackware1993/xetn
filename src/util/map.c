#include "util/map.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

typedef struct hash_node {
	Pair_t            pair;
	uint32_t          hash;
	struct hash_node* next;
	struct hash_node* sibling;
} HashNode_t, *HashNode;

PRIVATE INLINE HashNode HashNode_new(HashMap map, uint32_t hash,
		const char* key, void* val) {
	HashNode node   = (HashNode)calloc(1, sizeof(HashNode_t));
	node->pair.next = map->pairs;
	map->pairs      = &node->pair;
	node->hash      = hash;
	Pair pair       = &node->pair;
	pair->key       = key;
	pair->val       = val;
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
	Pair    p = map->pairs;
	Pair temp = NULL;

	while(p != NULL) {
		temp = p;
		p = p->next;
		free((void*)temp - offsetof(HashNode_t, pair));
	}
	map->pairs = NULL;
	memset(map->bucket, 0, sizeof(HashNode) * map->len);
}

HashMap HashMap_init(HashMap map) {
	if(map == NULL) {
		return NULL;
	}
	map->len    = INIT_BUCK_SIZE;
	map->bucket = (HashNode*)calloc(map->len, sizeof(HashNode));
	map->pairs  = NULL;
	return map;
}

void HashMap_free(HashMap map) {
	if(map == NULL) {
		return;
	}
	Pair    p = map->pairs;
	Pair temp = NULL;

	while(p != NULL) {
		temp = p;
		p = p->next;
		free((void*)temp - offsetof(HashNode_t, pair));
	}
	free(map->bucket);
	map->pairs = NULL;
}

void* HashMap_put(HashMap map, const char* key, void* val) {
	if(key == NULL) {
		return NULL;
	}
	uint32_t  hash   = BKDRHash(key);
	uint32_t  index  = hash % map->len;
	HashNode* bucket = map->bucket;
	HashNode  node   = NULL;
	HashNode  p      = bucket[index];
	if(p) {
		/* p points to the tail of list */
		HashNode tail = p;
		do {
			if(p->hash == hash) {
				HashNode* dp = &p;
				while(*dp != NULL) {
					/* if there already exists the node with same key */
					// TODO: there may exists stack overflow bug, consider using strncmp
					if(strcmp(key, (*dp)->pair.key) == 0) {
						return NULL;
					}
					dp = &(*dp)->sibling;
				}
				node = HashNode_new(map, hash, key, val); 
				(*dp)->sibling = node;
				break;
			}
			p = p->next;
		} while(p != tail);
		if(p == tail) {
			/* node with same hash doesn't exist */
			node->next = tail->next;
			node = HashNode_new(map, hash, key, val); 
			tail->next = node;
		}
	} else {
		/* link the only node to it self */
		node = HashNode_new(map, hash, key, val); 
		node->next = node;
	}
	/* setup the new tail */
	bucket[index] = node;
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
		HashNode tail = p;
		do {
			if(p->hash == hash) {
				HashNode* dp = &p;
				/* if found the target hash, then iterate its subling */
				/* search the pair through strcmp */
				// TODO: there may exists stack overflow bug, consider using strncmp
				while(*dp != NULL) {
					if(strcmp(key, (*dp)->pair.key) == 0) {
						/* if found the target key, then setup the res */
						res = (*dp)->pair.val;
						break;
					}
					dp = &(*dp)->sibling;
				}
				/* NOTICE: res can be NULL here */
				break;
			}
			p = p->next;
		} while(p != tail);
	}
	return res;
}

Pair HashMap_getPairList(HashMap map) {
	return map->pairs;
}

