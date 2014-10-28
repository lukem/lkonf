#include "internal.h"

int
lukonf_get_error_code(Lukonf * iLc)
{
	if (! iLc) {
		return ~0;
	}

	return iLc->error_code;
}
