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

#ifndef DEFAULT_H
#define DEFAULT_H

typedef struct {
    const char		*name;		/* option name */
    const char		*noArg;		/* value for non-argument options */
    const char		*fallback;	/* default value */
    keys_t		key;		/* key if not KEY_DUMMY */
    const char		*help;		/* user help (multiline) */
    unsigned		hash;		/* option name hashed. */
} cl_option_t;

#if 0
extern cl_option_t	*options;
extern int		num_options;	/* number of options */
#endif

#endif
