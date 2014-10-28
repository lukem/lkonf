#include "internal.h"

#include <lauxlib.h>

lkerr_t
lkonf_load_string(lkonf_t * iLc, const char * iString)
{
	if (! iLc) {
		return LK_LKONF_NULL;
	}

	if (LK_OK != lkonf_state_entry(iLc)) {
		return lkonf_state_exit(iLc);
	}

	if (! iString) {
		lkonf_set_error(iLc, LK_ARG_BAD, "iString NULL");
		return lkonf_state_exit(iLc);
	}

	if (luaL_loadstring(iLc->state, iString)) {
		lkonf_set_error_from_state(iLc, LK_LOAD_CHUNK);
		return lkonf_state_exit(iLc);
	}

	lkonf_call_chunk(iLc, 0, 0);

	return lkonf_state_exit(iLc);
}
