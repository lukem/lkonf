#include "internal.h"

int
lkonf_get_instruction_limit(lkonf_context * iLc)
{
	if (! iLc) {
		return -1;
	}

	return iLc->instruction_limit;
}
