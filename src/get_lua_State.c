#include "internal.h"

lua_State *
lkonf_get_lua_State(lkonf_context * iLc)
{
	if (! iLc) {
		return 0;
	}

	lki_reset_error(iLc);
	return iLc->state;
}
