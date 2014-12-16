#include "internal.h"

#include <assert.h>
#include <lauxlib.h>
#include <stdio.h>

lkonf_error
lki_format_keys(
	lkonf_context *	iLc,
	lkonf_keys	iKeys,
	size_t		iMaxKeys)
{
	if (! iLc) {
		return LK_INVALID_ARGUMENT;
	}

	luaL_Buffer lb;
	luaL_buffinit(iLc->state, &lb);

	size_t ki;
	for (ki = 0; iKeys[ki]; ++ki) {
		if (iMaxKeys && ki >= iMaxKeys) {
			break;
		}
		if (ki) {
			luaL_addstring(&lb, ".");
		}
		luaL_addstring(&lb, "\"");
		luaL_addstring(&lb, iKeys[ki]);
		luaL_addstring(&lb, "\"");
	}

	luaL_pushresult(&lb);

	return LK_OK;
}
