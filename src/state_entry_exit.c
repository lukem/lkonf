#include "internal.h"

#include <assert.h>

lkonf_error
lki_state_entry(lkonf_context * iLc)
{
	if (! iLc) {
		return LK_INVALID_ARGUMENT;
	}

	lki_reset_error(iLc);

	iLc->depth = -1;
	if (! iLc->state) {
		return lki_set_error(iLc,
			LK_INVALID_ARGUMENT, "Lua state NULL");
	}

	iLc->depth = lua_gettop(iLc->state);

	return iLc->error_code;
}

lkonf_error
lki_state_exit(lkonf_context * iLc)
{
	if (! iLc) {
		return LK_INVALID_ARGUMENT;
	}

	if (! iLc->state) {
		if (! iLc->error_code) {
			lki_set_error(iLc,
				LK_INVALID_ARGUMENT, "Lua state NULL");
		}
		return iLc->error_code;
	}

	assert(iLc->depth >= 0);

	const int diff = lua_gettop(iLc->state) - iLc->depth;
	assert(diff >= 0);

	if (diff > 0) {
		lua_pop(iLc->state, diff);
	}
	iLc->depth = -1;

	return iLc->error_code;
}
