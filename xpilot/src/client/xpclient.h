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

#ifndef XPCLIENT_H
#define XPCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <time.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>

#ifndef _WINDOWS
# include <unistd.h>
# include <pwd.h>
# ifndef __hpux
#  include <sys/time.h>
# endif
# ifdef _AIX
#  include <sys/select.h>
# endif
# include <sys/file.h>
# include <sys/param.h>
# include <sys/socket.h>
# include <sys/ioctl.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# if defined(SVR4) || defined(__svr4__)
#  include <sys/sockio.h>
# endif
# include <net/if.h>
# include <X11/X.h>
# include <X11/Xlib.h>
# include <X11/Xos.h>
# include <X11/Xutil.h>
# include <X11/keysym.h>
# include <X11/Xatom.h>
# include <X11/Xmd.h>
# include <X11/Xresource.h>
# ifdef	__apollo
#  include <X11/ap_keysym.h>
# endif
#endif

#ifdef _WINDOWS
# include "NT/winBitmap.h"
# include "NT/winClient.h"
# include "NT/winConfig.h"
# include "NT/winNet.h"
# include "NT/winAudio.h"
# include "NT/winX.h"
# include "NT/winX_.h"
# include "NT/winXThread.h"
# include "NT/winXKey.h"
# include "NT/winXXPilot.h"
# include <io.h>
# include <direct.h>
# define snprintf _snprintf
# define printf Trace
# define F_OK 0
# define W_OK 2
# define R_OK 4
# define X_OK 0
# define mkdir(A,B) _mkdir(A)
#endif

#ifdef VMS
# include <socket.h>
# include <in.h>
#endif

#include "version.h"
#include "xpconfig.h"
#include "astershape.h"
#include "bit.h"
#include "checknames.h"
#include "commonproto.h"
#include "const.h"
#include "error.h"
#include "keys.h"
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

#include "bitmaps.h"
#include "client.h"
#include "clientrank.h"
#include "configure.h"
#include "connectparam.h"
#include "datagram.h"
#include "dbuff.h"
#include "default.h"
#include "gfx2d.h"
#include "gfx3d.h"
#include "guimap.h"
#include "guiobjects.h"
#include "ignore.h"
#include "netclient.h"
#include "paint.h"
#include "paintdata.h"
#include "paintmacros.h"
#include "protoclient.h"
#include "record.h"
#include "recordfmt.h"
#include "talk.h"
#include "texture.h"
#include "widget.h"
#include "xevent.h"
#include "xeventhandlers.h"
#include "xinit.h"
#include "xpmread.h"

#endif /* XPCLIENT_H */

