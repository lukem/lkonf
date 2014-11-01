AM_CFLAGS = $(WARNCFLAGS)

include_HEADERS = \
	lkonf.h

noinst_HEADERS = \
	internal.h


lib_LTLIBRARIES = liblkonf.la

liblkonf_la_SOURCES = \
	call_chunk.c \
	construct.c \
	destruct.c \
	get_error_code.c \
	get_error_string.c \
	get_instruction_limit.c \
	get_lua_State.c \
	load_file.c \
	load_string.c \
	set_error.c \
	set_instruction_limit.c \
	state_entry_exit.c

liblkonf_la_CPPFLAGS = \
	-I$(srcdir) \
	$(LUA_CFLAGS)

liblkonf_la_LDFLAGS = \
	-version-info 0:0:0 \
	$(LUA_LIBS)
