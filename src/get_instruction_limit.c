#include "internal.h"

int
lkonf_get_instruction_limit(lkonf_t * iLc)
{
	if (! iLc) {
		return -1;
	}

	return iLc->instruction_limit;
}
