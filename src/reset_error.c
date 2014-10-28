#include "internal.h"

#include <assert.h>
#include <string.h>

void
lukonf_reset_error(Lukonf * iLc)
{
	assert(iLc && "iLc NULL");

	iLc->error_code = 0;
	memset(iLc->error_string, 0, sizeof(iLc->error_string));
}
