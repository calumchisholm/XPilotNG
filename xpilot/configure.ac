dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(XPilot, 4.5.4X2, kps@users.sourceforge.net, xpilot)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(src/client/xpilot.c)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AM_CONFIG_HEADER(config.h)

dnl  Copyright & stuff.
AC_SUBST([XP_AUTHORS],["Bjørn Stabell, Ken Ronny Schouten, Bert Gijsbers, Dick Balaska, Uoti Urpala, Juha Lindström, Kristian Söderblom and Erik Andersson"])
AC_SUBST([XP_COPYRIGHTDATE],["1991-2003"])
AC_SUBST([XP_COPYRIGHT],["Copyright © 1991-2003 by Bjørn Stabell, Ken Ronny Schouten, Bert Gijsbers, Dick Balaska, Uoti Urpala, Juha Lindström, Kristian Söderblom and Erik Andersson"])
AC_SUBST([XP_RELEASEDATE],["Sep 4, 2003"])
AC_SUBST([XP_INSTLIBDIR],$prefix/lib/xpilot)
AC_SUBST([XP_TITLE],["XPilot 4.5.4X2"])
AC_SUBST([XP_WINVERSION],["4.5.4X2-NT13"])
AC_SUBST([XP_VERSION_WINDOWS],["13"])


dnl  Checks for options.
dnl  We should have some general macro for these.
AC_ARG_ENABLE(dbe,
	AC_HELP_STRING([--enable-dbe],
			[support for X Doublebuffer Extension in client]),
[if test x$enable_dbe = xyes; then
	AC_DEFINE([DBE], 1, [support for X Doublebuffer Extension in client])
fi])

AC_ARG_ENABLE(mbx,
	AC_HELP_STRING([--enable-mbx],
			[support for X Multibuffer Extension in client]),
[if test x$enable_mbx = xyes; then
	AC_DEFINE([MBX], 1, [support for X Multibuffer Extension in client])
fi])

AC_ARG_ENABLE(serversound,
	AC_HELP_STRING([--enable-serversound],
			[Turns on sound support in the server (not client)]),
[if test x$enable_serversound = xyes; then
	AC_DEFINE([SERVER_SOUND], 1, [Turns on sound support in the server])
fi])

AC_ARG_ENABLE(plockserver,
	AC_HELP_STRING([--enable-plockserver],
			[lock the xpilot server (xpilots) into memory]),
[if test x$enable_plockserver = xyes; then
	AC_DEFINE([PLOCKSERVER], 1, [lock the xpilot server (xpilots) into memory])
fi])

AC_ARG_ENABLE(development,
	AC_HELP_STRING([--enable-development],
			[enable development code]),
[if test x$enable_development = xyes; then
	AC_DEFINE([DEVELOPMENT], 1, [enable development code])
fi])

AC_ARG_ENABLE(sdl_client,
	AC_HELP_STRING([--enable-sdl-client],
			[enable sdl client]),
[if test x$enable_sdl_client = xyes; then
	AC_DEFINE([SDL_CLIENT], 1, [enable sdl client])
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


dnl Figure out which math library to use
dnl (borrowed from from http://www.libsdl.org/opengl/SDLgears-1.0.2.tar.gz)
case "$target" in
    *-*-mingw32*)
        MATHLIB=""
        SYS_GL_LIBS="-lopengl32 -lglu32"
        ;;
    *-*-beos*)
        MATHLIB=""
        SYS_GL_LIBS="-lGL"
        ;;
    *-*-linux*)
        MATHLIB="-lm"
        SYS_GL_LIBS="-L/usr/X11R6/lib -lGL -lGLU"
        ;;
    *)
        MATHLIB="-lm"
        SYS_GL_LIBS="-lGL -LGLU"
        ;;
esac

dnl Stuff needed by SDL client.
if test x$enable_sdl_client = xyes; then

	dnl Check for SDL,
	dnl from http://www.libsdl.org/opengl/SDLgears-1.0.2.tar.gz
	SDL_VERSION=1.2.0
	AM_PATH_SDL($SDL_VERSION,
        	    :,
	            AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
	)
	CFLAGS="$CFLAGS $SDL_CFLAGS"

	if test x$no_sdl = xyes; then
	    AC_MSG_ERROR(SDL client will not be built.)
	fi

	dnl check for SDL_ttf and OpenGL only if SDL client is built.

	dnl kps - hack to check for SDL_ttf
	AC_MSG_CHECKING(for SDL_ttf)
	have_libsdl_ttf=no
	AC_TRY_COMPILE([
	 #include "SDL.h"
	 #include "SDL/SDL_ttf.h"
	],[
	],[
	have_libsdl_ttf=yes
	])
	AC_MSG_RESULT($have_libsdl_ttf)
	if test x$have_libsdl_ttf = xyes; then
	    SDL_EXTRA_LIBS="-lSDL_ttf"
	else
	    AC_MSG_ERROR(Unable to find SDL_ttf headers and libraries)
	fi
	AC_SUBST(SDL_EXTRA_LIBS)


	dnl Check for OpenGL
	AC_MSG_CHECKING(for OpenGL support)
	have_opengl=no
	AC_TRY_COMPILE([
	 #include <GL/gl.h>
	 #include <GL/glu.h>
	],[
	],[
	have_opengl=yes
	])
	AC_MSG_RESULT($have_opengl)
	if test x$have_opengl = xyes; then
	    GL_LIBS="$SDL_LIBS $SYS_GL_LIBS"
	else
	    AC_MSG_ERROR(Unable to find OpenGL headers and libraries)
	fi
	AC_SUBST(GL_LIBS)
fi

AM_CONDITIONAL([COND_SDL_CLIENT], [test x$enable_sdl_client = xyes])


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
	sys/lock.h \
	sys/mman.h \
	sys/param.h \
	sys/socket.h \
	sys/sockio.h \
	sys/time.h \
	values.h \
	X11/X.h \
	X11/Xlib.h \
	X11/Xos.h \
	X11/Xutil.h \
	X11/keysym.h \
	X11/Xatom.h \
	X11/Xmd.h \
	X11/extensions/Xdbe.h \
	X11/extensions/multibuf.h \
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
	strlcat \
	strlcpy \
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
	README \
	doc/Makefile \
	doc/man/Makefile \
	doc/man/xpilot.man \
	doc/man/xp-replay.man \
	doc/man/xp-mapedit.man \
	doc/man/xpilots.man \
	lib/Makefile \
	lib/fonts/Makefile \
	lib/maps/Makefile \
	lib/textures/Makefile \
	src/Makefile \
	src/common/Makefile \
	src/common/version.h \
	src/common/NT/Makefile \
	src/common/NT/bindist/Makefile \
	src/common/NT/bindist/README.txt \
	src/common/NT/bindist/READMEbin.txt \
	src/common/NT/bindist/ServerMOTD.txt \
	src/client/Makefile \
	src/client/items/Makefile \
	src/client/sdl/Makefile \
	src/server/Makefile \
	src/replay/Makefile \
	src/replay/tools/Makefile \
	])
AC_OUTPUT

