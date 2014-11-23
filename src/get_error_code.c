#include "internal.h"

lkerr_t
lkonf_get_error_code(lkonf_context * iLc)
{
	if (! iLc) {
		return LK_LKONF_NULL;
	}

	return iLc->error_code;
}
