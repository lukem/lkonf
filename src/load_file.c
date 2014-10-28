#include "internal.h"

#include <lauxlib.h>

lkerr_t
lkonf_load_file(lkonf_t * iLc, const char * iFile)
{
	if (! iLc) {
		return LK_LKONF_NULL;
	}

	if (LK_OK != lkonf_state_entry(iLc)) {
		return lkonf_state_exit(iLc);
	}

	if (! iFile) {
		lkonf_set_error(iLc, LK_ARG_BAD, "iFile NULL");
		return lkonf_state_exit(iLc);
	}

	if (luaL_loadfile(iLc->state, iFile)) {
		lkonf_set_error_from_state(iLc, LK_LOAD_CHUNK);
		return lkonf_state_exit(iLc);
	}

	lkonf_call_chunk(iLc, 0, 0);

	return lkonf_state_exit(iLc);
}
