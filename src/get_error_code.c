#include "internal.h"

lkerr_t
lkonf_get_error_code(lkonf_t * iLc)
{
	if (! iLc) {
		return ~0;
	}

	return iLc->error_code;
}
