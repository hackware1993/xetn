#include "../src/list.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

static int test_setup(void** state) {
	RingList list = (RingList)malloc(sizeof(RingList));
	*state = list;
	return 0;
}
static int test_teardown(void** state) {
	RingList_close((RingList)*state);
	return 0;
}

static void test_ringlist_put_get(void** state) {
	RingList list = (RingList)*state;

	RingList_init(list, 10);

	assert_int_equal(RingList_capacity(list), 16);
	assert_int_equal(RingList_length(list), 0);

	RingList_put(list, 1);
	RingList_put(list, 2);
	RingList_put(list, 3);
	RingList_put(list, 4);
	RingList_put(list, 5);
	assert_int_equal(RingList_length(list), 5);

	int i = (int)RingList_get(list);
	assert_int_equal(i, 1);
	i = (int)RingList_get(list);
	assert_int_equal(i, 2);
	i = (int)RingList_get(list);
	assert_int_equal(i, 3);
	i = (int)RingList_get(list);
	assert_int_equal(i, 4);
	i = (int)RingList_get(list);
	assert_int_equal(i, 5);
	assert_int_equal(RingList_length(list), 0);
}

int main() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_ringlist_put_get, test_setup, test_teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
