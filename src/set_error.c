#include "internal.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

void
lki_reset_error(lkonf_context * iLc)
{
	assert(iLc && "iLc NULL");

	iLc->error_code = LK_OK;
	memset(iLc->error_string, 0, sizeof(iLc->error_string));
}

lkonf_error
lki_set_error(
	lkonf_context *		iLc,
	const lkonf_error	iCode,
	const char *		iString)
{
	assert(iLc && "iLc NULL");

	iLc->error_code = iCode;
	snprintf(iLc->error_string, sizeof(iLc->error_string), "%s", iString);

	return iCode;
}

lkonf_error
lki_set_error_item(
	lkonf_context *		iLc,
	const lkonf_error	iCode,
	const char *		iString,
	const char *		iItem)
{
	assert(iLc && "iLc NULL");

	iLc->error_code = iCode;
	snprintf(iLc->error_string, sizeof(iLc->error_string),
		"%s: %s", iString, iItem);

	return iCode;
}

lkonf_error
lki_set_error_keys(
	lkonf_context *		iLc,
	const lkonf_error	iCode,
	const char *		iString,
	lkonf_keys		iKeys,
	size_t			iMaxKeys)
{
	assert(iLc && "iLc NULL");

		/* push formatted keys onto stack */
	iLc->error_code = lki_format_keys(iLc, iKeys, iMaxKeys);
	if (LK_OK != iLc->error_code)
		return iLc->error_code;

		/* format error */
	iLc->error_code = iCode;
	snprintf(iLc->error_string, sizeof(iLc->error_string),
		"%s: %s", iString, lua_tostring(iLc->state, -1));

		/* pop formatted keys off stack */
	lua_pop(iLc->state, 1);

	return iCode;
}

lkonf_error
lki_set_error_from_state(lkonf_context * iLc, lkonf_error iCode)
{
	return lki_set_error(iLc, iCode, lua_tostring(iLc->state, -1));
}
