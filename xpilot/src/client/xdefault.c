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

bool	titleFlip;		/* Do special title bar flipping? */
bool	showNastyShots = false;	/* show original flavor shots or the new 
				   "nasty shots" */

#ifdef DEVELOPMENT
static bool testxsync = false;
static bool testxdebug = false;
static bool testxafter = false;
static bool testxcolors = false;
#endif /* DEVELOPMENT */

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

    XP_BOOL_OPTION(
	"ignoreWindowManager",
	false,
	&ignoreWindowManager,
	NULL,
	"Ignore the window manager when opening the top level player window.\n"
	"This can be handy if you want to have your XPilot window on a\n"
	"preferred position without window manager borders.\n"
	"Also sometimes window managers may interfere when switching\n"
	"colormaps. This option may prevent that.\n"),

    XP_BOOL_OPTION(
	"titleFlip",
	true,
	&titleFlip,
	NULL,
	"Should the title bar change or not.\n"
	"Some window managers like twm may have problems with\n"
	"flipping title bars.  Hence this option to turn it off.\n"),

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

    XP_BOOL_OPTION(
	"showNastyShots",
	false,
	&showNastyShots,
	NULL,
	"Use the new Nasty Looking Shots or the original rectangle shots,\n"
	"You will probably want to increase your shotSize if you use this.\n"),

    /* X debug stuff */
#ifdef DEVELOPMENT
    XP_NOARG_OPTION(
        "testxsync",
	&testxsync,
        "Test XSynchronize() ?\n"),

    XP_NOARG_OPTION(
        "testxdebug",
	&testxdebug,
        "Test X_error_handler() ?\n"),

    XP_NOARG_OPTION(
        "testxafter",
	&testxafter,
        "Test XAfterFunction ?\n"),

    XP_NOARG_OPTION(
        "testxcolors",
	&testxcolors,
        "Do Colors_debug() ?\n"),
#endif


};

void Store_x_options(void)
{
    STORE_OPTIONS(xdefault_options);
}


#ifdef DEVELOPMENT
static int X_error_handler(Display *display, XErrorEvent *xev)
{
    char		buf[1024];

    fflush(stdout);
    fprintf(stderr, "X error\n");
    XGetErrorText(display, xev->error_code, buf, sizeof buf);
    buf[sizeof(buf) - 1] = '\0';
    fprintf(stderr, "%s\n", buf);
    fflush(stderr);
    *(double *) -3 = 2.10;	/*core dump*/
    exit(1);
    return 0;
}

static void X_after(Display *display)
{
    static int		n;

    (void)display;
    if (n < 1000)
	printf("_X_ %4d\n", n++);
}
#endif /* DEVELOPMENT */

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

#ifdef DEVELOPMENT
    if (testxsync) {
	XSynchronize(dpy, True);
	XSetErrorHandler(X_error_handler);
    }

    if (testxdebug)
	XSetErrorHandler(X_error_handler);

    if (testxafter) {
	XSetAfterFunction(dpy, (int (*)(
#if NeedNestedPrototypes
	    Display *
#endif
	    )) X_after);
    }

    if (testxcolors)
	Colors_debug();
#endif

}



bool Set_scaleFactor(xp_option_t *opt, double val)
{
    (void)opt;
    scaleFactor = val;
    Init_scale_array();
    /* Resize removed because it is not needed here */
    Scale_dashes();
    Bitmap_update_scale();
    return true;
}

bool Set_altScaleFactor(xp_option_t *opt, double val)
{
    (void)opt;
    scaleFactor_s = val;
    return true;
}





#endif


