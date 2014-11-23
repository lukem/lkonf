#include "internal.h"

#include <lauxlib.h>

lkerr_t
lkonf_load_string(lkonf_context * iLc, const char * iString)
{
	if (! iLc) {
		return LK_LKONF_NULL;
	}

	if (LK_OK != lki_state_entry(iLc)) {
		return lki_state_exit(iLc);
	}

	if (! iString) {
		lki_set_error(iLc, LK_ARG_BAD, "iString NULL");
		return lki_state_exit(iLc);
	}

	if (luaL_loadstring(iLc->state, iString)) {
		lki_set_error_from_state(iLc, LK_LOAD_CHUNK);
		return lki_state_exit(iLc);
	}

	lki_call_chunk(iLc, 0, 0);

	return lki_state_exit(iLc);
}
