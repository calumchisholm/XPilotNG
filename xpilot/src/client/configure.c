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

/*
 * Configure.c: real-time option control.
 * To add your own option to the XPilot client do the following:
 * 1: Define storage for the option value in either client.c/paint.c/xinit.c
 *    or use a bit in the instruments option set (using the SHOW_XXX macros).
 * 2: Add a declaration for this storage to either client.h/paint.h/xinit.h
 *    or, in case a bit in instruments is used, add a SHOW_ macro to client.h.
 * 3: Add an X resource record to the XrmOptionDescRec options[] table in
 *    default.c to have it recognised by the X resource manager routines.
 * 4: Have it set at startup by the Parse_options() routine in default.c.
 * 5: Add the functionality of your option, probably in the same file
 *    as the storage for the option was defined in.
 * 6: Add it to configure.c (this file) as follows:
 *   a) Determine if it needs either a bool/int/float widget
 *      and find a similar option from which you can copy code.
 *   b) Add the Config_create_XXX function prototype at the top of this file.
 *   c) Add the Config_create_XXX function name to the config_creator[] table.
 *      The order in this table determines the order of the options on screen.
 *   d) Define the Config_create_XXX function similar to one of the others.
 *   e) If it needs a callback when the value changes then add a
 *      Config_update_XXX() function after the other update callbacks
 *      and declare a prototype for the callback at the top of this file.
 *      The Config_update_XXX() function should be given as an argument to
 *      the Config_create_bool/int/float() creator in Config_create_XXX().
 *      If the option doesn't need a callback then the calback argument
 *      should be given as NULL.
 *   f) Add one line to the Config_save() routine to have the option saved.
 * 7: Document your option in the manual page for the client.
 * 8: Mail a context diff (diff -c old new) of your changes to
 *    xpilot@xpilot.org.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#ifndef _WINDOWS
# include <unistd.h>
# include <pwd.h>
# include <X11/Xlib.h>
# include <X11/Xos.h>
# include <X11/Xutil.h>
#endif

#ifdef _WINDOWS
# include "NT/winX.h"
# include "NT/winClient.h"
# include "NT/winXXPilot.h"
# include "NT/winConfig.h"
#endif

#include "version.h"
#include "config.h"
#include "const.h"
#include "paint.h"
#include "xinit.h"
#include "bit.h"
#include "keys.h"
#include "netclient.h"
#include "widget.h"
#include "configure.h"
#include "setup.h"
#include "error.h"
#include "protoclient.h"
#include "portability.h"
#include "commonproto.h"
#include "bitmaps.h"

char configure_version[] = VERSION;

#ifndef PATH_MAX
#define PATH_MAX	1023
#endif

extern const char	*Get_keyResourceString(keys_t key);
extern void		Get_xpilotrc_file(char *, unsigned);

static int Config_create_power(int widget_desc, int *height);
static int Config_create_turnSpeed(int widget_desc, int *height);
static int Config_create_turnResistance(int widget_desc, int *height);
static int Config_create_altPower(int widget_desc, int *height);
static int Config_create_altTurnSpeed(int widget_desc, int *height);
static int Config_create_altTurnResistance(int widget_desc, int *height);
static int Config_create_showMessages(int widget_desc, int *height);
static int Config_create_mapRadar(int widget_desc, int *height);
static int Config_create_clientRanker(int widget_desc, int *height);
static int Config_create_showShipShapes(int widget_desc, int *height);
static int Config_create_showMyShipShape(int widget_desc, int *height);
static int Config_create_ballMsgScan(int widget_desc, int *height);
static int Config_create_showLivesByShip(int widget_desc, int *height);
static int Config_create_showExtraBaseInfo(int widget_desc, int *height);
static int Config_create_treatZeroSpecial(int widget_desc, int *height);
static int Config_create_speedFactHUD(int widget_desc, int *height);
static int Config_create_speedFactPTR(int widget_desc, int *height);
static int Config_create_fuelNotify(int widget_desc, int *height);
static int Config_create_fuelWarning(int widget_desc, int *height);
static int Config_create_fuelCritical(int widget_desc, int *height);
static int Config_create_outlineWorld(int widget_desc, int *height);
static int Config_create_filledWorld(int widget_desc, int *height);
static int Config_create_texturedWalls(int widget_desc, int *height);
static int Config_create_texturedObjects(int widget_desc, int *height);
static int Config_create_fullColor(int widget_desc, int *height);
static int Config_create_slidingRadar(int widget_desc, int *height);
static int Config_create_showItemsTime(int widget_desc, int *height);
static int Config_create_showScoreDecimals(int widget_desc, int *height);
static int Config_create_backgroundPointDist(int widget_desc, int *height);
static int Config_create_backgroundPointSize(int widget_desc, int *height);
static int Config_create_sparkSize(int widget_desc, int *height);
static int Config_create_charsPerSecond(int widget_desc, int *height);
static int Config_create_toggleShield(int widget_desc, int *height);
static int Config_create_autoShield(int widget_desc, int *height);
static int Config_create_sparkProb(int widget_desc, int *height);
static int Config_create_shotSize(int widget_desc, int *height);
static int Config_create_teamShotSize(int widget_desc, int *height);
static int Config_create_teamShotColor(int widget_desc, int *height);
static int Config_create_showNastyShots(int widget_desc, int *height);
static int Config_create_hudColor(int widget_desc, int *height);
static int Config_create_hudHLineColor(int widget_desc, int *height);
static int Config_create_hudVLineColor(int widget_desc, int *height);
static int Config_create_hudItemsColor(int widget_desc, int *height);
static int Config_create_hudRadarEnemyColor(int widget_desc, int *height);
static int Config_create_hudRadarOtherColor(int widget_desc, int *height);
static int Config_create_hudRadarDotSize(int widget_desc, int *height);
static int Config_create_hudRadarScale(int widget_desc, int *height);
static int Config_create_hudRadarLimit(int widget_desc, int *height);
static int Config_create_hudSize(int widget_desc, int *height);
static int Config_create_hudLockColor(int widget_desc, int *height);
static int Config_create_visibilityBorderColor(int widget_desc, int *height);
static int Config_create_fuelGaugeColor(int widget_desc, int *height);
static int Config_create_dirPtrColor(int widget_desc, int *height);
static int Config_create_shipShapesHackColor(int widget_desc, int *height);
static int Config_create_msgScanBallColor(int widget_desc, int *height);
static int Config_create_msgScanSafeColor(int widget_desc, int *height);
static int Config_create_msgScanCoverColor(int widget_desc, int *height);
static int Config_create_msgScanPopColor(int widget_desc, int *height);
static int Config_create_selfLWColor(int widget_desc, int *height);
static int Config_create_enemyLWColor(int widget_desc, int *height);
static int Config_create_teamLWColor(int widget_desc, int *height);
static int Config_create_ballColor(int widget_desc, int *height);
static int Config_create_connColor(int widget_desc, int *height);
static int Config_create_fuelMeterColor(int widget_desc, int *height);
static int Config_create_powerMeterColor(int widget_desc, int *height);
static int Config_create_turnSpeedMeterColor(int widget_desc, int *height);
static int Config_create_packetSizeMeterColor(int widget_desc, int *height);
static int Config_create_packetLossMeterColor(int widget_desc, int *height);
static int Config_create_packetDropMeterColor(int widget_desc, int *height);
static int Config_create_packetLagMeterColor(int widget_desc, int *height);
static int Config_create_temporaryMeterColor(int widget_desc, int *height);
static int Config_create_meterBorderColor(int widget_desc, int *height);
static int Config_create_windowColor(int widget_desc, int *height);
static int Config_create_buttonColor(int widget_desc, int *height);
static int Config_create_borderColor(int widget_desc, int *height);
static int Config_create_clockColor(int widget_desc, int *height);
static int Config_create_scoreColor(int widget_desc, int *height);
static int Config_create_scoreSelfColor(int widget_desc, int *height);
static int Config_create_scoreInactiveColor(int widget_desc, int *height);
static int Config_create_scoreInactiveSelfColor(int widget_desc, int *height);
static int Config_create_scoreZeroColor(int widget_desc, int *height);
static int Config_create_scoreObjectColor(int widget_desc, int *height);
static int Config_create_scoreObjectTime(int widget_desc, int *height);
static int Config_create_baseWarningType(int widget_desc, int *height);
static int Config_create_wallColor(int widget_desc, int *height);
static int Config_create_fuelColor(int widget_desc, int *height);
static int Config_create_decorColor(int widget_desc, int *height);
static int Config_create_backgroundPointColor(int widget_desc, int *height);
static int Config_create_team0Color(int widget_desc, int *height);
static int Config_create_team1Color(int widget_desc, int *height);
static int Config_create_team2Color(int widget_desc, int *height);
static int Config_create_team3Color(int widget_desc, int *height);
static int Config_create_team4Color(int widget_desc, int *height);
static int Config_create_team5Color(int widget_desc, int *height);
static int Config_create_team6Color(int widget_desc, int *height);
static int Config_create_team7Color(int widget_desc, int *height);
static int Config_create_team8Color(int widget_desc, int *height);
static int Config_create_team9Color(int widget_desc, int *height);
static int Config_create_showDecor(int widget_desc, int *height);
static int Config_create_outlineDecor(int widget_desc, int *height);
static int Config_create_filledDecor(int widget_desc, int *height);
static int Config_create_texturedDecor(int widget_desc, int *height);
static int Config_create_maxFPS(int widget_desc, int *height);
static int Config_create_maxMessages(int widget_desc, int *height);
static int Config_create_messagesToStdout(int widget_desc, int *height);
static int Config_create_reverseScroll(int widget_desc, int *height);
static int Config_create_messagesColor(int widget_desc, int *height);
static int Config_create_oldMessagesColor(int widget_desc, int *height);
#ifdef SOUND
static int Config_create_maxVolume(int widget_desc, int *height);
#endif

static int Config_create_shipNameColor(int widget_desc, int *height);
static int Config_create_baseNameColor(int widget_desc, int *height);
static int Config_create_mineNameColor(int widget_desc, int *height);

static int Config_create_clockAMPM(int widget_desc, int *height);
static int Config_create_markingLights(int widget_desc, int *height);
#ifdef _WINDOWS
static int Config_create_threadedDraw(int widget_desc, int *height);
#endif
static int Config_create_scaleFactor(int widget_desc, int *height);
static int Config_create_altScaleFactor(int widget_desc, int *height);

static int Config_create_save(int widget_desc, int *height);

static int Config_update_bool(int widget_desc, void *data, bool *val);
static int Config_update_instruments(int widget_desc, void *data, bool *val);
static int Config_update_hackedInstruments(int widget_desc, void *data, bool *val);
static int Config_update_dots(int widget_desc, void *data, int *val);
static int Config_update_altPower(int widget_desc, void *data, DFLOAT *val);
static int Config_update_altTurnResistance(int widget_desc, void *data,
					   DFLOAT *val);
static int Config_update_altTurnSpeed(int widget_desc, void *data, DFLOAT *val);
static int Config_update_power(int widget_desc, void *data, DFLOAT *val);
static int Config_update_turnResistance(int widget_desc, void *data,
					DFLOAT *val);
static int Config_update_turnSpeed(int widget_desc, void *data, DFLOAT *val);
static int Config_update_sparkProb(int widget_desc, void *data, DFLOAT *val);
static int Config_update_charsPerSecond(int widget_desc, void *data, int *val);
static int Config_update_toggleShield(int widget_desc, void *data, bool *val);
static int Config_update_autoShield(int widget_desc, void *data, bool *val);
static int Config_update_maxFPS(int widget_desc, void *data, int *val);
static int Config_update_fullColor(int widget_desc, void *data, bool *val);
static int Config_update_texturedObjects(int widget_desc, void *data, bool *val);
static int Config_update_scaleFactor(int widget_desc, void *data, DFLOAT *val);

static int Config_close(int widget_desc, void *data, const char **strptr);
static int Config_next(int widget_desc, void *data, const char **strptr);
static int Config_prev(int widget_desc, void *data, const char **strptr);
static int Config_save(int widget_desc, void *data, const char **strptr);
static int Config_save_confirm_callback(int widget_desc, void *popup_desc,
					const char **strptr);

typedef struct xpilotrc {
    char	*line;
    short	size;
} xpilotrc_t;

static xpilotrc_t	*xpilotrc_ptr;
static int		num_xpilotrc, max_xpilotrc;

static bool		config_created = false,
			config_mapped = false;
static int		config_page,
			config_x,
			config_y,
			config_width,
			config_height,
			config_space,
			config_max,
			config_button_space,
			config_text_space,
			config_text_height,
			config_button_height,
			config_entry_height,
			config_bool_width,
			config_bool_height,
			config_int_width,
			config_float_width,
			config_arrow_width,
			config_arrow_height;
static int		*config_widget_desc,
			config_save_confirm_desc = NO_WIDGET;
static int	(*config_creator_default[])(int widget_desc, int *height) = {
    Config_create_power,
    Config_create_turnSpeed,
    Config_create_turnResistance,
    Config_create_altPower,
    Config_create_altTurnSpeed,
    Config_create_altTurnResistance,
    Config_create_showMessages,
    Config_create_maxMessages,
    Config_create_messagesToStdout,
    Config_create_reverseScroll,
    Config_create_mapRadar,
    Config_create_clientRanker,
    Config_create_showShipShapes,
    Config_create_showMyShipShape,
    Config_create_ballMsgScan,
    Config_create_showLivesByShip,
    Config_create_showExtraBaseInfo,
    Config_create_treatZeroSpecial,
    Config_create_speedFactHUD,
    Config_create_speedFactPTR,
    Config_create_fuelNotify,
    Config_create_fuelWarning,
    Config_create_fuelCritical,
    Config_create_outlineWorld,
    Config_create_filledWorld,
    Config_create_texturedWalls,
    Config_create_fullColor,
    Config_create_texturedObjects,
    Config_create_slidingRadar,
    Config_create_showItemsTime,
    Config_create_showScoreDecimals,
    Config_create_backgroundPointDist,
    Config_create_backgroundPointSize,
    Config_create_sparkSize,
    Config_create_sparkProb,
    Config_create_charsPerSecond,
    Config_create_markingLights,
    Config_create_toggleShield,
    Config_create_autoShield,
    Config_create_showNastyShots,
    Config_create_shotSize,
    Config_create_teamShotSize,
    Config_create_hudRadarDotSize,
    Config_create_hudRadarScale,
    Config_create_hudRadarLimit,
    Config_create_hudSize,
    Config_create_scoreObjectTime,
    Config_create_baseWarningType,
    Config_create_showDecor,
    Config_create_outlineDecor,
    Config_create_filledDecor,
    Config_create_texturedDecor,
    /*Config_create_texturedBalls,*/
    Config_create_maxFPS,
