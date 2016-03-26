#ifndef _LIST_H_
#define _LIST_H_

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

/* use this macro to get the specific pointer */
/* you should provide type and member to calculate the offset */
#define Element_getPtr(p, t, m) \
	((t*)((char*)p - offsetof(t, m)))

typedef void (*Element_free)(void*);

typedef struct slink {
	struct slink *next;
} slink_t, *SLink; 

typedef struct dlink {
	struct dlink* prev;
	struct dlink* next;
} dlink_t, *DLink;

/* LinkList can be used as stack or list */
/* BUT DO NOT use them together */
typedef struct link_list {
	SLink  list;
	size_t len;
} link_list_t, *LinkList;

LinkList LinkList_init(LinkList);
void     LinkList_free(LinkList, Element_free);
void     LinkList_clear(LinkList);
void     LinkList_put(LinkList, SLink);
SLink    LinkList_get(LinkList);
void     LinkList_push(LinkList, SLink);
SLink    LinkList_pop(LinkList);
void     LinkList_append(LinkList, LinkList);
void     LinkList_inverse(LinkList);
#define  LinkList_length(l) ((l)->len)

typedef struct array_list {
	void*    *list;
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


typedef struct ring_list {
	uint32_t head;
	uint32_t tail;
	uint32_t cap;
	uint32_t len;
	void**   zone;
} ring_list_t, *RingList;

RingList RingList_init(RingList, uint32_t);
void     RingList_put(RingList, void*);
void*    RingList_get(RingList);
#define  RingList_close(l) \
	free((l)->zone)
#define  RingList_length(l) \
	(l)->len
#define  RingList_capacity(l) \
	(l)->cap

#endif // _LIST_H_
