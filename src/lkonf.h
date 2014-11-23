#ifndef LKONF_H
#define LKONF_H 1

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
 * Use Lua as configuration for C applications.
 */

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

#include <lua.h>

	/*
	 * Types.
	 */


/**
 * Opaque type for lkonf_context.
 */
typedef struct lkonf_context_s lkonf_context;


/**
 * Error codes.
 */
typedef enum
{
	LK_OK			= 0,	/** No error. */
	LK_LKONF_NULL		= 1,	/** lkonf_context is NULL. */
	LK_STATE_NULL		= 2,	/** Lua state is NULL. */
	LK_ARG_BAD		= 3,	/** Method argument is bad */
	LK_LOAD_CHUNK		= 4,	/** Can't load chunk. */
	LK_CALL_CHUNK		= 5,	/** Can't call chunk. */
	LK_KEY_BAD		= 6,	/** Key/path-component not a table. */
	LK_VALUE_BAD		= 7,	/** Value of incorrect type. */
	LK_VALUE_NIL		= 8,	/** Value is NIL. */
	LK_MALLOC_FAILURE	= 10,	/** Can't allocate memory. */
} lkerr_t;


/**
 * Array of keys.
 */
typedef const char * const	lkonf_keys[];


	/*
	 * lkonf_context object management.
	 */


/**
 * Construct an lkonf_context.
 * The internal Lua state is created with luaL_newstate().
 * If there was an error constructing the Lua state, the error code will be set.
 * @return lkonf_context created with default parameters, or 0 on failure.
 */
LUA_API lkonf_context *
lkonf_construct(void);


/**
 * Destruct an lkonf_context.
 * @param iLc	lkonf_context to destroy.
 */
LUA_API void
lkonf_destruct(lkonf_context * iLc);


/**
 * Access internal Lua state.
 * @param iLc	lkonf_context to use.
 * @return	Lua state used by iLc. This may be useful for
 *		manipulating the sandbox, adjusting panic functions, etc.
 *		If iLc is 0 or there's a fault in the internal state,
 *		returns 0.
 *
 * @note Closing the Lua state underneath lkonf_context will result in undefined
 * behaviour.
 */
LUA_API lua_State *
lkonf_get_lua_State(lkonf_context * iLc);


	/*
	 * lkonf_context error management.
	 */


/**
 * Error code from most recent lkonf_context operation, if any.
 * @param iLc	lkonf_context to use.
 * @return	Error code, or LK_LKONF_NULL if iLc is 0.
 */
LUA_API lkerr_t
lkonf_get_error_code(lkonf_context * iLc);


/**
 * Error string from most recent lkonf operation, if any.
 * @param iLc	lkonf_context to use.
 * @return	Error string, or 0 if iLc is 0.
 */
LUA_API const char *
lkonf_get_error_string(lkonf_context * iLc);


	/*
	 * Chunk loading.
	 */

/**
 * Load file as a Lua chunk and execute in the sandbox.
 * @param iLc	lkonf_context.
 * @param iFile	Filename
 * @return 	Error code, or LK_OK if ok.
 */
LUA_API lkerr_t
lkonf_load_file(lkonf_context * iLc, const char * iFile);


/**
 * Load string as a Lua chunk and execute in the sandbox.
 * @param iLc		lkonf_context.
 * @param iString	String to load.
 * @return 		Error code, or LK_OK if ok.
 */
LUA_API lkerr_t
lkonf_load_string(lkonf_context * iLc, const char * iString);


	/*
	 * Sandboxing and instruction limits.
	 */

/**
 * Get the current instruction limit.
 * @param iLc	lkonf_context.
 * @return	Instruct limit in iLc. -1 if iLc is 0.
 */
LUA_API int
lkonf_get_instruction_limit(lkonf_context * iLc);


/**
 * Set the instruction limit.
 * @param iLc		lkonf_context.
 * @param iLimit	Limit to set.  0 is no limit.
 * @return 		Error code, or LK_OK if ok.
 */
LUA_API lkerr_t
lkonf_set_instruction_limit(lkonf_context * iLc, const int iLimit);


	/*
	 * Value retrieval.
	 */

/**
 * Get boolean value at iPath.
 * The value at iPath must be either a boolean
 * or a function that returns a boolean when called as function(iPath).
 * @param	iLc	lkonf_context.
 * @param	iPath	String of the form "[table[.table[...]].]key".
 * @param[out]	oValue	Result.
 * @return	Error code, or LK_OK if oValue populated.
 */
