#include "internal.h"

const char *
lkonf_get_error_string(lkonf_t * iLc)
{
	if (! iLc) {
		return 0;
	}

	return iLc->error_string;
}
