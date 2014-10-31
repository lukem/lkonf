#include <lkonf.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int
test_construct(void)
{
	printf("lkonf_construct()\n");

	/* construct lkonf_t, confirm initial state */
	{
		lkonf_t * tc1 = lkonf_construct();
		assert(tc1 && "lkonf_construct returned 0");

		const lkerr_t errcode = lkonf_get_error_code(tc1);
		assert(LK_OK == errcode && "errcode != LK_OK");

		const char * errstr = lkonf_get_error_string(tc1);
		assert(0 != errstr && "errstr == 0");
		assert(0 == errstr[0] && "errstr[0] not empty");

		lua_State * ls = lkonf_get_lua_State(tc1);
		assert(0 != ls && "lua_State == 0");

		lkonf_destruct(tc1);
	}

	return EXIT_SUCCESS;
}


int
test_destruct(void)
{
	printf("lkonf_destruct()\n");

	/* destruct a default lkonf_t */
	{
		lkonf_t * tc1 = lkonf_construct();
		assert(tc1 && "lkonf_construct returned 0");

		lkonf_destruct(tc1);
	}

	/* destruct a NULL pointer (should be a no-op) */
	{
		lkonf_destruct(0);
	}

	return EXIT_SUCCESS;
}


int
test_load_file(void)
{
	printf("lkonf_load_file()\n");

	/* fail: load null kconf_t: fail */
	{
		assert(LK_LKONF_NULL == lkonf_load_file(0, 0));
	}

/* TODO: fail: load kconf_t with null state */

	/* fail: load with filename null pointer */
	{
		lkonf_t * tc1 = lkonf_construct();
		assert(tc1 && "lkonf_construct returned 0");
		assert(LK_ARG_BAD == lkonf_load_file(tc1, 0));
		lkonf_destruct(tc1);
	}

/* TODO: finish */

	return EXIT_SUCCESS;
}


int
test_load_string(void)
{
	printf("lkonf_load_string()\n");

	/* fail: load null kconf_t: fail */
	{
		assert(LK_LKONF_NULL == lkonf_load_string(0, 0));
	}

/* TODO: fail: load kconf_t with null state */

	/* fail: load with string null pointer */
	{
		lkonf_t * tc1 = lkonf_construct();
		assert(tc1 && "lkonf_construct returned 0");
		assert(LK_ARG_BAD == lkonf_load_string(tc1, 0));
		lkonf_destruct(tc1);
	}

/* TODO: finish */

	return EXIT_SUCCESS;
}


/**
 * One flag per test.
 */
enum TestFlags {
	TF_construct		= 1<<0,
	TF_destruct		= 1<<1,
	TF_load_file		= 1<<2,
	TF_load_string		= 1<<3,
};


/**
 * Mapping of test name to TestFlags and function to execute.
 */
const struct {
	const char *		name;
	enum TestFlags		flag;
	int			(*function)(void);
} nameToTest[] = {
	{ "construct",		TF_construct,	test_construct },
	{ "destruct",		TF_destruct,	test_destruct },
	{ "load_file",		TF_load_file,	test_load_file },
	{ "load_string",	TF_load_string,	test_load_string,},
	{ 0,			0,		0 },
};


int
usage(const char * progname)
{
	fprintf(stderr, "Usage: %s <test> [...]\n", progname);
	fprintf(stderr, " Supported <test> values:\n");
	fprintf(stderr, "   all          all tests\n");
	int ti;
	for (ti = 0; nameToTest[ti].name; ++ti) {
		fprintf(stderr, "   %s\n", nameToTest[ti].name);
	}
	return EXIT_FAILURE;
}

int
main(int argc, char * argv[])
{
		/* determine progname */
	char * progname = strrchr(argv[0], '/');
	if (progname) {
		++progname;
	} else {
		progname = argv[0];
	}

		/* only test specific items */
	enum TestFlags tests = 0;
	int ai, ti;
	for (ai = 1; ai < argc; ++ai) {
		if (0 == strcmp("all", argv[ai])) {
			tests = ~0;
			continue;
		}
		for (ti = 0; nameToTest[ti].name; ++ti) {
			if (0 == strcmp(argv[ai], nameToTest[ti].name))
				break;
		}
		if (! nameToTest[ti].name) {
			fprintf(stderr, "%s: unknown argument '%s'\n",
				progname, argv[ai]);
			return usage(progname);
		}
		tests |= nameToTest[ti].flag;
	}

	if (! tests) {
		fprintf(stderr, "%s: no tests selected\n", progname);
		return usage(progname);
	}

		/* perform each test */
	int result = EXIT_SUCCESS;

	for (ti = 0; nameToTest[ti].name; ++ti) {
		if (! (tests & nameToTest[ti].flag)) {
			continue;
		}
		int res = nameToTest[ti].function();
		if (res > result) {
			result = res;
		}
	}

	return result;
}
