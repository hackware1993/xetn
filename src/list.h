#ifndef _LIST_H_
#define _LIST_H_

#include <stdint.h>

struct link_node;

typedef struct link_list {
	struct link_node* list;
	uint32_t len;
} link_list_t, *LinkList;

typedef struct array_list {
	void** list;
	uint32_t len;
	uint32_t pos;
} array_list_t, *ArrayList;

ArrayList ArrayList_init(ArrayList, uint32_t);
void      ArrayList_free(ArrayList);
void      ArrayList_clear(ArrayList);
void      ArrayList_push(ArrayList, void*);
void*     ArrayList_pop(ArrayList);
void      ArrayList_set(ArrayList, uint32_t, void*);
void*     ArrayList_get(ArrayList, uint32_t);
uint32_t  ArrayList_length(ArrayList);

LinkList LinkList_init(LinkList);
void     LinkList_free(LinkList);
void     LinkList_put(LinkList, void*);
void*    LinkList_get(LinkList);
void     LinkList_push(LinkList, void*);
void*    LinkList_pop(LinkList);
uint32_t LinkList_length(LinkList);

#endif // _LIST_H_
