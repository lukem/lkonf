#include <lkonf.h>

#include <assert.h>
#include <inttypes.h>
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
	TF_get_integer		= 1<<5,
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
		case LK_KEY_BAD:	return "LK_KEY_BAD";
		case LK_VALUE_BAD:	return "LK_VALUE_BAD";
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
	}
	const lkerr_t gec = lkonf_get_error_code(lk);
	if (code != gec) {
		fprintf(stderr, "%s: code %d (%s) != get_error_code %d (%s)\n",
			desc,
			code, err_to_str(code),
			gec, err_to_str(gec));
	}
	const char * lk_errstr = lkonf_get_error_string(lk);
	if (! streq(lk_errstr, expect_str)) {
		fprintf(stderr, "%s: errstr '%s' != expect_code '%s'\n",
			desc, lk_errstr, expect_str);
	}
	assert(code == expect_code);
	assert(code == gec);
	assert(streq(lk_errstr, expect_str) && "errstr != expect_str");
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

	/* fail: load null kconf_t */
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

	/* fail: load null kconf_t */
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

	/* fail: load null kconf_t */
	{
		assert(-1 == lkonf_get_instruction_limit(0));
		assert(LK_LKONF_NULL == lkonf_set_instruction_limit(0, 0));
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

void
exercise_get_integer(
	const char *		path,
	const lua_Integer	wanted,
	const lkerr_t		expect_code,
	const char *		expect_str)
{
	const char * gilua = "\
t1 = 1 \
t2 = { k1 = 2 } \
t3 = { k1 = { k2 = 33, b3 = false } } \
t4 = { f = function (x) return 4 end } \
t5 = function (x) return false end \
t6 = { [\"\"] = { k2 = 6 } } \
t7 = { [\"\"] = 777 } \
x = 1 \
t = {} \
loooooooooooooooooooooooooooong = { x = { yyyyyy = 99 }} \
justright = function (x) local f for i=1, 5 do f=i end return f end \
toolong = function (x) for f=1,1000 do end end \
badrun = function (x) print() end \
local hidden = 1 \
";
	char desc[128];
	snprintf(desc, sizeof(desc), "get_integer('%s')", path);
	if (LK_OK == expect_code) {
		printf("%s = %" PRId64 "\n", desc, (int64_t)wanted);
	} else {
		printf("%s fail; code %s '%s'\n",
			desc, err_to_str(expect_code), expect_str);
	}

	lkonf_t * lk = lkonf_construct();
	assert(lk && "lkonf_construct returned 0");

	const lkerr_t rls = lkonf_load_string(lk, gilua);
	ensure_result(lk, rls, "load_string gilua", LK_OK, "");

		/* limit to 100 instructions; after load */
	const lkerr_t sil = lkonf_set_instruction_limit(lk, 100);
	ensure_result(lk, sil, "set_instruction_limit(lk, 100)", LK_OK, "");

	lua_Integer v = -1;
	const lkerr_t res = lkonf_get_integer(lk, path, &v);
	ensure_result(lk, res, desc, expect_code, expect_str);
	if (LK_OK == expect_code) {
		assert(wanted == v);
	} else {
		assert(-1 == v);	/* didn't change */
	}

	lkonf_destruct(lk);
}

int
test_get_integer(void)
{
	printf("lkonf_get_integer()\n");

	/* fail: load null kconf_t */
	{
		assert(LK_LKONF_NULL == lkonf_get_integer(0, 0, 0));
	}

	/* fail: null ovalue */
	{
		lkonf_t * lk = lkonf_construct();
		assert(lk && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_get_integer(lk, "", 0);
		ensure_result(lk, res,
			"get_integer(lk, \"\", 0)", LK_ARG_BAD, "oValue NULL");

		lkonf_destruct(lk);
	}

	/* fail: null path */
	{
		lkonf_t * lk = lkonf_construct();
		assert(lk && "lkonf_construct returned 0");

		lua_Integer v = -1;
		const lkerr_t res = lkonf_get_integer(lk, 0, &v);
		ensure_result(lk, res,
			"get_integer(lk, 0, &v)", LK_ARG_BAD, "iPath NULL");

		lkonf_destruct(lk);
	}

	/* pass: t1 */
	exercise_get_integer("t1", 1, LK_OK, "");

	/* fail: missing top-level key */
	exercise_get_integer("missing", 0,
		LK_VALUE_BAD, "Not an integer: missing");

	/* pass: t2.k1 */
	exercise_get_integer("t2.k1", 2, LK_OK, "");

	/* pass: t3.k1.k2 */
	exercise_get_integer("t3.k1.k2", 33, LK_OK, "");

	/* fail: t3.k1.k2. */
	exercise_get_integer("t3.k1.k2.", 33, LK_KEY_BAD, "Not a table: k2");

	/* fail: t3.k1.b3 (not an integer)  */
	exercise_get_integer("t3.k1.b3", 0,
		LK_VALUE_BAD, "Not an integer: t3.k1.b3");

	/* fail: t3.k.k2 */
	exercise_get_integer("t3.k.k2", 0, LK_KEY_BAD, "Not a table: k");

	/* fail: t3.12345.3 */
	exercise_get_integer("t3.12345.3", 0, LK_KEY_BAD, "Not a table: 12345");

	/* pass: t4.f function returning int */
	exercise_get_integer("t4.f", 4, LK_OK, "");

	/* fail: t4.f. */
	exercise_get_integer("t4.f.", 4, LK_KEY_BAD, "Not a table: f");

	/* fail: t5 function not returning int */
	exercise_get_integer("t5", 0, LK_VALUE_BAD, "Not an integer: t5");

	/* fail: t6..k2 - empty key */
	exercise_get_integer("t6..k2", 6, LK_KEY_BAD, "Empty key");

	/* fail: t6...k2 */
	exercise_get_integer("t6...k2", 0, LK_KEY_BAD, "Empty key");

	/* fail: "" */
	exercise_get_integer("", -5, LK_KEY_BAD, "Empty key");

	/* fail: "." */
	exercise_get_integer(".", -5, LK_KEY_BAD, "Empty key");

	/* pass: x */
	exercise_get_integer("x", 1, LK_OK, "");

	/* pass: loooooooooooooooooooooooooooong.x.yyyyyy */
	exercise_get_integer("loooooooooooooooooooooooooooong.x.yyyyyy",
		99, LK_OK, "");

	/* fail: t7. */
	exercise_get_integer("t7.", 777, LK_KEY_BAD, "Empty key");

	/* fail: .t8 */
	exercise_get_integer(".t8", 777, LK_KEY_BAD, "Empty key");

	/* fail: t */
	exercise_get_integer("t", 0, LK_VALUE_BAD, "Not an integer: t");

	/* fail: t. */
	exercise_get_integer("t.", 0, LK_KEY_BAD, "Empty key");

	/* fail: t.k */
	exercise_get_integer("t.k", 0, LK_VALUE_BAD, "Not an integer: t.k");

	/* fail: toolong takes too long */
	exercise_get_integer("toolong", 0,
		LK_CALL_CHUNK, "Instruction count exceeded");

	/* fail: badrun calls unknown symbol */
	exercise_get_integer("badrun", 0,
		LK_CALL_CHUNK, "[string \"t1 = 1 t2 = { k1 = 2 } t3 = { k1 = { k2 = 3...\"]:1: attempt to call global 'print' (a nil value)");

	/* pass: justright */
	exercise_get_integer("justright", 5, LK_OK, "");

	/* fail: hidden */
	exercise_get_integer("hidden", 0,
		LK_VALUE_BAD, "Not an integer: hidden");

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
	{ "get_integer",	TF_get_integer, test_get_integer },
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
