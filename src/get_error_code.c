#include "internal.h"

int
luaconfig_get_error_code(LuaConfig * iLc)
{
	if (! iLc) {
		return ~0;
	}

	return iLc->error_code;
}
