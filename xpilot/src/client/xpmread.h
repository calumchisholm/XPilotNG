/* 
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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
#ifndef XPM_READ_H
#define XPM_READ_H

#include "gfx2d.h"
/*
 * This XPM loader is based upon the XPM 3.0 specification.
 * It was written from scratch because we needed a small
 * and dedicated XPM interface.  Dedicated because XPilot
 * has a peculiar way of dealing with colors.
 */

/*
 * Public interface to xpmread.c.
 */
extern Pixmap xpm_pixmap_from_data(const char **data);
extern Pixmap xpm_pixmap_from_file(char *filename);
extern int xpm_picture_from_file(xp_picture_t *pic, char *filename);
#ifndef _WINDOWS
extern XImage *xpm_image_from_pixmap(Pixmap pixmap);
#endif

#endif
