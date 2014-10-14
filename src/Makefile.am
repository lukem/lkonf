AM_CFLAGS = $(WARNCFLAGS)

include_HEADERS = \
	luaconfig.h

noinst_HEADERS = \
	internal.h


noinst_LTLIBRARIES = libluaconfig.la

libluaconfig_la_SOURCES = \
	construct.c \
	destruct.c

libluaconfig_la_CPPFLAGS = \
	-I$(srcdir) \
	$(LUA_CFLAGS)

libluaconfig_la_LDFLAGS = \
	$(LUA_LIBS)
