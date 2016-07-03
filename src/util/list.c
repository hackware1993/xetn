#include "common.h"
#include "util/list.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SLinkList SLinkList_init(SLinkList self) {
	self->list = NULL;
	self->len  = 0;
	return self;
}

void SLinkList_clear(SLinkList self) {
	SLink p = self->list->next;
	SLink temp;
	for(size_t i = 0; i < self->len; ++i) {
		temp = p;
		p = p->next;
		temp->next = NULL;
	}
	self->list = NULL;
	self->len  = 0;
}

void SLinkList_free(SLinkList self, Element_free free_cb) {
	SLink p = self->list->next;
	SLink temp;
	for(size_t i = 0; i < self->len; ++i) {
		temp = p;
		p = p->next;
		if(free_cb == NULL) {
			free(temp);
		} else {
			free_cb(temp);
		}
	}
	self->list = NULL;
	self->len  = 0;
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
void SLinkList_put(SLinkList self, SLink le) {
	if(UNLIKELY(le == NULL)) {
		return;
	}
	SLink tail = self->list;
	/* check if the list is empty */
	if(LIKELY(tail != NULL)) {
		/* put the node as the head */
		le->next   = tail->next;
		tail->next = le;
		/* set new node as the current node */
		self->list   = le;
	} else {
		/* link the node with itself */
		le->next = le;
		self->list = le;
	}
	++self->len;
}

/* get node from list as old head */
SLink SLinkList_get(SLinkList self) {
	SLink tail = self->list;
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
		self->list = NULL;
	}
	--self->len;
	/* reset the next pointer */
	head->next = NULL;
	return head;
}

void SLinkList_push(SLinkList self, SLink le) {
	if(UNLIKELY(le == NULL)) {
		return;
	}
	SLink tail = self->list;
	if(LIKELY(tail != NULL)) {
		le->next = tail->next;
		tail->next = le;
	} else {
		le->next = le;
		self->list = le;
	}
	++self->len;
}

SLink SLinkList_pop(SLinkList self) {
	SLink tail = self->list;
	if(UNLIKELY(tail == NULL)) {
		return NULL;
	}
	SLink head = tail->next;
	if(LIKELY(tail != head)) {
		tail->next = head->next;
	} else {
		self->list = NULL;
	}
	--self->len;
	/* reset the next pointer */
	head->next = NULL;
	return head;
}

void SLinkList_inverse(SLinkList self) {
	if(UNLIKELY(self->list == NULL)) {
		return;
	}
	SLink last = self->list;
	SLink cur  = last->next;
	SLink next = NULL;
	size_t len = self->len;
	for(size_t i = 0; i < len; ++i) {
		next = cur->next;
		cur->next = last;
		last = cur;
		cur  = next;
	}
	self->list = cur;
}

DLinkList DLinkList_init(DLinkList self) {
	self->list = NULL;
	self->len  = 0;
	return self;
}

void DLinkList_free(DLinkList self, Element_free free_cb) {
	DLink p = self->list;
	DLink temp;
	for(size_t i = 0; i < self->len; ++i) {
		temp = p;
		p = p->next;
		if(free_cb == NULL) {
			free(temp);
		} else {
			free_cb(temp);
		}
	}
	self->list = NULL;
	self->len  = 0;
}

void DLinkList_clear(DLinkList self) {
	DLink p = self->list;
	DLink temp;
	for(size_t i = 0; i < self->len; ++i) {
		temp = p;
		p = p->next;
		temp->next = NULL;
		temp->prev = NULL;
	}
	self->list = NULL;
	self->len  = 0;
}

void DLinkList_remove(DLinkList self, DLink le) {
	if(self->list == NULL) {
		return;
	}
	DLink prev = le->prev;
	DLink next = le->next;
	prev->next = next;
	next->prev = prev;
	if(self->list == le) {
		if(self->len == 1) {
			self->list = NULL;
		} else {
			self->list = next;
		}
	}
}

void DLinkList_put(DLinkList self, DLink le) {
	if(UNLIKELY(le == NULL)) {
		return;
	}
	DLink head = self->list;
	if(LIKELY(head)) {
		DLink tail = head->prev;
		tail->next = le;
		head->prev = le;
		le->prev   = tail;
		le->next   = head;
	} else {
		self->list = le;
		le->next   = le;
		le->prev   = le;
	}
	++self->len;
}

