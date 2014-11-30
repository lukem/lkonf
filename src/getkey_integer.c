#include "internal.h"

lkonf_error
lkonf_getkey_integer(lkonf_context * iLc, lkonf_keys iKeys, lua_Integer * oValue)
{
	if (! iLc) {
		return LK_INVALID_ARGUMENT;
	}

	if (LK_OK != lki_state_entry(iLc)) {
		return lki_state_exit(iLc);
	}

	if (! oValue) {
		lki_set_error(iLc, LK_INVALID_ARGUMENT, "oValue NULL");
		return lki_state_exit(iLc);
	}

	size_t last = 0;
	if (LK_OK != lki_find_table_by_keys(iLc, iKeys, &last)) {
		return lki_state_exit(iLc);
	}

	if (lua_isfunction(iLc->state, -1)) {
		lua_pushstring(iLc->state, iKeys[last]);
		if (LK_OK != lki_call_chunk(iLc, 1, 1)) {
			return lki_state_exit(iLc);
		}
	}

	if (LUA_TNIL == lua_type(iLc->state, -1)) {
		lki_set_error(iLc, LK_NOT_FOUND, "");
		return lki_state_exit(iLc);
	}

	if (LUA_TNUMBER != lua_type(iLc->state, -1)) {
		lki_set_error_item(iLc,
			LK_OUT_OF_RANGE, "Not an integer", iKeys[last]);
		return lki_state_exit(iLc);
	}

	*oValue = lua_tointeger(iLc->state, -1);

	return lki_state_exit(iLc);
}
