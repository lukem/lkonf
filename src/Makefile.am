AM_CFLAGS = $(WARNCFLAGS)

include_HEADERS = \
	lkonf.h

noinst_HEADERS = \
	internal.h


lib_LTLIBRARIES = liblkonf.la

liblkonf_la_SOURCES = \
	construct.c \
	copy_table_keys.c \
	destruct.c \
	error_to_string.c \
	get_boolean.c \
	get_double.c \
	get_error_code.c \
	get_error_string.c \
	get_instruction_limit.c \
	get_integer.c \
	get_lua_State.c \
	get_string.c \
	getkey_boolean.c \
	getkey_double.c \
	getkey_integer.c \
	getkey_string.c \
	load_file.c \
	load_string.c \
	set_instruction_limit.c

liblkonf_la_SOURCES += \
	call_chunk.c \
	find_table_by_keys.c \
	find_table_by_path.c \
	format_keys.c \
	set_error.c \
	state_entry_exit.c

liblkonf_la_CPPFLAGS = \
	-I$(srcdir) \
	$(LUA_CFLAGS)

liblkonf_la_LDFLAGS = \
	-version-info 0:0:0 \
	$(LUA_LIBS)

man3_MANS = \
	lkonf.3

EXTRA_DIST = \
	$(man3_MANS)
