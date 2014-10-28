#ifndef lukonf_H
#define lukonf_H 1

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
 * Opaque type for Lukonf.
 */
typedef struct Lukonf Lukonf;


/*
 * Lukonf object management.
 */

/**
 * Construct a Lukonf.
 * @return Lukonf created with default parameters.
 * The internal lua_State is created with luaL_newstate().
 * If there was an error constructing the lua_State, the error code will be set.
 */
LUA_API Lukonf *
lukonf_construct(void);

/**
 * Destruct a Lukonf.
 * @param iLc	Lukonf to destroy.
 */
LUA_API void
lukonf_destruct(Lukonf * iLc);

/**
 * Access internal lua_State.
 * @param iLc	Lukonf to use.
 * @return	lua_State used by Lukonf. This may be useful for
 *		manipulating the sandbox, adjusting panic functions, etc.
 *		If iLc is 0 or there's a fault in the internal state,
 *		returns 0.
 *
 * @note Closing the lua_State underneath Lukonf will result in undefined
 * behaviour.
 */
LUA_API lua_State *
lukonf_get_lua_State(Lukonf * iLc);


/*
 * Lukonf error management.
 */

/**
 * Error code from most recent Lukonf operation, if any.
 * @param iLc	Lukonf to use.
 * @return Integer value. 0 is no error. ~0 is iLc is 0.
 * @todo provide enum or #defines for error catalog?
 */
LUA_API int
lukonf_get_error_code(Lukonf * iLc);

/**
 * Error string from most recent Lukonf operation, if any.
 * @param iLc	Lukonf to use.
 * @return Error string. 0 is no error, or iLc is 0.
 */
LUA_API const char *
lukonf_get_error_string(Lukonf * iLc);


/*
 * Chunk loading.
 */

/**
 * Load file as a Lua chunk and execute in the sandbox.
 * @param iLc	Lukonf.
 * @param iFile	Filename
 * @return 	Error code.
 * @todo evaluate if ok.
 */
LUA_API int
lukonf_load_file(Lukonf * iLc, const char * iFile);

/**
 * Load string as a Lua chunk and execute in the sandbox.
 * @param iLc		Lukonf.
 * @param iString	String to load.
 * @return		Error code.
 * @todo evaluate if ok.
 */
LUA_API int
lukonf_load_string(Lukonf * iLc, const char * iString);


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

#endif /* lukonf_H */
