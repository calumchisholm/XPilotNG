/* 
 * XPilotNG, an XPilot-like multiplayer space war game.
 *
 * Copyright (C) 1991-2001 by
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * This file contains function wrappers around OS specific services.
 */

#include "xpcommon.h"

char portability_version[] = VERSION;


int Get_process_id(void)
{
#if defined(_WINDOWS)
    return _getpid();
#else
    return getpid();
#endif
}


void Get_login_name(char *buf, size_t size)
{
#if defined(_WINDOWS)
    long nsize = size;
    GetUserName(buf, &nsize);
    buf[size - 1] = '\0';
#else
    /* Unix */
    struct passwd *pwent = getpwuid(geteuid());
    strlcpy(buf, pwent->pw_name, size);
#endif
}


bool is_this_windows(void)
{
#ifdef _WINDOWS
    return true;
#else
    return false;
#endif
}


/*
 * Round to nearest integer.
 */
#ifdef _WINDOWS
double rint(double x)
{
    return floor((x < 0.0) ? (x - 0.5) : (x + 0.5));
}
#endif

