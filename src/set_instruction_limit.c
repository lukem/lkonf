#include "internal.h"

lkonf_error
lkonf_set_instruction_limit(lkonf_context * iLc, const int iLimit)
{
	if (! iLc) {
		return LK_INVALID_ARGUMENT;
	}

	lki_reset_error(iLc);

	if (iLimit < 0) {
		return lki_set_error(iLc, LK_INVALID_ARGUMENT, "iLimit < 0");
	}

	iLc->instruction_limit = iLimit;

	return LK_OK;
}
