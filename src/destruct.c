#include "internal.h"

#include <stdlib.h>

void
lkonf_destruct(lkonf_context * iLc)
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
