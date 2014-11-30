#include <lkonf.h>

const char *
lkonf_error_to_string(const lkonf_error iError)
{
	switch (iError) {
		case LK_OK:			return "Ok";
		case LK_INVALID_ARGUMENT:	return "Invalid argument";
		case LK_LUA_ERROR:		return "Lua error";
		case LK_NOT_FOUND:		return "Not found";
		case LK_RESOURCE_EXHAUSTED:	return "Resource exhausted";
		case LK_OUT_OF_RANGE:		return "Out of range";
	}

	return "";
}
