dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(XPilot, 4.5.4X2, ksoderbl@cc.hut.fi, xpilot)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(src/client/xpilot.c)
AM_INIT_AUTOMAKE

AM_CONFIG_HEADER(config.h)
dnl AC_CANONICAL_HOST


dnl  Checks for options.
dnl  We should have some general macro for these.
AC_ARG_ENABLE(dbe,
	AC_HELP_STRING([--enable-dbe],
			[support for X Doublebuffer Extension in client]),
[if test $enable_dbe = yes; then
	AC_DEFINE([DBE], 1, [support for X Doublebuffer Extension in client])
fi])

AC_ARG_ENABLE(mbx,
	AC_HELP_STRING([--enable-mbx],
			[support for X Multibuffer Extension in client]),
[if test $enable_dbe = yes; then
	AC_DEFINE([MBX], 1, [support for X Multibuffer Extension in client])
fi])

AC_ARG_ENABLE(log,
	AC_HELP_STRING([--enable-log],
			[Try to log every server start to log file]),
[if test $enable_log = yes; then
	AC_DEFINE([LOG], 1, [Try to log every server start to log file])
fi])

AC_ARG_ENABLE(silent,
	AC_HELP_STRING([--enable-silent],
			[Makes the server real silent]),
[if test $enable_silent = yes; then
	AC_DEFINE([SILENT], 1, [Makes the server real silent])
fi])

AC_ARG_ENABLE(serversound,
	AC_HELP_STRING([--enable-serversound],
			[Turns on sound support in the server (not client)]),
[if test $enable_serversound = yes; then
	AC_DEFINE([SERVER_SOUND], 1, [Turns on sound support in the server])
fi])

AC_ARG_ENABLE(plockserver,
	AC_HELP_STRING([--enable-plockserver],
			[lock the xpilot server (xpilots) into memory]),
[if test $enable_plockserver = yes; then
	AC_DEFINE([PLOCKSERVER], 1, [lock the xpilot server (xpilots) into memory])
fi])

AC_ARG_ENABLE(development,
	AC_HELP_STRING([--enable-development],
			[enable development code]),
[if test $enable_development = yes; then
	AC_DEFINE([DEVELOPMENT], 1, [enable development code])
fi])


dnl  Checks for programs.
AC_PROG_CC
dnl AC_PROG_GCC_TRADITIONAL
dnl AC_PROG_CPP
dnl AC_PROG_CXX
dnl AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_INSTALL

dnl  Checks for libraries.
AC_CHECK_LIB(m, cos)
AC_CHECK_LIB(z, gzopen)
AC_CHECK_LIB(expat, XML_ParserCreate)
AC_PATH_X
AC_PATH_XTRA

dnl  Checks for header files.
dnl AC_HEADER_STDC
AC_HEADER_TIME
dnl AC_HEADER_STDBOOL
AC_CHECK_HEADERS([ \
	arpa/inet.h \
	arpa/nameser.h \
	assert.h \
	bstring.h \
	ctype.h \
	errno.h \
	fcntl.h \
	float.h \
	limits.h \
	math.h \
	net/if.h \
	netdb.h \
	netinet/in.h \
	netinet/tcp.h \
	pwd.h \
	resolv.h \
	setjmp.h \
	signal.h \
	stdarg.h \
	stddef.h \
	sys/file.h \
	sys/filio.h \
	sys/fcntl.h \
	sys/inttypes.h \
	sys/ioctl.h \
	sys/param.h \
	sys/socket.h \
	sys/sockio.h \
	sys/time.h \
	values.h \
	])

dnl  Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
AC_TYPE_MODE_T
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_STRUCT_TM

dnl Check for socklen_t (in Unix98)
AC_MSG_CHECKING(for socklen_t)
AC_TRY_COMPILE([
	#include <sys/types.h>
	#include <sys/socket.h>
	socklen_t x;
], [],
[
	AC_MSG_RESULT(yes)
], [
	AC_TRY_COMPILE([
		#include <sys/types.h>
		#include <sys/socket.h>
		int accept(int, struct sockaddr *, size_t *);
	], [], [
		AC_MSG_RESULT(size_t)
		AC_DEFINE(socklen_t, size_t, [socklen_t size])
	], [
		AC_MSG_RESULT(int)
		AC_DEFINE(socklen_t, int, [socklen_t size])
	])
])

dnl  Checks for library functions.

dnl We probably don't need GNU compatible malloc and realloc functions,
dnl so these are disabled for now.
dnl AC_FUNC_MALLOC
dnl AC_FUNC_REALLOC

AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_SETVBUF_REVERSED
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([ \
	alarm \
	floor \
	gethostbyaddr \
	gethostbyname \
	gethostname \
	gettimeofday \
	inet_ntoa \
	isascii \
	memchr \
	memmove \
	memset \
	mkdir \
	pow \
	rint \
	select \
	socket \
	sqrt \
	strcasecmp \
	strchr \
	strdup \
	strerror \
	strncasecmp \
	strpbrk \
	strrchr \
	strspn \
	strstr \
	strtol \
	strtoul \
	])

AC_CONFIG_FILES([ \
	Makefile \
	lib/Makefile \
	lib/maps/Makefile \
	lib/textures/Makefile \
	src/Makefile \
	src/common/Makefile \
	src/client/Makefile \
	src/client/items/Makefile \
	src/server/Makefile \
	src/replay/Makefile \
	src/replay/tools/Makefile \
	])
AC_OUTPUT

