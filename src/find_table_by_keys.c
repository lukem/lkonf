#include "internal.h"

lkonf_error
lki_find_table_by_keys(lkonf_context * iLc, lkonf_keys iKeys, size_t * oMatch)
{
	if (! iLc) {
		return LK_LKONF_NULL;
	}

	if (! iKeys) {
		return lki_set_error(iLc, LK_ARG_BAD, "iKeys NULL");
	}

	if (! iKeys[0]) {
		return lki_set_error(iLc, LK_KEY_BAD, "Empty keys");
	}

	if (! iKeys[0][0]) {
		return lki_set_error(iLc, LK_KEY_BAD, "Empty top-level key");
	}

		/* Push globals table onto stack. */
#if 502 > LUA_VERSION_NUM
	lua_pushvalue(iLc->state, LUA_GLOBALSINDEX);
#else
	lua_pushglobaltable(iLc->state);		/* S: t */
#endif

		/* Push first key. */
	lua_pushstring(iLc->state, iKeys[0]);
	lua_gettable(iLc->state, -2);			/* S: t t[k] */
	lua_remove(iLc->state, -2);			/* S: t[k] */

		/* Iterate through remaining keys until NULL key. */
	size_t ki;
	for (ki = 1; 0 != iKeys[ki]; ++ki) {
		if (! lua_istable(iLc->state, -1)) {
			return lki_set_error_item(
				iLc, LK_KEY_BAD, "Not a table", iKeys[ki-1]);
		}

		lua_pushstring(iLc->state, iKeys[ki]);	/* S: t[k] k2 */
		lua_gettable(iLc->state, -2);		/* S: t[k] t[k][k2] */
		lua_remove(iLc->state, -2);		/* S: t[k][k2] */
	}

	if (oMatch) {
		*oMatch = ki - 1;
	}

	return LK_OK;
}
