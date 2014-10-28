#include "internal.h"

const char *
lukonf_get_error_string(Lukonf * iLc)
{
	if (! iLc) {
		return 0;
	}

	return iLc->error_string;
}
