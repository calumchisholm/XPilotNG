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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#ifndef _WINDOWS
# include <unistd.h>
# include <X11/Xlib.h>
#endif

#ifdef _WINDOWS
# include "NT/winX.h"
# include <io.h>
#endif

#include "version.h"
#include "xpconfig.h"
#include "const.h"
#include "paint.h"
#include "xinit.h"
#include "error.h"
#include "xpmread.h"
#include "texture.h"
#include "portability.h"
#include "bitmaps.h"

char texture_version[] = VERSION;

Pixmap Texture_wall(void)
{
    xp_bitmap_t *bmp;
    if (!texturedObjects)
	return None;
    if ((bmp = Bitmap_get(top, BM_WALL_TEXTURE, 0)) == NULL)
	return None;
    return bmp->bitmap;
}

Pixmap Texture_decor(void)
{
    xp_bitmap_t *bmp;
    if (!texturedObjects)
	return None;
    if ((bmp = Bitmap_get(top, BM_DECOR_TEXTURE, 0)) == NULL)
	return None;
    return bmp->bitmap;
}
