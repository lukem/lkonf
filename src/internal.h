#ifndef LKONF_INTERNAL_H
#define LKONF_INTERNAL_H 1

/*
 * Copyright (c) 2014 Luke Mewburn <Luke@Mewburn.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * lkonf_context internals.
 * Do not include directly.
 */

#include <lkonf.h>


/**
 * lkonf_context implementation object.
 */
struct lkonf_context_s
{
	/**
	 * Lua state.
	 */
	lua_State *	state;

	/**
	 * Current error code.
	 */
	lkonf_error	error_code;

	/**
	 * Current error string.
	 * Empty if no error.
	 */
	char		error_string[128];

	/**
	 * Instruction limit (mask count).
	 */
	int		instruction_limit;

	/**
	 * Lua state stack depth.
	 * Use by lki_state_entry() and lki_state_exit() during
	 * external API calls that modify the stack, to ensure
	 * that the stack is reset to its previous state.
	 */
	int		depth;
};


/**
 * Reset the lkonf_context error code and clear the error string.
 * @param iLc Context.
 * @warning Asserts that iLc is not 0.
 */
LUA_API void
lki_reset_error(lkonf_context * iLc);

/**
 * Set the lkonf_context error code and string.
 * @param iLc Context.
 * @param iCode Error code.
 * @param iString Error string.
 * @return iCode.
 * @warning Asserts that iLc is not 0.
 */
LUA_API lkonf_error
lki_set_error(lkonf_context * iLc, lkonf_error iCode, const char * iString);

/**
 * Set the lkonf_context error code and string.
 * Error string is of the form: iString + ": " + iItem.
 * @param iLc Context.
 * @param iCode Error code.
 * @param iString Error string.
 * @param iItem Item text.
 * @return iCode.
 * @warning Asserts that iLc is not 0.
 */
LUA_API lkonf_error
lki_set_error_item(
	lkonf_context *	iLc,
	lkonf_error	iCode,
	const char *	iString,
	const char *	iItem);

/**
 * Set the lkonf_context error code and string.
 * Error string is of the form: iString + ": " + lki_format_keys(iKeys).
 * @param iLc Context.
 * @param iCode Error code.
 * @param iString Error string.
 * @param iKeys Keys to format.
 * @return iCode.
 * @warning Asserts that iLc is not 0.
 */
LUA_API lkonf_error
lki_set_error_keys(
	lkonf_context *	iLc,
	lkonf_error	iCode,
	const char *	iString,
	lkonf_keys	iKeys);

/**
 * Set the error code to iCode and error string to the string at Lua stack top.
 * @param iLc Context.
 * @param iCode Error code.
 * @return iCode.
 * @warning Asserts that iLc is not 0.
 */
LUA_API lkonf_error
lki_set_error_from_state(lkonf_context * iLc, lkonf_error iCode);


/**
 * State entry validation and setup.
 * Call on entry to public methods to ensure Lua state is defined
 * and to remember Lua stack position.
 * The Lua stack will be reset to this point by lkonf_state_exit().
 * Also resets the error state if ok.
 * @param iLc Context.
 * @return Error code.
 */
LUA_API lkonf_error
lki_state_entry(lkonf_context * iLc);

/**
 * State exit validation and cleanup.
 * Call on exit from public methods that have called lkonf_state_entry().
 * @param iLc Context.
 * @return Current error code of iLc.
 * @warning Asserts that the Lua stack hasn't gone below the depth.
 */
LUA_API lkonf_error
lki_state_exit(lkonf_context * iLc);


/**
 * Call chunk at top of stack.
 * If there's an error the iLc error state will be setup.
 * @warning Asserts that iLc and iLc->state are not 0.
 * @param iLc		Context.
 * @param iNumArgs	Number of arguments.
 * @param iNumResults	Number of results.
 * @return Error code.
 * @todo sandbox
 */
LUA_API lkonf_error
lki_call_chunk(lkonf_context * iLc, const int iNumArgs, const int iNumResults);

/**
 * Raise lua error for maskcount exceeded.
 */
LUA_API void
lki_maskcount_exceeded(lua_State * iState, lua_Debug * iArg);


/**
 * Find table by path.
 * Sets error state appropriately.
 * @param iLc Context.
 * @param iPath Path of "."-separated table keys to traverse.
 * @return Error code.
 * @todo document possible error codes and strings?
 */
LUA_API lkonf_error
lki_find_table_by_path(lkonf_context * iLc, const char * iPath);

/**
 * Find table by keys.
 * Sets error state appropriately.
 * @param iLc		Context.
 * @param iKeys		Table keys to traverse.
 * @param[out] oMatch	If not NULL, set to last index processed if LK_OK.
 *			Unchanged on error.
 * @return Error code.
 * @todo document possible error codes and strings?
 */
LUA_API lkonf_error
lki_find_table_by_keys(lkonf_context * iLc, lkonf_keys iKeys, size_t * oMatch);

/**
 * Format iKeys as a human-readable string.
 * @param iKeys		Keys to format.
 * @param iMaxKeys	Maximum number of keys in iKeys to format, if > 0.
 *			If 0, all keys are formatted.
 * @param iBuffer	Buffer to format into.
 * @param iBufSize	Size of buffer.
 */
LUA_API void
lki_format_keys(
	lkonf_keys	iKeys,
	size_t		iMaxKeys,
	char *		iBuffer,
	size_t		iBufSize);


#endif /* LKONF_INTERNAL_H */
