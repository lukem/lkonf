#include "internal.h"

#include <assert.h>

lkerr_t
lki_call_chunk(lkonf_t * iLc, const int iNumArgs, const int iNumResults)
{
	assert(iLc);
	assert(iLc->state);

	if (iNumArgs < 0) {
		lki_set_error(iLc, LK_ARG_BAD, "iNumArgs < 0");
		return iLc->error_code;
	}

	if (iNumResults < 0) {
		lki_set_error(iLc, LK_ARG_BAD, "iNumResults < 0");
		return iLc->error_code;
	}

// TODO set instruction limit

// TODO sandbox

	if (lua_pcall(iLc->state, iNumArgs, iNumResults, 0)) {
		lki_set_error_from_state(iLc, LK_CALL_CHUNK);
	}

// TODO reset instruction limit

	return iLc->error_code;
}
