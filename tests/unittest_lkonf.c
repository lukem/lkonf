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
b = true d = 0.5 i = 11 \
t = {} \
loooooooooooooooooooooooooooong = { x = { yb = true, yd = 99.999, yi=99 }} \
jrb = function (x) local f for i=1, 5 do f=i end return true end \
jrd = function (x) local f for i=1, 5 do f=i end return f-0.1 end \
jri = function (x) local f for i=1, 5 do f=i end return f end \
jrs = function (x) local f for i=1, 5 do f=i end return \"just right!\" end \
toolong = function (x) for f=1,1000 do end end \
badrun = function (x) print() end \
local hidden = 1 \
";


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
		case LK_VALUE_NIL:	return "LK_VALUE_NIL";
		case LK_MALLOC_FAILURE:	return "LK_MALLOC_FAILURE";
	}
	return "<unknown>";
}

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
	const lkerr_t gec = lkonf_get_error_code(lc);
	if (code != gec) {
		fprintf(stderr, "%s: code %d (%s) != get_error_code %d (%s)\n",
			desc,
			code, err_to_str(code),
			gec, err_to_str(gec));
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

		const lkerr_t errcode = lkonf_get_error_code(lc);
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

	/* fail: load null kconf_t */
	{
		const lkerr_t res = lkonf_load_file(0, 0);
		assert(LK_LKONF_NULL == res);
	}

/* TODO: fail: load kconf_t with null state */

	/* fail: load with filename null pointer */
	{
		lkonf_context * lc = lkonf_construct();

		assert(lc && "lkonf_construct returned 0");
		const lkerr_t res = lkonf_load_file(lc, 0);
		ensure_result(lc, res,
			"load_file(lc, 0)", LK_ARG_BAD, "iFile NULL");

		lkonf_destruct(lc);
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
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_load_string(lc, 0);
		ensure_result(lc, res,
			"load_string(lc, 0)", LK_ARG_BAD, "iString NULL");

		lkonf_destruct(lc);
	}

	/* fail: LK_LOAD_CHUNK syntax error in string */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_load_string(lc, "junk");
		ensure_result(lc, res,
			"load_string(lc, \"junk\")",
			LK_LOAD_CHUNK,
			"[string \"junk\"]:1: '=' expected near '<eof>'");

		lkonf_destruct(lc);
	}

	/* fail: LK_CALL_CHUNK run-time error in string */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_load_string(lc, "junk()");
		ensure_result(lc, res,
			"load_string(lc, \"junk()\")",
			LK_CALL_CHUNK,
			"[string \"junk()\"]:1: attempt to call global 'junk' (a nil value)");

		lkonf_destruct(lc);
	}

	/* pass: "t=1" */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_load_string(lc, "t=1");
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

	/* fail: load null kconf_t */
	{
		assert(-1 == lkonf_get_instruction_limit(0));
		assert(LK_LKONF_NULL == lkonf_set_instruction_limit(0, 0));
	}

	/* fail: set_instruction_limit < 0 */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_set_instruction_limit(lc, -2);
		ensure_result(lc, res,
			"set_instruction_limit(lc, -2)",
			LK_ARG_BAD, "iLimit < 0");

		lkonf_destruct(lc);
	}

	/* pass: set & get */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const int	limit = 10;

		const lkerr_t sil = lkonf_set_instruction_limit(lc, limit);
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

		const lkerr_t sil = lkonf_set_instruction_limit(lc, limit);
		assert(LK_OK == sil);

		const lkerr_t res = lkonf_load_string(lc, expr);
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

		const lkerr_t sil = lkonf_set_instruction_limit(lc, limit);
		assert(LK_OK == sil);

		const lkerr_t res = lkonf_load_string(lc, expr);
		ensure_result(lc, res, expr,
			LK_CALL_CHUNK, "Instruction count exceeded");

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
	const char *		path,
	lkonf_keys		keys,
	const bool		wanted,
	const lkerr_t		expect_code,
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
		printf("; expect code %s '%s'",
			err_to_str(expect_code), expect_str);
	}
	printf("\n");

	lkonf_context * lc = lkonf_construct();
	assert(lc && "lkonf_construct returned 0");

	const lkerr_t rls = lkonf_load_string(lc, test_luastr);
	ensure_result(lc, rls, "load_string", LK_OK, "");

		/* limit to 100 instructions; after load */
	const lkerr_t sil = lkonf_set_instruction_limit(lc, 100);
	ensure_result(lc, sil, "set_instruction_limit(lc, 100)", LK_OK, "");

	bool v = !wanted;
	lkerr_t res = LK_LKONF_NULL;
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

	/* fail: load null kconf_t */
	{
		assert(LK_LKONF_NULL == lkonf_get_boolean(0, 0, 0));
	}

	/* fail: null ovalue */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_get_boolean(lc, "", 0);
		ensure_result(lc, res,
			"get_boolean(lc, \"\", 0)", LK_ARG_BAD, "oValue NULL");

		lkonf_destruct(lc);
	}

	/* fail: null path */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		bool v = false;
		const lkerr_t res = lkonf_get_boolean(lc, 0, &v);
		ensure_result(lc, res,
			"get_boolean(lc, 0, &v)", LK_ARG_BAD, "iPath NULL");

		lkonf_destruct(lc);
	}

	/* pass: b1 */
	exercise_get_boolean("b1", 0, true, LK_OK, "");

	/* pass: top-level key 'missing' not set */
	exercise_get_boolean("missing", 0, true, LK_VALUE_NIL, "");

	/* pass: t2.b */
	exercise_get_boolean("t2.b", 0, false, LK_OK, "");

	/* pass: t3.t.b3 */
	exercise_get_boolean("t3.t.b3", 0, false, LK_OK, "");

	/* pass: t3.t.absent not set */
	exercise_get_boolean("t3.t.absent", 0, true, LK_VALUE_NIL, "");

	/* fail: t3.t. */
	exercise_get_boolean("t3.t.", 0, false,
		LK_KEY_BAD, "Empty component in: t3.t.");

	/* fail: t3.t.b3.k4 */
	exercise_get_boolean("t3.t.b3.k4", 0, true,
		LK_KEY_BAD, "Not a table: t3.t.b3");

	/* fail: t3.t.i3 (not a boolean)  */
	exercise_get_boolean("t3.t.i3", 0, false,
		LK_VALUE_BAD, "Not a boolean: t3.t.i3");

	/* fail: t3.k.k2 */
	exercise_get_boolean("t3.k.k2", 0, false,
		LK_KEY_BAD, "Not a table: t3.k");

	/* fail: t3.12345.3 */
	exercise_get_boolean("t3.12345.3", 0, false,
		LK_KEY_BAD, "Not a table: t3.12345");

	/* pass: tf.b function returning boolean */
	exercise_get_boolean("tf.b", 0, true, LK_OK, "");

	/* fail: tf.b. (trailing .) */
	exercise_get_boolean("tf.b.", 0, true, LK_KEY_BAD, "Not a table: tf.b");

	/* fail: t5i function not returning boolean */
	exercise_get_boolean("t5i", 0, false,
		LK_VALUE_BAD, "Not a boolean: t5i");

	/* fail: tf.i function not returning boolean */
	exercise_get_boolean("tf.i", 0, false,
		LK_VALUE_BAD, "Not a boolean: tf.i");

	/* fail: t6..k2 - empty key */
	exercise_get_boolean("t6..k2", 0, true,
		LK_KEY_BAD, "Empty component in: t6..k2");

	/* fail: t6 "." k2 - not a table "." */
	exercise_get_boolean("t6...k2", 0, false,
		LK_KEY_BAD, "Empty component in: t6...k2");

	/* fail: "" */
	exercise_get_boolean("", 0, false, LK_KEY_BAD, "Empty path");

	/* fail: "." */
	exercise_get_boolean(".", 0, false,
		LK_KEY_BAD, "Empty component in: .");

	/* pass: x */
	exercise_get_boolean("b", 0, true, LK_OK, "");

	/* pass: loooooooooooooooooooooooooooong.x.yb */
	exercise_get_boolean("loooooooooooooooooooooooooooong.x.yb", 0, true,
		LK_OK, "");

	/* fail: t7. */
	exercise_get_boolean("t7.", 0, true,
		LK_KEY_BAD, "Empty component in: t7.");

	/* fail: .t8 */
	exercise_get_boolean(".t8", 0, true,
		LK_KEY_BAD, "Empty component in: .t8");

	/* fail: t9n.1 */
