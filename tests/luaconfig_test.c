#include <luaconfig.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int
test_construct(void)
{
	printf("luaconfig_construct()\n");

	/* construct luaconfig, confirm initial state */
	{
		LuaConfig * tc1 = luaconfig_construct();
		assert(tc1 && "luaconfig_construct returned 0");

		int errcode = luaconfig_get_error_code(tc1);
		assert(0 == errcode && "errcode != 0");

		const char * errstr = luaconfig_get_error_string(tc1);
		assert(0 != errstr && "errstr == 0");
		assert(0 == errstr[0] && "errstr[0] not empty");

		lua_State * ls = luaconfig_get_lua_State(tc1);
		assert(0 != ls && "lua_State == 0");

		luaconfig_destruct(tc1);
	}

	return EXIT_SUCCESS;
}


int
test_destruct(void)
{
	printf("luaconfig_destruct()\n");

	/* destruct a default luaconfig */
	{
		LuaConfig * tc1 = luaconfig_construct();
		assert(tc1 && "luaconfig_construct returned 0");

		luaconfig_destruct(tc1);
	}

	/* destruct a NULL pointer (should be a no-op) */
	{
		luaconfig_destruct(0);
	}

	return EXIT_SUCCESS;
}



enum TestMask {
	TEST_CONSTRUCT		= 1<<0,
	TEST_DESTRUCT		= 1<<1,
};

void
do_test(
	const enum TestMask	iTests,
	const enum TestMask	iFlag, 
	int 			(*iFunc)(void),
	int *			ioResult)
{
	if (! (iTests & iFlag)) {
		return;
	}

	int res = iFunc();
	if (res > *ioResult) {
		*ioResult = res;
	}

}

int
main(int argc, char * argv[])
{
	enum TestMask	tests = 0;

		/* determine progname */
	char * progname = strrchr(argv[0], '/');
	if (progname) {
		++progname;
	} else {
		progname = argv[0];
	}

		/* no arguments: test everything */
	if (argc == 1) {
		tests = ~0;
	}

		/* only test specific items */
	int i = 0;
	for (i = 1; i < argc; ++i) {
		if (0) {
		} else if (0 == strcmp("construct", argv[i])) {
			tests |= TEST_CONSTRUCT;
		} else if (0 == strcmp("destruct", argv[i])) {
			tests |= TEST_DESTRUCT;
		} else {
			fprintf(stderr, "%s: unknown argument '%s'\n",
				progname, argv[i]);
			return EXIT_FAILURE;
		}
	}

	assert(tests && "no tests selected");

		/* perform each test */
	int result = EXIT_SUCCESS;

	do_test(tests, TEST_CONSTRUCT, test_construct, &result);
	do_test(tests, TEST_DESTRUCT, test_destruct, &result);

	return result;
}
