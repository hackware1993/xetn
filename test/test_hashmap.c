#include "util/map.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>

typedef union variant {
	int inum;
	unsigned unum;
	double dreal;
} variant_t;

static int test_setup(void** state) {
	HashMap map = (HashMap)malloc(sizeof(HashMap_t));
	HashMap_init(map);
	*state = map;
	return 0;
}
static int test_teardown(void** state) {
	HashMap_free((HashMap)*state);
	free(*state);
	return 0;
}

static void test_hashmap_put_get(void** state) {
	HashMap map = (HashMap)*state;

	variant_t v1, v2, v3, v4, v5;
	v1.inum = 1;
	v2.inum = 2;
	v3.inum = 123;
	v4.inum = 456;
	v5.inum = 999;

	void* ret;
	ret = HashMap_put(map, "v1", &v1);
	assert_ptr_equal(ret, &v1);
	ret = HashMap_put(map, "v2", &v2);
	assert_ptr_equal(ret, &v2);
	ret = HashMap_put(map, "v3", &v3);
	assert_ptr_equal(ret, &v3);
	ret = HashMap_put(map, "v4", &v4);
	assert_ptr_equal(ret, &v4);
	ret = HashMap_put(map, "v5", &v5);
	assert_ptr_equal(ret, &v5);
	ret = HashMap_put(map, "v5", &v5);
	assert_ptr_equal(ret, NULL);

	ret = HashMap_get(map, "v5");
	assert_ptr_equal(ret, &v5);
	assert_int_equal(*(int*)ret, v5.inum);
	assert_int_equal(HashMap_contains(map, "v5"), 1);
	ret = HashMap_get(map, "v4");
	assert_ptr_equal(ret, &v4);
	assert_int_equal(*(int*)ret, v4.inum);
	assert_int_equal(HashMap_contains(map, "v4"), 1);
	ret = HashMap_get(map, "v3");
	assert_ptr_equal(ret, &v3);
	assert_int_equal(*(int*)ret, v3.inum);
	assert_int_equal(HashMap_contains(map, "v3"), 1);
	ret = HashMap_get(map, "v2");
	assert_ptr_equal(ret, &v2);
	assert_int_equal(*(int*)ret, v2.inum);
	assert_int_equal(HashMap_contains(map, "v2"), 1);
	ret = HashMap_get(map, "v1");
	assert_ptr_equal(ret, &v1);
	assert_int_equal(*(int*)ret, v1.inum);
	assert_int_equal(HashMap_contains(map, "v1"), 1);
	assert_int_equal(HashMap_contains(map, "vv"), 0);

	ret = HashMap_delete(map, "vv");
	assert_ptr_equal(ret, NULL);
	ret = HashMap_delete(map, "v3");
	assert_ptr_not_equal(ret, NULL);
	ret = HashMap_get(map, "v3");
	assert_ptr_equal(ret, NULL);
	
}


int main() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_hashmap_put_get, test_setup, test_teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