LUA_API lkerr_t
lkonf_get_boolean(lkonf_context * iLc, const char * iPath, bool * oValue);

/**
 * Get boolean value at iKeys.
 * The value at iKeys must be either a boolean
 * or a function that returns a boolean when called as function(iKeys[last]).
 * @param	iLc	lkonf_context.
 * @param	iKeys	Keys to traverse; array terminated with NULL key.
 * @param[out]	oValue	Result.
 * @return	Error code, or LK_OK if oValue populated.
 */
LUA_API lkerr_t
lkonf_getkey_boolean(lkonf_context * iLc, lkonf_keys iKeys, bool * oValue);


/**
 * Get double value at iPath.
 * The value at iPath must be either a double
 * or a function that returns a double when called as function(iPath).
 * @param	iLc	lkonf_context.
 * @param	iPath	String of the form "[table[.table[...]].]key".
 * @param[out]	oValue	Result.
 * @return	Error code, or LK_OK if oValue populated.
 */
LUA_API lkerr_t
lkonf_get_double(lkonf_context * iLc, const char * iPath, double * oValue);

/**
 * Get double value at iKeys.
 * The value at iKeys must be either a double
 * or a function that returns a double when called as function(iKeys[last]).
 * @param	iLc	lkonf_context.
 * @param	iKeys	Keys to traverse; array terminated with NULL key.
 * @param[out]	oValue	Result.
 * @return	Error code, or LK_OK if oValue populated.
 */
LUA_API lkerr_t
lkonf_getkey_double(lkonf_context * iLc, lkonf_keys iKeys, double * oValue);


/**
 * Get integer value at iPath.
 * The value at iPath must be either an integer
 * or a function that returns an integer when called as function(iPath).
 * @param	iLc	lkonf_context.
 * @param	iPath	String of the form "[table[.table[...]].]key".
 * @param[out]	oValue	Result.
 * @return	Error code, or LK_OK if oValue populated.
 */
LUA_API lkerr_t
lkonf_get_integer(lkonf_context * iLc, const char * iPath, lua_Integer * oValue);

/**
 * Get integer value at iKeys.
 * The value at iKeys must be either an integer
 * or a function that returns an integer when called as function(iKeys[last]).
 * @param	iLc	lkonf_context.
 * @param	iKeys	Keys to traverse; array terminated with NULL key.
 * @param[out]	oValue	Result.
 * @return	Error code, or LK_OK if oValue populated.
 */
LUA_API lkerr_t
lkonf_getkey_integer(lkonf_context * iLc, lkonf_keys iKeys, lua_Integer * oValue);


/**
 * Get string value at iPath.
 * The value at iPath must be either a string
 * or a function that returns a string when called as function(iPath).
 * Coercion from other types is not supported.
 * @param	iLc	lkonf_context.
 * @param	iPath	String of the form "[table[.table[...]].]key".
 * @param[out]	oValue	Result string. Caller must free if return is LK_OK.
 *			Is nul ('\0') terminated, but may contain nul
 *			characters; use oLen to obtain the full length.
 * @param[out]	oLen	Length of oValue, if oLen is not NULL.
 * @return	Error code, or LK_OK if oValue (and possibly oLen) populated.
 */
LUA_API lkerr_t
lkonf_get_string(
	lkonf_context *	iLc,
	const char *	iPath,
	char **		oValue,
	size_t *	oLen);

/**
 * Get string value at iKeys.
 * The value at iKeys must be either a string
 * or a function that returns a string when called as function(iKeys[last]).
 * Coercion from other types is not supported.
 * @param	iLc	lkonf_context.
 * @param	iKeys	Keys to traverse; array terminated with NULL key.
 * @param[out]	oValue	Result string. Caller must free if return is LK_OK.
 *			Is nul ('\0') terminated, but may contain nul
 *			characters; use oLen to obtain the full length.
 * @param[out]	oLen	Length of oValue, if oLen is not NULL.
 * @return	Error code, or LK_OK if oValue (and possibly oLen) populated.
 */
LUA_API lkerr_t
lkonf_getkey_string(
	lkonf_context *	iLc,
	lkonf_keys	iKeys,
	char **		oValue,
	size_t *	oLen);


/*
 * TODO
 *	- sandbox manipulation
 *	- isFunction()
 *	- helpers to call functions and extract results
 *	- internal protected wrappers for lua_*() per
 *		https://github.com/jmmv/lutok/blob/master/state.cpp
 */

#ifdef  __cplusplus
} /* extern "C" */
#endif

#endif /* LKONF_H */
