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

#ifndef XPCLIENT_X11_H
#define XPCLIENT_X11_H

#include "xpclient.h"

#ifdef HAVE_X11_X_H
#  include <X11/X.h>
#endif
#ifdef HAVE_X11_XLIB_H
#  include <X11/Xlib.h>
#endif
#ifdef HAVE_X11_XOS_H
#  include <X11/Xos.h>
#endif
#ifdef HAVE_X11_XUTIL_H
#  include <X11/Xutil.h>
#endif
#ifdef HAVE_X11_KEYSYM_H
#  include <X11/keysym.h>
#endif
#ifdef HAVE_X11_XATOM_H
#  include <X11/Xatom.h>
#endif
#ifdef HAVE_X11_XMD_H
#  include <X11/Xmd.h>
#endif

#include "meta.h"
#include "bitmaps.h"
#include "dbuff.h"
#include "paintdata.h"
#include "record.h"
#include "widget.h"
#include "xevent.h"
#include "xeventhandlers.h"
#include "xinit.h"
#include "xpaint.h"

#endif /* XPCLIENT_X11_H */

