#include "internal.h"

#include <assert.h>
#include <string.h>

void
luaconfig_reset_error(LuaConfig * iLc)
{
	assert(iLc && "iLc NULL");

	iLc->error_code = 0;
	memset(iLc->error_string, 0, sizeof(iLc->error_string));
}
