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

	Pair list = HashMap_getPairList(map);
	assert_ptr_not_equal(list, NULL);
	assert_string_equal(list->key, "v5");
	assert_ptr_equal(list->val, &v5);
	assert_ptr_not_equal(list->next, NULL);

	list = list->next;
	assert_ptr_not_equal(list, NULL);
	assert_string_equal(list->key, "v4");
	assert_ptr_equal(list->val, &v4);
	assert_ptr_not_equal(list->next, NULL);

	list = list->next;
	assert_ptr_not_equal(list, NULL);
	assert_string_equal(list->key, "v3");
	assert_ptr_equal(list->val, &v3);
	assert_ptr_not_equal(list->next, NULL);

	list = list->next;
	assert_ptr_not_equal(list, NULL);
	assert_string_equal(list->key, "v2");
	assert_ptr_equal(list->val, &v2);
	assert_ptr_not_equal(list->next, NULL);
	ret = HashMap_get(map, "v5");

	list = list->next;
	assert_ptr_not_equal(list, NULL);
	assert_string_equal(list->key, "v1");
	assert_ptr_equal(list->val, &v1);
	assert_ptr_equal(list->next, NULL);

	assert_ptr_equal(ret, &v5);
	assert_int_equal(*(int*)ret, v5.inum);
	ret = HashMap_get(map, "v4");
	assert_ptr_equal(ret, &v4);
	assert_int_equal(*(int*)ret, v4.inum);
	ret = HashMap_get(map, "v3");
	assert_ptr_equal(ret, &v3);
	assert_int_equal(*(int*)ret, v3.inum);
	ret = HashMap_get(map, "v2");
	assert_ptr_equal(ret, &v2);
	assert_int_equal(*(int*)ret, v2.inum);
	ret = HashMap_get(map, "v1");
	assert_ptr_equal(ret, &v1);
	assert_int_equal(*(int*)ret, v1.inum);

	HashMap_clear(map);
	assert_ptr_equal(map->pairs, NULL);
}


int main() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_hashmap_put_get, test_setup, test_teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
