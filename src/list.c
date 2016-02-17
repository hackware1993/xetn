#include "list.h"

#include <stdio.h>

#include "optimize.h"

#define PRIVATE static

typedef struct link_node {
	void* content;
	struct link_node* next;
} link_node_t, *LinkNode;

PRIVATE inline LinkNode LinkNode_new(void* c, LinkNode n) {
	LinkNode res = malloc(sizeof(link_node_t));
	res->content = c;
	res->next = n;
	return res;
}

LinkList LinkList_init(LinkList li) {
	li->list = NULL;
	li->len = 0;
	return li;
}
void LinkList_free(LinkList li) {
	if(UNLIKELY(li->list == NULL)) {
		return;
	}
	LinkNode p = li->list->next;
	LinkNode temp = NULL;
	while(LIKELY(p != li->list)) {
		temp = p;
		p = p->next;
		free(temp);
	}
	free(p);
	li->len = 0;
}

void LinkList_append(LinkList dest, LinkList src) {
	if(src->len == 0) {
		return;
	}
	if(LIKELY(dest->list != NULL)) {
		LinkNode temp = src->list->next;
		src->list->next = dest->list->next;
		dest->list->next = temp;
	} else {
		dest->list = src->list;
	}
	dest->len += src->len;
	src->list = NULL;
	src->len = 0;
}

void LinkList_put(LinkList li, void* content) {
	LinkNode n = LinkNode_new(content, NULL);
	if(LIKELY(li->list != NULL)) {
		n->next = li->list->next;
		li->list->next = n;
		li->list = n;
	} else {
		li->list = n;
		n->next = n;
	}
	++li->len;
}

void* LinkList_get(LinkList li) {
	if(UNLIKELY(li->list == NULL)) {
		return NULL;
	}
	void* res;
	LinkNode p = li->list->next;
	res = p->content;
	if(LIKELY(li->list != p)) {
		li->list->next = p->next;
	} else {
		li->list = NULL;
	}
	free(p);
	--li->len;
	return res;
}

void LinkList_push(LinkList li, void* content) {
	LinkNode n = LinkNode_new(content, NULL);
	if(LIKELY(li->list != NULL)) {
		n->next = li->list->next;
		li->list->next = n;
	} else {
		li->list = n;
		n->next = n;
	}
	++li->len;
}

void* LinkList_pop(LinkList li) {
	if(UNLIKELY(li->list == NULL)) {
		return NULL;
	}
	void* res;
	LinkNode p = li->list->next;
	res = p->content;
	if(LIKELY(li->list != p)) {
		li->list->next = p->next;
	} else {
		li->list = NULL;
	}
	free(p);
	--li->len;
	return res;
}

uint32_t LinkList_length(LinkList li) {
	return li->len;
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
