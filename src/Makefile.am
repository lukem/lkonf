AM_CFLAGS = $(WARNCFLAGS)

include_HEADERS = \
	lukonf.h

noinst_HEADERS = \
	internal.h


lib_LTLIBRARIES = liblukonf.la

liblukonf_la_SOURCES = \
	construct.c \
	destruct.c \
	get_error_code.c \
	get_error_string.c \
	get_lua_State.c \
	reset_error.c

liblukonf_la_CPPFLAGS = \
	-I$(srcdir) \
	$(LUA_CFLAGS)

liblukonf_la_LDFLAGS = \
	-version-info 0:0:0 \
	$(LUA_LIBS)
