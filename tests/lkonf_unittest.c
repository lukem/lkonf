#include <lkonf.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**
 * One flag per test.
 */
enum TestFlags
{
	TF_construct		= 1<<0,
	TF_destruct		= 1<<1,
	TF_load_file		= 1<<2,
	TF_load_string		= 1<<3,
	TF_instruction_limit	= 1<<4,
};


/**
 * Return string representation of a given lkerr_t code.
 */
const char *
err_to_str(const lkerr_t code)
{
	switch (code) {
		case LK_OK:		return "LK_OK";
		case LK_LKONF_NULL:	return "LK_LKONF_NULL";
		case LK_STATE_NULL:	return "LK_STATE_NULL";
		case LK_ARG_BAD:	return "LK_ARG_BAD";
		case LK_LOAD_CHUNK:	return "LK_LOAD_CHUNK";
		case LK_CALL_CHUNK:	return "LK_CALL_CHUNK";
	}
	return "<unknown>";
}

/**
 * Return non-zero if the strings are equal.
 */
int
streq(const char * lhs, const char * rhs)
{
	assert(lhs);
	assert(rhs);
	return (0 == strcmp(lhs, rhs));
}

/**
 * Validate that the code and error string were returned for a given test.
 */
void
ensure_result(
	lkonf_t *	lk,
	const lkerr_t	code,
	const char *	desc,
	const lkerr_t	expect_code,
	const char *	expect_str)
{
	if (code != expect_code) {
		fprintf(stderr, "%s: code %d (%s) != expected %d (%s)\n",
			desc,
			code, err_to_str(code),
			expect_code, err_to_str(expect_code));
		assert(code == expect_code);
	}
	const lkerr_t gec = lkonf_get_error_code(lk);
	if (code != gec) {
		fprintf(stderr, "%s: code %d (%s) != get_error_code %d (%s)\n",
			desc,
			code, err_to_str(code),
			gec, err_to_str(gec));
		assert(code == gec);
	}
	const char * lk_errstr = lkonf_get_error_string(lk);
	if (! streq(lk_errstr, expect_str)) {
		fprintf(stderr, "%s: errstr '%s' != expected '%s'\n",
			desc, lk_errstr, expect_str);
		assert(0 && "errstr != expected");
	}
}


int
test_construct(void)
{
	printf("lkonf_construct()\n");

	/* construct lkonf_t, confirm initial state */
	{
		lkonf_t * lk = lkonf_construct();
		assert(lk && "lkonf_construct returned 0");

		const lkerr_t errcode = lkonf_get_error_code(lk);
		assert(LK_OK == errcode && "errcode != LK_OK");

		const char * errstr = lkonf_get_error_string(lk);
		assert(0 != errstr && "errstr == 0");
		assert(0 == errstr[0] && "errstr[0] not empty");

		lua_State * ls = lkonf_get_lua_State(lk);
		assert(0 != ls && "lua_State == 0");

		lkonf_destruct(lk);
	}

	return EXIT_SUCCESS;
}


