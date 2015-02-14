#include "internal.h"

lkonf_error
lkonf_copy_table_keys(
	lkonf_context * iLc,
	const int	iSrcIdx,
	const int	iDstIdx,
	lkonf_keys	iKeys)
{
	if (! iLc) {
		return LK_INVALID_ARGUMENT;
	}

	if (LK_OK != lki_state_entry(iLc)) {
		return lki_state_exit(iLc);
	}

	if (! iKeys) {
		return lki_set_error(iLc, LK_INVALID_ARGUMENT, "iKeys NULL");
	}

	if (! lua_istable(iLc->state, iSrcIdx)) {
		return lki_set_error(iLc,
			LK_OUT_OF_RANGE, "iSrcIdx not a table");
	}

	if (! lua_istable(iLc->state, iDstIdx)) {
		return lki_set_error(iLc,
			LK_OUT_OF_RANGE, "iDstIdx not a table");
	}

	size_t ki;
	for (ki = 1; 0 != iKeys[ki]; ++ki) {
		lua_getfield(iLc->state, iSrcIdx, iKeys[ki]);
		lua_setfield(iLc->state, iDstIdx-1, iKeys[ki]);
	}

	return lki_state_exit(iLc);

}
