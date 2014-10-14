#include "internal.h"

#include <stdlib.h>

void
luaconfig_destruct(LuaConfig * iLc)
{
	if (! iLc) {
		return;
	}

	if (iLc->state) {
		lua_close(iLc->state);
		iLc->state = 0;
	}

	free(iLc);
}
