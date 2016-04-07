#include "common.h"
#include "util/list.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SLinkList SLinkList_init(SLinkList li) {
	li->list = NULL;
	li->len  = 0;
	return li;
}

void SLinkList_clear(SLinkList li) {
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

void SLinkList_free(SLinkList li, Element_free free_cb) {
	size_t i;
	SLink p = li->list->next;
	SLink temp;
	for(i = 0; i < li->len; ++i) {
		temp = p;
		p = p->next;
		if(free_cb == NULL) {
			free(temp);
		} else {
			free_cb(temp);
		}
	}
	li->list = NULL;
	li->len  = 0;
}

void SLinkList_append(SLinkList dest, SLinkList src) {
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
void SLinkList_put(SLinkList li, SLink le) {
	if(UNLIKELY(le == NULL)) {
		return;
	}
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
SLink SLinkList_get(SLinkList li) {
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

void SLinkList_push(SLinkList li, SLink le) {
	if(UNLIKELY(le == NULL)) {
		return;
	}
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

SLink SLinkList_pop(SLinkList li) {
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

void SLinkList_inverse(SLinkList li) {
	if(UNLIKELY(li->list == NULL)) {
		return;
	}
	SLink last = li->list;
	SLink cur  = last->next;
	SLink next = NULL;
	size_t len = li->len;
	for(size_t i = 0; i < len; ++i) {
		next = cur->next;
		cur->next = last;
		last = cur;
		cur  = next;
	}
	li->list = cur;
}

DLinkList DLinkList_init(DLinkList li) {
	li->list = NULL;
	li->len  = 0;
	return li;
}

void DLinkList_free(DLinkList li, Element_free free_cb) {
	size_t i;
	DLink p = li->list;
	DLink temp;
	for(i = 0; i < li->len; ++i) {
		temp = p;
		p = p->next;
		if(free_cb == NULL) {
			free(temp);
		} else {
			free_cb(temp);
		}
	}
	li->list = NULL;
	li->len  = 0;
}

void DLinkList_clear(DLinkList li) {
	size_t i;
	DLink p = li->list;
	DLink temp;
	for(i = 0; i < li->len; ++i) {
		temp = p;
		p = p->next;
		temp->next = NULL;
		temp->prev = NULL;
	}
	li->list = NULL;
	li->len  = 0;
}

void DLinkList_remove(DLinkList li, DLink le) {
	if(li->list == NULL) {
		return;
	}
	DLink prev = le->prev;
	DLink next = le->next;
	prev->next = next;
	next->prev = prev;
	if(li->list == le) {
		if(li->len == 1) {
			li->list = NULL;
		} else {
			li->list = next;
		}
	}
}

void DLinkList_put(DLinkList li, DLink le) {
	if(UNLIKELY(le == NULL)) {
		return;
	}
	DLink head = li->list;
	if(LIKELY(head)) {
		DLink tail = head->prev;
		tail->next = le;
		head->prev = le;
		le->prev   = tail;
		le->next   = head;
	} else {
		li->list  = le;
		le->next  = le;
		le->prev  = le;
	}
	++li->len;
}

DLink DLinkList_get(DLinkList li) {
	if(UNLIKELY(li->list == NULL)) {
		return NULL;
	}
	DLink res = li->list;
	if(LIKELY(li->len > 1)) {
		DLink head = res->next;
		DLink tail = res->prev;
		tail->next = head;
		head->prev = tail;
		li->list   = head;
	} else {
		li->list = NULL;
	}
	--li->len;
	res->prev = NULL;
	res->next = NULL;
	return res;
}

void DLinkList_push(DLinkList li, DLink le) {
	if(UNLIKELY(le == NULL)) {
		return;
	}
	DLink head = li->list;
	if(LIKELY(head != NULL)) {
		DLink tail = head->prev;
		tail->next = le;
		head->prev = le;
		le->next = head;
		le->prev = tail;
		li->list = le;
	} else {
		li->list = le;
		le->next = le;
		le->prev = le;
	}
	++li->len;
}

DLink DLinkList_pop(DLinkList li) {
	if(UNLIKELY(li->list == NULL)) {
		return NULL;
	}
	DLink res = li->list;
	if(LIKELY(li->len > 1)) {
		DLink tail = res->prev;
		DLink head = res->next;
		tail->next = head;
		head->prev = tail;
		li->list = head;
	} else {
		li->list = NULL;
	}
	--li->len;
	res->next = NULL;
	res->prev = NULL;
	return res;
}

void DLinkList_inverse(DLinkList li) {
	if(UNLIKELY(li->list == NULL)) {
		return;
	}
	DLink cur  = li->list;
	DLink temp = NULL;
	size_t len = li->len;
	for(size_t i = 0; i < len; ++i) {
		temp = cur->next;
		cur->next = cur->prev;
		cur->prev = temp;
		cur = temp;
	}
	li->list = cur->next;
}

void DLinkList_append(DLinkList dest, DLinkList src) {
	if(UNLIKELY(src->len == 0)) {
		return;
	}
	if(LIKELY(dest->list != NULL)) {
		DLink dhead = dest->list;
		DLink dtail = dhead->prev;
		DLink shead = src->list;
		DLink stail = shead->prev;

		dtail->next = shead;
		shead->prev = dtail;
		stail->next = dhead;
		dhead->prev = stail;
	} else {
		dest->list = src->list;
	}
	dest->len += src->len;
	/* reset src list*/
	src->list = NULL;
	src->len = 0;
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