#ifdef SOUND
    Config_create_maxVolume,
#endif
    Config_create_clockAMPM,
#ifdef _WINDOWS
    Config_create_threadedDraw,
#endif
    Config_create_scaleFactor,
    Config_create_altScaleFactor,
    Config_create_save			/* must be last */
};

static int	(*config_creator_colors[])(int widget_desc, int *height) = {
    Config_create_messagesColor,
    Config_create_oldMessagesColor,
    Config_create_teamShotColor,
    Config_create_hudColor,
    Config_create_hudHLineColor,
    Config_create_hudVLineColor,
    Config_create_hudItemsColor,
    Config_create_hudRadarEnemyColor,
    Config_create_hudRadarOtherColor,
    Config_create_hudLockColor,
    Config_create_visibilityBorderColor,
    Config_create_fuelGaugeColor,
    Config_create_dirPtrColor,
    Config_create_shipShapesHackColor,
    Config_create_msgScanBallColor,
    Config_create_msgScanSafeColor,
    Config_create_msgScanCoverColor,
    Config_create_msgScanPopColor,
    Config_create_selfLWColor,
    Config_create_enemyLWColor,
    Config_create_teamLWColor,
    Config_create_shipNameColor,
    Config_create_baseNameColor,
    Config_create_mineNameColor,
    Config_create_ballColor,
    Config_create_connColor,
    Config_create_fuelMeterColor,
    Config_create_powerMeterColor,
    Config_create_turnSpeedMeterColor,
    Config_create_packetSizeMeterColor,
    Config_create_packetLossMeterColor,
    Config_create_packetDropMeterColor,
    Config_create_packetLagMeterColor,
    Config_create_temporaryMeterColor,
    Config_create_meterBorderColor,
    Config_create_windowColor,
    Config_create_buttonColor,
    Config_create_borderColor,
    Config_create_clockColor,
    Config_create_scoreColor,
    Config_create_scoreSelfColor,
    Config_create_scoreInactiveColor,
    Config_create_scoreInactiveSelfColor,
    Config_create_scoreZeroColor,
    Config_create_scoreObjectColor,
    Config_create_wallColor,
    Config_create_fuelColor,
    Config_create_decorColor,
    Config_create_backgroundPointColor,
    Config_create_team0Color,
    Config_create_team1Color,
    Config_create_team2Color,
    Config_create_team3Color,
    Config_create_team4Color,
    Config_create_team5Color,
    Config_create_team6Color,
    Config_create_team7Color,
    Config_create_team8Color,
    Config_create_team9Color,
    Config_create_save			/* must be last */
};

