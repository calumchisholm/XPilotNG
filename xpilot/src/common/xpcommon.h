/* 
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2003 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef XPCOMMON_H
#define XPCOMMON_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef _WINDOWS
# define HAVE_ASSERT_H 1
# define HAVE_CTYPE_H 1
# define HAVE_ERRNO_H 1
# define HAVE_MATH_H 1
# define HAVE_SIGNAL_H 1
# define HAVE_STDARG_H 1
# define HAVE_LIMITS_H 1
# define HAVE_SETJMP_H 1
# define HAVE_STDLIB_H 1
# define HAVE_STRING_H 1
# define HAVE_SYS_STAT_H 1
# define HAVE_SYS_TYPES_H 1
# define HAVE_STRCASECMP 1
# define HAVE_STRNCASECMP 1
# define HAVE_LIBZ 1
# define PACKAGE "xpilot"
#endif

#include <stdio.h>
#if HAVE_STDARG_H
# include <stdarg.h>
#else
# error "ANSI C stdarg.h is needed to compile."
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif

#ifdef HAVE_CTYPE_H
# include <ctype.h>
#endif

#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif

#ifdef HAVE_MATH_H
# include <math.h>
#endif

#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifdef HAVE_FLOAT_H
# include <float.h>
#endif

#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif

#ifdef HAVE_PWD_H
# include <pwd.h>
#endif

#ifdef HAVE_SETJMP_H
# include <setjmp.h>
#endif

#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_SYS_INTTYPES_H
# include <sys/inttypes.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_NET_IF_H
# include <net/if.h>
#endif

#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif

#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif

/* Platform specific hacks. */

/* SGI hack. */
#ifdef HAVE_BSTRING_H
# include <bstring.h>
#endif

/* System V R4 hacks. */
#ifdef HAVE_SYS_FILIO_H
# include <sys/filio.h>
#endif

#ifdef HAVE_SYS_SOCKIO_H
# include <sys/sockio.h>
#endif

/* Sequent hack. */
#ifdef HAVE_SYS_FCNTL_H
# include <sys/fcntl.h>
#endif

/* Sun hacks. */
#ifdef HAVE_ARPA_NAMESER_H
# include <arpa/nameser.h>
#endif

#ifdef HAVE_RESOLV_H
# include <resolv.h>
#endif

#ifdef HAVE_VALUES_H
# include <values.h>
#endif

/* Evil Windows hacks. Yuck. */
#if defined(_WINDOWS)
# include "NT/winNet.h"
  /* need this for printf wrappers. */
# ifdef	_XPILOTNTSERVER_
#  include "../server/NT/winServer.h"
#  include "../server/NT/winSvrThread.h"
extern char *showtime(void);
# elif !defined(_XPMONNT_)
#  include "NT/winX.h"
#  include "../client/NT/winClient.h"
# endif
static void Win_show_error(char *errmsg);
# include <io.h>
# include <process.h>
# include "NT/winNet.h"
  /* Windows needs specific hacks for sockets: */
# undef close
# define close(x__) closesocket(x__)
# undef ioctl
# define ioctl(x__, y__, z__) ioctlsocket(x__, y__, z__)
# undef read
# define read(x__, y__, z__) recv(x__, y__, z__,0)
# undef write
# define write(x__, y__, z__) send(x__, y__, z__,0)
  /* Windows some more hacks: */
# define getpid() _getpid()
typedef int socklen_t;
#endif

/* Common XPilot header files. */

#include "version.h"
#include "xpconfig.h"
#include "astershape.h"
#include "bit.h"
#include "checknames.h"
#include "commonproto.h"
#include "const.h"
#include "draw.h"
#include "error.h"
#include "item.h"
#include "list.h"
#include "metaserver.h"
#include "net.h"
#include "pack.h"
#include "packet.h"
#include "portability.h"
#include "rules.h"
#include "setup.h"
#include "socklib.h"
#include "types.h"
#include "wreckshape.h"

#ifdef	SOUND
# include "audio.h"
#endif

#endif /* XPCOMMON_H */
