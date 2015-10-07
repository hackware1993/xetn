#include "../src/coroutine.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>

static int test_setup(void** state) {
	Coroutine coro = coroutine_new(1);
	*state = coro;
	return 0;
}
static int test_teardown(void** state) {
	coroutine_free(*state);
	return 0;
}

void run(Coroutine coro) {
	assert_true(coroutine_is_run(coro));
	char* str = "hello";
	assert_memory_equal(coro->res, str, strlen(str));

	coroutine_yield(coro);
	assert_true(coroutine_is_run(coro));
	str = "world";
	assert_memory_equal(coro->res, str, strlen(str));

	coroutine_yield(coro);
	assert_true(coroutine_is_run(coro));
}

static void test_resume_yield_status(void** state) {
	Coroutine coro = (Coroutine)*state;
	assert_true(coroutine_is_init(coro));

	coroutine_init(coro, &run);
	assert_true(coroutine_is_pend(coro));

	coro->res = "hello";
	coroutine_resume(coro);
	assert_true(coroutine_is_pend(coro));

	coro->res = "world";
	coroutine_resume(coro);
	assert_true(coroutine_is_pend(coro));

	coroutine_resume(coro);
	assert_true(coroutine_is_end(coro));
}

int main() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_resume_yield_status,
				test_setup, test_teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