static int (**config_creator)(int widget_desc, int *height);

/* this must be updated if new config menu items are added */
#define CONFIG_MAX_WIDGET_IDS \
(MAX(NELEM(config_creator_default), \
     NELEM(config_creator_colors)))

static int config_widget_ids[CONFIG_MAX_WIDGET_IDS];

/* this must be updated if new config menu items are added */
static int Nelem_config_creator(void)
{
    if (config_creator == config_creator_colors)
	return NELEM(config_creator_colors);
    if (config_creator == config_creator_default)
	return NELEM(config_creator_default);
    return 0;
}
 

static void Create_config(void)
{
    int			i,
			num,
			height,
			offset,
			width,
			widget_desc;
    bool		full;

    /*
     * Window dimensions relative to the top window.
     */
    config_x = 0;
    config_y = RadarHeight + ButtonHeight + 2;
    config_width = 256;
    config_height = top_height - config_y;

    /*
     * Space between label-text and label-border.
     */
    config_text_space = 3;
    /*
     * Height of a label window.
     */
    config_text_height = 2 * 1 + textFont->ascent + textFont->descent;

    /*
     * Space between button-text and button-border.
     */
    config_button_space = 3;
    /*
     * Height of a button window.
     */
    config_button_height = buttonFont->ascent + buttonFont->descent
			    + 2 * 1;

    config_entry_height = MAX(config_text_height, config_button_height);

    /*
     * Space between entries and between an entry and the border.
     */
    config_space = 6;

    /*
     * Sizes of the different widget types.
     */
    config_bool_width = XTextWidth(buttonFont, "Yes", 3)
			+ 2 * config_button_space;
    config_bool_height = config_button_height;
    config_arrow_height = config_text_height;
    config_arrow_width = config_text_height;
    config_int_width = 4 + XTextWidth(buttonFont, "1000", 4);
    config_float_width = 4 + XTextWidth(buttonFont, "0.22", 4);

    config_max = Nelem_config_creator();
    config_widget_desc = (int *) malloc(config_max * sizeof(int));
    if (config_widget_desc == NULL) {
	error("No memory for config");
	return;
    }

    num = -1;
    full = true;
    for (i = 0; i < Nelem_config_creator(); i++) {
	if (full == true) {
	    full = false;
	    num++;
	    config_widget_desc[num]
		= Widget_create_form(NO_WIDGET, top,
				     config_x, config_y,
				     config_width, config_height,
				     0);
	    if (config_widget_desc[num] == 0) {
		break;
	    }
	    height = config_height - config_space - config_button_height;
	    width = 2 * config_button_space + XTextWidth(buttonFont,
							  "PREV", 4);
	    offset = config_width - width - config_space;
	    widget_desc =
		Widget_create_activate(config_widget_desc[num],
				       offset, height,
				       width, config_button_height,
				       0, "PREV", Config_prev,
				       (void *)(long)num);
	    if (widget_desc == 0) {
		break;
	    }
	    width = 2 * config_button_space + XTextWidth(buttonFont,
							  "NEXT", 4);
	    offset = (config_width - width) / 2;
	    widget_desc =
		Widget_create_activate(config_widget_desc[num],
				       offset, height,
				       width, config_button_height,
				       0, "NEXT", Config_next,
				       (void *)(long)num);
	    if (widget_desc == 0) {
		break;
	    }
	    width = 2 * config_button_space + XTextWidth(buttonFont,
							  "CLOSE", 5);
	    offset = config_space;
	    widget_desc =
		Widget_create_activate(config_widget_desc[num],
				       offset, height,
				       width, config_button_height,
				       0, "CLOSE", Config_close,
				       (void *)(long)num);
	    if (widget_desc == 0) {
		break;
	    }
	    height = config_space;
	}
	if ((config_widget_ids[i] =
	     (*config_creator[i])(config_widget_desc[num], &height)) == 0) {
	    i--;
	    full = true;
	    if (height == config_space) {
		break;
	    }
	    continue;
	}
    }
    if (i < Nelem_config_creator()) {
	for (; num >= 0; num--) {
	    if (config_widget_desc[num] != 0) {
		Widget_destroy(config_widget_desc[num]);
	    }
	}
	config_created = false;
	config_mapped = false;
    } else {
	config_max = num + 1;
	config_widget_desc = (int *)realloc(config_widget_desc,
					    config_max * sizeof(int));
	config_page = 0;
	for (i = 0; i < config_max; i++) {
	    Widget_map_sub(config_widget_desc[i]);
	}
	config_created = true;
	config_mapped = false;
    }
}

static int Config_close(int widget_desc, void *data, const char **strptr)
{
    Widget_unmap(config_widget_desc[config_page]);
    config_mapped = false;
    return 0;
}

static int Config_next(int widget_desc, void *data, const char **strptr)
{
    int			prev_page = config_page;

    if (config_max > 1) {
	config_page = (config_page + 1) % config_max;
	Widget_raise(config_widget_desc[config_page]);
	Widget_unmap(config_widget_desc[prev_page]);
	config_mapped = true;
    }
    return 0;
}

static int Config_prev(int widget_desc, void *data, const char **strptr)
{
    int			prev_page = config_page;

    if (config_max > 1) {
	config_page = (config_page - 1 + config_max) % config_max;
	Widget_raise(config_widget_desc[config_page]);
	Widget_unmap(config_widget_desc[prev_page]);
	config_mapped = true;
    }
    return 0;
}

static int Config_create_bool(int widget_desc, int *height,
			      const char *str, bool val,
			      int (*callback)(int, void *, bool *),
			      void *data)
{
    int			offset,
			label_width,
			boolw;

    if (*height + 2*config_entry_height + 2*config_space >= config_height) {
	return 0;
    }
    label_width = XTextWidth(textFont, str, strlen(str))
		  + 2 * config_text_space;
    offset = config_width - (config_space + config_bool_width);
    if (config_space + label_width > offset) {
	if (*height + 3*config_entry_height + 2*config_space
	    >= config_height) {
	    return 0;
	}
    }

    Widget_create_label(widget_desc, config_space, *height
			    + (config_entry_height - config_text_height) / 2,
			label_width, config_text_height,
			0, str);
    if (config_space + label_width > offset) {
	*height += config_entry_height;
    }
    boolw = Widget_create_bool(widget_desc,
		       offset, *height
			   + (config_entry_height - config_bool_height) / 2,
		       config_bool_width,
		       config_bool_height,
		       0, val, callback, data);
    *height += config_entry_height + config_space;

    return boolw;
}

static int Config_create_int(int widget_desc, int *height,
			     const char *str, int *val, int min, int max,
			     int (*callback)(int, void *, int *), void *data)
{
    int			offset,
			label_width,
			intw;

    if (*height + 2*config_entry_height + 2*config_space >= config_height) {
	return 0;
    }
    label_width = XTextWidth(textFont, str, strlen(str))
		  + 2 * config_text_space;
    offset = config_width - (config_space + 2 * config_arrow_width
	    + config_int_width);
    if (config_space + label_width > offset) {
	if (*height + 3*config_entry_height + 2*config_space
	    >= config_height) {
	    return 0;
	}
    }
    Widget_create_label(widget_desc, config_space, *height
			+ (config_entry_height - config_text_height) / 2,
			label_width, config_text_height,
			0, str);
    if (config_space + label_width > offset) {
	*height += config_entry_height;
    }
    intw = Widget_create_int(widget_desc, offset, *height
			      + (config_entry_height - config_text_height) / 2,
			     config_int_width, config_text_height,
			     0, val, min, max, callback, data);
    offset += config_int_width;
    Widget_create_arrow_left(widget_desc, offset, *height
			     + (config_entry_height - config_arrow_height) / 2,
			     config_arrow_width, config_arrow_height,
			     0, intw);
    offset += config_arrow_width;
    Widget_create_arrow_right(widget_desc, offset, *height
			      + (config_entry_height - config_arrow_height) / 2,
			      config_arrow_width, config_arrow_height,
			      0, intw);
    *height += config_entry_height + config_space;

    return intw;
}

