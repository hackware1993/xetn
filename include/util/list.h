#ifndef _LIST_H_
#define _LIST_H_

#include <stdint.h>
#include <stddef.h>

/* #NOTICE#
 * You can use linklist as a list or a stack
 * BUT list can not use the operations of stack, vice versa
 */

/* 
 * when you use SLinkList or DLinkList,
 * this macro can help you get the real pointer of element
 */
#define Element_getPtr(p, t, m) \
	((t*)((char*)p - offsetof(t, m)))

/*
 * destruct callback function of element
 */
typedef void (*Element_free)(void*);

typedef struct slink {
	struct slink *next;
} SLink_t, *SLink; 

typedef struct dlink {
	struct dlink* prev;
	struct dlink* next;
} DLink_t, *DLink;

/* LinkedList with single link */
typedef struct slink_list {
	SLink  list;
	size_t len;
} SLinkList_t, *SLinkList;

SLinkList SLinkList_init(SLinkList);
void      SLinkList_free(SLinkList, Element_free);
void      SLinkList_clear(SLinkList);
void      SLinkList_put(SLinkList, SLink);
SLink     SLinkList_get(SLinkList);
void      SLinkList_push(SLinkList, SLink);
SLink     SLinkList_pop(SLinkList);
void      SLinkList_append(SLinkList, SLinkList);
void      SLinkList_inverse(SLinkList);
#define   SLinkList_length(l) ((l)->len)

/* LinkedList with double list */
typedef struct dlink_list {
	DLink list;
	size_t len;
} DLinkList_t, *DLinkList;

DLinkList DLinkList_init(DLinkList);
void      DLinkList_free(DLinkList, Element_free);
void      DLinkList_clear(DLinkList);
void      DLinkList_remove(DLinkList, DLink);
void      DLinkList_insert(DLinkList, DLink, DLink);
void      DLinkList_put(DLinkList, DLink);
DLink     DLinkList_get(DLinkList);
void      DLinkList_push(DLinkList, DLink);
DLink     DLinkList_pop(DLinkList);
void      DLinkList_append(DLinkList, DLinkList);
void      DLinkList_inverse(DLinkList);
#define   DLinkList_length(l) ((l)->len)

/* ArrayList with automatically expend mechanism */
typedef struct array_list {
	void*  *list;
	size_t cap;
	size_t len;
} ArrayList_t, *ArrayList;

ArrayList ArrayList_init(ArrayList, size_t);
void      ArrayList_free(ArrayList, Element_free);
void      ArrayList_push(ArrayList, void*);
void*     ArrayList_pop(ArrayList);
void*     ArrayList_top(ArrayList);
void      ArrayList_set(ArrayList, size_t, void*);
void*     ArrayList_get(ArrayList, size_t);
#define   ArrayList_length(l)   ((l)->len)
#define   ArrayList_capacity(l) ((l)->cap)
#define   ArrayList_clear(l)    ((l)->len = 0)

/* Size-fixed RingList */
typedef struct ring_list {
	size_t head;
	size_t tail;
	size_t cap;
	size_t len;
	void*  *zone;
} RingList_t, *RingList;

RingList RingList_init(RingList, size_t);
void     RingList_free(RingList, Element_free);
void     RingList_put(RingList, void*);
void*    RingList_get(RingList);
#define  RingList_length(l)   ((l)->len)
#define  RingList_capacity(l) ((l)->cap)
#define  RingList_clear(l) \
{ (l)->len = 0; (l)->head = 0; (l)->tail = 0; }

#endif // _LIST_H_
