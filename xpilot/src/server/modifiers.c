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

#include "xpserver.h"

char modifiers_version[] = VERSION;


/*
 * Fast conversion of `num' into `str' starting at position `i', returns
 * index of character after converted number.
 */
static int num2str(int num, char *str, int i)
{
    int	digits, t;

    if (num < 0) {
	str[i++] = '-';
	num = -num;
    }
    if (num < 10) {
	str[i++] = '0' + num;
	return i;
    }
    for (t = num, digits = 0; t; t /= 10, digits++)
	;
    for (t = i+digits-1; t >= 0; t--) {
	str[t] = num % 10;
	num /= 10;
    }
    return i + digits;
}

/*
 * modstr must be able to hold at least MAX_CHARS chars.
 */
void Modifiers_to_string(modifiers_t mods, char *modstr, size_t size)
{
    int i = 0, t;

    if (size < MAX_CHARS)
	return;
    t = Get_nuclear_modifier(mods);
    if (t & MODS_FULLNUCLEAR)
	modstr[i++] = 'F';
    if (t & MODS_NUCLEAR)
	modstr[i++] = 'N';
    if (Get_cluster_modifier(mods))
	modstr[i++] = 'C';
    if (Get_implosion_modifier(mods))
	modstr[i++] = 'I';
    t = Get_velocity_modifier(mods);
    if (t) {
	if (i) modstr[i++] = ' ';
	modstr[i++] = 'V';
	i = num2str(t, modstr, i);
    }
    t = Get_mini_modifier(mods);
    if (t) {
	if (i) modstr[i++] = ' ';
	modstr[i++] = 'X';
	i = num2str(t + 1, modstr, i);
    }
    t = Get_spread_modifier(mods);
    if (t) {
	if (i) modstr[i++] = ' ';
	modstr[i++] = 'Z';
	i = num2str(t, modstr, i);
    }
    t = Get_power_modifier(mods);
    if (t) {
	if (i) modstr[i++] = ' ';
	modstr[i++] = 'B';
	i = num2str(t, modstr, i);
    }
    t = Get_laser_modifier(mods);
    if (t) {
	if (i) modstr[i++] = ' ';
	modstr[i++] = 'L';
	if (t & MODS_LASER_STUN)
	    modstr[i++] = 'S';
	if (t & MODS_LASER_BLIND)
	    modstr[i++] = 'B';
    }
    modstr[i] = '\0';
}

