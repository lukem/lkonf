#include "internal.h"

#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>

lkonf_context *
lkonf_construct(void)
{
	lkonf_context * lc = 0;

	lc = calloc(1, sizeof(*lc));
	if (! lc) {
		return 0;
	}
// TODO set default lc->instruction_limit ?

	lki_reset_error(lc);

	lc->state = luaL_newstate();
	if (! lc->state) {
		lc->error_code = LK_STATE_NULL;
		snprintf(
			lc->error_string, sizeof(lc->error_string),
			"Can't allocate lua state");
	}

	return lc;
}
