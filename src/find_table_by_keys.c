#include "internal.h"

lkonf_error
lki_find_table_by_keys(lkonf_context * iLc, lkonf_keys iKeys, size_t * oMatch)
{
	if (! iLc) {
		return LK_INVALID_ARGUMENT;
	}

	if (! iKeys) {
		return lki_set_error(iLc, LK_INVALID_ARGUMENT, "iKeys NULL");
	}

	if (! iKeys[0]) {
		return lki_set_error(iLc, LK_OUT_OF_RANGE, "Empty keys");
	}

	if (! iKeys[0][0]) {
		return lki_set_error(iLc,
			LK_OUT_OF_RANGE, "Empty top-level key");
	}

		/* Push globals table onto stack. */
#if LUA_VERSION_NUM >= 502
	lua_pushglobaltable(iLc->state);		/* S: t */
#else
	lua_pushvalue(iLc->state, LUA_GLOBALSINDEX);	/* S: t */
#endif

		/* Push first key. */
	lua_pushstring(iLc->state, iKeys[0]);
	lua_gettable(iLc->state, -2);			/* S: t t[k] */
	lua_remove(iLc->state, -2);			/* S: t[k] */

		/* Iterate through remaining keys until NULL key. */
	size_t ki;
	for (ki = 1; 0 != iKeys[ki]; ++ki) {
		if (! lua_istable(iLc->state, -1)) {
			char kdesc[128];
			lki_format_keys(iKeys, ki, kdesc, sizeof(kdesc));
			return lki_set_error_item(iLc,
				LK_OUT_OF_RANGE, "Not a table", kdesc);
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
