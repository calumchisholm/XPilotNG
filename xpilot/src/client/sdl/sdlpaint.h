/*
 * XPilotNG/SDL, an SDL/OpenGL XPilot client.
 *
 * Copyright (C) 2003-2004 Juha Lindström <juhal@users.sourceforge.net>
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

#ifndef SDLPAINT_H
#define SDLPAINT_H

#include "xpclient_sdl.h"

#define MAX_VERTICES 10000

typedef unsigned int color_t;

extern int draw_depth;
extern double scale;              /* The opengl scale factor */

/* code to set the viewport size */
int Resize_Window( int width, int height );


#define STATIONARY_MODE 1
#define MOVING_MODE 	2
#define HUD_MODE    	4

extern int paintSetupMode;

void setupPaint_stationary(void);
void setupPaint_moving(void);
void setupPaint_HUD(void);

/* helper function in sdlgui.c, some may be removed soon */
extern int nullRGBA;
extern int blackRGBA;
extern int whiteRGBA;
extern int blueRGBA;
extern int redRGBA;
extern int greenRGBA;

extern int scoreInactiveSelfColorRGBA;
extern int scoreInactiveColorRGBA;
extern int scoreSelfColorRGBA;
extern int scoreColorRGBA;
extern int scoreOwnTeamColorRGBA;
extern int scoreEnemyTeamColorRGBA;

extern int GL_X(int x);
extern int GL_Y(int y);
extern void Segment_add(int color, int x_1, int y_1, int x_2, int y_2);
extern void Circle(int color, int x, int y, int radius, int filled);
extern void set_alphacolor(int color);

extern irec_t *select_bounds;
extern void Paint_select(void);

extern SDL_Surface  *MainSDLSurface;

#endif
