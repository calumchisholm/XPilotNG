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

#ifndef XPSERVER_H
#define XPSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <time.h>

/*#include <crypt.h>*/
#include <expat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

#ifndef _WINDOWS
# ifdef _AIX
#  ifndef _BSD_INCLUDES
#   define _BSD_INCLUDES
#  endif
# endif
# include <unistd.h>
# include <pwd.h>
# include <sys/file.h>
# include <sys/param.h>
# include <sys/socket.h>
# ifndef __hpux
#  include <sys/time.h>
# endif
# ifdef _AIX
#  include <sys/select.h> /* _BSD not defined in <sys/types.h>, so done by hand */
# endif
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netdb.h>
# ifdef __sgi
#  include <bstring.h>
# endif
#endif

#ifdef PLOCKSERVER
# if defined(__linux__)
#  include <sys/mman.h>
# else
#  include <sys/lock.h>
# endif
#endif

#ifdef _WINDOWS
# include "NT/winServer.h"
# include "NT/winSvrThread.h"
# include "../common/NT/winNet.h"
# include <io.h>
# define read(__a, __b, __c)	_read(__a, __b, __c)
# include <process.h>
# undef	va_start	/* there are bad versions in windows.h's "stdarg.h" */
# undef	va_end
# include <varargs.h>
#endif

#define SERVER
#include "version.h"
#include "xpconfig.h"
#include "bit.h"
#include "checknames.h"
#include "commonproto.h"
#include "const.h"
#include "draw.h"
#include "error.h"
#include "global.h"
#include "item.h"
#include "list.h"
#include "net.h"
#include "pack.h"
#include "portability.h"
#include "proto.h"
#include "rules.h"
#include "socklib.h"
#include "types.h"

#include "asteroid.h"
#include "auth.h"
#include "cannon.h"
#include "click.h"
#include "connection.h"
#include "defaults.h"
#include "map.h"
#include "metaserver.h"
#include "netserver.h"
#include "objpos.h"
#include "packet.h"
#include "rank.h"
#include "recwrap.h"
#include "robot.h"
#include "saudio.h"
#include "sched.h"
#include "setup.h"
#include "score.h"
#include "server.h"
#include "serverconst.h"
#include "srecord.h"
#include "tuner.h"
#include "walls.h"

#endif /* XPSERVER_H */
