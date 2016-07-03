#include "buffer.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

static int test_setup(void** state) {
	Buffer buf = Buffer_new(10);
	*state = buf;
	return 0;
}
static int test_teardown(void** state) {
	Buffer_free(*state);
	return 0;
}

static void test_buffer_set_get_macro(void** state) {
	buffer_t buf;
	buf.lim = 11;
	buf.cap = 12;
	buf.pos = 0;
	buf.end = 2;
	/* test for buffer_get_ptr */
	assert_non_null(Buffer_getPtr(&buf));
	/* test for buffer_get_cap */
	assert_int_equal(Buffer_getCap(&buf), 12);
	/* test for Buffer_get_lim & Buffer_set_lim */
	assert_int_equal(Buffer_getLim(&buf), 11);
	Buffer_setLim(&buf, 10);
	assert_int_equal(buf.lim, 10);
	/* test for Buffer_get_pos & Buffer_set_pos */
	assert_int_equal(Buffer_getPos(&buf), 0);
	Buffer_setPos(&buf, 1);
	assert_int_equal(buf.pos, 1);
	/* test for Buffer_get_len & Buffer_set_len */
	assert_int_equal(Buffer_getLen(&buf), 2);
	Buffer_setLen(&buf, 3);
	assert_int_equal(buf.end, 3);
}

static void test_buffer_is_macro(void** state) {
	struct buffer buf;
	/* test for Buffer_is_empty */
	buf.end = 0;
	assert_true(Buffer_isEmpty(&buf));
	buf.end = 1;
	assert_false(Buffer_isEmpty(&buf));
	/* test for Buffer_is_end */
	buf.pos = 1;
	assert_true(Buffer_isEnd(&buf));
	buf.pos = 0;
	assert_false(Buffer_isEnd(&buf));
	/* test for Buffer_is_full */
	buf.lim = 1;
	assert_true(Buffer_isFull(&buf));
	buf.lim = 2;
	assert_false(Buffer_isFull(&buf));
}

static void test_buffer_new(void** state) {
	Buffer buf = (Buffer)*state;
	assert_non_null(buf);
	//assert_non_null(Buffer_get_ptr(buf));
	assert_int_equal(Buffer_getPos(buf), 0);
	assert_int_equal(Buffer_getLen(buf), 0);
	assert_int_equal(Buffer_getLim(buf), 10);
	assert_int_equal(Buffer_getCap(buf), 10);
	assert_true(Buffer_isEmpty(buf));
}

static void test_buffer_put_get(void** state) {
	Buffer buf = (Buffer)*state;
	/* test for Buffer_put */
	Buffer_put(buf, 'h');
	assert_int_equal(Buffer_getLen(buf), 1);
	Buffer_put(buf, 'e');
	assert_int_equal(Buffer_getLen(buf), 2);
	Buffer_put(buf, '3');
	assert_int_equal(Buffer_getLen(buf), 3);
	assert_memory_equal(Buffer_getPtr(buf), "he3", Buffer_getLen(buf));
	assert_int_equal(Buffer_getByIndex(buf, 0), 'h');
	assert_int_equal(Buffer_getByIndex(buf, 1), 'e');
	assert_int_equal(Buffer_getByIndex(buf, 2), '3');

	Buffer_setByIndex(buf, 0, '0');
	Buffer_setByIndex(buf, 1, '1');
	Buffer_setByIndex(buf, 2, '2');
	assert_memory_equal(buf->ptr, "012", Buffer_getLen(buf));

	char ch;
	/* test for Buffer_get */
	assert_int_equal(Buffer_getPos(buf), 0);
	ch = Buffer_get(buf);
	assert_int_equal(Buffer_getPos(buf), 1);
	assert_int_equal(ch, '0');
	ch = Buffer_get(buf);
	assert_int_equal(Buffer_getPos(buf), 2);
	assert_int_equal(ch, '1');
	ch = Buffer_get(buf);
	assert_int_equal(Buffer_getPos(buf), 3);
	assert_int_equal(ch, '2');
	assert_true(Buffer_isEnd(buf));

	Buffer_clear(buf);
	/* test for Buffer_clear */
	assert_int_equal(Buffer_getPos(buf), 0);
	assert_int_equal(Buffer_getLen(buf), 0);
	assert_int_equal(Buffer_getLim(buf), 10);
	assert_int_equal(Buffer_getCap(buf), 10);
	assert_true(Buffer_isEmpty(buf));
}
static void test_buffer_get_str(void** state) {
	Buffer buf = (Buffer)*state;
	char str[] = "hello";
	unsigned len = Buffer_putArr(buf, str, 0, 5);
	assert_int_equal(len, 5);
	assert_int_equal(Buffer_getLen(buf), 5);
	assert_int_equal(Buffer_getStr(buf)[5], '\0');
	assert_memory_equal(Buffer_getStr(buf), "hello", 6);
}

static void test_buffer_get_put_arr(void** state) {
	Buffer buf = (Buffer)*state;
	unsigned len = 0;
	char sarr[] = "hello";
	len = Buffer_putArr(buf, sarr, 0, 5);
	assert_int_equal(len, 5);
	assert_int_equal(Buffer_getLen(buf), 5);
	assert_memory_equal(Buffer_getPtr(buf), "hello", 5);

	len = Buffer_putArr(buf, sarr, 1, 2);
	assert_int_equal(len, 2);
	assert_int_equal(Buffer_getLen(buf), 7);
	assert_memory_equal(Buffer_getPtr(buf), "helloel", 7);

	len = Buffer_putArr(buf, sarr, 1, 4);
	assert_int_equal(len, 3);
	assert_int_equal(Buffer_getLen(buf), 10);
	assert_memory_equal(Buffer_getPtr(buf), "helloelell", 10);

	assert_int_equal(Buffer_getPos(buf), 0);
	char darr[20];
	len = Buffer_getArr(buf, darr, 10, 20);
	assert_int_equal(len, 10);
	assert_int_equal(Buffer_getPos(buf), 10);
	assert_memory_equal(darr + 10, "helloelell", 10);

	Buffer_rewind(buf);
	assert_int_equal(Buffer_getPos(buf), 0);

	len = Buffer_getArr(buf, darr, 0, 3);
	assert_int_equal(len, 3);
	assert_int_equal(Buffer_getPos(buf), 3);
	assert_memory_equal(darr, "hel", 3);

	len = Buffer_getArr(buf, darr, 3, 3);
	assert_int_equal(len, 3);
	assert_int_equal(Buffer_getPos(buf), 6);
	assert_memory_equal(darr, "helloe", 6);

	len = Buffer_getArr(buf, darr, 6, 5);
	assert_int_equal(len, 4);
	assert_int_equal(Buffer_getPos(buf), 10);
	assert_memory_equal(darr, "helloelell", 10);
}

int main() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_buffer_set_get_macro),
		cmocka_unit_test(test_buffer_is_macro),
		cmocka_unit_test_setup_teardown(test_buffer_new, test_setup, test_teardown),
		cmocka_unit_test_setup_teardown(test_buffer_put_get, test_setup, test_teardown),
		cmocka_unit_test_setup_teardown(test_buffer_get_put_arr, test_setup, test_teardown),
		cmocka_unit_test_setup_teardown(test_buffer_get_str, test_setup, test_teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
