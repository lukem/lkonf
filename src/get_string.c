#include "internal.h"

LUA_API lkerr_t
lkonf_get_string(
	lkonf_t *	iLc,
	const char *	iPath,
	const char **	oValue,
	size_t *	oLen)
{
	if (! iLc) {
		return LK_LKONF_NULL;
	}

	if (LK_OK != lki_state_entry(iLc)) {
		return lki_state_exit(iLc);
	}

	if (! oValue) {
		lki_set_error(iLc, LK_ARG_BAD, "oValue NULL");
		return lki_state_exit(iLc);
	}

	if (LK_OK != lki_find_table_by_path(iLc, iPath)) {
		return lki_state_exit(iLc);
	}

	if (lua_isfunction(iLc->state, -1)) {
		lua_pushstring(iLc->state, iPath);
		if (LK_OK != lki_call_chunk(iLc, 1, 1)) {
			return lki_state_exit(iLc);
		}
	}

	if (LUA_TNIL == lua_type(iLc->state, -1)) {
		lki_set_error(iLc, LK_VALUE_NIL, "");
		return lki_state_exit(iLc);
	}

	if (LUA_TSTRING != lua_type(iLc->state, -1)) {
		lki_set_error_item(iLc, LK_VALUE_BAD, "Not a string", iPath);
		return lki_state_exit(iLc);
	}

	*oValue = lua_tolstring(iLc->state, -1, oLen);

	return lki_state_exit(iLc);
}