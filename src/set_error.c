#include "internal.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

void
lkonf_reset_error(lkonf_t * iLc)
{
	assert(iLc && "iLc NULL");

	iLc->error_code = LK_OK;
	memset(iLc->error_string, 0, sizeof(iLc->error_string));
}

void
lkonf_set_error(lkonf_t * iLc, lkerr_t iCode, const char * iString)
{
	assert(iLc && "iLc NULL");

	iLc->error_code = iCode;
	snprintf(iLc->error_string, sizeof(iLc->error_string), "%s", iString);
}

void
lkonf_set_error_from_state(lkonf_t * iLc, lkerr_t iCode)
{
	lkonf_set_error(iLc, iCode, lua_tostring(iLc->state, -1));
}