static int Config_create_float(int widget_desc, int *height,
			       const char *str, DFLOAT *val, DFLOAT min, DFLOAT max,
			       int (*callback)(int, void *, DFLOAT *),
			       void *data)
{
    int			offset,
			label_width,
			floatw;

    if (*height + 2*config_entry_height + 2*config_space >= config_height) {
	return 0;
    }
    label_width = XTextWidth(textFont, str, strlen(str))
		  + 2 * config_text_space;
    offset = config_width - (config_space + 2 * config_arrow_width
	    + config_float_width);
    if (config_space + label_width > offset) {
	if (*height + 3*config_entry_height + 2*config_space
	    >= config_height) {
	    return 0;
	}
    }
    Widget_create_label(widget_desc, config_space, *height
			+ (config_entry_height - config_text_height) / 2,
			label_width, config_text_height,
			0, str);
    if (config_space + label_width > offset) {
	*height += config_entry_height;
    }
    floatw = Widget_create_float(widget_desc, offset, *height
				 + (config_entry_height
				 - config_text_height) / 2,
				 config_float_width, config_text_height,
				 0, val, min, max, callback, data);
    offset += config_float_width;
    Widget_create_arrow_left(widget_desc, offset, *height
			     + (config_entry_height - config_arrow_height) / 2,
			     config_arrow_width, config_arrow_height,
			     0, floatw);
    offset += config_arrow_width;
    Widget_create_arrow_right(widget_desc, offset, *height
			      + (config_entry_height - config_arrow_height) / 2,
			      config_arrow_width, config_arrow_height,
			      0, floatw);
    *height += config_entry_height + config_space;

    return floatw;
}

static int Config_create_power(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			       "power", &power,
			       MIN_PLAYER_POWER, MAX_PLAYER_POWER,
			       Config_update_power, NULL);
}

static int Config_create_turnSpeed(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			       "turnSpeed", &turnspeed,
			       MIN_PLAYER_TURNSPEED, MAX_PLAYER_TURNSPEED,
			       Config_update_turnSpeed, NULL);
}

static int Config_create_turnResistance(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			       "turnResistance", &turnresistance,
			       MIN_PLAYER_TURNRESISTANCE,
			       MAX_PLAYER_TURNRESISTANCE,
			       Config_update_turnResistance, NULL);
}

static int Config_create_altPower(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			       "altPower", &power_s,
			       MIN_PLAYER_POWER, MAX_PLAYER_POWER,
			       Config_update_altPower, NULL);
}

static int Config_create_altTurnSpeed(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			       "altTurnSpeed", &turnspeed_s,
			       MIN_PLAYER_TURNSPEED, MAX_PLAYER_TURNSPEED,
			       Config_update_altTurnSpeed, NULL);
}

static int Config_create_altTurnResistance(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			       "altTurnResistance", &turnresistance_s,
			       MIN_PLAYER_TURNRESISTANCE,
			       MAX_PLAYER_TURNRESISTANCE,
			       Config_update_altTurnResistance, NULL);
}

static int Config_create_showMessages(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "showMessages",
			    BIT(instruments, SHOW_MESSAGES)
				? true : false,
			    Config_update_instruments,
			    (void *) SHOW_MESSAGES);
}

static int Config_create_maxMessages(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			   "maxMessages", &maxMessages, 1, MAX_MSGS,
			   NULL, NULL);
}

static int Config_create_messagesToStdout(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			   "messagesToStdout", &messagesToStdout, 0, 2,
			   NULL, NULL);
}

static int Config_create_reverseScroll(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "reverseScroll",
			    BIT(instruments, SHOW_REVERSE_SCROLL)
				? true : false,
			    Config_update_instruments,
			    (void *) SHOW_REVERSE_SCROLL);
}

#define CONFIG_CREATE_COLOR(c) \
Config_create_int(widget_desc, height, #c , &c, 0, maxColors - 1, NULL, NULL)

static int Config_create_messagesColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(messagesColor);
}

static int Config_create_oldMessagesColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(oldMessagesColor);
}

static int Config_create_mapRadar(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height,"mapRadar",
			      BIT(hackedInstruments, MAP_RADAR)
			      ? true : false,
			      Config_update_hackedInstruments,
			      (void *) MAP_RADAR);
}

static int Config_create_clientRanker(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "clientRanker",
			      BIT(hackedInstruments, CLIENT_RANKER)
			      ? true : false,
			      Config_update_hackedInstruments,
			      (void *) CLIENT_RANKER);
}

static int Config_create_showShipShapes(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height,"showShipShapes",
			      BIT(hackedInstruments, SHOW_SHIP_SHAPES)
			      ? true : false,
			      Config_update_hackedInstruments,
			      (void *) SHOW_SHIP_SHAPES);
}

static int Config_create_showMyShipShape(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height,"showMyShipShape",
			      BIT(hackedInstruments, SHOW_MY_SHIP_SHAPE)
			      ? true : false,
			      Config_update_hackedInstruments,
			      (void *) SHOW_MY_SHIP_SHAPE);
}

static int Config_create_ballMsgScan(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height,"ballMsgScan",
			      BIT(hackedInstruments, BALL_MSG_SCAN)
			          ? true : false,
			      Config_update_hackedInstruments,
			      (void *) BALL_MSG_SCAN);
}

static int Config_create_showLivesByShip(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "showLivesByShip",
			      BIT(hackedInstruments, SHOW_LIVES_BY_SHIP)
			      ? true : false,
			      Config_update_hackedInstruments,
			      (void *) SHOW_LIVES_BY_SHIP);
}

static int Config_create_showExtraBaseInfo(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "showExtraBaseInfo",
			      BIT(hackedInstruments, SHOW_EXTRA_BASE_INFO)
			      ? true : false,
			      Config_update_hackedInstruments,
			      (void *) SHOW_EXTRA_BASE_INFO);
}

static int Config_create_treatZeroSpecial(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "treatZeroSpecial",
			      BIT(hackedInstruments, TREAT_ZERO_SPECIAL)
			      ? true : false,
			      Config_update_hackedInstruments,
			      (void *) TREAT_ZERO_SPECIAL);
}

static int Config_create_speedFactHUD(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			       "speedFactHUD", &hud_move_fact, -10.0, 10.0,
			       NULL, NULL);
}

static int Config_create_speedFactPTR(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			       "speedFactPTR", &ptr_move_fact, -10.0, 10.0,
			       NULL, NULL);
}

static int Config_create_fuelNotify(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			     "fuelNotify", &fuelLevel3, 0, 1000,
			     NULL, NULL);
}

static int Config_create_fuelWarning(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			     "fuelWarning", &fuelLevel2, 0, 1000,
			     NULL, NULL);
}

static int Config_create_fuelCritical(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			     "fuelCritical", &fuelLevel1, 0, 1000,
			     NULL, NULL);
}

static int Config_create_outlineWorld(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "outlineWorld",
			      BIT(instruments, SHOW_OUTLINE_WORLD)
				  ? true : false,
			      Config_update_instruments,
			      (void *) SHOW_OUTLINE_WORLD);
}

static int Config_create_filledWorld(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "filledWorld",
			      BIT(instruments, SHOW_FILLED_WORLD)
				  ? true : false,
			      Config_update_instruments,
			      (void *) SHOW_FILLED_WORLD);
}

static int Config_create_texturedWalls(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "texturedWalls",
			      BIT(instruments, SHOW_TEXTURED_WALLS)
				  ? true : false,
			      Config_update_instruments,
			      (void *) SHOW_TEXTURED_WALLS);
}

static int Config_create_texturedObjects(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "texturedObjects",
			      (texturedObjects) ? true : false,
			      Config_update_texturedObjects,
			      NULL);
}

static int Config_create_fullColor(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "fullColor",
			      (fullColor) ? true : false,
			      Config_update_fullColor, NULL);
}

static int Config_create_slidingRadar(int widget_desc, int *height)
{
    if (Client_wrap_mode() == 0) {
	return 1;
    }
    return Config_create_bool(widget_desc, height, "slidingRadar",
			      BIT(instruments, SHOW_SLIDING_RADAR)
				  ? true : false,
			      Config_update_instruments,
			      (void *) SHOW_SLIDING_RADAR);
}

static int Config_create_backgroundPointDist(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			     "backgroundPointDist", &map_point_distance, 0, 10,
			     Config_update_dots, NULL);
}

static int Config_create_showItemsTime(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			     "showItemsTime", &showItemsTime,
			     MIN_SHOW_ITEMS_TIME,
			     MAX_SHOW_ITEMS_TIME,
			     NULL, NULL);
}

static int Config_create_showScoreDecimals(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			     "showScoreDecimals", &showScoreDecimals,
			     0,
			     2,
			     NULL, NULL);
}

