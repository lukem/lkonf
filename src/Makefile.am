AM_CFLAGS = $(WARNCFLAGS)

include_HEADERS = \
	luaconfig.h

noinst_HEADERS = \
	luaconfig_internal.h


noinst_LTLIBRARIES = libluaconfig.la

libluaconfig_la_SOURCES = \
	luaconfig_construct.c \
	luaconfig_destruct.c

libluaconfig_la_CPPFLAGS = \
	-I$(srcdir) \
	$(LUA_CFLAGS)

libluaconfig_la_LDFLAGS = \
	$(LUA_LIBS)
