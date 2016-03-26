#include "list.h"
#include "../optimize.h"

#include <stdio.h>

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

ArrayList ArrayList_init(ArrayList li, uint32_t len) {
	li->pos = 0;
	li->len = len;
	li->list = malloc(sizeof(void*) * len);
	return li;
}

void ArrayList_free(ArrayList li) {
	li->len = 0;
	li->pos = 0;
	free(li->list);
}

void ArrayList_push(ArrayList li, void* content) {
	if(li->pos >= li->len) {
		fprintf(stderr, "ERROR<ArrayList_push>: exceed the boundary of ArrayList\n");
		exit(EXIT_FAILURE);
	}
	li->list[li->pos++] = content;
}

void* ArrayList_pop(ArrayList li) {
	if(li->pos == 0) {
		fprintf(stderr, "ERROR<ArrayList_pop>: exceed the boundary of ArrayList\n");
		exit(EXIT_FAILURE);
	}
	return li->list[--li->pos];
}

void ArrayList_set(ArrayList li, uint32_t index, void* content) {
	if(index >= li->len) {
		fprintf(stderr, "ERROR<ArrayList_set>: exceed the boundary of ArrayList\n");
		exit(EXIT_FAILURE);
	}
	li->list[index] = content;
}

void* ArrayList_get(ArrayList li, uint32_t index) {
	if(index >= li->len) {
		fprintf(stderr, "ERROR<ArrayList_get>: exceed the boundary of ArrayList\n");
		exit(EXIT_FAILURE);
	}
	return li->list[index];
}
uint32_t ArrayList_length(ArrayList li) {
	return li->pos;
}

void ArrayList_clear(ArrayList li) {
	li->pos = 0;
}

RingList RingList_init(RingList list, uint32_t size) {
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

void RingList_put(RingList list, void* ele) {
	uint32_t tail = list->tail;
	list->zone[tail++] = ele;
	list->tail = tail & (list->cap - 1);
	++list->len;
}

void* RingList_get(RingList list) {
	uint32_t head = list->head;
	void* res = list->zone[head++];
	list->head = head & (list->cap - 1);
	--list->len;
	return res;
}