static int Config_create_backgroundPointSize(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			     "backgroundPointSize", &map_point_size,
			     MIN_MAP_POINT_SIZE, MAX_MAP_POINT_SIZE,
			     Config_update_dots, NULL);
}

static int Config_create_sparkSize(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			     "sparkSize", &spark_size,
			     MIN_SPARK_SIZE, MAX_SPARK_SIZE,
			     NULL, NULL);
}

static int Config_create_sparkProb(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			       "sparkProb", &spark_prob,
			       0.0, 1.0,
			       Config_update_sparkProb, NULL);
}

static int Config_create_charsPerSecond(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			     "charsPerSecond", &charsPerSecond,
			     10, 255,
			     Config_update_charsPerSecond, NULL);
}

static int Config_create_toggleShield(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "toggleShield",
			      (toggle_shield) ? true : false,
			      Config_update_toggleShield, NULL);
}

static int Config_create_autoShield(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "autoShield",
                              (auto_shield) ? true : false,
                              Config_update_autoShield, NULL);
}

static int Config_create_shotSize(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			   "shotSize", &shot_size,
			   MIN_SHOT_SIZE, MAX_SHOT_SIZE,
			   NULL, NULL);
}

static int Config_create_teamShotSize(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			   "teamShotSize", &teamshot_size,
			   MIN_TEAMSHOT_SIZE, MAX_TEAMSHOT_SIZE,
			   NULL, NULL);
}

static int Config_create_teamShotColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(teamShotColor);
}

static int Config_create_showNastyShots(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "nastyShots",
                              (showNastyShots) ? true : false,
                              Config_update_bool, &showNastyShots);
}

static int Config_create_hudColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(hudColor);
}

static int Config_create_hudHLineColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(hudHLineColor);
}

static int Config_create_hudVLineColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(hudVLineColor);
}

static int Config_create_hudItemsColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(hudItemsColor);
}

static int Config_create_hudRadarEnemyColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(hudRadarEnemyColor);
}

static int Config_create_hudRadarOtherColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(hudRadarOtherColor);
}

static int Config_create_hudRadarDotSize(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			     "hudRadarDotSize", &hudRadarDotSize,
			     1, SHIP_SZ,
			     NULL, NULL);
}

static int Config_create_hudRadarScale(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			       "hudRadarScale", &hudRadarScale, 0.5, 4.0,
			       NULL, NULL);
}

static int Config_create_hudRadarLimit(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			       "hudRadarLimit", &hudRadarLimit,
			       0.0, 5.0,
			       NULL, NULL);
}

static int Config_create_hudSize(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			     "hudSize", &hudSize,
			     MIN_HUD_SIZE, 6 * MIN_HUD_SIZE,
			     NULL, NULL);
}

static int Config_create_hudLockColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(hudLockColor);
}

static int Config_create_visibilityBorderColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(visibilityBorderColor);
}

static int Config_create_fuelGaugeColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(fuelGaugeColor);
}

static int Config_create_dirPtrColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(dirPtrColor);
}

static int Config_create_shipShapesHackColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(shipShapesHackColor);
}

static int Config_create_msgScanBallColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(msgScanBallColor);
}

static int Config_create_msgScanSafeColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(msgScanSafeColor);
}

static int Config_create_msgScanCoverColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(msgScanCoverColor);
}

static int Config_create_msgScanPopColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(msgScanPopColor);
}

static int Config_create_selfLWColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(selfLWColor);
}

static int Config_create_enemyLWColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(enemyLWColor);
}

static int Config_create_teamLWColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(teamLWColor);
}

static int Config_create_shipNameColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(shipNameColor);
}

static int Config_create_baseNameColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(baseNameColor);
}

static int Config_create_mineNameColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(mineNameColor);
}

static int Config_create_ballColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(ballColor);
}

static int Config_create_connColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(connColor);
}

static int Config_create_fuelMeterColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(fuelMeterColor);
}

static int Config_create_powerMeterColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(powerMeterColor);
}

static int Config_create_turnSpeedMeterColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(turnSpeedMeterColor);
}

static int Config_create_packetSizeMeterColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(packetSizeMeterColor);
}

static int Config_create_packetLossMeterColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(packetLossMeterColor);
}

static int Config_create_packetDropMeterColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(packetDropMeterColor);
}

static int Config_create_packetLagMeterColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(packetLagMeterColor);
}

static int Config_create_temporaryMeterColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(temporaryMeterColor);
}

static int Config_create_meterBorderColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(meterBorderColor);
}

static int Config_create_windowColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(windowColor);
}

static int Config_create_buttonColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(buttonColor);
}

static int Config_create_borderColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(borderColor);
}

static int Config_create_clockColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(clockColor);
}

static int Config_create_scoreColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(scoreColor);
}

static int Config_create_scoreSelfColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(scoreSelfColor);
}

static int Config_create_scoreInactiveColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(scoreInactiveColor);
}

static int Config_create_scoreInactiveSelfColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(scoreInactiveSelfColor);
}

static int Config_create_scoreZeroColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(scoreZeroColor);
}

static int Config_create_scoreObjectColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(scoreObjectColor);
}

static int Config_create_scoreObjectTime(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			       "scoreObjectTime", &scoreObjectTime, 0.0, 10.0,
			       NULL, NULL);
}

static int Config_create_baseWarningType(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			     "baseWarningType", &baseWarningType, 0, 3,
			     NULL, NULL);
}

static int Config_create_wallColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(wallColor);
}

static int Config_create_fuelColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(fuelColor);
}

static int Config_create_decorColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(decorColor);
}

static int Config_create_backgroundPointColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(backgroundPointColor);
}

static int Config_create_team0Color(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(team0Color);
}
static int Config_create_team1Color(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(team1Color);
}
static int Config_create_team2Color(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(team2Color);
}
static int Config_create_team3Color(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(team3Color);
}
static int Config_create_team4Color(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(team4Color);
}
static int Config_create_team5Color(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(team5Color);
}
static int Config_create_team6Color(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(team6Color);
}
static int Config_create_team7Color(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(team7Color);
}
static int Config_create_team8Color(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(team8Color);
}
static int Config_create_team9Color(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(team9Color);
}

static int Config_create_showDecor(int widget_desc, int *height)
{
    /* kps - remove */
    return Config_create_bool(widget_desc, height, "showDecor",
			      BIT(instruments, SHOW_DECOR)
			      ? true : false,
			      Config_update_instruments,
			      (void *) SHOW_DECOR);
}

static int Config_create_outlineDecor(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "outlineDecor",
			      BIT(instruments, SHOW_OUTLINE_DECOR)
				  ? true : false,
			      Config_update_instruments,
			      (void *) SHOW_OUTLINE_DECOR);
}

static int Config_create_filledDecor(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "filledDecor",
			      BIT(instruments, SHOW_FILLED_DECOR)
			      ? true : false,
			      Config_update_instruments,
			      (void *) SHOW_FILLED_DECOR);
}

static int Config_create_texturedDecor(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "texturedDecor",
			      BIT(instruments, SHOW_TEXTURED_DECOR)
				  ? true : false,
			      Config_update_instruments,
			      (void *) SHOW_TEXTURED_DECOR);
}

#ifdef SOUND
static int Config_create_maxVolume(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			     "maxVolume", &maxVolume, 0, 255,
			     NULL, NULL);
}
#endif

static int Config_create_maxFPS(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			     "maxFPS", &maxFPS, 1, 200,
			     Config_update_maxFPS, NULL);
}

static int Config_create_clockAMPM(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "clockAMPM",
			      BIT(instruments, SHOW_CLOCK_AMPM_FORMAT)
				  ? true : false,
			      Config_update_instruments,
			      (void *) SHOW_CLOCK_AMPM_FORMAT);
}

#ifdef _WINDOWS
static int Config_create_threadedDraw(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "threadedDraw",
			      ThreadedDraw, Config_update_bool, &ThreadedDraw);
}
#endif

static int Config_create_scaleFactor(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			       "scaleFactor", &scaleFactor,
			       MIN_SCALEFACTOR, MAX_SCALEFACTOR,
			       Config_update_scaleFactor, NULL);
}

static int Config_create_altScaleFactor(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
                               "altScaleFactor", &scaleFactor_s,
                               MIN_SCALEFACTOR, MAX_SCALEFACTOR,
                               NULL, NULL);
}

static int Config_create_markingLights(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "markingLights",
			      markingLights,
			      Config_update_bool, &markingLights);
}


