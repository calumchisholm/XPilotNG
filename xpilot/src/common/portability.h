/* 
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
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

/*
 * Include portability related stuff in one file.
 */
#ifndef PORTABILITY_H
#define PORTABILITY_H

#ifdef _WINDOWS
#undef max
#undef min

#define	strncasecmp(__a, __b, __c)	strnicmp(__a, __b, __c)

/* there are tons of "conversion from 'double ' to 'int '", stop warning us */
#pragma warning (disable : 4244)

#endif /* _WINDOWS */

/*
 * In Windows, just exiting won't tell the user the reason.
 * So, try to gracefully shutdown just the server thread
 */
#ifdef _WINDOWS
extern	int ServerKilled;
#define	ServerExit() ServerKilled = TRUE; return;
#else
#define	ServerExit() exit(1);
#endif

/*
 * Macros to block out Windows only code (and never Windows code)
 */
#ifdef _WINDOWS
#define IFWINDOWS(x)	x
#else
#define IFWINDOWS(x)
#endif

#ifndef _WINDOWS
#define IFNWINDOWS(x)	x
#else
#define IFNWINDOWS(x)
#endif


#ifdef _WINDOWS
#define PATHNAME_SEP    '\\'
#else
#define PATHNAME_SEP    '/'
#endif


/*
 * Prototypes for OS function wrappers in portability.c.
 */
extern int Get_process_id(void);	/* getpid */
extern void Get_login_name(char *buf, size_t size);

/*
 * Prototypes for testing if we are running under a certain OS.
 */
extern bool is_this_windows(void);


/*
 * Round to nearest integer.
 */
#ifdef _WINDOWS
double rint(double x);
#endif


#endif /* PORTABILITY_H */
