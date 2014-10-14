#include "internal.h"

const char *
luaconfig_get_error_string(LuaConfig * iLc)
{
	if (! iLc) {
		return 0;
	}

	return iLc->error_string;
}
