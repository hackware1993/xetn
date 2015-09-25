#include "../src/buffer.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

static int test_setup(void** state) {
	Buffer buf = buffer_new(10);
	*state = buf;
	return 0;
}
static int test_teardown(void** state) {
	buffer_free(*state);
	return 0;
}

static void test_buffer_set_get_macro(void** state) {
	struct buffer buf;
	buf.lim = 11;
	buf.cap = 12;
	buf.pos = 0;
	buf.end = 2;
	/* test for buffer_get_ptr */
	assert_non_null(buffer_get_ptr(&buf));
	/* test for buffer_get_cap */
	assert_int_equal(buffer_get_cap(&buf), 12);
	/* test for buffer_get_lim & buffer_set_lim */
	assert_int_equal(buffer_get_lim(&buf), 11);
	buffer_set_lim(&buf, 10);
	assert_int_equal(buf.lim, 10);
	/* test for buffer_get_pos & buffer_set_pos */
	assert_int_equal(buffer_get_pos(&buf), 0);
	buffer_set_pos(&buf, 1);
	assert_int_equal(buf.pos, 1);
	/* test for buffer_get_len & buffer_set_len */
	assert_int_equal(buffer_get_len(&buf), 2);
	buffer_set_len(&buf, 3);
	assert_int_equal(buf.end, 3);
}

static void test_buffer_is_macro(void** state) {
	struct buffer buf;
	/* test for buffer_is_empty */
	buf.end = 0;
	assert_true(buffer_is_empty(&buf));
	buf.end = 1;
	assert_false(buffer_is_empty(&buf));
	/* test for buffer_is_end */
	buf.pos = 1;
	assert_true(buffer_is_end(&buf));
	buf.pos = 0;
	assert_false(buffer_is_end(&buf));
	/* test for buffer_is_full */
	buf.lim = 1;
	assert_true(buffer_is_full(&buf));
	buf.lim = 2;
	assert_false(buffer_is_full(&buf));
}

static void test_buffer_new(void** state) {
	Buffer buf = (Buffer)*state;
	assert_non_null(buf);
	//assert_non_null(buffer_get_ptr(buf));
	assert_int_equal(buffer_get_pos(buf), 0);
	assert_int_equal(buffer_get_len(buf), 0);
	assert_int_equal(buffer_get_lim(buf), 10);
	assert_int_equal(buffer_get_cap(buf), 10);
	assert_true(buffer_is_empty(buf));
}

static void test_buffer_put_get(void** state) {
	Buffer buf = (Buffer)*state;
	/* test for buffer_put */
	buffer_put(buf, 'h');
	assert_int_equal(buffer_get_len(buf), 1);
	buffer_put(buf, 'e');
	assert_int_equal(buffer_get_len(buf), 2);
	buffer_put(buf, '3');
	assert_int_equal(buffer_get_len(buf), 3);
	assert_memory_equal(buffer_get_ptr(buf), "he3", buffer_get_len(buf));
	assert_int_equal(buffer_get_by_index(buf, 0), 'h');
	assert_int_equal(buffer_get_by_index(buf, 1), 'e');
	assert_int_equal(buffer_get_by_index(buf, 2), '3');

	buffer_set_by_index(buf, 0, '0');
	buffer_set_by_index(buf, 1, '1');
	buffer_set_by_index(buf, 2, '2');
	assert_memory_equal(buf->ptr, "012", buffer_get_len(buf));

	char ch;
	/* test for buffer_get */
	assert_int_equal(buffer_get_pos(buf), 0);
	ch = buffer_get(buf);
	assert_int_equal(buffer_get_pos(buf), 1);
	assert_int_equal(ch, '0');
	ch = buffer_get(buf);
	assert_int_equal(buffer_get_pos(buf), 2);
	assert_int_equal(ch, '1');
	ch = buffer_get(buf);
	assert_int_equal(buffer_get_pos(buf), 3);
	assert_int_equal(ch, '2');
	assert_true(buffer_is_end(buf));

	buffer_clear(buf);
	/* test for buffer_clear */
	assert_int_equal(buffer_get_pos(buf), 0);
	assert_int_equal(buffer_get_len(buf), 0);
	assert_int_equal(buffer_get_lim(buf), 10);
	assert_int_equal(buffer_get_cap(buf), 10);
	assert_true(buffer_is_empty(buf));
}

static void test_buffer_get_put_arr(void** state) {
	Buffer buf = (Buffer)*state;
	unsigned len = 0;
	char sarr[] = "hello";
	len = buffer_put_arr(buf, sarr, 0, 5);
	assert_int_equal(len, 5);
	assert_int_equal(buffer_get_len(buf), 5);
	assert_memory_equal(buffer_get_ptr(buf), "hello", 5);

	len = buffer_put_arr(buf, sarr, 1, 2);
	assert_int_equal(len, 2);
	assert_int_equal(buffer_get_len(buf), 7);
	assert_memory_equal(buffer_get_ptr(buf), "helloel", 7);

	len = buffer_put_arr(buf, sarr, 1, 4);
	assert_int_equal(len, 3);
	assert_int_equal(buffer_get_len(buf), 10);
	assert_memory_equal(buffer_get_ptr(buf), "helloelell", 10);

	assert_int_equal(buffer_get_pos(buf), 0);
	char darr[20];
	len = buffer_get_arr(buf, darr, 10, 20);
	assert_int_equal(len, 10);
	assert_int_equal(buffer_get_pos(buf), 10);
	assert_memory_equal(darr + 10, "helloelell", 10);

	buffer_rewind(buf);
	assert_int_equal(buffer_get_pos(buf), 0);

	len = buffer_get_arr(buf, darr, 0, 3);
	assert_int_equal(len, 3);
	assert_int_equal(buffer_get_pos(buf), 3);
	assert_memory_equal(darr, "hel", 3);

	len = buffer_get_arr(buf, darr, 3, 3);
	assert_int_equal(len, 3);
	assert_int_equal(buffer_get_pos(buf), 6);
	assert_memory_equal(darr, "helloe", 6);

	len = buffer_get_arr(buf, darr, 6, 5);
	assert_int_equal(len, 4);
	assert_int_equal(buffer_get_pos(buf), 10);
	assert_memory_equal(darr, "helloelell", 10);
}

int main() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_buffer_set_get_macro),
		cmocka_unit_test(test_buffer_is_macro),
		cmocka_unit_test_setup_teardown(test_buffer_new, test_setup, test_teardown),
		cmocka_unit_test_setup_teardown(test_buffer_put_get, test_setup, test_teardown),
		cmocka_unit_test_setup_teardown(test_buffer_get_put_arr, test_setup, test_teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
