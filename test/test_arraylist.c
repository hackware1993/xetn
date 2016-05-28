#include "util/list.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>

static int test_setup(void** state) {
	ArrayList list = (ArrayList)malloc(sizeof(ArrayList_t));
	ArrayList_init(list, 4);
	*state = list;
	return 0;
}
static int test_teardown(void** state) {
	ArrayList_free(*state, NULL);
	free(*state);
	return 0;
}

static void test_linklist_push_pop(void** state) {
	ArrayList list = (ArrayList)*state;

	int arr[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

	for(int i = 0; i < 17; ++i) {
		ArrayList_push(list, arr + i);
	}
	for(int i = 16; i >= 0; --i) {
		int ret = *(int*)ArrayList_pop(list);
		assert_int_equal(ret, arr[i]);
	}
}

static void test_linklist_set_get(void** state) {
	ArrayList list = (ArrayList)*state;
	assert_int_equal(ArrayList_capacity(list), 4);
	assert_int_equal(ArrayList_length(list), 0);

	
	int arr[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

	for(int i = 0; i < 17; ++i) {
		ArrayList_push(list, arr + i);
	}
	assert_int_equal(ArrayList_length(list), 17);

	for(int i = 0; i < 17; ++i) {
		ArrayList_set(list, i, arr + 16 - i);
	}
	for(int i = 16; i >= 0; --i) {
		int* ret = (int*)ArrayList_get(list, i);
		assert_ptr_equal(ret, arr + 16 - i);
	}
}

static void test_linklist_length(void** state) {
	ArrayList list = (ArrayList)*state;

	int arr[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

	for(int i = 0; i < 17; ++i) {
		assert_int_equal(ArrayList_length(list), i);
		int j = 0;
		for(j = 4; j < i; j <<= 1) {}
		assert_int_equal(ArrayList_capacity(list), j);
		ArrayList_push(list, arr + i);
	}
	for(int i = 0; i < 17; ++i) {
		assert_int_equal(ArrayList_length(list), 17 - i);
		ArrayList_pop(list);
	}
	assert_int_equal(ArrayList_capacity(list), 32);
}

int main() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_linklist_push_pop, test_setup, test_teardown),
		cmocka_unit_test_setup_teardown(test_linklist_set_get,  test_setup, test_teardown),
		cmocka_unit_test_setup_teardown(test_linklist_length,   test_setup, test_teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