int
test_destruct(void)
{
	printf("lkonf_destruct()\n");

	/* destruct a default lkonf_t */
	{
		lkonf_t * lk = lkonf_construct();
		assert(lk && "lkonf_construct returned 0");

		lkonf_destruct(lk);
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
		const lkerr_t res = lkonf_load_file(0, 0);
		assert(LK_LKONF_NULL == res);
	}

/* TODO: fail: load kconf_t with null state */

	/* fail: load with filename null pointer */
	{
		lkonf_t * lk = lkonf_construct();

		assert(lk && "lkonf_construct returned 0");
		const lkerr_t res = lkonf_load_file(lk, 0);
		ensure_result(lk, res,
			"load_file(lk, 0)", LK_ARG_BAD, "iFile NULL");

		lkonf_destruct(lk);
	}

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
		lkonf_t * lk = lkonf_construct();
		assert(lk && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_load_string(lk, 0);
		ensure_result(lk, res,
			"load_string(lk, 0)", LK_ARG_BAD, "iString NULL");

		lkonf_destruct(lk);
	}

	/* fail: LK_LOAD_CHUNK syntax error in string */
	{
		lkonf_t * lk = lkonf_construct();
		assert(lk && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_load_string(lk, "junk");
		ensure_result(lk, res,
			"load_string(lk, \"junk\")",
			LK_LOAD_CHUNK,
			"[string \"junk\"]:1: '=' expected near '<eof>'");

		lkonf_destruct(lk);
	}

	/* fail: LK_CALL_CHUNK run-time error in string */
	{
		lkonf_t * lk = lkonf_construct();
		assert(lk && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_load_string(lk, "junk()");
		ensure_result(lk, res,
			"load_string(lk, \"junk()\")",
			LK_CALL_CHUNK,
			"[string \"junk()\"]:1: attempt to call global 'junk' (a nil value)");

		lkonf_destruct(lk);
	}

	/* pass: "t=1" */
	{
		lkonf_t * lk = lkonf_construct();
		assert(lk && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_load_string(lk, "t=1");
		ensure_result(lk, res, "load_string(lk, \"t=1\")", LK_OK, "");

		lkonf_destruct(lk);
	}

/* TODO: sandbox tests */

	return EXIT_SUCCESS;
}

int
test_instruction_limit(void)
{
	printf("lkonf_set_instruction_limit()\n");

	/* fail: load null kconf_t: fail */
	{
		assert(LK_LKONF_NULL == lkonf_load_string(0, 0));
	}

	/* fail: set_instruction_limit < 0 */
	{
		lkonf_t * lk = lkonf_construct();
		assert(lk && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_set_instruction_limit(lk, -2);
		ensure_result(lk, res,
			"set_instruction_limit(lk, -2)",
			LK_ARG_BAD, "iLimit < 0");

		lkonf_destruct(lk);
	}

	/* pass: set & get */
	{
		lkonf_t * lk = lkonf_construct();
		assert(lk && "lkonf_construct returned 0");

		const int	limit = 10;

		const lkerr_t sil = lkonf_set_instruction_limit(lk, limit);
		ensure_result(lk, sil,
			"set_instruction_limit(lk, 10)", LK_OK, "");

		const int gil = lkonf_get_instruction_limit(lk);
		assert(limit == gil);

		lkonf_destruct(lk);
	}

	/* pass: long loop, no limit */
	{
		lkonf_t * lk = lkonf_construct();
		assert(lk && "lkonf_construct returned 0");

		const int	limit = 0;
		const char *	expr = "for i=1, 1000 do end";

		const lkerr_t sil = lkonf_set_instruction_limit(lk, limit);
		assert(LK_OK == sil);

		const lkerr_t res = lkonf_load_string(lk, expr);
		ensure_result(lk, res, expr,
			LK_OK, "");

		const int gil = lkonf_get_instruction_limit(lk);
		assert(limit == gil);

		lkonf_destruct(lk);
	}

	/* pass: long loop, limited and terminated */
	{
		lkonf_t * lk = lkonf_construct();
		assert(lk && "lkonf_construct returned 0");

		const int	limit = 100;
		const char *	expr = "for i=1, 1000 do end";

		const lkerr_t sil = lkonf_set_instruction_limit(lk, limit);
		assert(LK_OK == sil);

		const lkerr_t res = lkonf_load_string(lk, expr);
		ensure_result(lk, res, expr,
			LK_CALL_CHUNK, "Instruction count exceeded");

		const int gil = lkonf_get_instruction_limit(lk);
		assert(limit == gil);

		lkonf_destruct(lk);
	}

	return EXIT_SUCCESS;
}


/**
 * Mapping of test name to TestFlags and function to execute.
 */
const struct
{
	const char *		name;
	enum TestFlags		flag;
	int			(*function)(void);
} nameToTest[] = {
	{ "construct",		TF_construct,	test_construct },
	{ "destruct",		TF_destruct,	test_destruct },
	{ "load_file",		TF_load_file,	test_load_file },
	{ "load_string",	TF_load_string,	test_load_string },
	{ "instruction_limit",	TF_instruction_limit, test_instruction_limit },
	{ 0,			0,		0 },
};


int
usage(const char * progname)
{
	fprintf(stderr, "Usage: %s <test> [...]\n", progname);
	fprintf(stderr, "  Supported <test> values:\n");
	fprintf(stderr, "   all         all tests\n");
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
		if (streq("all", argv[ai])) {
			tests = ~0;
			continue;
		}
		for (ti = 0; nameToTest[ti].name; ++ti) {
			if (streq(argv[ai], nameToTest[ti].name))
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
