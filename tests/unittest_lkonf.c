#include <lkonf.h>

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



static const char * test_luastr = "\
b1 = true d1 = 1.01 i1 = 1 s1 = \"1\" \
t2 = { b = false, d = 2.714, i = 2, empty = \"\", [\"2\"] = \"two\", } \
t3 = { t = { b3 = false, d3 = 3.1415, i3 = 33, s3=\"thirty three\", } } \
tf = { \
	b = function(x) return true end, \
	d = function (x) return -4.01 end, \
	i = function (x) return 4 end, \
	s = function (x) return \"tf path=\"..x end, \
} \
t5b = function (x) return false end \
t5i = function (x) return 55 end \
t6 = { \
  [\"\"] = { [\"6.6\"] = { bm = true, dm = 6.001, im = 6, sm=\".\" }}, \
  [\".\"] = { b = false, d = -6.001, i = -6, s=\"dot\" }, \
} \
t7 = { [\"\"] = 777.0 } \
t9n = { [1] = true, [2] = 6.1, [3] = 6, [4] = \"six\" } \
t9s = { [\"1\"] = true, [\"2\"] = 6.1, [\"3\"] = 6, [\"4\"] = \"six\" } \
b = true d = 0.5 i = 11 s='sooooper' \
t = {} \
loooooooooooooooooooooooooooong = { x = { yb = true, yd = 99.999, yi=99, ys='yus', }} \
jrb = function (x) local f for i=1, 5 do f=i end return true end \
jrd = function (x) local f for i=1, 5 do f=i end return f-0.1 end \
jri = function (x) local f for i=1, 5 do f=i end return f end \
jrs = function (x) local f for i=1, 5 do f=i end return \"just right!\" end \
toolong = function (x) for f=1,1000 do end end \
badrun = function (x) print() end \
local hidden = 1 \
";

static const char * badrun_error =
#if LUA_VERSION_NUM >= 502
"[string \"b1 = true d1 = 1.01 i1 = 1 s1 = \"1\" t2 = { b ...\"]:1: attempt to call global 'print' (a nil value)"
#else
"[string \"b1 = true d1 = 1.01 i1 = 1 s1 = \"1\" t2 = { ...\"]:1: attempt to call global 'print' (a nil value)"
#endif
;


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
	TF_get_boolean		= 1<<5,
	TF_get_double		= 1<<6,
	TF_get_integer		= 1<<7,
	TF_get_string		= 1<<8,
	TF_getkey_boolean	= 1<<9,
	TF_getkey_double	= 1<<10,
	TF_getkey_integer	= 1<<11,
	TF_getkey_string	= 1<<12,
};


/**
 * Return non-zero if the strings are equal.
 */
bool
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
	lkonf_context *	lc,
	const lkonf_error	code,
	const char *	desc,
	const lkonf_error	expect_code,
	const char *	expect_str)
{
	if (code != expect_code) {
		fprintf(stderr, "%s: code %d (%s) != expected %d (%s)\n",
			desc,
			code, lkonf_error_to_string(code),
			expect_code, lkonf_error_to_string(expect_code));
	}
	const lkonf_error gec = lkonf_get_error_code(lc);
	if (code != gec) {
		fprintf(stderr, "%s: code %d (%s) != get_error_code %d (%s)\n",
			desc,
			code, lkonf_error_to_string(code),
			gec, lkonf_error_to_string(gec));
	}
	const char * lk_errstr = lkonf_get_error_string(lc);
	if (! streq(lk_errstr, expect_str)) {
		fprintf(stderr, "%s: errstr '%s' != expect_str '%s'\n",
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

	/* construct lkonf_context, confirm initial state */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkonf_error errcode = lkonf_get_error_code(lc);
		assert(LK_OK == errcode && "errcode != LK_OK");

		const char * errstr = lkonf_get_error_string(lc);
		assert(0 != errstr && "errstr == 0");
		assert(0 == errstr[0] && "errstr[0] not empty");

		lua_State * ls = lkonf_get_lua_State(lc);
		assert(0 != ls && "lua_State == 0");

		lkonf_destruct(lc);
	}

	return EXIT_SUCCESS;
}


int
test_destruct(void)
{
	printf("lkonf_destruct()\n");

	/* destruct a default lkonf_context */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		lkonf_destruct(lc);
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

	/* fail: load null lkonf_context */
	{
		const lkonf_error res = lkonf_load_file(0, 0);
		assert(LK_INVALID_ARGUMENT == res);
	}

/* TODO: fail: load lkonf_context with null state */

	/* fail: load with filename null pointer */
	{
		lkonf_context * lc = lkonf_construct();

		assert(lc && "lkonf_construct returned 0");
		const lkonf_error res = lkonf_load_file(lc, 0);
		ensure_result(lc, res,
			"load_file(lc, 0)", LK_INVALID_ARGUMENT, "iFile NULL");

		lkonf_destruct(lc);
	}

	return EXIT_SUCCESS;
}


int
test_load_string(void)
{
	printf("lkonf_load_string()\n");

	/* fail: load null lkonf_context */
	{
		assert(LK_INVALID_ARGUMENT == lkonf_load_string(0, 0));
	}

/* TODO: fail: load lkonf_context with null state */

	/* fail: load with string null pointer */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkonf_error res = lkonf_load_string(lc, 0);
		ensure_result(lc, res,
			"load_string(lc, 0)",
			LK_INVALID_ARGUMENT, "iString NULL");

		lkonf_destruct(lc);
	}

	/* fail: LK_LUA_ERROR syntax error in string */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkonf_error res = lkonf_load_string(lc, "junk");
		ensure_result(lc, res,
			"load_string(lc, \"junk\")",
			LK_LUA_ERROR,
#if LUA_VERSION_NUM >= 502
			"[string \"junk\"]:1: syntax error near <eof>"
#else
			"[string \"junk\"]:1: '=' expected near '<eof>'"
#endif
			);

		lkonf_destruct(lc);
	}

	/* fail: LK_LUA_ERROR run-time error in string */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkonf_error res = lkonf_load_string(lc, "junk()");
		ensure_result(lc, res,
			"load_string(lc, \"junk()\")",
			LK_LUA_ERROR,
			"[string \"junk()\"]:1: attempt to call global 'junk' (a nil value)");

		lkonf_destruct(lc);
	}

	/* pass: "t=1" */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkonf_error res = lkonf_load_string(lc, "t=1");
		ensure_result(lc, res, "load_string(lc, \"t=1\")", LK_OK, "");

		lkonf_destruct(lc);
	}

/* TODO: sandbox tests */

	return EXIT_SUCCESS;
}

int
test_instruction_limit(void)
{
	printf("lkonf_set_instruction_limit()\n");

	/* fail: load null lkonf_context */
	{
		assert(-1 == lkonf_get_instruction_limit(0));
		assert(LK_INVALID_ARGUMENT == lkonf_set_instruction_limit(0, 0));
	}

	/* fail: set_instruction_limit < 0 */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkonf_error res = lkonf_set_instruction_limit(lc, -2);
		ensure_result(lc, res,
			"set_instruction_limit(lc, -2)",
			LK_INVALID_ARGUMENT, "iLimit < 0");

		lkonf_destruct(lc);
	}

	/* pass: set & get */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const int	limit = 10;

		const lkonf_error sil = lkonf_set_instruction_limit(lc, limit);
		ensure_result(lc, sil,
			"set_instruction_limit(lc, 10)", LK_OK, "");

		const int gil = lkonf_get_instruction_limit(lc);
		assert(limit == gil);

		lkonf_destruct(lc);
	}

	/* pass: long loop, no limit */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const int	limit = 0;
		const char *	expr = "for i=1, 1000 do end";

		const lkonf_error sil = lkonf_set_instruction_limit(lc, limit);
		assert(LK_OK == sil);

		const lkonf_error res = lkonf_load_string(lc, expr);
		ensure_result(lc, res, expr,
			LK_OK, "");

		const int gil = lkonf_get_instruction_limit(lc);
		assert(limit == gil);

		lkonf_destruct(lc);
	}

	/* pass: long loop, limited and terminated */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const int	limit = 100;
		const char *	expr = "for i=1, 1000 do end";

		const lkonf_error sil = lkonf_set_instruction_limit(lc, limit);
		assert(LK_OK == sil);

		const lkonf_error res = lkonf_load_string(lc, expr);
		ensure_result(lc, res, expr,
			LK_LUA_ERROR, "Instruction count exceeded");

		const int gil = lkonf_get_instruction_limit(lc);
		assert(limit == gil);

		lkonf_destruct(lc);
	}

	return EXIT_SUCCESS;
}

