#include "internal.h"

#include <assert.h>
#include <lauxlib.h>

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

	if (iLc->instruction_limit > 0) {
		lua_sethook(
			iLc->state,
			&lki_maskcount_exceeded,
			LUA_MASKCOUNT,
			iLc->instruction_limit);
	}

// TODO sandbox

	if (lua_pcall(iLc->state, iNumArgs, iNumResults, 0)) {
		lki_set_error_from_state(iLc, LK_CALL_CHUNK);
	}

	if (iLc->instruction_limit > 0) {
		lua_sethook(
			iLc->state,
			&lki_maskcount_exceeded,
			LUA_MASKCOUNT,
			0);
	}

	return iLc->error_code;
}


void
lki_maskcount_exceeded(lua_State * iState, lua_Debug * iArg)
{
        luaL_error(iState, "Instruction count exceeded");
}
