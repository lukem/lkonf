#include "internal.h"

LUA_API lkerr_t
lkonf_get_integer(lkonf_t * iLc, const char * iPath, lua_Integer * oValue)
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

/* TODO handle optional (LUA_TNIL) here  */

	if (LUA_TNUMBER != lua_type(iLc->state, -1)) {
		lki_set_error_item(iLc, LK_VALUE_BAD, "Not an integer", iPath);
		return lki_state_exit(iLc);
	}

	*oValue = lua_tointeger(iLc->state, -1);

	return lki_state_exit(iLc);
}
