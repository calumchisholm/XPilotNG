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

#ifndef SERVER_H
#define SERVER_H

enum TeamPickType {
    PickForHuman	= 1,
    PickForRobot	= 2
};

#ifndef	_WINDOWS
#define	APPNAME	"xpilots"
#else
#define	APPNAME	"XPilotServer"
#endif

/* server types, could be put into servertypes.h */

/* map related types */
typedef struct fuel fuel_t;
typedef struct grav grav_t;
typedef struct base base_t;
typedef struct baseorder baseorder_t;
typedef struct cannon cannon_t;
typedef struct check check_t;
typedef struct item item_t;
typedef struct asteroid asteroid_t;
typedef enum { WORM_NORMAL, WORM_IN, WORM_OUT } wormType;
typedef struct wormhole wormhole_t;
typedef struct treasure treasure_t;
typedef struct target target_t;
typedef struct team team_t;
typedef struct item_concentrator item_concentrator_t;
typedef struct asteroid_concentrator asteroid_concentrator_t;

/* object related types */
typedef struct player player;

#endif


