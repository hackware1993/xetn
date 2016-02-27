#include "../src/list.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

static int test_setup(void** state) {
	LinkList list = (LinkList)malloc(sizeof(link_list_t));
	LinkList_init(list);
	*state = list;
	return 0;
}
static int test_teardown(void** state) {
	LinkList_free(*state);
	free(*state);
	return 0;
}

static void test_linklist_push_pop(void** state) {
	LinkList list = (LinkList)*state;

	LinkList_push(list, 1);
	LinkList_push(list, 2);
	LinkList_push(list, 3);
	LinkList_push(list, 4);
	LinkList_push(list, 5);
	int ret = 0;
	ret = (int)LinkList_pop(list);
	assert_int_equal(ret, 5);
	ret = (int)LinkList_pop(list);
	assert_int_equal(ret, 4);
	ret = (int)LinkList_pop(list);
	assert_int_equal(ret, 3);
	ret = (int)LinkList_pop(list);
	assert_int_equal(ret, 2);
	ret = (int)LinkList_pop(list);
	assert_int_equal(ret, 1);
}

static void test_linklist_put_get(void** state) {
	LinkList list = (LinkList)*state;

	LinkList_put(list, 1);
	LinkList_put(list, 2);
	LinkList_put(list, 3);
	LinkList_put(list, 4);
	LinkList_put(list, 5);
	int ret = 0;
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 1);
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 2);
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 3);
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 4);
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 5);
}
static void test_linklist_length(void** state) {
	LinkList list = (LinkList)*state;

	assert_int_equal(LinkList_length(list), 0);
	LinkList_put(list, 1);
	assert_int_equal(LinkList_length(list), 1);
	LinkList_put(list, 2);
	assert_int_equal(LinkList_length(list), 2);
	LinkList_put(list, 3);
	assert_int_equal(LinkList_length(list), 3);
	LinkList_get(list);
	assert_int_equal(LinkList_length(list), 2);
	LinkList_get(list);
	assert_int_equal(LinkList_length(list), 1);
	LinkList_get(list);
	assert_int_equal(LinkList_length(list), 0);
}

static void test_linklist_append(void** state) {
	LinkList list = (LinkList)*state;
	assert_int_equal(LinkList_length(list), 0);

	link_list_t li1, li2;
	LinkList_init(&li1);
	LinkList_init(&li2);

	LinkList_put(&li1, 1);
	assert_int_equal(LinkList_length(&li1), 1);
	assert_int_equal(LinkList_length(&li2), 0);

	LinkList_append(list, &li1);
	assert_int_equal(LinkList_length(list), 1);

	LinkList_append(list, &li2);
	assert_int_equal(LinkList_length(list), 1);

	LinkList_put(&li1, 3);
	LinkList_put(&li1, 4);
	LinkList_put(&li1, 5);
	assert_int_equal(LinkList_length(&li1), 3);

	LinkList_put(list, 2);
	assert_int_equal(LinkList_length(list), 2);
	LinkList_append(list, &li1);
	assert_int_equal(LinkList_length(list), 5);
	assert_int_equal(LinkList_length(&li1), 0);

	LinkList_free(&li1);
	LinkList_free(&li2);

	int ret = 0;
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 1);
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 2);
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 3);
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 4);
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 5);
}
static void test_linklist_inverse(void** state) {
	LinkList list = (LinkList)*state;

	LinkList_put(list, 1);
	LinkList_put(list, 2);
	LinkList_put(list, 3);
	LinkList_put(list, 4);
	LinkList_put(list, 5);

	assert_int_equal(LinkList_length(list), 5);
	LinkList_inverse(list);
	assert_int_equal(LinkList_length(list), 5);

	int ret = 0;
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 5);
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 4);
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 3);
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 2);
	ret = (int)LinkList_get(list);
	assert_int_equal(ret, 1);
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
