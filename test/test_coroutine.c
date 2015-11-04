#include "../src/coroutine.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>

static int test_setup(void** state) {
	Coroutine coro = Coroutine_new(1);
	*state = coro;
	return 0;
}
static int test_teardown(void** state) {
	Coroutine_free(*state);
	return 0;
}

void run(Coroutine coro) {
	assert_true(Coroutine_isRun(coro));
	char* str = "hello";
	assert_memory_equal(coro->res, str, strlen(str));

	Coroutine_yield(coro);
	assert_true(Coroutine_isRun(coro));
	str = "world";
	assert_memory_equal(coro->res, str, strlen(str));

	Coroutine_yield(coro);
	assert_true(Coroutine_isRun(coro));
}

static void test_resume_yield_status(void** state) {
	Coroutine coro = (Coroutine)*state;
	assert_true(Coroutine_isInit(coro));

	Coroutine_init(coro, &run);
	assert_true(Coroutine_isPend(coro));

	coro->res = "hello";
	Coroutine_resume(coro);
	assert_true(Coroutine_isPend(coro));

	coro->res = "world";
	Coroutine_resume(coro);
	assert_true(Coroutine_isPend(coro));

	Coroutine_resume(coro);
	assert_true(Coroutine_isEnd(coro));
}

int main() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_resume_yield_status,
				test_setup, test_teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
