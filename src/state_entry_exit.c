#include "internal.h"

#include <assert.h>

lkonf_error
lki_state_entry(lkonf_context * iLc)
{
	if (! iLc) {
		return LK_LKONF_NULL;
	}

	lki_reset_error(iLc);

	iLc->depth = -1;
	if (! iLc->state) {
		return lki_set_error(iLc, LK_STATE_NULL, "Lua state null");
	}

	iLc->depth = lua_gettop(iLc->state);

	return iLc->error_code;
}

lkonf_error
lki_state_exit(lkonf_context * iLc)
{
	if (! iLc) {
		return LK_LKONF_NULL;
	}

	if (! iLc->state) {
		if (! iLc->error_code) {
			lki_set_error(iLc, LK_STATE_NULL, "Lua state null");
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
