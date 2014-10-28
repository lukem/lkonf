#include "internal.h"

#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>

Lukonf *
lukonf_construct(void)
{
	Lukonf * lc = 0;

	lc = calloc(1, sizeof(*lc));
	if (! lc) {
		return 0;
	}

	lukonf_reset_error(lc);

	lc->state = luaL_newstate();
	if (! lc->state) {
		lc->error_code = -1;
		snprintf(
			lc->error_string, sizeof(lc->error_string),
			"Can't allocate lua state");
	}

	return lc;
}
