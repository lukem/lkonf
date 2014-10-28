#include "internal.h"

#include <stdlib.h>

void
lukonf_destruct(Lukonf * iLc)
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
