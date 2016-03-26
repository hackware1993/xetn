#include "list.h"
#include "../optimize.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PRIVATE static

LinkList LinkList_init(LinkList li) {
	li->list = NULL;
	li->len  = 0;
	return li;
}

void LinkList_clear(LinkList li) {
	size_t i;
	SLink p = li->list->next;
	SLink temp;
	for(i = 0; i < li->len; ++i) {
		temp = p;
		p = p->next;
		temp->next = NULL;
	}
	li->list = NULL;
	li->len  = 0;
}

void LinkList_free(LinkList li, Element_free free_cb) {
	size_t i;
	SLink p = li->list->next;
	SLink temp;
	for(i = 0; i < li->len; ++i) {
		temp = p;
		p = p->next;
		temp->next = NULL;
		if(free_cb == NULL) {
			free(temp);
		} else {
			free_cb(temp);
		}
	}
	li->list = NULL;
	li->len  = 0;
}

void LinkList_append(LinkList dest, LinkList src) {
	if(UNLIKELY(src->len == 0)) {
		return;
	}
	if(LIKELY(dest->list != NULL)) {
		SLink dtail = dest->list;
		SLink stail = src->list;
		SLink temp  = dtail->next;
		dtail->next = stail->next;
		stail->next = temp;
	}
	dest->len += src->len;
	dest->list = src->list;
	/* reset src list*/
	src->list = NULL;
	src->len = 0;
}

/* put node to the list as new tail */
void LinkList_put(LinkList li, SLink le) {
	SLink tail = li->list;
	/* check if the list is empty */
	if(LIKELY(tail != NULL)) {
		/* put the node as the head */
		le->next   = tail->next;
		tail->next = le;
		/* set new node as the current node */
		li->list   = le;
	} else {
		/* link the node with itself */
		le->next = le;
		li->list = le;
	}
	++li->len;
}

/* get node from list as old head */
SLink LinkList_get(LinkList li) {
	SLink tail = li->list;
	/* of course, return NULL if the list is empty */
	if(UNLIKELY(tail == NULL)) {
		return NULL;
	}
	SLink head = tail->next;
	/* check if the list is going to be empty */
	if(LIKELY(tail != head)) {
		tail->next = head->next;
	} else {
		/* if head == tail, set list to NULL */
		li->list = NULL;
	}
	--li->len;
	/* reset the next pointer */
	head->next = NULL;
	return head;
}

void LinkList_push(LinkList li, SLink le) {
	SLink tail = li->list;
	if(LIKELY(tail != NULL)) {
		le->next = tail->next;
		tail->next = le;
	} else {
		le->next = le;
		li->list = le;
	}
	++li->len;
}

SLink LinkList_pop(LinkList li) {
	SLink tail = li->list;
	if(UNLIKELY(tail == NULL)) {
		return NULL;
	}
	SLink head = tail->next;
	if(LIKELY(tail != head)) {
		tail->next = head->next;
	} else {
		li->list = NULL;
	}
	--li->len;
	/* reset the next pointer */
	head->next = NULL;
	return head;
}

void LinkList_inverse(LinkList li) {
	SLink cur_node = li->list;
	SLink pnext = cur_node->next;
	SLink nxt_node = NULL;
	li->list = pnext;
	size_t i;
	for(i = 0; i < li->len; ++i) {
		nxt_node = pnext;
		pnext    = nxt_node->next;
		nxt_node->next = cur_node;
		cur_node = nxt_node;
	}
}

ArrayList ArrayList_init(ArrayList li, size_t len) {
	li->len = 0;
	li->cap = len;
	li->list = malloc(sizeof(void*) * len);
	return li;
}

void ArrayList_free(ArrayList li, Element_free free_cb) {
	void** list = li->list;
	if(free_cb == NULL) {
		free_cb = free;
	}
	size_t i;
	for(i = 0; i < li->len; ++i) {
		free_cb(list[i]);
	}
	li->len = 0;
	li->cap = 0;
	free(list);
}

void ArrayList_push(ArrayList li, void* content) {
	size_t cur = li->len;
	size_t cap = li->cap;
	while(cur >= cap) {
		cap <<= 1;
	}
	if(cap != li->cap) {
		li->cap = cap;
		li->list = realloc(li->list, cap);
	}
	li->list[li->len++] = content;
}

void* ArrayList_pop(ArrayList li) {
	return li->list[--li->len];
}

void ArrayList_set(ArrayList li, size_t index, void* content) {
	size_t cap = li->cap;
	while(index >= cap) {
		cap <<= 1;
	}
	if(cap != li->cap) {
		li->cap = cap;
		li->list = realloc(li->list, cap);
	}
	li->list[index] = content;
}

void* ArrayList_get(ArrayList li, size_t index) {
	return li->list[index];
}

RingList RingList_init(RingList list, size_t size) {
	uint32_t cap = 1;
	while(cap < size) {
		cap <<= 1;
	}
	list->cap  = cap;
	list->len  = 0;
	list->head = 0;
	list->tail = 0;
	list->zone = (void*)malloc(sizeof(void*) * cap);
	return list;
}

void RingList_free(RingList list, Element_free free_cb) {
	if(free_cb == NULL) {
		free_cb = free;
	}
	size_t head = list->head;
	size_t i;
	for(i = 0; i < list->len; ++i) {
		free_cb(list->zone[head++]);
		head = head & (list->cap - 1);
	}
	list->head = 0;
	list->tail = 0;
	list->len  = 0;
}

void RingList_put(RingList list, void* ele) {
	size_t tail = list->tail;
	list->zone[tail++] = ele;
	list->tail = tail & (list->cap - 1);
	++list->len;
}

void* RingList_get(RingList list) {
	size_t head = list->head;
	void* res = list->zone[head++];
	list->head = head & (list->cap - 1);
	--list->len;
	return res;
}
