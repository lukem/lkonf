AM_CFLAGS = $(WARNCFLAGS)

include_HEADERS = \
	lkonf.h

noinst_HEADERS = \
	internal.h


lib_LTLIBRARIES = liblkonf.la

liblkonf_la_SOURCES = \
	construct.c \
	destruct.c \
	get_error_code.c \
	get_error_string.c \
	get_instruction_limit.c \
	get_integer.c \
	get_lua_State.c \
	get_string.c \
	load_file.c \
	load_string.c \
	set_instruction_limit.c

liblkonf_la_SOURCES += \
	call_chunk.c \
	find_table.c \
	set_error.c \
	state_entry_exit.c

liblkonf_la_CPPFLAGS = \
	-I$(srcdir) \
	$(LUA_CFLAGS)

liblkonf_la_LDFLAGS = \
	-version-info 0:0:0 \
	$(LUA_LIBS)