/* TODO: fix path lookup to support integer lookup? */
	exercise_get_boolean("t9n.1", 0, true, LK_VALUE_NIL, "");

	/* pass: t9s.1 */
	exercise_get_boolean("t9s.1", 0, true, LK_OK, "");

	/* fail: t */
	exercise_get_boolean("t", 0, false, LK_VALUE_BAD, "Not a boolean: t");

	/* fail: t. */
	exercise_get_boolean("t.", 0, false,
		LK_KEY_BAD, "Empty component in: t.");

	/* pass: t.k nil VALUE */
	exercise_get_boolean("t.k", 0, true, LK_VALUE_NIL, "");

	/* fail: toolong takes too long */
	exercise_get_boolean("toolong", 0, true,
		LK_CALL_CHUNK, "Instruction count exceeded");

	/* fail: badrun calls unknown symbol */
	exercise_get_boolean("badrun", 0, false,
		LK_CALL_CHUNK, "[string \"b1 = true d1 = 1.01 i1 = 1 s1 = \"1\" t2 = { ...\"]:1: attempt to call global 'print' (a nil value)");

	/* pass: jrb */
	exercise_get_boolean("jrb", 0, true, LK_OK, "");

	/* fail: hidden */
	exercise_get_boolean("hidden", 0, true, LK_VALUE_NIL, "");

	return EXIT_SUCCESS;
}

