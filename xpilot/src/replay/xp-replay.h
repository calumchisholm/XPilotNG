/* 
 * XP-Replay, playback an XPilot session.  Copyright (C) 1994-98 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Steven Singer        (S.Singer@ph.surrey.ac.uk)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERC_HANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef	XP_REPLAY_H
#define	XP_REPLAY_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef _WINDOWS
# include <unistd.h>
# include <X11/Xlib.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef _WINDOWS
# ifndef __hpux
#  include <sys/time.h>
# endif
# ifdef _AIX
#  include <sys/select.h> /* _BSD not defined in <sys/types.h>, so done by hand */
# endif
# include <stdarg.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
#endif

#ifdef _SEQUENT_
# include <sys/procstats.h>
# define gettimeofday(T,X)	get_process_stats(T, PS_SELF, \
					(struct process_stats *)NULL, \
					(struct process_stats *)NULL)
#endif

#include "recordfmt.h"
#include "item.h"
#include "buttons.h"

#define BLACK               0
#define WHITE               1
#define BLUE                2
#define RED                 3

enum MemTypes {
    MEM_STRING,
    MEM_FRAME,
    MEM_SHAPE,
    MEM_POINT,
    MEM_GC,
    MEM_MISC,
    MEM_UI,
    NUM_MEMTYPES
};

void *MyMalloc(size_t, enum MemTypes);

#endif
