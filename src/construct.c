#include "internal.h"

#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>

LuaConfig *
luaconfig_construct(void)
{
	LuaConfig * lc = 0;

	lc = calloc(1, sizeof(*lc));
	if (! lc) {
		return 0;
	}

	lc->state = luaL_newstate();
	if (! lc->state) {
		lc->error_code = -1;
		snprintf(
			lc->error_string, sizeof(lc->error_string),
			"Can't allocate lua state");
	}

	luaconfig_reset_error(lc);

	return lc;
}