static int Config_create_save(int widget_desc, int *height)
{
    static char		save_str[] = "Save Configuration";
    int			space,
			button_desc,
			width = 2 * config_button_space
				+ XTextWidth(buttonFont, save_str,
					     strlen(save_str));

    space = config_height - (*height + 2*config_entry_height + 2*config_space);
    if (space < 0) {
	return 0;
    }
    button_desc =
	Widget_create_activate(widget_desc,
			       (config_width - width) / 2,
			       *height + space / 2,
			       width, config_button_height,
			       0, save_str,
			       Config_save, (void *)save_str);
    if (button_desc == NO_WIDGET) {
	return 0;
    }
    *height += config_entry_height + config_space + space;

    return 1;
}

/* General purpose update callback for booleans.
 * Requires that a pointer to the boolean value has been given as
 * client_data argument, and updates this value to the real value.
 */
static int Config_update_bool(int widget_desc, void *data, bool *val)
{
    bool*	client_data = (bool *) data;
    *client_data = *val;
    return 0;
}


static int Config_update_instruments(int widget_desc, void *data, bool *val)
{
    long		old_instruments = instruments;
    long		bit = (long) data;
    long		outline_mask = SHOW_OUTLINE_WORLD
				     | SHOW_FILLED_WORLD
				     | SHOW_TEXTURED_WALLS;

    if (*val == false) {
	CLR_BIT(instruments, bit);
    } else {
	SET_BIT(instruments, bit);
    }
    if (bit == SHOW_SLIDING_RADAR) {
	Paint_sliding_radar();
    }
    else if (bit == SHOW_DECOR) {
	if (oldServer)
	    Map_dots();
	Paint_world_radar();
    }
    
    if (BIT(bit, outline_mask) && oldServer) {
	/* only do the map recalculations if really needed. */
	if (!BIT(old_instruments, outline_mask)
	     != !BIT(instruments, outline_mask)) {
	    Map_restore(0, 0, Setup->x, Setup->y);
	    Map_blue(0, 0, Setup->x, Setup->y);
	}
    }
    if (packetDropMeterColor || packetLossMeterColor) {
	Net_init_measurement();
    }
    if (packetLagMeterColor) {
	Net_init_lag_measurement();
    }
    if (BIT(bit, SHOW_REVERSE_SCROLL)) {
	/* a callback for `reverseScroll' in the config menu */
	IFNWINDOWS( Talk_reverse_cut() );
    }

    return 0;
}

static int Config_update_hackedInstruments(int widget_desc, void *data,
					   bool *val)
{
    long		bit = (long) data;

    if (*val == false) {
	CLR_BIT(hackedInstruments, bit);
    } else {
	SET_BIT(hackedInstruments, bit);
    }

    return 0;
}


static int Config_update_dots(int widget_desc, void *data, int *val)
{
    if (val == &map_point_size && map_point_size > 1) {
	return 0;
    }
    if (oldServer)
	Map_dots();
    return 0;
}

static int Config_update_power(int widget_desc, void *data, DFLOAT *val)
{
    Send_power(*val);
    controlTime = CONTROL_TIME;
    return 0;
}

static int Config_update_turnSpeed(int widget_desc, void *data, DFLOAT *val)
{
    Send_turnspeed(*val);
    controlTime = CONTROL_TIME;
    return 0;
}

static int Config_update_turnResistance(int widget_desc, void *data, DFLOAT *val)
{
    Send_turnresistance(*val);
    return 0;
}

static int Config_update_altPower(int widget_desc, void *data, DFLOAT *val)
{
    Send_power_s(*val);
    return 0;
}

static int Config_update_altTurnSpeed(int widget_desc, void *data, DFLOAT *val)
{
    Send_turnspeed_s(*val);
    return 0;
}

static int Config_update_altTurnResistance(int widget_desc, void *data, DFLOAT *val)
{
    Send_turnresistance_s(*val);
    return 0;
}

static int Config_update_sparkProb(int widget_desc, void *data, DFLOAT *val)
{
    spark_rand = (int)(spark_prob * MAX_SPARK_RAND + 0.5f);
    Send_display();
    return 0;
}

static int Config_update_charsPerSecond(int widget_desc, void *data, int *val)
{
    return 0;
}

static int Config_update_toggleShield(int widget_desc, void *data, bool *val)
{
    Set_toggle_shield(*val != false);
    return 0;
}

static int Config_update_autoShield(int widget_desc, void *data, bool *val)
{
    Set_auto_shield(*val != false);
    return 0;
}

static int Config_update_maxFPS(int widget_desc, void *data, int *val)
{
    Check_client_fps();
    return 0;
}

static int Config_update_fullColor(int widget_desc, void *data, bool *val)
{
    if ((*val != false) != fullColor) {
	if (fullColor == false) {
	    /* see if we can use fullColor at all. */
	    fullColor = true;
	    if (Colors_init_block_bitmaps() == -1) {
		/* no we can't have fullColor. */
		fullColor = false;
		/* and redraw our widget as false. */
		*val = false;
		return 1;
	    }
	}
	else {
	    Colors_free_block_bitmaps();
	    fullColor = false;
	    texturedObjects = false;
	}
    }
    return 0;
}

static int Config_update_texturedObjects(int widget_desc, void *data, bool *val)
{
    if ((*val != false) != texturedObjects) {
	if (texturedObjects == false) {
	    /* Can't use texturedObjects without fullColor */
	    texturedObjects = true;
	    if (!fullColor) {
		/* no we can't have texturedObjects. */
		texturedObjects = false;
		/* and redraw our widget as false. */
		*val = false;
		return 1;
	    }
	}
	else {
	    texturedObjects = false;
	}
    }
    return 0;
}

static int Config_update_scaleFactor(int widget_desc, void *data, DFLOAT *val)
{
    Init_scale_array();
    
    /* Resize removed because it is not needed here */

    Scale_dashes();
    Bitmap_update_scale();
    return 0;
}

static void Config_save_failed(const char *reason, const char **strptr)
{
    if (config_save_confirm_desc != NO_WIDGET) {
	Widget_destroy(config_save_confirm_desc);
    }
    config_save_confirm_desc
	= Widget_create_confirm(reason, Config_save_confirm_callback);
    if (config_save_confirm_desc != NO_WIDGET) {
	Widget_raise(config_save_confirm_desc);
    }
    *strptr = "Saving failed...";
}

#ifndef _WINDOWS
static int Xpilotrc_add(char *line)
{
    int			size;
    char		*str;

    if (strncmp(line, "XPilot", 6) != 0 && strncmp(line, "xpilot", 6) != 0) {
	return 0;
    }
    if (line[6] != '.' && line[6] != '*') {
	return 0;
    }
    if ((str = strchr(line + 7, ':')) == NULL) {
	return 0;
    }
    size = str - (line + 7);
    if (max_xpilotrc <= 0 || xpilotrc_ptr == NULL) {
	num_xpilotrc = 0;
	max_xpilotrc = 75;
	if ((xpilotrc_ptr = (xpilotrc_t *)
		malloc(max_xpilotrc * sizeof(xpilotrc_t))) == NULL) {
	    max_xpilotrc = 0;
	    return -1;
	}
    }
    if (num_xpilotrc >= max_xpilotrc) {
	max_xpilotrc *= 2;
	if ((xpilotrc_ptr = (xpilotrc_t *) realloc(xpilotrc_ptr,
		max_xpilotrc * sizeof(xpilotrc_t))) == NULL) {
	    max_xpilotrc = 0;
	    return -1;
	}
    }
    if ((str = xp_strdup(line)) == NULL) {
	return -1;
    }
    xpilotrc_ptr[num_xpilotrc].line = str;
    xpilotrc_ptr[num_xpilotrc].size = size;
    num_xpilotrc++;
    return 0;
}

static void Xpilotrc_end(FILE *fp)
{
    int			i;

    if (max_xpilotrc <= 0 || xpilotrc_ptr == NULL) {
	return;
    }
    for (i = 0; i < num_xpilotrc; i++) {
	/* a bug in 3.2.8 saved maxFPS, which is wrong!  don't save maxFPS! */
	if (strncmp(xpilotrc_ptr[i].line + 7, "maxFPS:",
		    xpilotrc_ptr[i].size + 1) != 0) {
	    fprintf(fp, "%s", xpilotrc_ptr[i].line);
	}
	free(xpilotrc_ptr[i].line);
    }
    free(xpilotrc_ptr);
    xpilotrc_ptr = NULL;
    max_xpilotrc = 0;
    num_xpilotrc = 0;
}