int
test_getkey_boolean(void)
{
	printf("lkonf_getkey_boolean()\n");

	/* fail: load null kconf_t */
	{
		assert(LK_LKONF_NULL == lkonf_getkey_boolean(0, 0, 0));
	}

	/* fail: null oValue */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_getkey_boolean(lc,
			(lkonf_keys){ 0 }, 0);
		ensure_result(lc, res, "getkey_boolean(lc, {0}, 0)",
			LK_ARG_BAD, "oValue NULL");

		lkonf_destruct(lc);
	}

	/* fail: null iKeys */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		bool v = false;
		const lkerr_t res = lkonf_getkey_boolean(lc, 0, &v);
		ensure_result(lc, res, "getkey_boolean(lc, 0, &v)",
			LK_ARG_BAD, "iKeys NULL");

		lkonf_destruct(lc);
	}

	/* fail: empty iKeys */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		bool v = false;
		const lkerr_t res = lkonf_getkey_boolean(lc,
			(lkonf_keys){ 0 }, &v);
		ensure_result(lc, res, "getkey_boolean(lc, {0}, &v)",
			LK_KEY_BAD, "Empty keys");

		lkonf_destruct(lc);
	}

	/* pass: b1 */
	exercise_get_boolean("b1", (lkonf_keys){ "b1", 0 }, true, LK_OK, "");

	/* pass: top-level key 'missing' not set */
	exercise_get_boolean("missing", (lkonf_keys){ "missing", 0 },
		true, LK_VALUE_NIL, "");

	/* pass: t2.b */
	exercise_get_boolean("t2 b", (lkonf_keys){"t2", "b", 0},
		false, LK_OK, "");

	/* pass: t3.t.b3 */
	exercise_get_boolean("t3 t b3", (lkonf_keys){"t3", "t", "b3", 0},
		false, LK_OK, "");

	/* pass: t6.""."6.6".bm */
	exercise_get_boolean("t6 \"\" \"6.6\" bm",
		(lkonf_keys){"t6", "", "6.6", "bm", 0},
		true, LK_OK, "");

	/* pass: t3.t.absent not set */
	exercise_get_boolean("t3 t absent",
		(lkonf_keys){"t3", "t", "absent", 0},
		true, LK_VALUE_NIL, "");

	/* pass: t3.t."" not set */
	exercise_get_boolean("t3 t \"\"", (lkonf_keys){"t3", "t", "", 0},
		true, LK_VALUE_NIL, "");

	/* fail: t3.t.b3.k4 */
	exercise_get_boolean("t3 t b3 k4",
		(lkonf_keys){"t3", "t", "b3", "k4", 0},
		true, LK_KEY_BAD, "Not a table: b3");

	/* fail: t3.t.i3 (not a boolean)  */
	exercise_get_boolean("t3 t i3",
		(lkonf_keys){"t3", "t", "i3", 0},
		false, LK_VALUE_BAD, "Not a boolean: i3");

	/* fail: t3.k.k2 */
	exercise_get_boolean("t3 k k2",
		(lkonf_keys){"t3", "k", "k2", 0},
		false, LK_KEY_BAD, "Not a table: k");

	/* fail: t3.12345.3 */
	exercise_get_boolean("t3 12345 3",
		(lkonf_keys){"t3", "12345", "3", 0},
		false, LK_KEY_BAD, "Not a table: 12345");

	/* pass: tf.b function returning boolean */
	exercise_get_boolean("tf b", (lkonf_keys){"tf", "b", 0},
		true, LK_OK, "");

	/* fail: tf b "" (trailing .) */
	exercise_get_boolean("tf b \"\"",
		(lkonf_keys){"tf", "b", "", 0},
		true, LK_KEY_BAD, "Not a table: b");

	/* fail: tf.i function not returning boolean */
	exercise_get_boolean("tf i", (lkonf_keys){"tf", "i", 0},
		false, LK_VALUE_BAD, "Not a boolean: i");

	/* pass: t6 "" k2 - missing key k2 */
	exercise_get_boolean("t6 \"\" k2", (lkonf_keys){"t6", "", "k2", 0},
		true, LK_VALUE_NIL, "");

	/* pass: t6 "." b */
	exercise_get_boolean("t6 \".\" b", (lkonf_keys){"t6", ".", "b", 0},
		false, LK_OK, "");

	/* fail: "" empty key */
	exercise_get_boolean("", (lkonf_keys){"", 0},
		true, LK_KEY_BAD, "Empty top-level key");

	/* pass: "." absent */
	exercise_get_boolean(".", (lkonf_keys){".", 0},
		true, LK_VALUE_NIL, "");

	/* pass: x */
	exercise_get_boolean("b", (lkonf_keys){"b", 0}, true, LK_OK, "");

	/* pass: loooooooooooooooooooooooooooong.x.yb */
	exercise_get_boolean("loooooooooooooooooooooooooooong x yb",
		(lkonf_keys){"loooooooooooooooooooooooooooong", "x", "yb", 0},
		true, LK_OK, "");

	/* fail: t7 "" - not a bool */
	exercise_get_boolean("t7 \"\"", (lkonf_keys){"t7", "", 0},
		true, LK_VALUE_BAD, "Not a boolean: ");

	/* fail: "" t8 */
	exercise_get_boolean("\"\" t8", (lkonf_keys){"", "t8", 0},
		true, LK_KEY_BAD, "Empty top-level key");

	/* fail: t */
	exercise_get_boolean("t", (lkonf_keys){"t", 0},
		false, LK_VALUE_BAD, "Not a boolean: t");

	/* pass: t."" */
	exercise_get_boolean("t.", (lkonf_keys){"t", "", 0},
		true, LK_VALUE_NIL, "");

	/* pass: t.k nil VALUE */
	exercise_get_boolean("t.k", (lkonf_keys){"t", "k", 0},
		true, LK_VALUE_NIL, "");

	/* fail: toolong takes too long */
	exercise_get_boolean("toolong", (lkonf_keys){"toolong", 0},
		true, LK_CALL_CHUNK, "Instruction count exceeded");

	/* fail: badrun calls unknown symbol */
	exercise_get_boolean("badrun", (lkonf_keys){"badrun", 0},
		false,
		LK_CALL_CHUNK, "[string \"b1 = true d1 = 1.01 i1 = 1 s1 = \"1\" t2 = { ...\"]:1: attempt to call global 'print' (a nil value)");

	/* pass: jrb */
	exercise_get_boolean("jrb", (lkonf_keys){"jrb", 0}, true, LK_OK, "");

	/* fail: hidden */
	exercise_get_boolean("hidden", (lkonf_keys){"hidden", 0},
		true, LK_VALUE_NIL, "");

	return EXIT_SUCCESS;
}


