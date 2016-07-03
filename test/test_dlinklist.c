#include "util/list.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>

typedef struct node {
	int val;
	DLink_t link;
} node_t, *Node;

static int test_setup(void** state) {
	DLinkList list = (DLinkList)malloc(sizeof(DLinkList_t));
	DLinkList_init(list);
	*state = list;
	return 0;
}
static int test_teardown(void** state) {
	free(*state);
	return 0;
}

static void test_linklist_push_pop(void** state) {
	DLinkList list = (DLinkList)*state;

	node_t n1 = {1, {NULL, NULL}};
	node_t n2 = {2, {NULL, NULL}};
	node_t n3 = {3, {NULL, NULL}};
	node_t n4 = {4, {NULL, NULL}};
	node_t n5 = {5, {NULL, NULL}};
	DLinkList_push(list, &n1.link);
	DLinkList_push(list, &n2.link);
	DLinkList_push(list, &n3.link);
	DLinkList_push(list, &n4.link);
	DLinkList_push(list, &n5.link);
	DLink ret;
	Node n;
	ret = DLinkList_pop(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(5, n->val);
	assert_ptr_equal(&n5, n);
	ret = DLinkList_pop(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(4, n->val);
	assert_ptr_equal(&n4, n);
	ret = DLinkList_pop(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(3, n->val);
	assert_ptr_equal(&n3, n);
	ret = DLinkList_pop(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(2, n->val);
	assert_ptr_equal(&n2, n);
	ret = DLinkList_pop(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(1, n->val);
	assert_ptr_equal(&n1, n);
}

static void test_linklist_put_get(void** state) {
	DLinkList list = (DLinkList)*state;

	node_t n1 = {1, {NULL, NULL}};
	node_t n2 = {2, {NULL, NULL}};
	node_t n3 = {3, {NULL, NULL}};
	node_t n4 = {4, {NULL, NULL}};
	node_t n5 = {5, {NULL, NULL}};
	DLinkList_put(list, &n1.link);
	DLinkList_put(list, &n2.link);
	DLinkList_put(list, &n3.link);
	DLinkList_put(list, &n4.link);
	DLinkList_put(list, &n5.link);
	DLink ret;
	Node n;
	ret = DLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(1, n->val);
	assert_ptr_equal(n, &n1);
	ret = DLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(2, n->val);
	assert_ptr_equal(n, &n2);
	ret = DLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(3, n->val);
	assert_ptr_equal(n, &n3);
	ret = DLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(4, n->val);
	assert_ptr_equal(n, &n4);
	ret = DLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(5, n->val);
	assert_ptr_equal(n, &n5);
}
static void test_linklist_length(void** state) {
	DLinkList list = (DLinkList)*state;

	node_t n1 = {1, {NULL, NULL}};
	node_t n2 = {2, {NULL, NULL}};
	node_t n3 = {3, {NULL, NULL}};

	assert_int_equal(DLinkList_length(list), 0);
	DLinkList_put(list, &n1.link);
	assert_int_equal(DLinkList_length(list), 1);
	DLinkList_put(list, &n2.link);
	assert_int_equal(DLinkList_length(list), 2);
	DLinkList_put(list, &n3.link);
	assert_int_equal(DLinkList_length(list), 3);
	DLinkList_get(list);
	assert_int_equal(DLinkList_length(list), 2);
	DLinkList_get(list);
	assert_int_equal(DLinkList_length(list), 1);
	DLinkList_get(list);
	assert_int_equal(DLinkList_length(list), 0);
}

static void test_linklist_append(void** state) {
	DLinkList list = (DLinkList)*state;
	assert_int_equal(DLinkList_length(list), 0);

	DLinkList_t li1, li2;
	DLinkList_init(&li1);
	DLinkList_init(&li2);

	node_t n1 = {1, {NULL, NULL}};
	node_t n2 = {2, {NULL, NULL}};
	node_t n3 = {3, {NULL, NULL}};
	node_t n4 = {4, {NULL, NULL}};
	node_t n5 = {5, {NULL, NULL}};

	DLinkList_put(&li1, &n1.link);
	assert_int_equal(DLinkList_length(&li1), 1);
	assert_int_equal(DLinkList_length(&li2), 0);

	DLinkList_append(list, &li1);
	assert_int_equal(DLinkList_length(list), 1);

	DLinkList_append(list, &li2);
	assert_int_equal(DLinkList_length(list), 1);

	DLinkList_put(&li1, &n3.link);
	DLinkList_put(&li1, &n4.link);
	DLinkList_put(&li1, &n5.link);
	assert_int_equal(DLinkList_length(&li1), 3);

	DLinkList_put(list, &n2.link);
	assert_int_equal(DLinkList_length(list), 2);
	DLinkList_append(list, &li1);
	assert_int_equal(DLinkList_length(list), 5);
	assert_int_equal(DLinkList_length(&li1), 0);

	DLink ret;
	Node  n;
	ret = DLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(1, n->val);
	assert_ptr_equal(n, &n1);
	ret = DLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(2, n->val);
	assert_ptr_equal(n, &n2);
	ret = DLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(3, n->val);
	assert_ptr_equal(n, &n3);
	ret = DLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(4, n->val);
	assert_ptr_equal(n, &n4);
	ret = DLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(5, n->val);
	assert_ptr_equal(n, &n5);
}
static void test_linklist_inverse(void** state) {
	DLinkList list = (DLinkList)*state;

	node_t n1 = {1, {NULL, NULL}};
	node_t n2 = {2, {NULL, NULL}};
	node_t n3 = {3, {NULL, NULL}};
	node_t n4 = {4, {NULL, NULL}};
	node_t n5 = {5, {NULL, NULL}};

	DLinkList_put(list, &n1.link);
	DLinkList_put(list, &n2.link);
	DLinkList_put(list, &n3.link);
	DLinkList_put(list, &n4.link);
	DLinkList_put(list, &n5.link);

	assert_int_equal(DLinkList_length(list), 5);
	DLinkList_inverse(list);
	assert_int_equal(DLinkList_length(list), 5);

	DLink ret;
	Node  n;
	ret = DLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(5, n->val);
	assert_ptr_equal(&n5, n);
	ret = DLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(4, n->val);
	assert_ptr_equal(&n4, n);
	ret = DLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(3, n->val);
	assert_ptr_equal(&n3, n);
	ret = DLinkList_get(list);
	n   = Element_getPtr(ret, node_t, link);
	assert_int_equal(2, n->val);
	assert_ptr_equal(&n2, n);
	ret = DLinkList_get(list);
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
		cmocka_unit_test_setup_teardown(test_linklist_inverse,  test_setup, test_teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
