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
 * lkonf_t internals.
 * Do not include directly.
 */ 

#include <lkonf.h>


/**
 * lkonf_t object.
 */
struct lkonf_s
{
	lua_State *	state;
	lkerr_t		error_code;
	char		error_string[128];
	int		depth;
};


/**
 * Reset the lkonf_t error state.
 * @warning Asserts that iLc is not 0.
 */
void
lkonf_reset_error(lkonf_t * iLc);

/**
 * Set the lkonf_t error state and string.
 * @warning Asserts that iLc is not 0.
 */
void
lkonf_set_error(lkonf_t * iLc, lkerr_t iCode, const char * iString);

/**
 * Set the lkonf_t error state and error string at top of the Lua stack.
 * @warning Asserts that iLc is not 0.
 */
void
lkonf_set_error_from_state(lkonf_t * iLc, lkerr_t iCode);


/**
 * State entry validation.
 * Call on entry to public methods to ensure Lua state is defined
 * and to remember Lua stack position.
 * The Lua stack will be reset to this point by lkonf_state_exit().
 * Also resets the error state if ok.
 * @returns Error state if not ok.
 * @warning Asserts that iLc is not 0.
 */
lkerr_t
lkonf_state_entry(lkonf_t * iLc);

/**
 * State exit validation and cleanup.
 * Call on exit from public methods that have called lkonf_state_entry().
 * Returns the current error code of the iLc.
 * @warning Asserts that iLc is not 0.
 * @warning Asserts that the Lua stack hasn't gone below the depth.
 */
lkerr_t
lkonf_state_exit(lkonf_t * iLc);


/**
 * Call chunk at top of stack.
 * If there's an error the iLc error state will be setup.
 * @param iLc		Context.
 * @param iNumArgs	Number of arguments.
 * @param iNumRes	Number of results.
 * @todo sandbox
 */
lkerr_t
lkonf_call_chunk(lkonf_t * iLc, const int iNumArgs, const int iNumRes);

#endif /* LKONF_INTERNAL_H */
