#include "internal.h"

#include <assert.h>


lkerr_t
lkonf_state_entry(lkonf_t * iLc)
{
	assert(iLc && "iLc NULL");

	lkonf_reset_error(iLc);

	iLc->depth = -1;
	if (! iLc->state) {
		lkonf_set_error(iLc, LK_LKONF_NULL, "Lua state null");
		return iLc->error_code;
	}

	iLc->depth = lua_gettop(iLc->state);

	return iLc->error_code;
}

lkerr_t
lkonf_state_exit(lkonf_t * iLc)
{
	assert(iLc && "iLc NULL");

	if (! iLc->state) {
		if (! iLc->error_code) {
			lkonf_set_error(iLc, LK_LKONF_NULL, "Lua state null");
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