static void Xpilotrc_use(char *line)
{
    int			i;

    for (i = 0; i < num_xpilotrc; i++) {
	if (strncmp(xpilotrc_ptr[i].line + 7, line + 7,
		    xpilotrc_ptr[i].size + 1) == 0) {
	    free(xpilotrc_ptr[i].line);
	    xpilotrc_ptr[i--] = xpilotrc_ptr[--num_xpilotrc];
	}
    }
}
#endif


#ifndef _WINDOWS
static void Config_save_resource(FILE *fp, const char *resource, char *value)
{
    char		buf[256];
    int			len, numtabs, i;

    sprintf(buf, "xpilot.%s:", resource);
    len = (int) strlen(buf);
#define TABSIZE 8
    /* assume tabs are max size of TABSIZE */
    numtabs = ((5 * TABSIZE - 1) - len) / TABSIZE;
    for (i = 0; i < numtabs; i++)
	strcat(buf, "\t");
    sprintf(buf + strlen(buf), "%s\n", value);
#if 0
    sprintf(buf, "xpilot.%s:\t\t%s\n", resource, value);
#endif
    Xpilotrc_use(buf);
    fprintf(fp, "%s", buf);
}
#endif


static void Config_save_comment(FILE *fp, const char *comment)
{
    fprintf(fp, "%s", comment);
}

static void Config_save_float(FILE *fp, const char *resource, DFLOAT value)
{
    char		buf[40];

    sprintf(buf, "%.3f", value);
    Config_save_resource(fp, resource, buf);
}

static void Config_save_int(FILE *fp, const char *resource, int value)
{
    char		buf[20];

    sprintf(buf, "%d", value);
    Config_save_resource(fp, resource, buf);
}

static void Config_save_bool(FILE *fp, const char *resource, int value)
{
    char		buf[20];

    sprintf(buf, "%s", (value != 0) ? "True" : "False");
    Config_save_resource(fp, resource, buf);
}


/*
 * Find a key in keyDefs[].
 * On success set output pointer to index into keyDefs[] and return true.
 * On failure return false.
 */
static int Config_find_key(keys_t key, int start, int end, int *key_index)
{
    int			i;

    for (i = start; i < end; i++) {
	if (keyDefs[i].key == key) {
	    *key_index = i;
	    return true;
	}
    }

    return false;
}

static void Config_save_keys(FILE *fp)
{
    int			i, j;
    KeySym		ks;
    keys_t		key;
    const char		*str,
			*res;
    char		buf[512];

    buf[0] = '\0';
    for (i = 0; i < maxKeyDefs; i++) {
	ks = keyDefs[i].keysym;
	key = keyDefs[i].key;

	/* try and see if we have already saved this key. */
	if (Config_find_key(key, 0, i, &j) == true) {
	    /* yes, saved this one before.  skip it now. */
	    continue;
	}

	if ((str = XKeysymToString(ks)) == NULL) {
	    continue;
	}

	if ((res = Get_keyResourceString(key)) != NULL) {
	    strlcpy(buf, str, sizeof(buf));
	    /* find all other keysyms which map to the same key. */
	    j = i;
	    while (Config_find_key(key, j + 1, maxKeyDefs, &j) == true) {
		ks = keyDefs[j].keysym;
		if ((str = XKeysymToString(ks)) != NULL) {
		    strcat(buf, " ");
		    strcat(buf, str);
		}
	    }
	    Config_save_resource(fp, res, buf);
	}
    }
}


