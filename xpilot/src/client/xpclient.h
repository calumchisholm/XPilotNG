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

#include "xpcommon.h"

#ifdef HAVE_LIBZ
#  include <zlib.h>
#else
#  error "Header zlib.h missing. Please install zlib."
#endif

#ifdef _WINDOWS
# include "NT/winBitmap.h"
# include "NT/winConfig.h"
# include "NT/winAudio.h"
# include "NT/winX.h"
# include "NT/winX_.h"
# include "NT/winXThread.h"
# include "NT/winXKey.h"
# include "NT/winXXPilot.h"
# include <direct.h>
# define snprintf _snprintf
# define printf Trace
# define F_OK 0
# define W_OK 2
# define R_OK 4
# define X_OK 0
# define mkdir(A,B) _mkdir(A)
#endif

#include "client.h"
#include "clientrank.h"
#include "connectparam.h"
#include "datagram.h"
#include "gfx2d.h"
#include "gfx3d.h"
#include "guimap.h"      /* guimap.c depends on X11 */
#include "guiobjects.h"
#include "ignore.h"
#include "netclient.h"
#include "protoclient.h" /* misc. hacks */
#include "recordfmt.h"
#include "talk.h"        /* talk.c depends on X11 */

#endif /* XPCLIENT_H */

