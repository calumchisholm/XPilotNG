dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)
AC_INIT(xpilot, 4.5.4X)
AC_CONFIG_SRCDIR(src/client/xpilot.c)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)
dnl AC_CONFIG_AUX_DIR(config)
dnl AC_CANONICAL_HOST

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
AC_HEADER_STDC
AC_HEADER_TIME
dnl AC_HEADER_STDBOOL
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h limits.h memory.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h unistd.h values.h])

dnl  Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
AC_TYPE_MODE_T
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_STRUCT_TM

dnl  Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_SETVBUF_REVERSED
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([alarm floor gethostbyaddr gethostbyname gethostname gettimeofday inet_ntoa isascii memchr memmove memset mkdir pow rint select socket sqrt strcasecmp strchr strdup strerror strncasecmp strpbrk strrchr strspn strstr strtol strtoul])

AC_CONFIG_FILES([Makefile \
	lib/Makefile \
	lib/maps/Makefile \
	lib/textures/Makefile \
	src/Makefile \
	src/common/Makefile \
	src/client/Makefile \
	src/server/Makefile \
	src/replay/Makefile \
	])
AC_OUTPUT