DLink DLinkList_get(DLinkList self) {
	if(UNLIKELY(self->list == NULL)) {
		return NULL;
	}
	DLink res = self->list;
	if(LIKELY(self->len > 1)) {
		DLink head = res->next;
		DLink tail = res->prev;
		tail->next = head;
		head->prev = tail;
		self->list = head;
	} else {
		self->list = NULL;
	}
	--self->len;
	res->prev = NULL;
	res->next = NULL;
	return res;
}

void DLinkList_push(DLinkList self, DLink le) {
	if(UNLIKELY(le == NULL)) {
		return;
	}
	DLink head = self->list;
	if(LIKELY(head != NULL)) {
		DLink tail = head->prev;
		tail->next = le;
		head->prev = le;
		le->next   = head;
		le->prev   = tail;
		self->list = le;
	} else {
		self->list = le;
		le->next   = le;
		le->prev   = le;
	}
	++self->len;
}

DLink DLinkList_pop(DLinkList self) {
	if(UNLIKELY(self->list == NULL)) {
		return NULL;
	}
	DLink res = self->list;
	if(LIKELY(self->len > 1)) {
		DLink tail = res->prev;
		DLink head = res->next;
		tail->next = head;
		head->prev = tail;
		self->list = head;
	} else {
		self->list = NULL;
	}
	--self->len;
	res->next = NULL;
	res->prev = NULL;
	return res;
}

void DLinkList_inverse(DLinkList self) {
	if(UNLIKELY(self->list == NULL)) {
		return;
	}
	DLink cur  = self->list;
	DLink temp = NULL;
	size_t len = self->len;
	for(size_t i = 0; i < len; ++i) {
		temp = cur->next;
		cur->next = cur->prev;
		cur->prev = temp;
		cur = temp;
	}
	self->list = cur->next;
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

ArrayList ArrayList_init(ArrayList self, size_t len) {
	self->len = 0;
	self->cap = len;
	self->list = (void*)malloc(sizeof(void*) * len);
	return self;
}

void ArrayList_free(ArrayList self, Element_free free_cb) {
	void** list = self->list;
	if(free_cb) {
		for(size_t i = 0; i < self->len; ++i) {
			free_cb(list[i]);
		}
	}
	self->len = 0;
	self->cap = 0;
	free(list);
}

void ArrayList_push(ArrayList self, void* content) {
	size_t cur = self->len;
	size_t cap = self->cap;
	if(cur >= cap) {
		while(cur >= cap) {
			cap <<= 1;
		}
		self->list = (void**)realloc(self->list, sizeof(void*) * cap);
		self->cap  = cap;
	}
	self->list[cur] = content;
	self->len = cur + 1;
}

void* ArrayList_pop(ArrayList self) {
	if(self->len > 0) {
		return self->list[--self->len];
	}
	return NULL;
}

void* ArrayList_top(ArrayList self) {
	size_t len = self->len;
	if(len > 0) {
		return self->list[len - 1];
	}
	return NULL;
}

void ArrayList_set(ArrayList self, size_t index, void* content) {
	if(index >= 0 && index < self->len) {
		self->list[index] = content;
	}
}

void* ArrayList_get(ArrayList self, size_t index) {
	if(index >= 0 && index < self->len) {
		return self->list[index];
	}
	return NULL;
}

RingList RingList_init(RingList self, size_t size) {
	uint32_t cap = 1;
	while(cap < size) {
		cap <<= 1;
	}
	self->cap  = cap;
	self->len  = 0;
	self->head = 0;
	self->tail = 0;
	self->zone = (void*)malloc(sizeof(void*) * cap);
	return self;
}

void RingList_free(RingList self, Element_free free_cb) {
	if(free_cb == NULL) {
		free_cb = free;
	}
	size_t head = self->head;
	for(size_t i = 0; i < self->len; ++i) {
		free_cb(self->zone[head++]);
		head = head & (self->cap - 1);
	}
	self->head = 0;
	self->tail = 0;
	self->len  = 0;
}

void RingList_put(RingList self, void* ele) {
	size_t tail = self->tail;
	self->zone[tail++] = ele;
	self->tail = tail & (self->cap - 1);
	++self->len;
}

void* RingList_get(RingList self) {
	size_t head = self->head;
	void*  res  = self->zone[head++];
	self->head  = head & (self->cap - 1);
	--self->len;
	return res;
}

