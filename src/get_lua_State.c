#include "internal.h"

lua_State *
lukonf_get_lua_State(Lukonf * iLc)
{
	if (! iLc) {
		return 0;
	}

	lukonf_reset_error(iLc);
	return iLc->state;
}
