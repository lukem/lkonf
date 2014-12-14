#include "internal.h"

#include <assert.h>
#include <stdio.h>

void
lki_format_keys(
	lkonf_keys	iKeys,
	size_t		iMaxKeys,
	char *		iBuffer,
	size_t		iBufSize)
{
	assert(iBuffer);
	assert(iBufSize);

// TODO rework to format with truncated path at start not end?

	const char * end = iBuffer + iBufSize;
	size_t ki = 0;
	for (ki = 0; iKeys[ki]; ++ki) {
		if (iMaxKeys && ki >= iMaxKeys) {
			break;
		}
		if (ki) {
			iBuffer += snprintf(iBuffer, end-iBuffer, ".");
			if (iBuffer >= end)
				break;
		}
// TODO if a 'simple' key, don't wrap in ""
		iBuffer += snprintf(iBuffer, end-iBuffer, "\"%s\"", iKeys[ki]);
		if (iBuffer >= end)
			break;
	}
}
