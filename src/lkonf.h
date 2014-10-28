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

#include <lua.h>

/*
 * Types.
 */

/**
 * Opaque type for lkonf_t.
 */
typedef struct lkonf_s lkonf_t;

/**
 * Error codes.
 */
typedef enum
{
	LK_OK		= 0,	/** No error */
} lkerr_t;


/*
 * lkonf_t object management.
 */

/**
 * Construct an lkonf_t.
 * @return lkonf_t created with default parameters.
 * The internal lua_State is created with luaL_newstate().
 * If there was an error constructing the lua_State, the error code will be set.
 */
LUA_API lkonf_t *
lkonf_construct(void);

/**
 * Destruct an lkonf_t.
 * @param iLc	lkonf_t to destroy.
 */
LUA_API void
lkonf_destruct(lkonf_t * iLc);

/**
 * Access internal lua_State.
 * @param iLc	lkonf_t to use.
 * @return	lua_State used by lkonf_t. This may be useful for
 *		manipulating the sandbox, adjusting panic functions, etc.
 *		If iLc is 0 or there's a fault in the internal state,
 *		returns 0.
 *
 * @note Closing the lua_State underneath lkonf_t will result in undefined
 * behaviour.
 */
LUA_API lua_State *
lkonf_get_lua_State(lkonf_t * iLc);


/*
 * lkonf_t error management.
 */

/**
 * Error code from most recent lkonf_t operation, if any.
 * @param iLc	lkonf_t to use.
 * @return Error code.
 */
LUA_API lkerr_t
lkonf_get_error_code(lkonf_t * iLc);

/**
 * Error string from most recent lkonf operation, if any.
 * @param iLc	lkonf_t to use.
 * @return Error string. 0 is no error, or iLc is 0.
 */
LUA_API const char *
lkonf_get_error_string(lkonf_t * iLc);


/*
 * Chunk loading.
 */

/**
 * Load file as a Lua chunk and execute in the sandbox.
 * @param iLc	lkonf_t.
 * @param iFile	Filename
 * @return 	Error code.
 * @todo evaluate if ok.
 */
LUA_API lkerr_t
lkonf_load_file(lkonf_t * iLc, const char * iFile);

/**
 * Load string as a Lua chunk and execute in the sandbox.
 * @param iLc		lkonf_t.
 * @param iString	String to load.
 * @return		Error code.
 * @todo evaluate if ok.
 */
LUA_API lkerr_t
lkonf_load_string(lkonf_t * iLc, const char * iString);


/*
 * TODO
 *	- enum for error_code ?
 *	- loadfile
 *	- loadstring
 *	- instruction limits
 *	- sandbox manipulation
 *	- get_TYPE.  api variations
 *		- "path" or { "key", "key", ...} variations
 *		- optional (with default).  or always?
 *	- isFunction()
 *	- helpers to call functions and extract results
 *	- stack cleaner?
 *	- path walker
 *	- internal protected wrappers for lua_*() per
 *		https://github.com/jmmv/lutok/blob/master/state.cpp
 */

#endif /* LKONF_H */
