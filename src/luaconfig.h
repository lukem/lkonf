#ifndef LUACONFIG_H
#define LUACONFIG_H 1

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
 * Opaque type for LuaConfig.
 */
typedef struct LuaConfig LuaConfig;


/*
 * LuaConfig object management.
 */

/**
 * Construct a LuaConfig.
 * @return LuaConfig created with default parameters.
 * The internal lua_State is created with luaL_newstate().
 * If there was an error constructing the lua_State, the error code will be set.
 */
LUA_API LuaConfig *
luaconfig_construct(void);

/**
 * Destruct a LuaConfig.
 * @param iLc	LuaConfig to destroy.
 */
LUA_API void
luaconfig_destruct(LuaConfig * iLc);

/**
 * Access internal lua_State.
 * @param iLc	LuaConfig to use.
 * @return	lua_State used by LuaConfig. This may be useful for
 *		manipulating the sandbox, adjusting panic functions, etc.
 *		If iLc is 0 or there's a fault in the internal state,
 *		returns 0.
 *
 * @note Closing the lua_State underneath LuaConfig will result in undefined
 * behaviour.
 */
LUA_API lua_State *
luaconfig_get_lua_State(LuaConfig * iLc);


/*
 * LuaConfig error management.
 */

/**
 * Error code from most recent LuaConfig operation, if any.
 * @param iLc	LuaConfig to use.
 * @return Integer value. 0 is no error. ~0 is iLc is 0.
 * @todo provide enum or #defines for error catalog?
 */
LUA_API int
luaconfig_get_error_code(LuaConfig * iLc);

/**
 * Error string from most recent LuaConfig operation, if any.
 * @param iLc	LuaConfig to use.
 * @return Error string. 0 is no error, or iLc is 0.
 */
LUA_API const char *
luaconfig_get_error_string(LuaConfig * iLc);


/*
 * TODO
 *	- loadfile
 *	- loadstring
 *	- instruction limits
 *	- sandbox manipulation
 *	- get_TYPE.  api variations
 *		- "path" or { "key", "key", ...} variations
 *		- optional (with default).  or always?
 * 	- return internal lua_State
 *	- isFunction()
 *	- helpers to call functions and extract results
 *	- stack cleaner?
 *	- path walker
 */

#endif /* LUACONFIG_H */
