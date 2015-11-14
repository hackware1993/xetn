#include "list.h"

#include <stdlib.h>
#include <stdio.h>

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
	if(li->list == NULL) {
		return;
	}
	LinkNode p = li->list->next;
	LinkNode temp = NULL;
	while(p != li->list) {
		temp = p;
		p = p->next;
		free(temp);
	}
	free(p);
	li->len = 0;
}

void LinkList_put(LinkList li, void* content) {
	LinkNode n = LinkNode_new(content, NULL);
	if(li->list != NULL) {
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
	if(li->list == NULL) {
		return NULL;
	}
	void* res;
	LinkNode p = li->list->next;
	res = p->content;
	if(li->list != p) {
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
	if(li->list != NULL) {
		n->next = li->list->next;
		li->list->next = n;
	} else {
		li->list = n;
		n->next = n;
	}
	++li->len;
}

void* LinkList_pop(LinkList li) {
	if(li->list == NULL) {
		return NULL;
	}
	void* res;
	LinkNode p = li->list->next;
	res = p->content;
	if(li->list != p) {
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