#if 0
const char *
format_keys(lkonf_keys keys)
{
	static char keydesc[200];

	char * c = keydesc;
	const char * e = keydesc + sizeof(keydesc);
	size_t ki = 0;
	for (ki = 0; keys[ki]; ++ki) {
		if (ki) {
			c += snprintf(c, e-c, ",");
			if (c >= e)
				break;
		}
		c += snprintf(c, e-c, "\"%s\"", keys[ki]);
		if (c >= e)
			break;
	}
	return keydesc;
}
#endif

void
exercise_get_boolean(
	const bool		wanted,
	const char *		path,
	lkonf_keys		keys,
	const lkonf_error	expect_code,
	const char *		expect_str)
{
	assert(path);
	char desc[300];
#if 0
	if (keys) {
		snprintf(desc, sizeof(desc), "getkey_boolean({%s,0})",
			format_keys(keys));
	} else {
		snprintf(desc, sizeof(desc), "get_boolean(\"%s\")", path);
	}
#endif
	snprintf(desc, sizeof(desc), "%s_boolean(\"%s\")",
		keys ? "getkey" : "get", path);
	printf("%s = %s", desc, wanted ? "true" : "false");
	if (LK_OK != expect_code) {
		printf("; expect code %d [%s] '%s'",
			expect_code, lkonf_error_to_string(expect_code),
			expect_str);
	}
	printf("\n");

	lkonf_context * lc = lkonf_construct();
	assert(lc && "lkonf_construct returned 0");

	const lkonf_error rls = lkonf_load_string(lc, test_luastr);
	ensure_result(lc, rls, "load_string", LK_OK, "");

		/* limit to 100 instructions; after load */
	const lkonf_error sil = lkonf_set_instruction_limit(lc, 100);
	ensure_result(lc, sil, "set_instruction_limit(lc, 100)", LK_OK, "");

	bool v = !wanted;
	lkonf_error res = LK_INVALID_ARGUMENT;
	if (keys) {
		res = lkonf_getkey_boolean(lc, keys, &v);
	} else {
		res = lkonf_get_boolean(lc, path, &v);
	}

	ensure_result(lc, res, desc, expect_code, expect_str);
	if (LK_OK == expect_code) {
		if (wanted != v) {
			printf("wanted %d != v %d\n", wanted, v);
		}
		assert(wanted == v);
	} else {
		assert(!wanted == v);	/* didn't change */
	}

	lkonf_destruct(lc);
}

