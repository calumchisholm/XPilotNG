/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2003 by
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

#include "xpclient_x11.h"

char xdefault_version[] = VERSION;

#ifdef OPTIONHACK


#define DISPLAY_ENV	"DISPLAY"
#define DISPLAY_DEF	":0.0"
#define KEYBOARD_ENV	"KEYBOARD"

/*
 * Default fonts
 */
#define GAME_FONT	"-*-times-*-*-*--18-*-*-*-*-*-iso8859-1"
#define MESSAGE_FONT	"-*-times-*-*-*--14-*-*-*-*-*-iso8859-1"
#define SCORE_LIST_FONT	"-*-fixed-bold-*-*--13-*-*-*-c-*-iso8859-1"
#define BUTTON_FONT	"-*-*-bold-o-*--14-*-*-*-*-*-iso8859-1"
#define TEXT_FONT	"-*-*-bold-i-*--14-*-*-*-p-*-iso8859-1"
#define TALK_FONT	"-*-fixed-bold-*-*--15-*-*-*-c-*-iso8859-1"
#define KEY_LIST_FONT	"-*-fixed-medium-r-*--10-*-*-*-c-*-iso8859-1"
#define MOTD_FONT	"-*-courier-bold-r-*--14-*-*-*-*-*-iso8859-1"

static char displayName[MAX_DISP_LEN];
static char keyboardName[MAX_DISP_LEN];

xp_option_t xdefault_options[] = {
    XP_BOOL_OPTION(
	"fullColor",
	true,
	&fullColor,
	NULL,            /* kps - need a setfunc here */
	"Whether to use a colors as close as possible to the specified ones\n"
	"or use a few standard colors for everything. May require more\n"
	"resources from your system.\n"),

    XP_BOOL_OPTION(
	"texturedObjects",
	true,
	&texturedObjects,
	NULL,
	"Whether to draw certain game objects with textures.\n"
	"Be warned that this requires more graphics speed.\n"
	"fullColor must be on for this to work.\n"
	"You may also need to enable multibuffering or double-buffering.\n"),

    XP_STRING_OPTION(
	"display",
	"",
	displayName,
	sizeof displayName,
	NULL, NULL,
	"Set the X display.\n"),

    XP_STRING_OPTION(
	"keyboard",
	"",
	keyboardName,
	sizeof keyboardName,
	NULL, NULL,
	"Set the X keyboard input if you want keyboard input from\n"
	"another display.  The default is to use the keyboard input from\n"
	"the X display.\n"),

    XP_STRING_OPTION(
	"visual",
	"",
	visualName,
	sizeof visualName,
	NULL, NULL,
	"Specify which visual to use for allocating colors.\n"
	"To get a listing of all possible visuals on your dislay\n"
	"set the argument for this option to list.\n"),

    XP_BOOL_OPTION(
	"colorSwitch",
	true,
	&colorSwitch,
	NULL,
	"Use color buffering or not.\n"
	"Usually color buffering is faster, especially on 8-bit\n"
	"PseudoColor displays.\n"),

    XP_BOOL_OPTION(
	"multibuffer",
	false,
	&multibuffer,
	NULL,
	"Use the X windows multibuffer extension if present.\n"),

    XP_STRING_OPTION(
	"gameFont",
	GAME_FONT,
	gameFontName,
	sizeof gameFontName,
	NULL, NULL,
	"The font used on the HUD and for most other text.\n"),

    XP_STRING_OPTION(
	"scoreListFont",
	SCORE_LIST_FONT,
	scoreListFontName,
	sizeof scoreListFontName,
	NULL, NULL,
	"The font used on the score list.\n"
	"This must be a non-proportional font.\n"),

    XP_STRING_OPTION(
	"buttonFont",
	BUTTON_FONT,
	buttonFontName,
	sizeof buttonFontName,
	NULL, NULL,
	"The font used on all buttons.\n"),

    XP_STRING_OPTION(
	"textFont",
	TEXT_FONT,
	textFontName,
	sizeof textFontName,
	NULL, NULL,
	"The font used in the help and about windows.\n"),

    XP_STRING_OPTION(
	"talkFont",
	TALK_FONT,
	talkFontName,
	sizeof talkFontName,
	NULL, NULL,
	"The font used in the talk window.\n"),

    XP_STRING_OPTION(
	"motdFont",
	MOTD_FONT,
	motdFontName,
	sizeof motdFontName,
	NULL, NULL,
	"The font used in the MOTD window and key list window.\n"
	"This must be a non-proportional font.\n"),

    XP_STRING_OPTION(
	"messageFont",
	MESSAGE_FONT,
	messageFontName,
	sizeof messageFontName,
	NULL, NULL,
	"The font used for drawing messages.\n"),


};

void Store_x_options(void)
{
    STORE_OPTIONS(xdefault_options);
}

void Handle_x_options(void)
{
    char *ptr;

    /* handle display */
    assert(displayName);
    if (strlen(displayName) == 0) {
	if ((ptr = getenv(DISPLAY_ENV)) != NULL)
	    Set_option("display", ptr);
	else
	    Set_option("display", DISPLAY_DEF);
    }

    if ((dpy = XOpenDisplay(displayName)) == NULL)
	fatal("Can't open display '%s'.", displayName);

    /* handle keyboard */
    assert(keyboardName);
    if (strlen(keyboardName) == 0) {
	if ((ptr = getenv(KEYBOARD_ENV)) != NULL)
	    Set_option("keyboard", ptr);
    }

    if (strlen(keyboardName) == 0)
	kdpy = NULL;
    else if ((kdpy = XOpenDisplay(keyboardName)) == NULL)
	fatal("Can't open keyboard '%s'.", keyboardName);

    /* handle visual */
    assert(visualName);
    if (strncasecmp(visualName, "list", 4) == 0) {
	List_visuals();
	exit(0);
    }


    

}

#endif



