/* 
 * XPilot NG, a multiplayer space war game.
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

#ifndef	TUNER_H
#define	TUNER_H

#ifndef MAP_H
# include "map.h"
#endif

void tuner_plock(world_t *world);
void tuner_shipmass(world_t *world);
void tuner_ballmass(world_t *world);
void tuner_maxrobots(world_t *world);
void tuner_minrobots(world_t *world);
void tuner_allowshields(world_t *world);
void tuner_playerstartsshielded(world_t *world);
void tuner_worldlives(world_t *world);
void tuner_cannonsmartness(world_t *world);
void tuner_teamcannons(world_t *world);
void tuner_cannonsuseitems(world_t *world);
void tuner_wormhole_stable_ticks(world_t *world);
void tuner_modifiers(world_t *world);
void tuner_gameduration(world_t *world);
void tuner_racelaps(world_t *world);
void tuner_allowalliances(world_t *world);
void tuner_announcealliances(world_t *world);
void tuner_playerwallbouncetype(world_t *world);

#endif