void
exercise_get_double(
	const char *	path,
	const double	wanted,
	const lkerr_t	expect_code,
	const char *	expect_str)
{
	char desc[128];
	snprintf(desc, sizeof(desc), "get_double('%s')", path);
	printf("%s = %g", desc, wanted);
	if (LK_OK != expect_code) {
		printf("; expect code %s '%s'",
			err_to_str(expect_code), expect_str);
	}
	printf("\n");

	lkonf_context * lc = lkonf_construct();
	assert(lc && "lkonf_construct returned 0");

	const lkerr_t rls = lkonf_load_string(lc, test_luastr);
	ensure_result(lc, rls, "load_string", LK_OK, "");

		/* limit to 100 instructions; after load */
	const lkerr_t sil = lkonf_set_instruction_limit(lc, 100);
	ensure_result(lc, sil, "set_instruction_limit(lc, 100)", LK_OK, "");

	double v = -wanted;
	const lkerr_t res = lkonf_get_double(lc, path, &v);
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

	/* fail: load null kconf_t */
	{
		assert(LK_LKONF_NULL == lkonf_get_double(0, 0, 0));
	}

	/* fail: null ovalue */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_get_double(lc, "", 0);
		ensure_result(lc, res,
			"get_double(lc, \"\", 0)", LK_ARG_BAD, "oValue NULL");

		lkonf_destruct(lc);
	}

	/* fail: null path */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		double v = -1;
		const lkerr_t res = lkonf_get_double(lc, 0, &v);
		ensure_result(lc, res,
			"get_double(lc, 0, &v)", LK_ARG_BAD, "iPath NULL");

		lkonf_destruct(lc);
	}

	/* pass: d1 */
	exercise_get_double("d1", 1.01, LK_OK, "");

	/* pass: top-level key 'missing' not set */
	exercise_get_double("missing", 5, LK_VALUE_NIL, "");

	/* pass: t2.d */
	exercise_get_double("t2.d", 2.714, LK_OK, "");

	/* pass: t3.t.d3 */
	exercise_get_double("t3.t.d3", 3.1415, LK_OK, "");

	/* pass: t3.t.absent not set */
	exercise_get_double("t3.t.absent", 5, LK_VALUE_NIL, "");

	/* fail: t3.t. */
	exercise_get_double("t3.t.", 0, LK_KEY_BAD,
		"Empty component in: t3.t.");

	/* fail: t3.t.d3.k4 */
	exercise_get_double("t3.t.d3.k4", 33,
		LK_KEY_BAD, "Not a table: t3.t.d3");

	/* fail: t3.t.b3 (not a double)  */
	exercise_get_double("t3.t.b3", 0,
		LK_VALUE_BAD, "Not a double: t3.t.b3");

	/* fail: t3.k.d3 */
	exercise_get_double("t3.k.d3", 0, LK_KEY_BAD, "Not a table: t3.k");

	/* fail: t3.12345.3 */
	exercise_get_double("t3.12345.3", 0,
		LK_KEY_BAD, "Not a table: t3.12345");

	/* pass: tf.d function returning double */
	exercise_get_double("tf.d", -4.01, LK_OK, "");

	/* fail: tf.d. (trailing .) */
	exercise_get_double("tf.d.", 4, LK_KEY_BAD, "Not a table: tf.d");

	/* fail: t5b function not returning double */
	exercise_get_double("t5b", 0, LK_VALUE_BAD, "Not a double: t5b");

	/* fail: tf.s function not returning double */
	exercise_get_double("tf.s", 0, LK_VALUE_BAD, "Not a double: tf.s");

	/* fail: t6..k2 - empty key */
	exercise_get_double("t6..k2", 6,
		LK_KEY_BAD, "Empty component in: t6..k2");

	/* fail: t6...k2 */
	exercise_get_double("t6...k2", 0,
		LK_KEY_BAD, "Empty component in: t6...k2");

	/* fail: "" */
	exercise_get_double("", -5, LK_KEY_BAD, "Empty path");

	/* fail: "." */
	exercise_get_double(".", -5, LK_KEY_BAD, "Empty component in: .");

	/* pass: d */
	exercise_get_double("d", 0.5, LK_OK, "");

	/* pass: loooooooooooooooooooooooooooong.x.yd */
	exercise_get_double("loooooooooooooooooooooooooooong.x.yd",
		99.999, LK_OK, "");

	/* fail: t7. */
	exercise_get_double("t7.", 7,
		LK_KEY_BAD, "Empty component in: t7.");

	/* fail: .t8 */
	exercise_get_double(".t8", 8,
		LK_KEY_BAD, "Empty component in: .t8");

	/* fail: t */
	exercise_get_double("t", 0, LK_VALUE_BAD, "Not a double: t");

	/* fail: t. */
	exercise_get_double("t.", 0, LK_KEY_BAD, "Empty component in: t.");

	/* pass: t.k nil VALUE */
	exercise_get_double("t.k", 999, LK_VALUE_NIL, "");

	/* fail: toolong takes too long */
	exercise_get_double("toolong", 0,
		LK_CALL_CHUNK, "Instruction count exceeded");

	/* fail: badrun calls unknown symbol */
	exercise_get_double("badrun", -1,
		LK_CALL_CHUNK, "[string \"b1 = true d1 = 1.01 i1 = 1 s1 = \"1\" t2 = { ...\"]:1: attempt to call global 'print' (a nil value)");

	/* pass: jrd */
	exercise_get_double("jrd", 4.9, LK_OK, "");

	/* fail: hidden */
	exercise_get_double("hidden", 0, LK_VALUE_NIL, "");

	return EXIT_SUCCESS;
}


