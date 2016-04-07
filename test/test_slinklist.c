#include "util/list.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>

typedef struct node {
	int val;
	slink_t link;
} node_t, *Node;

static int test_setup(void** state) {
	SLinkList list = (SLinkList)malloc(sizeof(slink_list_t));
	SLinkList_init(list);
	*state = list;
	return 0;
}
static int test_teardown(void** state) {
	free(*state);
	return 0;
}

static void test_linklist_push_pop(void** state) {
	SLinkList list = (SLinkList)*state;

	node_t n1 = {1, {NULL}};
	node_t n2 = {2, {NULL}};
	node_t n3 = {3, {NULL}};
	node_t n4 = {4, {NULL}};
	node_t n5 = {5, {NULL}};
	SLinkList_push(list, &n1.link);
	SLinkList_push(list, &n2.link);
	SLinkList_push(list, &n3.link);
	SLinkList_push(list, &n4.link);
	SLinkList_push(list, &n5.link);
	SLink ret;
	Node n;
	ret = SLinkList_pop(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(5, n->val);
	assert_ptr_equal(&n5, n);
	ret = SLinkList_pop(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(4, n->val);
	assert_ptr_equal(&n4, n);
	ret = SLinkList_pop(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(3, n->val);
	assert_ptr_equal(&n3, n);
	ret = SLinkList_pop(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(2, n->val);
	assert_ptr_equal(&n2, n);
	ret = SLinkList_pop(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(1, n->val);
	assert_ptr_equal(&n1, n);
}

static void test_linklist_put_get(void** state) {
	SLinkList list = (SLinkList)*state;

	node_t n1 = {1, {NULL}};
	node_t n2 = {2, {NULL}};
	node_t n3 = {3, {NULL}};
	node_t n4 = {4, {NULL}};
	node_t n5 = {5, {NULL}};
	SLinkList_put(list, &n1.link);
	SLinkList_put(list, &n2.link);
	SLinkList_put(list, &n3.link);
	SLinkList_put(list, &n4.link);
	SLinkList_put(list, &n5.link);
	SLink ret;
	Node n;
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(1, n->val);
	assert_ptr_equal(n, &n1);
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(2, n->val);
	assert_ptr_equal(n, &n2);
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(3, n->val);
	assert_ptr_equal(n, &n3);
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(4, n->val);
	assert_ptr_equal(n, &n4);
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(5, n->val);
	assert_ptr_equal(n, &n5);
}
static void test_linklist_length(void** state) {
	SLinkList list = (SLinkList)*state;

	node_t n1 = {1, {NULL}};
	node_t n2 = {2, {NULL}};
	node_t n3 = {3, {NULL}};

	assert_int_equal(SLinkList_length(list), 0);
	SLinkList_put(list, &n1.link);
	assert_int_equal(SLinkList_length(list), 1);
	SLinkList_put(list, &n2.link);
	assert_int_equal(SLinkList_length(list), 2);
	SLinkList_put(list, &n3.link);
	assert_int_equal(SLinkList_length(list), 3);
	SLinkList_get(list);
	assert_int_equal(SLinkList_length(list), 2);
	SLinkList_get(list);
	assert_int_equal(SLinkList_length(list), 1);
	SLinkList_get(list);
	assert_int_equal(SLinkList_length(list), 0);
}

static void test_linklist_append(void** state) {
	SLinkList list = (SLinkList)*state;
	assert_int_equal(SLinkList_length(list), 0);

	slink_list_t li1, li2;
	SLinkList_init(&li1);
	SLinkList_init(&li2);

	node_t n1 = {1, {NULL}};
	node_t n2 = {2, {NULL}};
	node_t n3 = {3, {NULL}};
	node_t n4 = {4, {NULL}};
	node_t n5 = {5, {NULL}};

	SLinkList_put(&li1, &n1.link);
	assert_int_equal(SLinkList_length(&li1), 1);
	assert_int_equal(SLinkList_length(&li2), 0);

	SLinkList_append(list, &li1);
	assert_int_equal(SLinkList_length(list), 1);

	SLinkList_append(list, &li2);
	assert_int_equal(SLinkList_length(list), 1);

	SLinkList_put(&li1, &n3.link);
	SLinkList_put(&li1, &n4.link);
	SLinkList_put(&li1, &n5.link);
	assert_int_equal(SLinkList_length(&li1), 3);

	SLinkList_put(list, &n2.link);
	assert_int_equal(SLinkList_length(list), 2);
	SLinkList_append(list, &li1);
	assert_int_equal(SLinkList_length(list), 5);
	assert_int_equal(SLinkList_length(&li1), 0);

	SLink ret;
	Node  n;
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(1, n->val);
	assert_ptr_equal(n, &n1);
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(2, n->val);
	assert_ptr_equal(n, &n2);
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(3, n->val);
	assert_ptr_equal(n, &n3);
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(4, n->val);
	assert_ptr_equal(n, &n4);
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(5, n->val);
	assert_ptr_equal(n, &n5);
}
static void test_linklist_inverse(void** state) {
	SLinkList list = (SLinkList)*state;

	node_t n1 = {1, {NULL}};
	node_t n2 = {2, {NULL}};
	node_t n3 = {3, {NULL}};
	node_t n4 = {4, {NULL}};
	node_t n5 = {5, {NULL}};

	SLinkList_put(list, &n1.link);
	SLinkList_put(list, &n2.link);
	SLinkList_put(list, &n3.link);
	SLinkList_put(list, &n4.link);
	SLinkList_put(list, &n5.link);

	assert_int_equal(SLinkList_length(list), 5);
	SLinkList_inverse(list);
	assert_int_equal(SLinkList_length(list), 5);

	SLink ret;
	Node  n;
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(5, n->val);
	assert_ptr_equal(&n5, n);
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(4, n->val);
	assert_ptr_equal(&n4, n);
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(3, n->val);
	assert_ptr_equal(&n3, n);
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(2, n->val);
	assert_ptr_equal(&n2, n);
	ret = SLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(1, n->val);
	assert_ptr_equal(&n1, n);
}

int main() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_linklist_push_pop, test_setup, test_teardown),
		cmocka_unit_test_setup_teardown(test_linklist_put_get,  test_setup, test_teardown),
		cmocka_unit_test_setup_teardown(test_linklist_length,   test_setup, test_teardown),
		cmocka_unit_test_setup_teardown(test_linklist_append,   test_setup, test_teardown),
		cmocka_unit_test_setup_teardown(test_linklist_inverse,   test_setup, test_teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
