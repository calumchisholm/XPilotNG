/* 
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2003 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
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
#else
/* If we are not using autoconf, we do the best we can. */
# define STDC_HEADERS 1
# define HAVE_LIMITS_H 1
# define HAVE_SETJMP_H 1
# define HAVE_STDLIB_H 1
# define HAVE_STRING_H 1
# define HAVE_SYS_FILE_H 1
# define HAVE_SYS_STAT_H 1
# define HAVE_SYS_TYPES_H 1
# ifndef _WINDOWS
#  define HAVE_PWD_H 1
#  define HAVE_UNISTD_H 1
#  define HAVE_ARPA_INET_H 1
#  define HAVE_NETINET_IN_H 1
#  define HAVE_NETINET_TCP_H 1
#  define HAVE_NET_IF_H 1
#  define HAVE_NETDB_H 1
#  define HAVE_SYS_IOCTL_H 1
#  define HAVE_SYS_PARAM_H 1
#  define HAVE_SYS_SELECT_H 1
#  define HAVE_SYS_SOCKET_H 1
#  ifdef _AIX
#   ifndef _BSD_INCLUDES
#    define _BSD_INCLUDES 1
#   endif
#  endif
#  ifndef __hpux
#   define HAVE_SYS_TIME_H 1
#   define TIME_WITH_SYS_TIME 1
#  endif
#  ifdef __linux__
#   define HAVE_INTTYPES_H 1
#  endif
#  ifdef __FreeBSD__
#   define HAVE_SYS_INTTYPES_H 1    
#  endif
#  ifndef _SEQUENT_
#   define HAVE_FCNTL_H 1
#  else
#   define HAVE_SYS_FCNTL_H 1
#  endif
#  ifdef __sgi
#   define HAVE_BSTRING_H 1
#  endif
#  ifdef __sun__
#   define HAVE_ARPA_NAMESER_H 1
#   define HAVE_RESOLV_H 1
#  endif
#  if defined(SVR4) || defined(__svr4__)
#   define HAVE_SYS_FILIO_H 1
#   define HAVE_SYS_SOCKIO_H 1
#  endif
#  ifdef VMS
#   define HAVE_SOCKET_H
#   define HAVE_IN_H
#  endif
# endif /* _WINDOWS */
#endif

#ifdef STDC_HEADERS
# include <stdio.h>
# include <assert.h>
# include <ctype.h>
# include <errno.h>
# include <math.h>
# include <signal.h>
# include <time.h>
#endif

#ifdef HAVE_SETJMP_H
# include <setjmp.h>
#endif

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif

#ifdef HAVE_STRING_H
# include <string.h>
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_PWD_H
# include <pwd.h>
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

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#if defined(HAVE_SYS_TIME_H) && defined(TIME_WITH_SYS_TIME)
# include <sys/time.h>
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif

#ifdef HAVE_SYS_INTTYPES_H
# include <sys/inttypes.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_NET_IF_H
# include <net/if.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
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

/* VMS hacks. */
#ifdef HAVE_SOCKET_H
# include <socket.h>
#endif

#ifdef HAVE_IN_H
# include <in.h>
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
# undef	va_start
# undef	va_end
# include <varargs.h>
  /* Windows needs specific system calls for sockets: */
# undef close
# define close(x__) closesocket(x__)
# undef ioctl
# define ioctl(x__, y__, z__) ioctlsocket(x__, y__, z__)
# undef read
# define read(x__, y__, z__) recv(x__, y__, z__,0)
# undef write
# define write(x__, y__, z__) send(x__, y__, z__,0)
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

#undef HAVE_STDARG
#undef HAVE_VARARG
#ifndef _WINDOWS
# if (defined(__STDC__) && !defined(__sun__) || defined(__cplusplus))
#  define HAVE_STDARG 1
# else
#  define HAVE_VARARG 1
# endif
#endif

#endif /* XPCOMMON_H */
