#include "internal.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/**
 * Push next key to stack.
 * If next key is empty, no key is pushed.
 * @param	iLc	lkonf_context
 * @param	iPath	Pointer to key path to parse ("a.bb.c")
 * Returns		Pointer to end of key.
 */
static const char *
push_next_key(lkonf_context * iLc, const char * iPath)
{
	assert(iLc);
	assert(iPath);

	if ('.' == *iPath) {
		++iPath;
	}

	const size_t len = strcspn(iPath, ".");
	if (!len) {
		return 0;
	}
	if (len) {
		lua_pushlstring(iLc->state, iPath, len);
	}

	return iPath + len;
}


lkonf_error
lki_find_table_by_path(lkonf_context * iLc, const char * iPath)
{
	if (! iLc) {
		return LK_LKONF_NULL;
	}

	if (! iPath) {
		return lki_set_error(iLc, LK_ARG_BAD, "iPath NULL");
	}

	if (! *iPath) {
		return lki_set_error(iLc, LK_KEY_BAD, "Empty path");
	}

	if ('.' == *iPath) {
		return lki_set_error_item(iLc, LK_KEY_BAD,
			"Empty component in", iPath);
	}

		/* Push globals table onto stack. */
#if 502 > LUA_VERSION_NUM
	lua_pushvalue(iLc->state, LUA_GLOBALSINDEX);
#else
	lua_pushglobaltable(iLc->state);		/* S: t */
#endif

		/* Push first key. */
	const char * end = push_next_key(iLc, iPath);	/* S: t k */
	if (! end) {
		return lki_set_error_item(iLc, LK_KEY_BAD,
			"Empty component in", iPath);
	}

	lua_gettable(iLc->state, -2);			/* S: t t[k] */
	lua_remove(iLc->state, -2);			/* S: t[k] */

		/* Iterate through remaining keys until EOS or empty key. */
	while (*end) {
		if (! lua_istable(iLc->state, -1)) {
			char path[sizeof(iLc->error_string)];
			snprintf(path, sizeof(path), "%.*s",
				(int)(end - iPath), iPath);
			return lki_set_error_item(
				iLc, LK_KEY_BAD, "Not a table", path);
		}

		end = push_next_key(iLc, end);		/* S: t[k] k2 */
		if (! end) {
			return lki_set_error_item(iLc, LK_KEY_BAD,
				"Empty component in", iPath);
		}

		lua_gettable(iLc->state, -2);		/* S: t[k] t[k][k2] */
		lua_remove(iLc->state, -2);		/* S: t[k][k2] */
	}

	return LK_OK;
}