void
exercise_get_integer(
	const char *		path,
	const lua_Integer	wanted,
	const lkerr_t		expect_code,
	const char *		expect_str)
{
	char desc[128];
	snprintf(desc, sizeof(desc), "get_integer('%s')", path);
	printf("%s = %" PRId64, desc, (int64_t)wanted);
	if (LK_OK != expect_code) {
		printf("; expect code %s '%s'",
			err_to_str(expect_code), expect_str);
	}
	printf("\n");

	lkonf_context * lc = lkonf_construct();
	assert(lc && "lkonf_construct returned 0");

	const lkerr_t rls = lkonf_load_string(lc, test_luastr);
	ensure_result(lc, rls, "load_string", LK_OK, "");

		/* limit to 100 instructions; after load */
	const lkerr_t sil = lkonf_set_instruction_limit(lc, 100);
	ensure_result(lc, sil, "set_instruction_limit(lc, 100)", LK_OK, "");

	lua_Integer v = -wanted;
	const lkerr_t res = lkonf_get_integer(lc, path, &v);
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

	/* fail: load null kconf_t */
	{
		assert(LK_LKONF_NULL == lkonf_get_integer(0, 0, 0));
	}

	/* fail: null ovalue */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_get_integer(lc, "", 0);
		ensure_result(lc, res,
			"get_integer(lc, \"\", 0)", LK_ARG_BAD, "oValue NULL");

		lkonf_destruct(lc);
	}

	/* fail: null path */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		lua_Integer v = -1;
		const lkerr_t res = lkonf_get_integer(lc, 0, &v);
		ensure_result(lc, res,
			"get_integer(lc, 0, &v)", LK_ARG_BAD, "iPath NULL");

		lkonf_destruct(lc);
	}

	/* pass: i1 */
	exercise_get_integer("i1", 1, LK_OK, "");

	/* pass: top-level key 'missing' not set */
	exercise_get_integer("missing", 5, LK_VALUE_NIL, "");

	/* pass: t2.i */
	exercise_get_integer("t2.i", 2, LK_OK, "");

	/* pass: t3.t.i3 */
	exercise_get_integer("t3.t.i3", 33, LK_OK, "");

	/* pass: t3.t.absent not set */
	exercise_get_integer("t3.t.absent", 5, LK_VALUE_NIL, "");

	/* fail: t3.t. */
	exercise_get_integer("t3.t.", 0, LK_KEY_BAD,
		"Empty component in: t3.t.");

	/* fail: t3.t.i3.k4 */
	exercise_get_integer("t3.t.i3.k4", 33,
		LK_KEY_BAD, "Not a table: t3.t.i3");

	/* fail: t3.t.b3 (not an integer)  */
	exercise_get_integer("t3.t.b3", 0,
		LK_VALUE_BAD, "Not an integer: t3.t.b3");

	/* fail: t3.k.i3 */
	exercise_get_integer("t3.k.i3", 0, LK_KEY_BAD, "Not a table: t3.k");

	/* fail: t3.12345.3 */
	exercise_get_integer("t3.12345.3", 0,
		LK_KEY_BAD, "Not a table: t3.12345");

	/* pass: tf.i function returning integer */
	exercise_get_integer("tf.i", 4, LK_OK, "");

	/* fail: tf.i. (trailing .) */
	exercise_get_integer("tf.i.", 4, LK_KEY_BAD, "Not a table: tf.i");

	/* fail: t5b function not returning integer */
	exercise_get_integer("t5b", 0, LK_VALUE_BAD, "Not an integer: t5b");

	/* fail: tf.d function not returning integer */
	exercise_get_integer("tf.b", 2, LK_VALUE_BAD, "Not an integer: tf.b");

	/* fail: t6..k2 - empty key */
	exercise_get_integer("t6..k2", 6,
		LK_KEY_BAD, "Empty component in: t6..k2");

	/* fail: t6...k2 */
	exercise_get_integer("t6...k2", 0,
		LK_KEY_BAD, "Empty component in: t6...k2");

	/* fail: "" */
	exercise_get_integer("", -5, LK_KEY_BAD, "Empty path");

	/* fail: "." */
	exercise_get_integer(".", -5, LK_KEY_BAD, "Empty component in: .");

	/* pass: i */
	exercise_get_integer("i", 11, LK_OK, "");

	/* pass: loooooooooooooooooooooooooooong.x.yi */
	exercise_get_integer("loooooooooooooooooooooooooooong.x.yi",
		99, LK_OK, "");

	/* fail: t7. */
	exercise_get_integer("t7.", 7,
		LK_KEY_BAD, "Empty component in: t7.");

	/* fail: .t8 */
	exercise_get_integer(".t8", 8,
		LK_KEY_BAD, "Empty component in: .t8");

	/* fail: t */
	exercise_get_integer("t", 0, LK_VALUE_BAD, "Not an integer: t");

	/* fail: t. */
	exercise_get_integer("t.", 0, LK_KEY_BAD, "Empty component in: t.");

	/* pass: t.k nil VALUE */
	exercise_get_integer("t.k", 999, LK_VALUE_NIL, "");

	/* fail: toolong takes too long */
	exercise_get_integer("toolong", 0,
		LK_CALL_CHUNK, "Instruction count exceeded");

	/* fail: badrun calls unknown symbol */
	exercise_get_integer("badrun", 0,
		LK_CALL_CHUNK, "[string \"b1 = true d1 = 1.01 i1 = 1 s1 = \"1\" t2 = { ...\"]:1: attempt to call global 'print' (a nil value)");

	/* pass: jri */
	exercise_get_integer("jri", 5, LK_OK, "");

	/* fail: hidden */
	exercise_get_integer("hidden", 0, LK_VALUE_NIL, "");

	return EXIT_SUCCESS;
}


