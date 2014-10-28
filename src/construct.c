#include "internal.h"

#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>

lkonf_t *
lkonf_construct(void)
{
	lkonf_t * lc = 0;

	lc = calloc(1, sizeof(*lc));
	if (! lc) {
		return 0;
	}

	lkonf_reset_error(lc);

	lc->state = luaL_newstate();
	if (! lc->state) {
		lc->error_code = LK_NO_LUA_STATE;
		snprintf(
			lc->error_string, sizeof(lc->error_string),
			"Can't allocate lua state");
	}

	return lc;
}
