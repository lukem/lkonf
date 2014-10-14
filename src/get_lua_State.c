#include "internal.h"

lua_State *
luaconfig_get_lua_State(LuaConfig * iLc)
{
	if (! iLc) {
		return 0;
	}

	luaconfig_reset_error(iLc);
	return iLc->state;
}