static int Config_save(int widget_desc, void *button_str, const char **strptr)
{
    int			i;
    FILE		*fp = NULL;
    char		buf[512];
#ifndef _WINDOWS	/* Windows does no file handling on its own. */
	char	oldfile[PATH_MAX + 1],
			newfile[PATH_MAX + 1];

    *strptr = "Saving...";
    Widget_draw(widget_desc);
    Client_flush();

    Get_xpilotrc_file(oldfile, sizeof(oldfile));
    if (oldfile[0] == '\0') {
	Config_save_failed("Can't find .xpilotrc file", strptr);
	return 1;
    }
    if ((fp = fopen(oldfile, "r")) != NULL) {
	while (fgets(buf, sizeof buf, fp)) {
	    Xpilotrc_add(buf);
	}
	fclose(fp);
    }
    sprintf(newfile, "%s.new", oldfile);
    unlink(newfile);
    if ((fp = fopen(newfile, "w")) == NULL) {
	Config_save_failed("Can't open file to save to.", strptr);
	return 1;
    }
#endif

    Config_save_comment(fp,
			";\n"
			"; Config\n"
			";\n"
			"; General configuration options\n"
			";\n");
    Config_save_resource(fp, "name", name);
    Config_save_float(fp, "power", power);
    Config_save_float(fp, "turnSpeed", turnspeed);
    Config_save_float(fp, "turnResistance", turnresistance);
    Config_save_float(fp, "altPower", power_s);
    Config_save_float(fp, "altTurnSpeed", turnspeed_s);
    Config_save_float(fp, "altTurnResistance", turnresistance_s);
    Config_save_float(fp, "speedFactHUD", hud_move_fact);
    Config_save_float(fp, "speedFactPTR", ptr_move_fact);
    Config_save_float(fp, "fuelNotify", (DFLOAT)fuelLevel3);
    Config_save_float(fp, "fuelWarning", (DFLOAT)fuelLevel2);
    Config_save_float(fp, "fuelCritical", (DFLOAT)fuelLevel1);
    Config_save_bool(fp, "showMessages", BIT(instruments, SHOW_MESSAGES));
    Config_save_int(fp, "maxMessages", maxMessages);
    Config_save_int(fp, "messagesToStdout", messagesToStdout);
    Config_save_bool(fp, "reverseScroll", BIT(instruments, SHOW_REVERSE_SCROLL));
    Config_save_bool(fp, "mapRadar", BIT(hackedInstruments, MAP_RADAR));
    Config_save_bool(fp, "clientRanker", BIT(hackedInstruments, CLIENT_RANKER));
    Config_save_bool(fp, "showLivesByShip", BIT(hackedInstruments, SHOW_LIVES_BY_SHIP));
    Config_save_bool(fp, "showExtraBaseInfo", BIT(hackedInstruments, SHOW_EXTRA_BASE_INFO));
    Config_save_bool(fp, "treatZeroSpecial", BIT(hackedInstruments, TREAT_ZERO_SPECIAL));
    Config_save_bool(fp, "showShipShapes", BIT(hackedInstruments, SHOW_SHIP_SHAPES));
    Config_save_bool(fp, "showMyShipShape", BIT(hackedInstruments, SHOW_MY_SHIP_SHAPE));
    Config_save_bool(fp, "ballMsgScan", BIT(hackedInstruments, BALL_MSG_SCAN));
    Config_save_bool(fp, "slidingRadar", BIT(instruments, SHOW_SLIDING_RADAR));
    Config_save_float(fp, "showItemsTime", showItemsTime);
    Config_save_int(fp, "showScoreDecimals", showScoreDecimals);
    Config_save_bool(fp, "outlineWorld", BIT(instruments, SHOW_OUTLINE_WORLD));
    Config_save_bool(fp, "filledWorld", BIT(instruments, SHOW_FILLED_WORLD));
    Config_save_bool(fp, "texturedWalls", BIT(instruments, SHOW_TEXTURED_WALLS));
    Config_save_bool(fp, "fullColor", fullColor);
    Config_save_bool(fp, "texturedObjects", texturedObjects);
    Config_save_bool(fp, "clockAMPM", BIT(instruments, SHOW_CLOCK_AMPM_FORMAT));
    Config_save_int(fp, "backgroundPointDist", map_point_distance);
    Config_save_int(fp, "backgroundPointSize", map_point_size);
    Config_save_int(fp, "sparkSize", spark_size);
    Config_save_float(fp, "sparkProb", spark_prob);
    Config_save_int(fp, "shotSize", shot_size);
    Config_save_int(fp, "teamShotSize", teamshot_size);
    Config_save_bool(fp, "showNastyShots", showNastyShots);
    Config_save_int(fp, "hudRadarDotSize", hudRadarDotSize);
    Config_save_float(fp, "hudRadarScale", hudRadarScale);
    Config_save_float(fp, "hudRadarLimit", hudRadarLimit);
    Config_save_int(fp, "hudSize", hudSize);
    Config_save_float(fp, "scoreObjectTime", scoreObjectTime);
    Config_save_int(fp, "baseWarningType", baseWarningType);
    Config_save_bool(fp, "showDecor", BIT(instruments, SHOW_DECOR));
    Config_save_bool(fp, "outlineDecor", BIT(instruments, SHOW_OUTLINE_DECOR));
    Config_save_bool(fp, "filledDecor", BIT(instruments, SHOW_FILLED_DECOR));
    Config_save_bool(fp, "texturedDecor", BIT(instruments, SHOW_TEXTURED_DECOR));
    Config_save_int(fp, "receiveWindowSize", receive_window_size);
    Config_save_int(fp, "charsPerSecond", charsPerSecond);
    Config_save_bool(fp, "markingLights", markingLights);
    Config_save_bool(fp, "toggleShield", toggle_shield);
    Config_save_bool(fp, "autoShield", auto_shield);
    Config_save_int(fp, "clientPortStart", clientPortStart);
    Config_save_int(fp, "clientPortEnd", clientPortEnd);
#if SOUND
    Config_save_int(fp, "maxVolume", maxVolume);
#endif
#ifdef _WINDOWS
    Config_save_bool(fp, "threadedDraw", ThreadedDraw);
#endif
    Config_save_float(fp, "scaleFactor", scaleFactor);
    Config_save_float(fp, "altScaleFactor", scaleFactor_s);
    /* don't save maxFPS */

    /* colors */
    Config_save_comment(fp,
			";\n"
			"; Colors\n"
			";\n"
			"; The value 0 means transparent for the color "
			"options.\n"
			";\n");
    Config_save_int(fp, "maxColors", maxColors);
    Config_save_int(fp, "messagesColor", messagesColor);
    Config_save_int(fp, "oldMessagesColor", oldMessagesColor);
    Config_save_int(fp, "teamShotColor", teamShotColor);
    Config_save_int(fp, "hudColor", hudColor);
    Config_save_int(fp, "hudHLineColor", hudHLineColor);
    Config_save_int(fp, "hudVLineColor", hudVLineColor);
    Config_save_int(fp, "hudItemsColor", hudItemsColor);
    Config_save_int(fp, "hudRadarEnemyColor", hudRadarEnemyColor);
    Config_save_int(fp, "hudRadarOtherColor", hudRadarOtherColor);
    Config_save_int(fp, "hudLockColor", hudLockColor);
    Config_save_int(fp, "visibilityBorderColor", visibilityBorderColor);
    Config_save_int(fp, "fuelGaugeColor", fuelGaugeColor);
    Config_save_int(fp, "dirPtrColor", dirPtrColor);
    Config_save_int(fp, "shipShapesHackColor", shipShapesHackColor);
    Config_save_int(fp, "msgScanBallColor", msgScanBallColor);
    Config_save_int(fp, "msgScanSafeColor", msgScanSafeColor);
    Config_save_int(fp, "msgScanCoverColor", msgScanCoverColor);
    Config_save_int(fp, "msgScanPopColor", msgScanPopColor);
    Config_save_int(fp, "selfLWColor", selfLWColor);
    Config_save_int(fp, "enemyLWColor", enemyLWColor);
    Config_save_int(fp, "teamLWColor", teamLWColor);
    Config_save_int(fp, "shipNameColor", shipNameColor);
    Config_save_int(fp, "baseNameColor", baseNameColor);
    Config_save_int(fp, "mineNameColor", mineNameColor);
    Config_save_int(fp, "ballColor", ballColor);
    Config_save_int(fp, "connColor", connColor);
    Config_save_int(fp, "fuelMeterColor", fuelMeterColor);
    Config_save_int(fp, "powerMeterColor", powerMeterColor);
    Config_save_int(fp, "turnSpeedMeterColor", turnSpeedMeterColor);
    Config_save_int(fp, "packetSizeMeterColor", packetSizeMeterColor);
    Config_save_int(fp, "packetLossMeterColor", packetLossMeterColor);
    Config_save_int(fp, "packetDropMeterColor", packetDropMeterColor);
    Config_save_int(fp, "packetLagMeterColor", packetLagMeterColor);
    Config_save_int(fp, "temporaryMeterColor", temporaryMeterColor);
    Config_save_int(fp, "meterBorderColor", meterBorderColor);
    Config_save_int(fp, "windowColor", windowColor);
    Config_save_int(fp, "buttonColor", buttonColor);
    Config_save_int(fp, "borderColor", borderColor);
    Config_save_int(fp, "clockColor", clockColor);
    Config_save_int(fp, "scoreColor", scoreColor);
    Config_save_int(fp, "scoreSelfColor", scoreSelfColor);
    Config_save_int(fp, "scoreInactiveColor", scoreInactiveColor);
    Config_save_int(fp, "scoreInactiveSelfColor", scoreInactiveSelfColor);
    Config_save_int(fp, "scoreZeroColor", scoreZeroColor);
    Config_save_int(fp, "scoreObjectColor", scoreObjectColor);
    Config_save_int(fp, "wallColor", wallColor);
    Config_save_int(fp, "fuelColor", fuelColor);
    Config_save_int(fp, "decorColor", decorColor);
    Config_save_int(fp, "backgroundPointColor", backgroundPointColor);
    Config_save_int(fp, "team0Color", team0Color);
    Config_save_int(fp, "team1Color", team1Color);
    Config_save_int(fp, "team2Color", team2Color);
    Config_save_int(fp, "team3Color", team3Color);
    Config_save_int(fp, "team4Color", team4Color);
    Config_save_int(fp, "team5Color", team5Color);
    Config_save_int(fp, "team6Color", team6Color);
    Config_save_int(fp, "team7Color", team7Color);
    Config_save_int(fp, "team8Color", team8Color);
    Config_save_int(fp, "team9Color", team9Color);

    Config_save_comment(fp,
			";\n"
			"; Keys\n"
			";\n"
			"; The X Window System program xev can be used to\n"
			"; find out the names of keyboard keys.\n"
			";\n");
    Config_save_keys(fp);

    Config_save_comment(fp,
			";\n"
			"; Modifiers\n"
			";\n"
			"; These modify how your weapons work.\n"
			";\n");
    for (i = 0; i < NUM_MODBANKS; i++) {
	sprintf(buf, "modifierBank%d", i + 1);
	Config_save_resource(fp, buf, modBankStr[i]);
    }

    IFWINDOWS( Config_save_window_positions() );
    Config_save_comment(fp,
			";\n"
			"; Other options\n"
			";\n");


#ifndef _WINDOWS
    Xpilotrc_end(fp);
    fclose(fp);
    sprintf(newfile, "%s.bak", oldfile);
    rename(oldfile, newfile);
    unlink(oldfile);
    sprintf(newfile, "%s.new", oldfile);
    rename(newfile, oldfile);
#endif

    if (config_save_confirm_desc != NO_WIDGET) {
	Widget_destroy(config_save_confirm_desc);
	config_save_confirm_desc = NO_WIDGET;
    }

    *strptr = (char *) button_str;
    return 1;
}

static int Config_save_confirm_callback(int widget_desc, void *popup_desc, const char **strptr)
{
    if (config_save_confirm_desc != NO_WIDGET) {
	Widget_destroy((int)(long int)popup_desc);
	config_save_confirm_desc = NO_WIDGET;
    }
    return 0;
}

int Config(bool doit, int what)
{
    IFWINDOWS( Trace("***Config %d\n", doit) );

    /* kps - get rid of the old widgets, it's the most easy way */
    Config_destroy();
    if (doit == false)
	return false;

    if (what == CONFIG_DEFAULT)
	config_creator = config_creator_default;
    else if (what == CONFIG_COLORS)
	config_creator = config_creator_colors;

    Create_config();
    if (config_created == false)
	return false;

    Widget_raise(config_widget_desc[config_page]);
    config_mapped = true;
    return true;
}

void Config_destroy(void)
{
    int			i;

    if (config_created == true) {
	if (config_mapped == true) {
	    Widget_unmap(config_widget_desc[config_page]);
	    config_mapped = false;
	}
	for (i = 0; i < config_max; i++) {
	    Widget_destroy(config_widget_desc[i]);
	}
	config_created = false;
	free(config_widget_desc);
	config_widget_desc = NULL;
	config_max = 0;
	config_page = 0;
    }
}

void Config_resize(void)
{
    bool		mapped = config_mapped;

    if (config_created == true) {
	Config_destroy();
	if (mapped == true) {
	  Config(mapped, CONFIG_NONE /* kps ??? */);
	}
    }
}

void Config_redraw(void)
{
    int i;

    if (!config_mapped)
	return;

    for (i = 0; i < Nelem_config_creator(); i++) {
	Widget_draw(config_widget_ids[i]);
    }
}
