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

#ifndef XPAINT_H
#define XPAINT_H

/* how to draw a selection */
#define DRAW_EMPHASIZED		BLUE

#if 0
#define FIND_NAME_WIDTH(other)                                          \
    if ((other)->name_width == 0) {                                     \
        (other)->name_len = strlen((other)->name);                      \
        (other)->name_width = 2 + XTextWidth(gameFont, (other)->name,   \
                                         (other)->name_len);            \
    }
#endif /* 0 */
 
#define FIND_NAME_WIDTH(other)                                          \
    if ((other)->name_width == 0) {                                     \
        (other)->name_len = strlen((other)->id_string);                 \
        (other)->name_width = 2 + XTextWidth(gameFont, (other)->id_string,\
                                         (other)->name_len);            \
    }

/* The fonts used in the game */
extern XFontStruct* gameFont;
extern XFontStruct* messageFont;
extern XFontStruct* scoreListFont;
extern XFontStruct* buttonFont;
extern XFontStruct* textFont;
extern XFontStruct* talkFont;
extern XFontStruct* motdFont;

/* The name of the fonts used in the game */
#define FONT_LEN	256
extern char gameFontName[FONT_LEN];
extern char messageFontName[FONT_LEN];
extern char scoreListFontName[FONT_LEN];
extern char buttonFontName[FONT_LEN];
extern char textFontName[FONT_LEN];
extern char talkFontName[FONT_LEN];
extern char motdFontName[FONT_LEN];

extern Display	*dpy;			/* Display of player (pointer) */
extern Display	*kdpy;			/* Keyboard display */
extern short	about_page;		/* Which page is the player on? */
extern bool	players_exposed;	/* Is score window exposed? */
extern int	radar_exposures;	/* Is radar window exposed? */

					/* windows has 2 sets of item bitmaps */
#define	ITEM_HUD	0		/* one color for the HUD */
#define	ITEM_PLAYFIELD	1		/* and one color for the playfield */
#ifdef _WINDOWS
extern Pixmap	itemBitmaps[][2];
#else
extern Pixmap	itemBitmaps[];
#endif

extern GC	gameGC, messageGC, radarGC, buttonGC;
extern GC	scoreListGC, textGC, talkGC, motdGC;
extern XGCValues gcv;
extern Window	topWindow, drawWindow, keyboardWindow;
extern Window	radarWindow, playersWindow;
#ifdef _WINDOWS				/* see paint.c for details */
extern Window	textWindow, msgWindow, buttonWindow;
#endif
extern Pixmap	drawPixmap;		/* Drawing area pixmap */
extern Pixmap	radarPixmap;		/* Radar drawing pixmap */
extern Pixmap	radarPixmap2;		/* Second radar drawing pixmap */
extern long	dpl_1[2];		/* Used by radar hack */
extern long	dpl_2[2];		/* Used by radar hack */
extern Window	aboutWindow;		/* The About window */
extern Window	about_close_b;		/* About close button */
extern Window	about_next_b;		/* About next page button */
extern Window	about_prev_b;		/* About prev page button */
extern Window	talkWindow;		/* Talk window */
extern XColor	colors[MAX_COLORS];	/* Colors */
extern Colormap	colormap;		/* Private colormap */
extern int	maxColors;		/* Max. number of colors to use */

extern bool	showNastyShots;		/* show original flavor shots or the new "nasty shots" */

extern int	(*radarDrawRectanglePtr)/* Function to draw player on radar */
		(Display *disp, Drawable d, GC gc,
		 int x, int y, unsigned width, unsigned height);

extern unsigned long	current_foreground;

static inline void SET_FG(unsigned long fg)
{
    if (fg != current_foreground)
	XSetForeground(dpy, gameGC, current_foreground = fg);
}

void Paint_item_symbol(int type, Drawable d, GC mygc,
		       int x, int y, int color);
void Paint_item(int type, Drawable d, GC mygc, int x, int y);
void Gui_paint_item_symbol(int type, Drawable d, GC mygc, int x, int y, int c);
void Gui_paint_item(int type, Drawable d, GC mygc, int x, int y);

#endif
