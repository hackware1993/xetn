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
	//node->pair.next = map->pairs;
	//map->pairs      = &node->pair;
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
	/*
	Pair    p = map->pairs;
	Pair temp = NULL;

	while(p != NULL) {
		temp = p;
		p = p->next;
		free((void*)temp - offsetof(HashNode_t, pair));
	}
	map->pairs = NULL;
	*/
	HashNode temp;
	HashNode* bucket = map->bucket;
	HashNode loc, sloc;
	for(uint32_t i = 0; i < map->len; ++i) {
		loc = bucket[i];
		while(loc) {
			if(loc->sibling) {
				sloc = loc->sibling;
				while(sloc) {
					temp = sloc;
					sloc = sloc->sibling;
					free(temp);
				}
			}
			temp = loc;
			loc  = loc->next;
			free(temp);
		}
		bucket[i] = NULL;
	}
	//memset(map->bucket, 0, sizeof(HashNode) * map->len);
}

HashMap HashMap_init(HashMap map) {
	if(map == NULL) {
		return NULL;
	}
	map->len    = INIT_BUCK_SIZE;
	map->bucket = (HashNode*)calloc(map->len, sizeof(HashNode));
	return map;
}

void HashMap_free(HashMap map) {
	if(map == NULL) {
		return;
	}
	/*
	Pair    p = map->pairs;
	Pair temp = NULL;

	while(p != NULL) {
		temp = p;
		p = p->next;
		free((void*)temp - offsetof(HashNode_t, pair));
	}
	*/
	HashNode temp;
	HashNode* bucket = map->bucket;
	HashNode loc, sloc;
	for(uint32_t i = 0; i < map->len; ++i) {
		loc = bucket[i];
		while(loc) {
			if(loc->sibling) {
				sloc = loc->sibling;
				while(sloc) {
					temp = sloc;
					sloc = sloc->sibling;
					free(temp);
				}
			}
			temp = loc;
			loc  = loc->next;
			free(temp);
		}
	}
	free(map->bucket);
}

void* HashMap_put(HashMap map, const char* key, void* val) {
	if(key == NULL) {
		return NULL;
	}
	uint32_t  hash   = BKDRHash(key);
	uint32_t  index  = hash % map->len;
	HashNode  node   = NULL;
	/* loc the the pointer of place whihc is going to point to the new node */
	HashNode* loc    = map->bucket + index;
	while(*loc) {
		if((*loc)->hash == hash) {
			/* if there already exists the node with same key */
			while(*loc) {
				// TODO: there may exists stack overflow bug, consider using strncmp
				if(strcmp(key, (*loc)->pair.key) == 0) {
					return NULL;
				}
				loc = &(*loc)->sibling;
			}
			break;
		}
		loc = &(*loc)->next;
	}
	*loc = HashNode_new(map, hash, key, val);
	return val;
}

void* HashMap_get(HashMap map, const char* key) {
	if(key == NULL) {
		return NULL;
	}
	uint32_t hash  = BKDRHash(key);
	uint32_t index = hash % map->len;
	HashNode loc  = map->bucket[index];
	while(loc) {
		if(loc->hash == hash) {
			/* if found the target hash, then iterate its subling */
			while(loc) {
				// TODO: there may exists stack overflow bug, consider using strncmp
				if(strcmp(key, loc->pair.key) == 0) {
					return loc->pair.val;
				}
				loc = loc->sibling;
			}
			break;
		}
		loc = loc->next;
	}
	return NULL;
}

//Pair HashMap_getPairList(HashMap map) {
//	return map->pairs;
//}

int8_t HashMap_contains(HashMap map, const char* key) {
	if(key == NULL) {
		return 0;
	}
	uint32_t hash = BKDRHash(key);
	uint32_t index = hash % map->len;
	HashNode p = map->bucket[index];
	while(p) {
		if(p->hash == hash) {
			while(p) {
				// TODO: there may exists stack overflow bug, consider using strncmp
				if(strcmp(key, p->pair.key) == 0) {
					return 1;
				}
				p = p->sibling;
			}
			break;
		}
		p = p->next;
	}
	return 0;
}

void* HashMap_delete(HashMap map, const char* key) {
	if(key == NULL) {
		return NULL;
	}
	uint32_t hash  = BKDRHash(key);
	uint32_t index = hash % map->len;
	HashNode* loc  = map->bucket + index;
	HashNode temp;
	while(*loc) {
		if((*loc)->hash == hash) {
			uint32_t i = 0;
			/* if found the target hash, then iterate its subling */
			while(*loc) {
				// TODO: there may exists stack overflow bug, consider using strncmp
				if(strcmp(key, (*loc)->pair.key) == 0) {
					temp = *loc;
					if(i == 0) {
						if(temp->sibling) {
							*loc = temp->sibling;
							(*loc)->next = temp->next;
						} else {
							*loc = temp->next;
						}
					} else {
						*loc = temp->sibling;
					}
					void* res = temp->pair.val;
					free(temp);
					return res;
				}
				loc = &(*loc)->sibling;
				++i;
			}
			break;
		}
		loc = &(*loc)->next;
	}
	return NULL;
}