void
exercise_get_string(
	const char *		path,
	const const char *	wantstr,
	const size_t		wantlen,
	const lkerr_t		expect_code,
	const char *		expect_str)
{
	char desc[128];
	snprintf(desc, sizeof(desc), "get_string('%s')", path);
	printf("%s = '%s'/%zu", desc, wantstr, wantlen);
	if (LK_OK != expect_code) {
		printf("; expect code %s '%s'",
			err_to_str(expect_code), expect_str);
	}
	printf("\n");

	lkonf_context * lc = lkonf_construct();
	assert(lc && "lkonf_construct returned 0");

	const lkerr_t rls = lkonf_load_string(lc, test_luastr);
	ensure_result(lc, rls, "load_string", LK_OK, "");

		/* limit to 100 instructions; after load */
	const lkerr_t sil = lkonf_set_instruction_limit(lc, 100);
	ensure_result(lc, sil, "set_instruction_limit(lc, 100)", LK_OK, "");

	char * gotstr = 0;
	size_t gotlen = -wantlen;
	const lkerr_t res = lkonf_get_string(lc, path, &gotstr, &gotlen);
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

	/* fail: load null kconf_t */
	{
		assert(LK_LKONF_NULL == lkonf_get_string(0, 0, 0, 0));
	}

	/* fail: null ovalue */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		const lkerr_t res = lkonf_get_string(lc, "", 0, 0);
		ensure_result(lc, res,
			"get_string(lc, \"\", 0, 0)",
			LK_ARG_BAD, "oValue NULL");

		lkonf_destruct(lc);
	}

	/* fail: null path */
	{
		lkonf_context * lc = lkonf_construct();
		assert(lc && "lkonf_construct returned 0");

		char * v = 0;
		const lkerr_t res = lkonf_get_string(lc, 0, &v, 0);
		ensure_result(lc, res,
			"get_integer(lc, 0, &v)", LK_ARG_BAD, "iPath NULL");

		lkonf_destruct(lc);
	}

	/* pass: s1 */
	exercise_get_string("s1", "1", 1, LK_OK, "");

	/* pass: top-level key 'missing' not set */
	exercise_get_string("missing", "", 0, LK_VALUE_NIL, "");

	/* pass: t2.empty */
	exercise_get_string("t2.empty", "", 0, LK_OK, "");

	/* pass: t2.2 */
	exercise_get_string("t2.2", "two", 3, LK_OK, "");

	/* pass: t3.t.s3 */
	exercise_get_string("t3.t.s3", "thirty three", 12, LK_OK, "");

	/* pass: t3.t.absent not set */
	exercise_get_string("t3.t.absent", "", 0, LK_VALUE_NIL, "");

	/* fail: t3.t. */
	exercise_get_string("t3.t.", "", 0, LK_KEY_BAD,
		"Empty component in: t3.t.");

	/* fail: t3.t.i3.k4 */
	exercise_get_string("t3.t.i3.k4", "", 0,
		LK_KEY_BAD, "Not a table: t3.t.i3");

	/* fail: t3.t.b3 (not a string)  */
	exercise_get_string("t3.t.b3", "", 0,
		LK_VALUE_BAD, "Not a string: t3.t.b3");

	/* fail: t3.k.i3 */
	exercise_get_string("t3.k.i3", "", 0, LK_KEY_BAD, "Not a table: t3.k");

	/* fail: t3.12345.3 */
	exercise_get_string("t3.12345.3", "", 0,
		LK_KEY_BAD, "Not a table: t3.12345");

	/* pass: tf.s function returning string */
	exercise_get_string("tf.s", "tf path=tf.s", 12, LK_OK, "");

	/* fail: tf.s. (trailing .) */
	exercise_get_string("tf.s.", "", 4, LK_KEY_BAD, "Not a table: tf.s");

	/* fail: t5b function not returning string */
	exercise_get_string("t5b", "", 0, LK_VALUE_BAD, "Not a string: t5b");

	/* fail: tf.i function not returning string */
	exercise_get_string("tf.i", "", 0, LK_VALUE_BAD, "Not a string: tf.i");

	/* fail: t6..k2 - empty key */
	exercise_get_string("t6..k2", "", 6,
		LK_KEY_BAD, "Empty component in: t6..k2");

	/* fail: t6...k2 */
	exercise_get_string("t6...k2", "", 0,
		LK_KEY_BAD, "Empty component in: t6...k2");

	/* fail: "" */
	exercise_get_string("", "", 0, LK_KEY_BAD, "Empty path");

	/* fail: "." */
	exercise_get_string(".", "", 0, LK_KEY_BAD, "Empty component in: .");

	/* fail: toolong takes too long */
	exercise_get_string("toolong", "", 0,
		LK_CALL_CHUNK, "Instruction count exceeded");

	/* fail: badrun calls unknown symbol */
	exercise_get_string("badrun", "", 0,
		LK_CALL_CHUNK, "[string \"b1 = true d1 = 1.01 i1 = 1 s1 = \"1\" t2 = { ...\"]:1: attempt to call global 'print' (a nil value)");

	/* pass: jrs */
	exercise_get_string("jrs", "just right!", 11, LK_OK, "");

	/* fail: hidden */
	exercise_get_string("hidden", "", 0, LK_VALUE_NIL, "");

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
	{ "get_integer",	TF_get_integer,		test_get_integer },
	{ "get_string",		TF_get_string,		test_get_string },
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
