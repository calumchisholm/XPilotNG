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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

#ifndef _WINDOWS
# include <unistd.h>
# include <pwd.h>
# ifndef __hpux
#  include <sys/time.h>
# endif
# ifdef _AIX
#  ifndef _BSD_INCLUDES
#   define _BSD_INCLUDES
#  endif
#  include <sys/select.h>
# endif
# include <sys/file.h>
# include <sys/param.h>
# include <sys/ioctl.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <netdb.h>
#endif

#ifdef __sgi
# include <bstring.h>
#endif

#ifdef SVR4
# include <sys/filio.h>
#endif

#ifdef _SEQUENT_
# include <sys/fcntl.h>
#else
# include <fcntl.h>
#endif

#ifdef __sun__
# include <arpa/nameser.h>
# include <resolv.h>
#endif

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
#endif

#ifdef _WINDOWS
# include <windows.h>
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

#include "version.h"
#include "xpconfig.h"
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
#include "socklib.h"

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
