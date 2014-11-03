#include "internal.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/**
 * Push next key to stack.
 * If no key currently pushed (*iLen == 0), first key is pushed.
 * If next key is empty, no key is pushed.
 * @param[in]		iLc	lkonf_t
 * @param[in]		iPath	Pointer to key path to parse ("a.bb.c")
 * @param[in,out]	ioLen	Length of previous key. First call must be 0.
 *				On return, length of returned key.
 * Returns			Pointer to pushed key.
 */
static const char *
push_next_key(lkonf_t * iLc, const char * iPath, size_t * ioLen)
{
	assert(iLc);
	assert(iPath);
	assert(ioLen);

	if (*ioLen) {
			/* Move to next key */
		iPath += *ioLen;
		if ('.' == *iPath)
			++iPath;
	}

	*ioLen = strcspn(iPath, ".");

	if (*ioLen) {
		lua_pushlstring(iLc->state, iPath, *ioLen);
	}

	return iPath;
}


lkerr_t
lki_find_table_by_path(lkonf_t * iLc, const char * iPath)
{
	if (! iLc) {
		return LK_LKONF_NULL;
	}

	if (! iPath) {
		return lki_set_error(iLc, LK_ARG_BAD, "iPath NULL");
	}

		/* Push globals table onto stack. */
#if 502 > LUA_VERSION_NUM
	lua_pushvalue(iLc->state, LUA_GLOBALSINDEX);
#else
	lua_pushglobaltable(iLc->state);		/* S: t */
#endif

		/* Push first key. */
	const char *	cur = iPath;
	size_t		len = 0;
	cur = push_next_key(iLc, cur, &len);		/* S: t k */
	if (! len) {
		return lki_set_error(iLc, LK_KEY_BAD, "Empty key");
	}

	lua_gettable(iLc->state, -2);			/* S: t t[k] */
	lua_remove(iLc->state, -2);			/* S: t[k] */

		/* Iterate through remaining keys until EOS or empty key. */
	while (cur[len]) {
		if (! lua_istable(iLc->state, -1)) {
			char path[sizeof(iLc->error_string)];
			snprintf(path, sizeof(path), "%.*s", (int)len, cur);
			return lki_set_error_item(
				iLc, LK_KEY_BAD, "Not a table", path);
		}

		cur = push_next_key(iLc, cur, &len);	/* S: t[k] k2 */
		if (! len) {
			return lki_set_error(iLc, LK_KEY_BAD, "Empty key");
		}

		lua_gettable(iLc->state, -2);		/* S: t[k] t[k][k2] */
		lua_remove(iLc->state, -2);		/* S: t[k][k2] */
	}

	return LK_OK;
}
