#include "coroutine.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>

void* run(Coroutine coro, void* arg) {
	assert_true(Coroutine_isRun(coro));

	char* str = "hello";
	assert_memory_equal(arg, str, strlen(str));

	arg = Coroutine_yield(coro, "yield1");
	assert_true(Coroutine_isRun(coro));

	str = "world";
	assert_memory_equal(arg, str, strlen(str));

	arg = Coroutine_yield(coro, "yield2");
	assert_true(Coroutine_isRun(coro));

	str = "done";
	assert_memory_equal(arg, str, strlen(str));

	return "finish";
}

static int test_setup(void** state) {
	Coroutine coro = Coroutine_new(run, 4096);
	*state = coro;
	return 0;
}
static int test_teardown(void** state) {
	Coroutine_close((Coroutine)*state);
	return 0;
}

static void test_resume_yield_status(void** state) {
	Coroutine coro = (Coroutine)*state;
	assert_true(Coroutine_isInit(coro));

	char* arg;
	arg = Coroutine_resume(coro, "hello");
	assert_true(Coroutine_isPend(coro));
	assert_memory_equal(arg, "yield1", 6);

	arg = Coroutine_resume(coro, "world");
	assert_true(Coroutine_isPend(coro));
	assert_memory_equal(arg, "yield2", 6);

	arg = Coroutine_resume(coro, "done");
	assert_true(Coroutine_isEnd(coro));
	assert_memory_equal(arg, "finish", 6);
}

int main() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_resume_yield_status,
				test_setup, test_teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
