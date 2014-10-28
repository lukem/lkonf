#include "internal.h"

#include <assert.h>
#include <string.h>

void
lkonf_reset_error(lkonf_t * iLc)
{
	assert(iLc && "iLc NULL");

	iLc->error_code = LK_OK;
	memset(iLc->error_string, 0, sizeof(iLc->error_string));
}