int
test_get_boolean(void)
{
	printf("lkonf_get_boolean()\n");

	/* fail: load null lkonf_context */
	{
		assert(LK_INVALID_ARGUMENT == lkonf_get_boolean(0, 0, 0));
	}

	/* fail: null oValue */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkonf_error res = lkonf_get_boolean(lc, "", 0);
		ensure_result(lc, res,
			"get_boolean(lc, \"\", 0)",
			LK_INVALID_ARGUMENT, "oValue NULL");

		lkonf_destruct(lc);
	}

	/* fail: null path */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		bool v = false;
		const lkonf_error res = lkonf_get_boolean(lc, 0, &v);
		ensure_result(lc, res,
			"get_boolean(lc, 0, &v)",
			LK_INVALID_ARGUMENT, "iPath NULL");

		lkonf_destruct(lc);
	}

	/* pass: b1 */
	exercise_get_boolean(true,
		"b1", NULL,
		LK_OK, "");

	/* fail: top-level key 'missing' not set */
	exercise_get_boolean(true,
		"missing", NULL,
		LK_NOT_FOUND, "");

	/* pass: t2.b */
	exercise_get_boolean(false,
		"t2.b", NULL,
		LK_OK, "");

	/* pass: t3.t.b3 */
	exercise_get_boolean(false,
		"t3.t.b3", NULL,
		LK_OK, "");

	/* (no test for t6 "" 6.6 bm) */

	/* pass: t3.t.absent not set */
	exercise_get_boolean(true,
		"t3.t.absent", NULL,
		LK_NOT_FOUND, "");

	/* fail: t3.t. */
	exercise_get_boolean(false,
		"t3.t.", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t3.t.");

	/* fail: t3.t.b3.k4 */
	exercise_get_boolean(true,
		"t3.t.b3.k4", NULL,
		LK_OUT_OF_RANGE, "Not a table: t3.t.b3");

	/* fail: t3.t.i3 (not a boolean)  */
	exercise_get_boolean(false,
		"t3.t.i3", NULL,
		LK_OUT_OF_RANGE, "Not a boolean: t3.t.i3");

	/* fail: t3.k.k2 */
	exercise_get_boolean(false,
		"t3.k.k2", NULL,
		LK_OUT_OF_RANGE, "Not a table: t3.k");

	/* fail: t3.12345.3 */
	exercise_get_boolean(false,
		"t3.12345.3", NULL,
		LK_OUT_OF_RANGE, "Not a table: t3.12345");

	/* pass: tf.b function returning boolean */
	exercise_get_boolean(true,
		"tf.b", NULL,
		LK_OK, "");

	/* fail: tf.b. (trailing .) */
	exercise_get_boolean(true,
		"tf.b.", NULL,
		LK_OUT_OF_RANGE, "Not a table: tf.b");

	/* fail: t5i function not returning boolean */
	exercise_get_boolean(false,
		"t5i", NULL,
		LK_OUT_OF_RANGE, "Not a boolean: t5i");

	/* fail: tf.i function not returning boolean */
	exercise_get_boolean(false,
		"tf.i", NULL,
		LK_OUT_OF_RANGE, "Not a boolean: tf.i");

	/* fail: t6..k2 - empty key */
	exercise_get_boolean(true,
		"t6..k2", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t6..k2");

	/* fail: t6 "." k2 - not a table "." */
	exercise_get_boolean(false,
		"t6...k2", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t6...k2");

	/* fail: "" */
	exercise_get_boolean(false,
		"", NULL,
		LK_OUT_OF_RANGE, "Empty path");

	/* fail: "." */
	exercise_get_boolean(false,
		".", NULL,
		LK_OUT_OF_RANGE, "Empty component in: .");

	/* pass: b */
	exercise_get_boolean(true,
		"b", NULL,
		LK_OK, "");

	/* pass: loooooooooooooooooooooooooooong.x.yb */
	exercise_get_boolean(true,
		"loooooooooooooooooooooooooooong.x.yb", NULL,
		LK_OK, "");

	/* fail: t7. */
	exercise_get_boolean(true,
		"t7.", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t7.");

	/* fail: .t8 */
	exercise_get_boolean(true,
		".t8", NULL,
		LK_OUT_OF_RANGE, "Empty component in: .t8");

	/* fail: t9n.1 */
/* TODO: fix path lookup to support integer lookup? */
	exercise_get_boolean(true,
		"t9n.1", NULL,
		LK_NOT_FOUND, "");

	/* pass: t9s.1 */
	exercise_get_boolean(true,
		"t9s.1", NULL,
		LK_OK, "");

	/* fail: t */
	exercise_get_boolean(false,
		"t", NULL,
		LK_OUT_OF_RANGE, "Not a boolean: t");

	/* fail: t. */
	exercise_get_boolean(false,
		"t.", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t.");

	/* fail: t.k nil VALUE */
	exercise_get_boolean(true,
		"t.k", NULL,
		LK_NOT_FOUND, "");

	/* fail: toolong takes too long */
	exercise_get_boolean(true,
		"toolong", NULL,
		LK_LUA_ERROR, "Instruction count exceeded");

	/* fail: badrun calls unknown symbol */
	exercise_get_boolean(false,
		"badrun", NULL,
		LK_LUA_ERROR, badrun_error);

	/* pass: jrb */
	exercise_get_boolean(true,
		"jrb", NULL,
		LK_OK, "");

	/* fail: hidden */
	exercise_get_boolean(true,
		"hidden", NULL,
		LK_NOT_FOUND, "");

	return EXIT_SUCCESS;
}

int
test_getkey_boolean(void)
{
	printf("lkonf_getkey_boolean()\n");

	/* fail: load null lkonf_context */
	{
		assert(LK_INVALID_ARGUMENT == lkonf_getkey_boolean(0, 0, 0));
	}

	/* fail: null oValue */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkonf_error res = lkonf_getkey_boolean(lc,
			(lkonf_keys){ 0 }, 0);
		ensure_result(lc, res, "getkey_boolean(lc, {0}, 0)",
			LK_INVALID_ARGUMENT, "oValue NULL");

		lkonf_destruct(lc);
	}

	/* fail: null iKeys */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		bool v = false;
		const lkonf_error res = lkonf_getkey_boolean(lc, 0, &v);
		ensure_result(lc, res, "getkey_boolean(lc, 0, &v)",
			LK_INVALID_ARGUMENT, "iKeys NULL");

		lkonf_destruct(lc);
	}

	/* fail: empty iKeys */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		bool v = false;
		const lkonf_error res = lkonf_getkey_boolean(lc,
			(lkonf_keys){ 0 }, &v);
		ensure_result(lc, res, "getkey_boolean(lc, {0}, &v)",
			LK_OUT_OF_RANGE, "Empty keys");

		lkonf_destruct(lc);
	}

	/* pass: b1 */
	exercise_get_boolean(true,
		"b1", (lkonf_keys){ "b1", 0 },
		LK_OK, "");

	/* fail: top-level key 'missing' not set */
	exercise_get_boolean(true,
		"missing", (lkonf_keys){ "missing", 0 },
		LK_NOT_FOUND, "");

	/* pass: t2 b */
	exercise_get_boolean(false,
		"t2 b", (lkonf_keys){"t2", "b", 0},
		LK_OK, "");

	/* pass: t3 t b3 */
	exercise_get_boolean(false,
		"t3 t b3", (lkonf_keys){"t3", "t", "b3", 0},
		LK_OK, "");

	/* pass: t6 "" "6.6" bm */
	exercise_get_boolean(true,
		"t6 \"\" \"6.6\" bm", (lkonf_keys){"t6", "", "6.6", "bm", 0},
		LK_OK, "");

	/* fail: t3 t absent not set */
	exercise_get_boolean(true,
		"t3 t absent", (lkonf_keys){"t3", "t", "absent", 0},
		LK_NOT_FOUND, "");

	/* fail: t3 t "" not set */
	exercise_get_boolean(true,
		"t3 t \"\"", (lkonf_keys){"t3", "t", "", 0},
		LK_NOT_FOUND, "");

	/* fail: t3 t b3 k4 */
	exercise_get_boolean(true,
		"t3 t b3 k4", (lkonf_keys){"t3", "t", "b3", "k4", 0},
		LK_OUT_OF_RANGE, "Not a table: b3");

	/* fail: t3 t i3 (not a boolean)  */
	exercise_get_boolean(false,
		"t3 t i3", (lkonf_keys){"t3", "t", "i3", 0},
		LK_OUT_OF_RANGE, "Not a boolean: i3");

	/* fail: t3 k k2 */
	exercise_get_boolean(false,
		"t3 k k2", (lkonf_keys){"t3", "k", "k2", 0},
		LK_OUT_OF_RANGE, "Not a table: k");

	/* fail: t3 12345 3 */
	exercise_get_boolean(false,
		"t3 12345 3", (lkonf_keys){"t3", "12345", "3", 0},
		LK_OUT_OF_RANGE, "Not a table: 12345");

	/* pass: tf b function returning boolean */
	exercise_get_boolean(true,
		"tf b", (lkonf_keys){"tf", "b", 0},
		LK_OK, "");

	/* fail: tf b "" */
	exercise_get_boolean(true,
		"tf b \"\"", (lkonf_keys){"tf", "b", "", 0},
		LK_OUT_OF_RANGE, "Not a table: b");

	/* fail: t5i function not returning boolean */
	exercise_get_boolean(false,
		"t5i", (lkonf_keys){"t5i", 0},
		LK_OUT_OF_RANGE, "Not a boolean: t5i");

	/* fail: tf i function not returning boolean */
	exercise_get_boolean(false,
		"tf i", (lkonf_keys){"tf", "i", 0},
		LK_OUT_OF_RANGE, "Not a boolean: i");

	/* fail: t6 "" k2 - missing key k2 */
	exercise_get_boolean(true,
		"t6 \"\" k2", (lkonf_keys){"t6", "", "k2", 0},
		LK_NOT_FOUND, "");

	/* pass: t6 "." b */
	exercise_get_boolean(false,
		"t6 \".\" b", (lkonf_keys){"t6", ".", "b", 0},
		LK_OK, "");

	/* fail: "" empty key */
	exercise_get_boolean(true,
		"", (lkonf_keys){"", 0},
		LK_OUT_OF_RANGE, "Empty top-level key");

	/* fail: "." */
	exercise_get_boolean(true,
		".", (lkonf_keys){".", 0},
		LK_NOT_FOUND, "");

	/* pass: b */
	exercise_get_boolean(true,
		"b", (lkonf_keys){"b", 0},
		LK_OK, "");

	/* pass: loooooooooooooooooooooooooooong x yb */
	exercise_get_boolean(true,
		"loooooooooooooooooooooooooooong x yb",
		(lkonf_keys){"loooooooooooooooooooooooooooong", "x", "yb", 0},
		LK_OK, "");

	/* fail: t7 "" - not a bool */
	exercise_get_boolean(true,
		"t7 \"\"", (lkonf_keys){"t7", "", 0},
		LK_OUT_OF_RANGE, "Not a boolean: ");

	/* fail: "" t8 */
	exercise_get_boolean(true,
		"\"\" t8", (lkonf_keys){"", "t8", 0},
		LK_OUT_OF_RANGE, "Empty top-level key");

	/* fail: t9n 1 */
/* TODO: fix path lookup to support integer lookup? */
	exercise_get_boolean(true,
		"t9n 1", (lkonf_keys){"t9n", "1", 0},
		LK_NOT_FOUND, "");

	/* pass: t9s 1 */
	exercise_get_boolean(true,
		"t9s 1", (lkonf_keys){"t9s", "1", 0},
		LK_OK, "");


	/* fail: t */
	exercise_get_boolean(false,
		"t", (lkonf_keys){"t", 0},
		LK_OUT_OF_RANGE, "Not a boolean: t");

	/* fail: t "" */
	exercise_get_boolean(true,
		"t \"\"", (lkonf_keys){"t", "", 0},
		LK_NOT_FOUND, "");

	/* fail: t k nil VALUE */
	exercise_get_boolean(true,
		"t.k", (lkonf_keys){"t", "k", 0},
		LK_NOT_FOUND, "");

	/* fail: toolong takes too long */
	exercise_get_boolean(true,
		"toolong", (lkonf_keys){"toolong", 0},
		LK_LUA_ERROR, "Instruction count exceeded");

	/* fail: badrun calls unknown symbol */
	exercise_get_boolean(false,
		"badrun", (lkonf_keys){"badrun", 0},
		LK_LUA_ERROR, badrun_error);

	/* pass: jrb */
	exercise_get_boolean(true,
		"jrb", (lkonf_keys){"jrb", 0},
		LK_OK, "");

	/* fail: hidden */
	exercise_get_boolean(true,
		"hidden", (lkonf_keys){"hidden", 0},
		LK_NOT_FOUND, "");

	return EXIT_SUCCESS;
}


void
exercise_get_double(
	const double		wanted,
	const char *		path,
	lkonf_keys		keys,
	const lkonf_error	expect_code,
	const char *		expect_str)
{
	char desc[128];
	snprintf(desc, sizeof(desc), "%s_double('%s')",
		keys ? "getkey" : "get", path);
	printf("%s = %g", desc, wanted);
	if (LK_OK != expect_code) {
		printf("; expect code %d [%s] '%s'",
			expect_code, lkonf_error_to_string(expect_code),
			expect_str);
	}
	printf("\n");

	lkonf_context * lc = lkonf_construct();
	assert(lc && "lkonf_construct returned 0");

	const lkonf_error rls = lkonf_load_string(lc, test_luastr);
	ensure_result(lc, rls, "load_string", LK_OK, "");

		/* limit to 100 instructions; after load */
	const lkonf_error sil = lkonf_set_instruction_limit(lc, 100);
	ensure_result(lc, sil, "set_instruction_limit(lc, 100)", LK_OK, "");

	double v = -wanted;
	lkonf_error res = LK_INVALID_ARGUMENT;
	if (keys) {
		res = lkonf_getkey_double(lc, keys, &v);
	} else {
		res = lkonf_get_double(lc, path, &v);
	}
	ensure_result(lc, res, desc, expect_code, expect_str);
	if (LK_OK == expect_code) {
		if (wanted != v) {
			printf("wanted %g != v %g", wanted, v);
		}
		assert(wanted == v);
	} else {
		assert(-wanted == v);	/* didn't change */
	}

	lkonf_destruct(lc);
}

int
test_get_double(void)
{
	printf("lkonf_get_double()\n");

	/* fail: load null lkonf_context */
	{
		assert(LK_INVALID_ARGUMENT == lkonf_get_double(0, 0, 0));
	}

	/* fail: null oValue */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkonf_error res = lkonf_get_double(lc, "", 0);
		ensure_result(lc, res,
			"get_double(lc, \"\", 0)",
			LK_INVALID_ARGUMENT, "oValue NULL");

		lkonf_destruct(lc);
	}

	/* fail: null path */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		double v = -1;
		const lkonf_error res = lkonf_get_double(lc, 0, &v);
		ensure_result(lc, res,
			"get_double(lc, 0, &v)",
			LK_INVALID_ARGUMENT, "iPath NULL");

		lkonf_destruct(lc);
	}

	/* pass: d1 */
	exercise_get_double(1.01,
		"d1", NULL,
		LK_OK, "");

	/* fail: top-level key 'missing' not set */
	exercise_get_double(5,
		"missing", NULL,
		LK_NOT_FOUND, "");

	/* pass: t2.d */
	exercise_get_double(2.714,
		"t2.d", NULL,
		LK_OK, "");

	/* pass: t3.t.d3 */
	exercise_get_double(3.1415,
		"t3.t.d3", NULL,
		LK_OK, "");

	/* pass: t3.t.absent not set */
	exercise_get_double(5,
		"t3.t.absent", NULL,
		LK_NOT_FOUND, "");

	/* fail: t3.t. */
	exercise_get_double(0,
		"t3.t.", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t3.t.");

	/* fail: t3.t.d3.k4 */
	exercise_get_double(33,
		"t3.t.d3.k4", NULL,
		LK_OUT_OF_RANGE, "Not a table: t3.t.d3");

	/* fail: t3.t.b3 (not a double)  */
	exercise_get_double(0,
		"t3.t.b3", NULL,
		LK_OUT_OF_RANGE, "Not a double: t3.t.b3");

	/* fail: t3.k.d3 */
	exercise_get_double(0,
		"t3.k.d3", NULL,
		LK_OUT_OF_RANGE, "Not a table: t3.k");

	/* fail: t3.k.k2 */
	exercise_get_double(0,
		"t3.k.k2", NULL,
		LK_OUT_OF_RANGE, "Not a table: t3.k");

	/* fail: t3.12345.3 */
	exercise_get_double(0,
		"t3.12345.3", NULL,
		LK_OUT_OF_RANGE, "Not a table: t3.12345");

	/* pass: tf.d function returning double */
	exercise_get_double(-4.01,
		"tf.d", NULL,
		LK_OK, "");

	/* fail: tf.d. (trailing .) */
	exercise_get_double(4,
		"tf.d.", NULL,
		LK_OUT_OF_RANGE, "Not a table: tf.d");

	/* fail: t5b function not returning double */
	exercise_get_double(0,
		"t5b", NULL,
		LK_OUT_OF_RANGE, "Not a double: t5b");

	/* fail: tf.s function not returning double */
	exercise_get_double(0,
		"tf.s", NULL,
		LK_OUT_OF_RANGE, "Not a double: tf.s");

	/* fail: t6..k2 - empty key */
	exercise_get_double(6,
		"t6..k2", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t6..k2");

	/* fail: t6 "." d - not a table "." */
	exercise_get_double(0,
		"t6...d", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t6...d");

	/* fail: "" */
	exercise_get_double(-5,
		"", NULL,
		LK_OUT_OF_RANGE, "Empty path");

	/* fail: "." */
	exercise_get_double(-5,
		".", NULL,
		LK_OUT_OF_RANGE, "Empty component in: .");

	/* pass: d */
	exercise_get_double(0.5,
		"d", NULL,
		LK_OK, "");

	/* pass: loooooooooooooooooooooooooooong.x.yd */
	exercise_get_double(99.999,
		"loooooooooooooooooooooooooooong.x.yd", NULL,
		LK_OK, "");

	/* fail: t7. */
	exercise_get_double(7,
		"t7.", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t7.");

	/* fail: .t8 */
	exercise_get_double(8,
		".t8", NULL,
		LK_OUT_OF_RANGE, "Empty component in: .t8");

	/* fail: t9n.2 */
/* TODO: fix path lookup to support integer lookup? */
	exercise_get_double(6.1,
		"t9n.2", NULL,
		LK_NOT_FOUND, "");

	/* pass: t9s.2 */
	exercise_get_double(6.1,
		"t9s.2", NULL,
		LK_OK, "");

	/* fail: t */
	exercise_get_double(0,
		"t", NULL,
		LK_OUT_OF_RANGE, "Not a double: t");

	/* fail: t. */
	exercise_get_double(0,
		"t.", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t.");

	/* pass: t.k nil VALUE */
	exercise_get_double(999,
		"t.k", NULL,
		LK_NOT_FOUND, "");

	/* fail: toolong takes too long */
	exercise_get_double(0,
		"toolong", NULL,
		LK_LUA_ERROR, "Instruction count exceeded");

	/* fail: badrun calls unknown symbol */
	exercise_get_double(-1,
		"badrun", NULL,
		LK_LUA_ERROR, badrun_error);

	/* pass: jrd */
	exercise_get_double(4.9,
		"jrd", NULL,
		LK_OK, "");

	/* fail: hidden */
	exercise_get_double(0,
		"hidden", NULL,
		LK_NOT_FOUND, "");

	return EXIT_SUCCESS;
}

int
test_getkey_double(void)
{
	printf("lkonf_getkey_double()\n");

	/* fail: load null lkonf_context */
	{
		assert(LK_INVALID_ARGUMENT == lkonf_getkey_double(0, 0, 0));
	}

	/* fail: null oValue */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkonf_error res = lkonf_getkey_double(lc,
			(lkonf_keys){ 0 }, 0);
		ensure_result(lc, res,
			"getkey_double(lc, {0}, 0)",
			LK_INVALID_ARGUMENT, "oValue NULL");

		lkonf_destruct(lc);
	}

	/* fail: null iKeys */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		double v = -1;
		const lkonf_error res = lkonf_getkey_double(lc, 0, &v);
		ensure_result(lc, res,
			"getkey_double(lc, 0, &v)",
			LK_INVALID_ARGUMENT, "iKeys NULL");

		lkonf_destruct(lc);
	}

	/* fail: empty iKeys */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		double v = 1.1;
		const lkonf_error res = lkonf_getkey_double(lc,
			(lkonf_keys){ 0 }, &v);
		ensure_result(lc, res, "getkey_double(lc, {0}, &v)",
			LK_OUT_OF_RANGE, "Empty keys");

		lkonf_destruct(lc);
	}

	/* pass: d1 */
	exercise_get_double(1.01,
		"d1", (lkonf_keys){ "d1", 0 },
		LK_OK, "");

	/* fail: top-level key 'missing' not set */
	exercise_get_double(5,
		"missing", (lkonf_keys){ "missing", 0 },
		LK_NOT_FOUND, "");

	/* pass: t2 d */
	exercise_get_double(2.714,
		"t2 d", (lkonf_keys){"t2", "d", 0},
		LK_OK, "");

	/* pass: t3 t d3 */
	exercise_get_double(3.1415,
		"t3 t d3", (lkonf_keys){"t3", "t", "d3", 0},
		LK_OK, "");

	/* fail: t3 t absent not set */
	exercise_get_double(5,
		"t3 t absent", (lkonf_keys){"t3", "t", "absent", 0},
		LK_NOT_FOUND, "");

	/* fail: t3 t "" not set */
	exercise_get_double(0,
		"t3 t \"\"", (lkonf_keys){"t3", "t", "", 0},
		LK_NOT_FOUND, "");

	/* fail: t3 t d3 k4 */
	exercise_get_double(33,
		"t3 t d3 k4", (lkonf_keys){"t3", "t", "d3", "k4", 0},
		LK_OUT_OF_RANGE, "Not a table: d3");

	/* fail: t3 t b3 (not a double)  */
	exercise_get_double(0,
		"t3 t b3", (lkonf_keys){"t3", "t", "b3", 0},
		LK_OUT_OF_RANGE, "Not a double: b3");

	/* fail: t3 k d3 */
	exercise_get_double(0,
		"t3 k d3", (lkonf_keys){"t3", "k", "d3", 0},
		LK_OUT_OF_RANGE, "Not a table: k");

	/* fail: t3 k k2 */
	exercise_get_double(0,
		"t3 k k2", (lkonf_keys){"t3", "k", "k2", 0},
		LK_OUT_OF_RANGE, "Not a table: k");

	/* fail: t3 12345 3 */
	exercise_get_double(0,
		"t3 12345 3", (lkonf_keys){"t3", "12345", "3", 0},
		LK_OUT_OF_RANGE, "Not a table: 12345");

	/* pass: tf d function returning double */
	exercise_get_double(-4.01,
		"tf d", (lkonf_keys){"tf", "d", 0},
		LK_OK, "");

	/* fail: tf d "" */
	exercise_get_double(4,
		"tf d \"\"", (lkonf_keys){"tf", "d", "", 0},
		LK_OUT_OF_RANGE, "Not a table: d");

	/* fail: t5b function not returning double */
	exercise_get_double(0,
		"t5b", (lkonf_keys){"t5b", 0},
		LK_OUT_OF_RANGE, "Not a double: t5b");

	/* fail: tf s function not returning double */
	exercise_get_double(0,
		"tf s", (lkonf_keys){"tf", "s", 0},
		LK_OUT_OF_RANGE, "Not a double: s");

	/* fail: t6 "" k2 - missing key k2 */
	exercise_get_double(6,
		"t6 \"\" k2", (lkonf_keys){"t6", "", "k2", 0},
		LK_NOT_FOUND, "");

	/* pass: t6 "." d */
	exercise_get_double(-6.001,
		"t6 \".\" d", (lkonf_keys){"t6", ".", "d", 0},
		LK_OK, "");

	/* fail: "" empty key */
	exercise_get_double(-5,
		"", (lkonf_keys){"", 0},
		LK_OUT_OF_RANGE, "Empty top-level key");

	/* fail: "." absent */
	exercise_get_double(-5,
		".", (lkonf_keys){".", 0},
		LK_NOT_FOUND, "");

	/* pass: d */
	exercise_get_double(0.5,
		"d", (lkonf_keys){"d", 0},
		LK_OK, "");

	/* pass: loooooooooooooooooooooooooooong x yd */
	exercise_get_double(99.999,
		"loooooooooooooooooooooooooooong x yd",
		(lkonf_keys){"loooooooooooooooooooooooooooong", "x", "yd", 0},
		LK_OK, "");

	/* pass: t7 "" */
	exercise_get_double(777,
		"t7 \"\"", (lkonf_keys){"t7", "", 0},
		LK_OK, "");

	/* fail: "" t8 */
	exercise_get_double(8,
		"\"\" t8", (lkonf_keys){"", "t8", 0},
		LK_OUT_OF_RANGE, "Empty top-level key");

	/* fail: t9n 2 */
/* TODO: fix path lookup to support integer lookup? */
	exercise_get_double(6.1,
		"t9n 2", (lkonf_keys){"t9n", "2", 0},
		LK_NOT_FOUND, "");

	/* pass: t9s.2 */
	exercise_get_double(6.1,
		"t9s 2", (lkonf_keys){"t9s", "2", 0},
		LK_OK, "");

	/* fail: t */
	exercise_get_double(0,
		"t", (lkonf_keys){"t", 0},
		LK_OUT_OF_RANGE, "Not a double: t");

	/* fail: t "" */
	exercise_get_double(0,
		"t \"\"", (lkonf_keys){"t", "", 0},
		LK_NOT_FOUND, "");

	/* fail: t.k nil VALUE */
	exercise_get_double(0,
		"t k", (lkonf_keys){"t", "k", 0},
		LK_NOT_FOUND, "");

	/* fail: toolong takes too long */
	exercise_get_double(0,
		"toolong", (lkonf_keys){"toolong", 0},
		LK_LUA_ERROR, "Instruction count exceeded");

	/* fail: badrun calls unknown symbol */
	exercise_get_double(-1,
		"badrun", (lkonf_keys){"badrun", 0},
		LK_LUA_ERROR, badrun_error);

	/* pass: jrd */
	exercise_get_double(4.9,
		"jrd", (lkonf_keys){"jrd", 0},
		LK_OK, "");

	/* fail: hidden */
	exercise_get_double(0,
		"hidden", (lkonf_keys){"hidden", 0},
		LK_NOT_FOUND, "");

	return EXIT_SUCCESS;
}


void
exercise_get_integer(
	const lua_Integer	wanted,
	const char *		path,
	lkonf_keys		keys,
	const lkonf_error	expect_code,
	const char *		expect_str)
{
	char desc[128];
	snprintf(desc, sizeof(desc), "%s_integer('%s')",
		keys ? "getkey" : "get", path);
	printf("%s = %" PRId64, desc, (int64_t)wanted);
	if (LK_OK != expect_code) {
		printf("; expect code %d [%s] '%s'",
			expect_code, lkonf_error_to_string(expect_code),
			expect_str);
	}
	printf("\n");

	lkonf_context * lc = lkonf_construct();
	assert(lc && "lkonf_construct returned 0");

	const lkonf_error rls = lkonf_load_string(lc, test_luastr);
	ensure_result(lc, rls, "load_string", LK_OK, "");

		/* limit to 100 instructions; after load */
	const lkonf_error sil = lkonf_set_instruction_limit(lc, 100);
	ensure_result(lc, sil, "set_instruction_limit(lc, 100)", LK_OK, "");

	lua_Integer v = -wanted;
	lkonf_error res = LK_INVALID_ARGUMENT;
	if (keys) {
		res = lkonf_getkey_integer(lc, keys, &v);
	} else {
		res = lkonf_get_integer(lc, path, &v);
	}
	ensure_result(lc, res, desc, expect_code, expect_str);
	if (LK_OK == expect_code) {
		if (wanted != v) {
			printf("wanted %" PRId64 " != v %" PRId64 "\n",
				(int64_t)wanted, (int64_t)v);
		}
		assert(wanted == v);
	} else {
		assert(-wanted == v);	/* didn't change */
	}

	lkonf_destruct(lc);
}

int
test_get_integer(void)
{
	printf("lkonf_get_integer()\n");

	/* fail: load null lkonf_context */
	{
		assert(LK_INVALID_ARGUMENT == lkonf_get_integer(0, 0, 0));
	}

	/* fail: null oValue */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkonf_error res = lkonf_get_integer(lc, "", 0);
		ensure_result(lc, res,
			"get_integer(lc, \"\", 0)",
			LK_INVALID_ARGUMENT, "oValue NULL");

		lkonf_destruct(lc);
	}

	/* fail: null path */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		lua_Integer v = -1;
		const lkonf_error res = lkonf_get_integer(lc, 0, &v);
		ensure_result(lc, res,
			"get_integer(lc, 0, &v)",
			LK_INVALID_ARGUMENT, "iPath NULL");

		lkonf_destruct(lc);
	}

	/* pass: i1 */
	exercise_get_integer(1,
		"i1", NULL,
		LK_OK, "");

	/* fail: top-level key 'missing' not set */
	exercise_get_integer(5,
		"missing", NULL,
		LK_NOT_FOUND, "");

	/* pass: t2.i */
	exercise_get_integer(2,
		"t2.i", NULL,
		LK_OK, "");

	/* pass: t3.t.i3 */
	exercise_get_integer(33,
		"t3.t.i3", NULL,
		LK_OK, "");

	/* pass: t3.t.absent not set */
	exercise_get_integer(5,
		"t3.t.absent", NULL,
		LK_NOT_FOUND, "");

	/* fail: t3.t. */
	exercise_get_integer(0,
		"t3.t.", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t3.t.");

	/* fail: t3.t.i3.k4 */
	exercise_get_integer(33,
		"t3.t.i3.k4", NULL,
		LK_OUT_OF_RANGE, "Not a table: t3.t.i3");

	/* fail: t3.t.b3 (not an integer)  */
	exercise_get_integer(0,
		"t3.t.b3", NULL,
		LK_OUT_OF_RANGE, "Not an integer: t3.t.b3");

	/* fail: t3.k.i3 */
	exercise_get_integer(0,
		"t3.k.i3", NULL,
		LK_OUT_OF_RANGE, "Not a table: t3.k");

	/* fail: t3.k.k2 */
	exercise_get_integer(0,
		"t3.k.k2", NULL,
		LK_OUT_OF_RANGE, "Not a table: t3.k");

	/* fail: t3.12345.3 */
	exercise_get_integer(0,
		"t3.12345.3", NULL,
		LK_OUT_OF_RANGE, "Not a table: t3.12345");

	/* pass: tf.i function returning integer */
	exercise_get_integer(4,
		"tf.i", NULL,
		LK_OK, "");

	/* fail: tf.i. (trailing .) */
	exercise_get_integer(4,
		"tf.i.", NULL,
		LK_OUT_OF_RANGE, "Not a table: tf.i");

	/* fail: t5b function not returning integer */
	exercise_get_integer(0,
		"t5b", NULL,
		LK_OUT_OF_RANGE, "Not an integer: t5b");

	/* fail: tf.b function not returning integer */
	exercise_get_integer(2,
		"tf.b", NULL,
		LK_OUT_OF_RANGE, "Not an integer: tf.b");

	/* fail: t6..k2 - empty key */
	exercise_get_integer(6,
		"t6..k2", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t6..k2");

	/* fail: t6 "." d - not a table "." */
	exercise_get_integer(0,
		"t6...d", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t6...d");

	/* fail: "" */
	exercise_get_integer(-5,
		"", NULL,
		LK_OUT_OF_RANGE, "Empty path");

	/* fail: "." */
	exercise_get_integer(0,
		".", NULL,
		LK_OUT_OF_RANGE, "Empty component in: .");

	/* pass: i */
	exercise_get_integer(11,
		"i", NULL,
		LK_OK, "");

	/* pass: loooooooooooooooooooooooooooong.x.yi */
	exercise_get_integer(99,
		"loooooooooooooooooooooooooooong.x.yi", NULL,
		LK_OK, "");

	/* fail: t7. */
	exercise_get_integer(7,
		"t7.", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t7.");

	/* fail: .t8 */
	exercise_get_integer(8,
		".t8", NULL,
		LK_OUT_OF_RANGE, "Empty component in: .t8");

	/* fail: t9n.3 */
/* TODO: fix path lookup to support integer lookup? */
	exercise_get_integer(6,
		"t9n.3", NULL,
		LK_NOT_FOUND, "");

	/* pass: t9s.3 */
	exercise_get_integer(6,
		"t9s.3", NULL,
		LK_OK, "");

	/* fail: t */
	exercise_get_integer(0,
		"t", NULL,
		LK_OUT_OF_RANGE, "Not an integer: t");

	/* fail: t. */
	exercise_get_integer(0,
		"t.", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t.");

	/* pass: t.k nil VALUE */
	exercise_get_integer(999,
		"t.k", NULL,
		LK_NOT_FOUND, "");

	/* fail: toolong takes too long */
	exercise_get_integer(0,
		"toolong", NULL,
		LK_LUA_ERROR, "Instruction count exceeded");

	/* fail: badrun calls unknown symbol */
	exercise_get_integer(0,
		"badrun", NULL,
		LK_LUA_ERROR, badrun_error);

	/* pass: jri */
	exercise_get_integer(5,
		"jri", NULL,
		LK_OK, "");

	/* fail: hidden */
	exercise_get_integer(0,
		"hidden", NULL,
		LK_NOT_FOUND, "");

	return EXIT_SUCCESS;
}

int
test_getkey_integer(void)
{
	printf("lkonf_getkey_integer()\n");

	/* fail: load null lkonf_context */
	{
		assert(LK_INVALID_ARGUMENT == lkonf_getkey_integer(0, 0, 0));
	}

	/* fail: null oValue */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkonf_error res = lkonf_getkey_integer(lc,
			(lkonf_keys){ 0 }, 0);
		ensure_result(lc, res,
			"getkey_integer(lc, {0}, 0)",
			LK_INVALID_ARGUMENT, "oValue NULL");

		lkonf_destruct(lc);
	}

	/* fail: null iKeys */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		lua_Integer v = -1;
		const lkonf_error res = lkonf_getkey_integer(lc, 0, &v);
		ensure_result(lc, res,
			"getkey_integer(lc, 0, &v)",
			LK_INVALID_ARGUMENT, "iKeys NULL");

		lkonf_destruct(lc);
	}

	/* pass: i1 */
	exercise_get_integer(1,
		"i1", (lkonf_keys){"i1", 0},
		LK_OK, "");

	/* fail: top-level key 'missing' not set */
	exercise_get_integer(5,
		"missing", (lkonf_keys){"missing", 0},
		LK_NOT_FOUND, "");

	/* pass: t2 i */
	exercise_get_integer(2,
		"t2 i", (lkonf_keys){"t2", "i", 0},
		LK_OK, "");

	/* pass: t3 t i3 */
	exercise_get_integer(33,
		"t3 t i3", (lkonf_keys){"t3", "t", "i3", 0},
		LK_OK, "");

	/* fail: t3 t absent not set */
	exercise_get_integer(5,
		"t3 t absent", (lkonf_keys){"t3", "t", "absent", 0},
		LK_NOT_FOUND, "");

	/* fail: t3 t "" not set */
	exercise_get_integer(0,
		"t3 t \"\"", (lkonf_keys){"t3", "t", "", 0},
		LK_NOT_FOUND, "");

	/* fail: t3 t i3 k4 */
	exercise_get_integer(33,
		"t3 t i3 k4", (lkonf_keys){"t3", "t", "i3", "k4", 0},
		LK_OUT_OF_RANGE, "Not a table: i3");

	/* fail: t3 t b3 (not an integer)  */
	exercise_get_integer(0,
		"t3 t b3", (lkonf_keys){"t3", "t", "b3", 0},
		LK_OUT_OF_RANGE, "Not an integer: b3");

	/* fail: t3 k i3 */
	exercise_get_integer(0,
		"t3 k i3", (lkonf_keys){"t3", "k", "i3", 0},
		LK_OUT_OF_RANGE, "Not a table: k");

	/* fail: t3 k k2 */
	exercise_get_integer(0,
		"t3 k k2", (lkonf_keys){"t3", "k", "k2", 0},
		LK_OUT_OF_RANGE, "Not a table: k");

	/* fail: t3 12345 3 */
	exercise_get_integer(0,
		"t3 12345 3", (lkonf_keys){"t3", "12345", "3", 0},
		LK_OUT_OF_RANGE, "Not a table: 12345");

	/* pass: tf i function returning integer */
	exercise_get_integer(4,
		"tf i", (lkonf_keys){"tf", "i", 0},
		LK_OK, "");

	/* fail: tf i "" */
	exercise_get_integer(4,
		"tf i \"\"", (lkonf_keys){"tf", "i", "", 0},
		LK_OUT_OF_RANGE, "Not a table: i");

	/* fail: t5b function not returning integer */
	exercise_get_integer(0,
		"t5b", (lkonf_keys){"t5b", 0},
		LK_OUT_OF_RANGE, "Not an integer: t5b");

	/* fail: tf b function not returning integer */
	exercise_get_integer(2,
		"tf b", (lkonf_keys){"tf", "b", 0},
		LK_OUT_OF_RANGE, "Not an integer: b");

	/* fail: t6 "" k2 - missing key k2 */
	exercise_get_integer(6,
		"t6 \"\" k2", (lkonf_keys){"t6", "", "k2", 0},
		LK_NOT_FOUND, "");

	/* pass: t6 "." i */
	exercise_get_integer(-6,
		"t6 \".\" i", (lkonf_keys){"t6", ".", "i", 0},
		LK_OK, "");

	/* fail: "" empty key */
	exercise_get_integer(-5,
		"", (lkonf_keys){"", 0},
		LK_OUT_OF_RANGE, "Empty top-level key");

	/* fail: "." absent */
	exercise_get_integer(0,
		".", (lkonf_keys){".", 0},
		LK_NOT_FOUND, "");

	/* pass: i */
	exercise_get_integer(11,
		"i", (lkonf_keys){"i", 0},
		LK_OK, "");

	/* pass: loooooooooooooooooooooooooooong x yi */
	exercise_get_integer(99,
		"loooooooooooooooooooooooooooong x yi",
		(lkonf_keys){"loooooooooooooooooooooooooooong", "x", "yi", 0},
		LK_OK, "");

	/* pass: t7 "" */
	exercise_get_integer(777,
		"t7 \"\"", (lkonf_keys){"t7", "", 0},
		LK_OK, "");

	/* fail: "" t8 */
	exercise_get_integer(8,
		"\"\" t8", (lkonf_keys){"", "t8", 0},
		LK_OUT_OF_RANGE, "Empty top-level key");

	/* fail: t9n 3 */
/* TODO: fix path lookup to support integer lookup? */
	exercise_get_integer(6,
		"t9n 3", (lkonf_keys){"t9n", "3", 0},
		LK_NOT_FOUND, "");

	/* pass: t9s 3 */
	exercise_get_integer(6,
		"t9s 3", (lkonf_keys){"t9s", "3", 0},
		LK_OK, "");

	/* fail: t */
	exercise_get_integer(0,
		"t", (lkonf_keys){"t", 0},
		LK_OUT_OF_RANGE, "Not an integer: t");

	/* fail: t "" */
	exercise_get_integer(0,
		"t \"\"", (lkonf_keys){"t", "", 0},
		LK_NOT_FOUND, "");

	/* pass: t.k nil VALUE */
	exercise_get_integer(999,
		"t k", (lkonf_keys){"t", "k", 0},
		LK_NOT_FOUND, "");

	/* fail: toolong takes too long */
	exercise_get_integer(0,
		"toolong", (lkonf_keys){"toolong", 0},
		LK_LUA_ERROR, "Instruction count exceeded");

	/* fail: badrun calls unknown symbol */
	exercise_get_integer(0,
		"badrun", (lkonf_keys){"badrun", 0},
		LK_LUA_ERROR, badrun_error);

	/* pass: jri */
	exercise_get_integer(5,
		"jri", (lkonf_keys){"jri", 0},
		LK_OK, "");

	/* fail: hidden */
	exercise_get_integer(0,
		"hidden", (lkonf_keys){"hidden", 0},
		LK_NOT_FOUND, "");

	return EXIT_SUCCESS;
}


void
exercise_get_string(
	const char *		wantstr,
	const size_t		wantlen,
	const char *		path,
	lkonf_keys		keys,
	const lkonf_error	expect_code,
	const char *		expect_str)
{
	char desc[128];
	snprintf(desc, sizeof(desc), "%s_string('%s')",
		keys ? "getkey" : "get", path);
	printf("%s = '%s'/%zu", desc, wantstr, wantlen);
	if (LK_OK != expect_code) {
		printf("; expect code %d [%s] '%s'",
			expect_code, lkonf_error_to_string(expect_code),
			expect_str);
	}
	printf("\n");

	lkonf_context * lc = lkonf_construct();
	assert(lc && "lkonf_construct returned 0");

	const lkonf_error rls = lkonf_load_string(lc, test_luastr);
	ensure_result(lc, rls, "load_string", LK_OK, "");

		/* limit to 100 instructions; after load */
	const lkonf_error sil = lkonf_set_instruction_limit(lc, 100);
	ensure_result(lc, sil, "set_instruction_limit(lc, 100)", LK_OK, "");

	char * gotstr = 0;
	size_t gotlen = -wantlen;
	lkonf_error res = LK_INVALID_ARGUMENT;
	if (keys) {
		res = lkonf_getkey_string(lc, keys, &gotstr, &gotlen);
	} else {
		res = lkonf_get_string(lc, path, &gotstr, &gotlen);
	}
	ensure_result(lc, res, desc, expect_code, expect_str);
	if (LK_OK == expect_code) {
		if (wantlen != gotlen) {
			printf("wantlen %zu != gotlen %zu\n", wantlen, gotlen);
		}
		if (!streq(wantstr, gotstr)) {
			printf("wantstr '%s' != gotstr '%s'\n",
				wantstr, gotstr);
		}
		assert(wantlen == gotlen);
		assert(streq(wantstr, gotstr));
		free(gotstr);
		gotstr = 0;
	} else {
		assert(-wantlen == gotlen);	/* didn't change */
	}

	lkonf_destruct(lc);
}

int
test_get_string(void)
{
	printf("lkonf_get_string()\n");

	/* fail: load null lkonf_context */
	{
		assert(LK_INVALID_ARGUMENT == lkonf_get_string(0, 0, 0, 0));
	}

	/* fail: null oValue */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkonf_error res = lkonf_get_string(lc, "", 0, 0);
		ensure_result(lc, res,
			"get_string(lc, \"\", 0, 0)",
			LK_INVALID_ARGUMENT, "oValue NULL");

		lkonf_destruct(lc);
	}

	/* fail: null path */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		char * v = 0;
		const lkonf_error res = lkonf_get_string(lc, 0, &v, 0);
		ensure_result(lc, res,
			"get_string(lc, 0, &v)",
			LK_INVALID_ARGUMENT, "iPath NULL");

		lkonf_destruct(lc);
	}

	/* pass: s1 */
	exercise_get_string("1", 1,
		"s1", NULL,
		LK_OK, "");

	/* fail: top-level key 'missing' not set */
	exercise_get_string("", 0,
		"missing", NULL,
		LK_NOT_FOUND, "");

	/* pass: t2.empty */
	exercise_get_string("", 0,
		"t2.empty", NULL,
		LK_OK, "");

	/* pass: t2.2 */
	exercise_get_string("two", 3,
		"t2.2", NULL,
		LK_OK, "");

	/* pass: t3.t.s3 */
	exercise_get_string("thirty three", 12,
		"t3.t.s3", NULL,
		LK_OK, "");

	/* pass: t3.t.absent not set */
	exercise_get_string("", 0,
		"t3.t.absent", NULL,
		LK_NOT_FOUND, "");

	/* fail: t3.t. */
	exercise_get_string("", 0,
		"t3.t.", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t3.t.");

	/* fail: t3.t.s3.k4 */
	exercise_get_string("", 0,
		"t3.t.s3.k4", NULL,
		LK_OUT_OF_RANGE, "Not a table: t3.t.s3");

	/* fail: t3.t.b3 (not a string)  */
	exercise_get_string("", 0,
		"t3.t.b3", NULL,
		LK_OUT_OF_RANGE, "Not a string: t3.t.b3");

	/* fail: t3.k.s3 */
	exercise_get_string("", 0,
		"t3.k.s3", NULL,
		LK_OUT_OF_RANGE, "Not a table: t3.k");

	/* fail: t3.12345.3 */
	exercise_get_string("", 0,
		"t3.12345.3", NULL,
		LK_OUT_OF_RANGE, "Not a table: t3.12345");

	/* pass: tf.s function returning string */
	exercise_get_string("tf path=tf.s", 12,
		"tf.s", NULL,
		LK_OK, "");

	/* fail: tf.s. (trailing .) */
	exercise_get_string("", 4,
		"tf.s.", NULL,
		LK_OUT_OF_RANGE, "Not a table: tf.s");

	/* fail: t5b function not returning string */
	exercise_get_string("", 0,
		"t5b", NULL,
		LK_OUT_OF_RANGE, "Not a string: t5b");

	/* fail: tf.i function not returning string */
	exercise_get_string("", 0,
		"tf.i", NULL,
		LK_OUT_OF_RANGE, "Not a string: tf.i");

	/* fail: t6..k2 - empty key */
	exercise_get_string("", 6,
		"t6..k2", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t6..k2");

	/* fail: t6 "." d - not a table "." */
	exercise_get_string("", 0,
		"t6...d", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t6...d");

	/* fail: "" */
	exercise_get_string("", 0,
		"", NULL,
		LK_OUT_OF_RANGE, "Empty path");

	/* fail: "." */
	exercise_get_string("", 0,
		".", NULL,
		LK_OUT_OF_RANGE, "Empty component in: .");

	/* pass: s */
	exercise_get_string("sooooper", 8,
		"s", NULL,
		LK_OK, "");

	/* pass: loooooooooooooooooooooooooooong.x.ys */
	exercise_get_string("yus", 3,
		"loooooooooooooooooooooooooooong.x.ys", NULL,
		LK_OK, "");

	/* fail: t7. */
	exercise_get_string("", 0,
		"t7.", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t7.");

	/* fail: .t8 */
	exercise_get_string("", 0,
		".t8", NULL,
		LK_OUT_OF_RANGE, "Empty component in: .t8");

	/* fail: t9n.4 */
/* TODO: fix path lookup to support integer lookup? */
	exercise_get_string("six", 3,
		"t9n.4", NULL,
		LK_NOT_FOUND, "");

	/* pass: t9s.4 */
	exercise_get_string("six", 3,
		"t9s.4", NULL,
		LK_OK, "");

	/* fail: t */
	exercise_get_string("", 0,
		"t", NULL,
		LK_OUT_OF_RANGE, "Not a string: t");

	/* fail: t. */
	exercise_get_string("", 0,
		"t.", NULL,
		LK_OUT_OF_RANGE, "Empty component in: t.");

	/* pass: t.k nil VALUE */
	exercise_get_string("", 0,
		"t.k", NULL,
		LK_NOT_FOUND, "");

	/* fail: toolong takes too long */
	exercise_get_string("", 0,
		"toolong", NULL,
		LK_LUA_ERROR, "Instruction count exceeded");

	/* fail: badrun calls unknown symbol */
	exercise_get_string("", 0,
		"badrun", NULL,
		LK_LUA_ERROR, badrun_error);

	/* pass: jrs */
	exercise_get_string("just right!", 11,
		"jrs", NULL,
		LK_OK, "");

	/* fail: hidden */
	exercise_get_string("", 0,
		"hidden", NULL,
		LK_NOT_FOUND, "");

	return EXIT_SUCCESS;
}

int
test_getkey_string(void)
{
	printf("lkonf_getkey_string()\n");

	/* fail: load null lkonf_context */
	{
		assert(LK_INVALID_ARGUMENT == lkonf_getkey_string(0, 0, 0, 0));
	}

	/* fail: null oValue */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkonf_error res = lkonf_getkey_string(lc,
			(lkonf_keys){ 0 }, 0, 0);
		ensure_result(lc, res,
			"getkey_string(lc, {0}, 0, 0)",
			LK_INVALID_ARGUMENT, "oValue NULL");

		lkonf_destruct(lc);
	}

	/* fail: null iKeys */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		char * v = 0;
		const lkonf_error res = lkonf_getkey_string(lc, 0, &v, 0);
		ensure_result(lc, res,
			"getkey_string(lc, 0, &v)",
			LK_INVALID_ARGUMENT, "iKeys NULL");

		lkonf_destruct(lc);
	}

	/* pass: s1 */
	exercise_get_string("1", 1,
		"s1", (lkonf_keys){"s1", 0},
		LK_OK, "");

	/* fail: top-level key 'missing' not set */
	exercise_get_string("", 0,
		"missing", (lkonf_keys){"missing", 0},
		LK_NOT_FOUND, "");

	/* pass: t2 empty */
	exercise_get_string("", 0,
		"t2 empty", (lkonf_keys){"t2", "empty", 0},
		LK_OK, "");

	/* pass: t2 2 */
	exercise_get_string("two", 3,
		"t2 2", (lkonf_keys){"t2", "2", 0},
		LK_OK, "");

	/* pass: t3 t s3 */
	exercise_get_string("thirty three", 12,
		"t3 t s3", (lkonf_keys){"t3", "t", "s3", 0},
		LK_OK, "");

	/* pass: t3 t absent not set */
	exercise_get_string("", 0,
		"t3 t absent", (lkonf_keys){"t3", "t", "absent", 0},
		LK_NOT_FOUND, "");

	/* fail: t3 t "" */
	exercise_get_string("", 0,
		"t3 t \"\"", (lkonf_keys){"t3", "t", "", 0},
		LK_NOT_FOUND, "");

	/* fail: t3 t s3 k4 */
	exercise_get_string("", 0,
		"t3 t s3 k4", (lkonf_keys){"t3", "t", "s3", "k4", 0},
		LK_OUT_OF_RANGE, "Not a table: s3");

	/* fail: t3 t b3 (not a string)  */
	exercise_get_string("", 0,
		"t3 t b3", (lkonf_keys){"t3", "t", "b3", 0},
		LK_OUT_OF_RANGE, "Not a string: b3");

	/* fail: t3 k s3 */
	exercise_get_string("", 0,
		"t3 k 3", (lkonf_keys){"t3", "k", "3", 0},
		LK_OUT_OF_RANGE, "Not a table: k");

	/* fail: t3 h2345 3 */
	exercise_get_string("", 0,
		"t3 12345 3", (lkonf_keys){"t3", "12345", "3", 0},
		LK_OUT_OF_RANGE, "Not a table: 12345");

	/* pass: tf.s function returning string */
	exercise_get_string("tf path=s", 9,
		"tf s", (lkonf_keys){"tf", "s", 0},
		LK_OK, "");

	/* fail: tf.s. (trailing .) */
	exercise_get_string("", 4,
		"tf s \"\"", (lkonf_keys){"tf", "s", "", 0},
		LK_OUT_OF_RANGE, "Not a table: s");

	/* fail: t5b function not returning string */
	exercise_get_string("", 0,
		"t5b", (lkonf_keys){"t5b", 0},
		LK_OUT_OF_RANGE, "Not a string: t5b");

	/* fail: tf.i function not returning string */
	exercise_get_string("", 0,
		"tf i", (lkonf_keys){"tf", "i", 0},
		LK_OUT_OF_RANGE, "Not a string: i");

	/* fail: t6 "" k2 - missing key k2 */
	exercise_get_string("", 6,
		"t6 \"\" k2", (lkonf_keys){"t6", "", "k2", 0},
		LK_NOT_FOUND, "");

	/* pass: t6 "." s */
	exercise_get_string("dot", 3,
		"t6 \".\" s", (lkonf_keys){"t6", ".", "s", 0},
		LK_OK, "");

	/* fail: "" */
	exercise_get_string("", 0,
		"", (lkonf_keys){"", 0},
		LK_OUT_OF_RANGE, "Empty top-level key");

	/* fail: "." absent */
	exercise_get_string("", 0,
		".", (lkonf_keys){".", 0},
		LK_NOT_FOUND, "");

	/* pass: s */
	exercise_get_string("sooooper", 8,
		"s", (lkonf_keys){"s", 0},
		LK_OK, "");

	/* pass: loooooooooooooooooooooooooooong x ys */
	exercise_get_string("yus", 3,
		"loooooooooooooooooooooooooooong x ys",
		(lkonf_keys){"loooooooooooooooooooooooooooong", "x", "ys", 0},
		LK_OK, "");

	/* pass: t7 "" */
	exercise_get_string("", 0,
		"t7 \"\"", (lkonf_keys){"t7", "", 0},
		LK_OK, "");

	/* fail: "" t8 */
	exercise_get_string("", 0,
		"\"\" t8", (lkonf_keys){"", "t8", 0},
		LK_OUT_OF_RANGE, "Empty top-level key");

	/* fail: t9n 4 */
/* TODO: fix path lookup to support integer lookup? */
	exercise_get_string("six", 3,
		"t9n.4", (lkonf_keys){"t9n", "4", 0},
		LK_NOT_FOUND, "");

	/* pass: t9s 4 */
	exercise_get_string("six", 3,
		"t9s.4", (lkonf_keys){"t9s", "4", 0},
		LK_OK, "");

	/* fail: t */
	exercise_get_string("", 0,
		"t", (lkonf_keys){"t", 0},
		LK_OUT_OF_RANGE, "Not a string: t");

	/* fail: t "" */
	exercise_get_string("", 0,
		"t \"\"", (lkonf_keys){"t", "", 0},
		LK_OUT_OF_RANGE, "Empty component in: t.");

	/* pass: t.k nil VALUE */
	exercise_get_string("", 0,
		"t k", (lkonf_keys){"t", "k", 0},
		LK_NOT_FOUND, "");

	/* fail: toolong takes too long */
	exercise_get_string("", 0,
		"toolong", (lkonf_keys){"toolong", 0},
		LK_LUA_ERROR, "Instruction count exceeded");

	/* fail: badrun calls unknown symbol */
	exercise_get_string("", 0,
		"badrun", (lkonf_keys){"badrun", 0},
		LK_LUA_ERROR, badrun_error);

	/* pass: jrs */
	exercise_get_string("just right!", 11,
		"jrs", (lkonf_keys){"jrs", 0},
		LK_OK, "");

	/* fail: hidden */
	exercise_get_string("", 0,
		"hidden", (lkonf_keys){"hidden", 0},
		LK_NOT_FOUND, "");

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
	{ "construct",		TF_construct,		test_construct },
	{ "destruct",		TF_destruct,		test_destruct },
	{ "load_file",		TF_load_file,		test_load_file },
	{ "load_string",	TF_load_string,		test_load_string },
	{ "instruction_limit",	TF_instruction_limit,	test_instruction_limit },
	{ "get_boolean",	TF_get_boolean,		test_get_boolean },
	{ "getkey_boolean",	TF_getkey_boolean,	test_getkey_boolean },
	{ "get_double",		TF_get_double,		test_get_double },
	{ "getkey_double",	TF_getkey_double,	test_getkey_double },
	{ "get_integer",	TF_get_integer,		test_get_integer },
	{ "getkey_integer",	TF_getkey_integer,	test_getkey_integer },
	{ "get_string",		TF_get_string,		test_get_string },
	{ "getkey_string",	TF_getkey_string,	test_getkey_string },
	{ 0,			0,			0 },
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
