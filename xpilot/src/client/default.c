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

#include "xpclient.h"

char default_version[] = VERSION;

char myName[] = "xpilot";
char myClass[] = "XPilot";

int	baseWarningType;	/* Which type of base warning you prefer */
int	hudRadarDotSize;	/* Size for hudradar dot drawing */
double	hudRadarScale;		/* Scale for hudradar drawing */
double	hudRadarLimit;		/* Hudradar dots are not drawn if closer to
				   your ship than this factor of visible
				   range */
int	hudSize;		/* Size for HUD drawing */

/*
 * kps TODO:
 * make sure this does not exit if nickname is set using \set command.
 */
static bool Set_nickName(xp_option_t *opt, const char *value)
{
    (void)opt;
    assert(value);

    /*
     * This is a hack. User name will be used as nickname,
     * look in Set_userName().
     */
    if (strlen(value) == 0)
	return true;

    strlcpy(connectParam.nick_name, value, sizeof(connectParam.nick_name));
    CAP_LETTER(connectParam.nick_name[0]);
    if (connectParam.nick_name[0] < 'A' || connectParam.nick_name[0] > 'Z') {
	warn("Your player name \"%s\" should start with an uppercase letter.",
	     connectParam.nick_name);
	exit(1);
    }

    if (Check_nick_name(connectParam.nick_name) == NAME_ERROR) {
	xpprintf("Fixing nick from \"%s\" ", connectParam.nick_name);
	Fix_nick_name(connectParam.nick_name);
	xpprintf("to \"%s\".\n", connectParam.nick_name);
    }

    return true;
}

static bool Set_userName(xp_option_t *opt, const char *value)
{
    char *cp = getenv("XPILOTUSER");

    (void)opt;
    assert(value);

    if (cp)
	strlcpy(connectParam.user_name, cp, sizeof(connectParam.user_name));
    else
	Get_login_name(connectParam.user_name, sizeof(connectParam.user_name));

    if (strlen(value) > 0)
	strlcpy(connectParam.user_name, value, sizeof(connectParam.user_name));

    if (Check_user_name(connectParam.user_name) == NAME_ERROR) {
	xpprintf("Fixing username from \"%s\" ", connectParam.user_name);
	Fix_user_name(connectParam.user_name);
	xpprintf("to \"%s\".\n", connectParam.user_name);
    }

    /* hack - if nickname is not set, set nickname to username */
    if (strlen(connectParam.nick_name) == 0)
	Set_nickName(NULL, connectParam.user_name);

    return true;
}

static bool Set_hostName(xp_option_t *opt, const char *value)
{
    char *cp = getenv("XPILOTHOST");

    (void)opt;
    assert(value);

    connectParam.host_name[0] = '\0';
    if (cp)
	strlcpy(connectParam.host_name, cp, sizeof(connectParam.host_name));
    else
        sock_get_local_hostname(connectParam.host_name,
				sizeof(connectParam.host_name), 0);

    if (strlen(value) > 0)
	strlcpy(connectParam.host_name, value, sizeof(connectParam.host_name));

    if (Check_host_name(connectParam.host_name) == NAME_ERROR) {
	xpprintf("Fixing host from \"%s\" ", connectParam.host_name);
	Fix_host_name(connectParam.host_name);
	xpprintf("to \"%s\".\n", connectParam.host_name);
    }

    return true;
}

static const char *Get_nickName(xp_option_t *opt)
{
    (void)opt;
    return connectParam.nick_name;
}

static const char *Get_userName(xp_option_t *opt)
{
    (void)opt;
    return connectParam.user_name;
}

static const char *Get_hostName(xp_option_t *opt)
{
    (void)opt;
    return connectParam.host_name;
}

static bool Set_team(xp_option_t *opt, int value)
{
    (void)opt;
    if (value >= 0 && value < MAX_TEAMS)
	connectParam.team = value;
    else
	connectParam.team = TEAM_NOT_SET;

    return true;
}

static bool Set_texturePath(xp_option_t *opt, const char *value)
{
    (void)opt;
    if (texturePath)
	xp_free(texturePath);

    texturePath = xp_safe_strdup(value);
    warn("texturePath is now %s", texturePath);
    return true;
}
static const char *Get_texturePath(xp_option_t *opt)
{
    (void)opt;
    return texturePath;
}

/*
 * This function trys to set the shipShape variable.
 *
 * First it looks if ship_shape would be a suitable value.
 * If not, it assumes ship_shape is the name of a shape
 * to be loaded from ship_shape_file.
 */
static bool tryToSetShipShape(char *ship_shape, char *ship_shape_file)
{
    int retval;

    if (!ship_shape)
	return false;

    if (!strchr(ship_shape, '(' )) {
	FILE *fp;
	if (!ship_shape_file || strlen(ship_shape_file) == 0)
	    return false;

	/*
	 * Ok, now we assume ship_shape is the name of the shipshape
	 * and ship_shape_file contains it.
	 */
	fp = fopen(ship_shape_file, "r");
	if (!fp)
	    warn("%s: %s", ship_shape_file, strerror(errno));
	else {
	    char *ptr;
	    char *str;
	    char line[1024];

	    /*
	     * kps - this probably does not work if the ship name is
	     * at the border of two 1024 byte blocks.
	     */
	    while (fgets(line, sizeof line, fp)) {
		if ((str = strstr(line, "(name:" )) != NULL
		    || (str = strstr(line, "(NM:" )) != NULL) {
		    str = strchr(str, ':');
		    while (*++str == ' ');
		    if ((ptr = strchr(str, ')' )) != NULL)
			*ptr = '\0';
		    /* kps - don't bother about case in shipnames. */
		    if (!strcasecmp(str, ship_shape)) {
			/* Gotcha */
			if (ptr != NULL)
			    *ptr = ')';
			ship_shape = str;
			break;
		    }
		}
	    }
	    fclose(fp);
	}
    }

    /* shape definition */
    retval = Validate_shape_str(ship_shape);
    /* free previous ship shape ? */
    if (retval) {
	shipShape = ship_shape;
	return true;
    }
    return false;
}

static char *shipShapeSetting = NULL;
static char *shipShapeFileSetting = NULL;

/*
 * Shipshape options.
 */
static bool Set_shipShape(xp_option_t *opt, const char *value)
{
    (void)opt;
    if (shipShapeSetting)
	xp_free(shipShapeSetting);
    shipShapeSetting = xp_strdup(value);

    tryToSetShipShape(shipShapeSetting, shipShapeFileSetting);

    return true;
}

static const char *Get_shipShape(xp_option_t *opt)
{
    (void)opt;
    return shipShapeSetting;
}

static bool Set_shipShapeFile(xp_option_t *opt, const char *value)
{
    (void)opt;
   if (shipShapeFileSetting)
	xp_free(shipShapeFileSetting);
    shipShapeFileSetting = xp_strdup(value);

    if (shipShapeSetting)
	tryToSetShipShape(shipShapeSetting, shipShapeFileSetting);

    return true;
}

static const char *Get_shipShapeFile(xp_option_t *opt)
{
    (void)opt;
    return shipShapeFileSetting;
}

static bool Set_power(xp_option_t *opt, double val)
{
    (void)opt;
    Send_power(val);
    power = val;
    controlTime = CONTROL_TIME;
    return true;
}
static bool Set_turnSpeed(xp_option_t *opt, double val)
{
    (void)opt;
    Send_turnspeed(val);
    turnspeed = val;
    controlTime = CONTROL_TIME;
    return true;
}
static bool Set_turnResistance(xp_option_t *opt, double val)
{
    (void)opt;
    Send_turnresistance(val);
    turnresistance = val;
    return true;
}

static bool Set_altPower(xp_option_t *opt, double val)
{
    (void)opt;
    Send_power_s(val);
    power_s = val;
    controlTime = CONTROL_TIME;
    return true;
}
static bool Set_altTurnSpeed(xp_option_t *opt, double val)
{
    (void)opt;
    Send_turnspeed_s(val);
    turnspeed_s = val;
    controlTime = CONTROL_TIME;
    return true;
}
static bool Set_altTurnResistance(xp_option_t *opt, double val)
{
    (void)opt;
    Send_turnresistance_s(val);
    turnresistance_s = val;
    return true;
}

static bool Set_autoShield(xp_option_t *opt, bool val)
{
    (void)opt;
    Set_auto_shield(val);
    return true;
}

static bool Set_toggleShield(xp_option_t *opt, bool val)
{
    (void)opt;
    Set_toggle_shield(val);
    return true;
}

static bool Set_maxFPS(xp_option_t *opt, int val)
{
    (void)opt;
    maxFPS = val;
    Check_client_fps();
    return true;
}

static bool Set_sparkProb(xp_option_t *opt, double val)
{
    (void)opt;
    sparkProb = val;
    spark_rand = (int)(sparkProb * MAX_SPARK_RAND + 0.5);
    Check_view_dimensions();
    return true;
}

static bool Set_backgroundPointDist(xp_option_t *opt, int val)
{
    (void)opt;
    map_point_distance = val;
    if (oldServer)
	Map_dots();
    return true;
}

static bool Set_backgroundPointSize(xp_option_t *opt, int val)
{
    (void)opt;
    map_point_size = val;
    if (oldServer)
	Map_dots();
    return true;
}

static bool Set_showSlidingRadar(xp_option_t *opt, bool val)
{
    (void)opt;
    instruments.showSlidingRadar = val;
    Paint_sliding_radar();
    return true;
}

static bool Set_showReverseScroll(xp_option_t *opt, bool val)
{
    (void)opt;
    instruments.showReverseScroll = val;
    Talk_reverse_cut();
    return true;
}

static bool Set_showOutlineWorld(xp_option_t *opt, bool val)
{
    (void)opt;
    instruments.showOutlineWorld = val;
    if (oldServer && Setup) {
	/* don't bother to check if recalculations are really needed. */
	Map_restore(0, 0, Setup->x, Setup->y);
	Map_blue(0, 0, Setup->x, Setup->y);
    }
    return true;
}

static bool Set_showFilledWorld(xp_option_t *opt, bool val)
{
    (void)opt;
    instruments.showFilledWorld = val;
    if (oldServer && Setup) {
	/* don't bother to check if recalculations are really needed. */
	Map_restore(0, 0, Setup->x, Setup->y);
	Map_blue(0, 0, Setup->x, Setup->y);
    }
    return true;
}

static bool Set_showTexturedWalls(xp_option_t *opt, bool val)
{
    (void)opt;
    instruments.showTexturedWalls = val;
    if (oldServer && Setup) {
	/* don't bother to check if recalculations are really needed. */
	Map_restore(0, 0, Setup->x, Setup->y);
	Map_blue(0, 0, Setup->x, Setup->y);
    }
    return true;
}

static bool Set_showDecor(xp_option_t *opt, bool val)
{
    (void)opt;
    instruments.showDecor = val;
    if (!Setup)
	return true;
    if (oldServer) 
	Map_dots();
    Paint_world_radar();
    return true;
}


/* steering stuff ends */

xp_option_t default_options[] = {

    XP_NOARG_OPTION(
	"help",
	&xpArgs.help,
	XP_OPTFLAG_DEFAULT,
	"Display this help message.\n"),

    XP_NOARG_OPTION(
	"version",
	&xpArgs.version,
	XP_OPTFLAG_DEFAULT,
	"Show the source code version.\n"),

    XP_NOARG_OPTION(
	"join",
	&xpArgs.auto_connect,
	XP_OPTFLAG_DEFAULT,
	"Join the game immediately, no questions asked.\n"),

    XP_NOARG_OPTION(
	"text",
	&xpArgs.text,
	XP_OPTFLAG_DEFAULT,
	"Use the simple text interface to contact a server\n"
	"instead of the graphical user interface.\n"),

    XP_NOARG_OPTION(
	"list",
	&xpArgs.list_servers,
	XP_OPTFLAG_DEFAULT,
	"List all servers running on the local network.\n"),

    XP_STRING_OPTION(
	"shipShape",
	"",
	NULL, 0,
	Set_shipShape, NULL, Get_shipShape,
	XP_OPTFLAG_DEFAULT,
	"Define the ship shape to use.  Because the argument to this option\n"
	"is rather large (up to 500 bytes) the recommended way to set\n"
	"this option is in the .xpilotrc file in your home directory.\n"
	"The exact format is defined in the file doc/README.SHIPS in the\n"
	"XPilot distribution. Note that there is a nifty Unix tool called\n"
	"editss for easy ship creation. There is XPShipEditor for Windows\n"
	"and Ship Shaper for Java.  See the XPilot FAQ for details.\n"
	"See also the \"shipShapeFile\" option below.\n"),

    XP_STRING_OPTION(
	"shipShapeFile",
	CONF_SHIP_FILE,
	NULL, 0,
	Set_shipShapeFile, NULL, Get_shipShapeFile,
	XP_OPTFLAG_DEFAULT,
	"An optional file where shipshapes can be stored.\n"
	"If this resource is defined and it refers to an existing file\n"
	"then shipshapes can be referenced to by their name.\n"
	"For instance if you define shipShapeFile to be\n"
	"/home/myself/.shipshapes and this file contains one or more\n"
	"shipshapes then you can select the shipshape by starting xpilot as:\n"
	"	xpilot -shipShape myshipshapename\n"
	"Where \"myshipshapename\" should be the \"name:\" or \"NM:\" of\n"
	"one of the shipshapes defined in /home/myself/.shipshapes.\n"
	"Each shipshape definition should be defined on only one line,\n"
	"where all characters up to the first left parenthesis don't matter.\n"
	/* shipshopshapshepshit getting nuts from all these shpshp-s. */),

    XP_STRING_OPTION(
	"shutdown",
	"",
	xpArgs.shutdown_reason,
	sizeof xpArgs.shutdown_reason,
	NULL, NULL, NULL,
	XP_OPTFLAG_DEFAULT,
	"Shutdown the server with a message.\n"
	"The message used is the first argument to this option.\n"),

    XP_STRING_OPTION(
	"name",
	"",
	NULL, 0,
	Set_nickName, NULL, Get_nickName,
	XP_OPTFLAG_DEFAULT,
	"Set the nickname.\n"),

    XP_STRING_OPTION(
	"user",
	"",
	NULL, 0,
	Set_userName, NULL, Get_userName,
	XP_OPTFLAG_DEFAULT,
	"Set the username.\n"),

    XP_STRING_OPTION(
	"host",
	"",
	NULL, 0,
	Set_hostName, NULL, Get_hostName,
	XP_OPTFLAG_DEFAULT,
	"Set the hostname.\n"),

    XP_INT_OPTION(
	"team",
	TEAM_NOT_SET,
	0,
	TEAM_NOT_SET,
	&connectParam.team,
	Set_team,
	XP_OPTFLAG_DEFAULT,
	"Set the team to join.\n"),

    XP_INT_OPTION(
	"port",
	SERVER_PORT,
	0,
	65535,
	&connectParam.contact_port,
	NULL,
	XP_OPTFLAG_DEFAULT,
	"Set the port number of the server.\n"
	"Almost all servers use the default port, which is the recommended\n"
	"policy.  You can find out about which port is used by a server by\n"
	"querying the XPilot Meta server.\n"),

    XP_INT_OPTION(
	"clientPortStart",
	0,
	0,
	65535,
	&clientPortStart,
	NULL,
	XP_OPTFLAG_DEFAULT,
	"Use UDP ports clientPortStart - clientPortEnd (for firewalls).\n"
	/* TODO: describe what value 0 means */),

    XP_INT_OPTION(
	"clientPortEnd",
	0,
	0,
	65535,
	&clientPortEnd,
	NULL,
	XP_OPTFLAG_DEFAULT,
	"Use UDP ports clientPortStart - clientPortEnd (for firewalls).\n"),
    /*
     * kps - steering stuff, note that set functions might have to be
     * specified here.
     */

    XP_DOUBLE_OPTION(
	"power",
	55.0,
	MIN_PLAYER_POWER,
	MAX_PLAYER_POWER,
	&power,
	Set_power,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Set the engine power.\n"
	"Valid values are in the range 5-55.\n"),

    XP_DOUBLE_OPTION(
	"turnSpeed",
	35.0,
	MIN_PLAYER_TURNSPEED,
	MAX_PLAYER_TURNSPEED,
	&turnspeed,
	Set_turnSpeed,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Set the ship's turn speed.\n"
	"Valid values are in the range 4-64.\n"
	"See also turnResistance.\n"),

    XP_DOUBLE_OPTION(
	"turnResistance",
	0.0,
	MIN_PLAYER_TURNRESISTANCE,
	MAX_PLAYER_TURNRESISTANCE,
	&turnresistance,
	Set_turnResistance,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Set the ship's turn resistance.\n"
	"This determines the speed at which a ship stops turning.\n"
	"Valid values are in the range 0.0-1.0.\n"
	"This should always be 0, other values are for compatibility.\n"
	"See also turnSpeed.\n"),

    XP_DOUBLE_OPTION(
	"altPower",
	35.0,
	MIN_PLAYER_POWER,
	MAX_PLAYER_POWER,
	&power_s,
	Set_altPower,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Set the ship's alternate engine power.\n"
	"See also the keySwapSettings option.\n"),

    XP_DOUBLE_OPTION(
	"altTurnSpeed",
	25.0,
	MIN_PLAYER_TURNSPEED,
	MAX_PLAYER_TURNSPEED,
	&turnspeed_s,
	Set_altTurnSpeed,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Set the ship's alternate turn speed.\n"
	"See also the keySwapSettings option.\n"),

    XP_DOUBLE_OPTION(
	"altTurnResistance",
	0.0,
	MIN_PLAYER_TURNRESISTANCE,
	MAX_PLAYER_TURNRESISTANCE,
	&turnresistance_s,
	Set_altTurnResistance,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Set the ship's alternate turn resistance.\n"
	"See also the keySwapSettings option.\n"),

    XP_DOUBLE_OPTION(
	"scaleFactor",
	1.0,
	MIN_SCALEFACTOR,
	MAX_SCALEFACTOR,
	&scaleFactor,
	Set_scaleFactor,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Specifies scaling factor for the drawing window.\n"),

    XP_DOUBLE_OPTION(
        "altScaleFactor",
        2.0,
	MIN_SCALEFACTOR,
	MAX_SCALEFACTOR,
	&scaleFactor_s,
	Set_altScaleFactor,
	XP_OPTFLAG_CONFIG_DEFAULT,
        "Specifies alternative scaling factor for the drawing window.\n"),

    XP_INT_OPTION(
	"maxFPS",
	100,
	1,
	MAX_SUPPORTED_FPS,
	&maxFPS,
	Set_maxFPS,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Set maximum FPS supported by the client. The server will try to\n"
	"send at most this many frames per second to the client.\n"),

    XP_DOUBLE_OPTION(
	"sparkProb",
	0.4,
	0.0,
	1.0,
	&sparkProb,
	Set_sparkProb,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"The probablilty that sparks are drawn.\n"
	"This gives a sparkling effect.\n"
	"Valid values are in the range [0.0-1.0]\n"),


    /* hud stuff */
    XP_INT_OPTION(
	"hudRadarDotSize",
	8,
	1,
	SHIP_SZ,
	&hudRadarDotSize,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Which size to use for drawing the hudradar dots.\n"),

    XP_DOUBLE_OPTION(
	"hudRadarScale",
	1.5,
	0.5,
	4.0,
	&hudRadarScale,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"The relative size of the hudradar.\n"),
	
    XP_DOUBLE_OPTION(
	"hudRadarLimit",
	0.05,
	0.0,
	5.0,
	&hudRadarLimit,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Hudradar dots closer than this to your ship are not drawn.\n"
	"A value of 1.0 means that the dots are not drawn for ships in\n"
	"your active view area.\n"),

    /* kps - change to hudScale */
    XP_INT_OPTION(
	"hudSize",
	MIN_HUD_SIZE * 2,
	MIN_HUD_SIZE,
	MIN_HUD_SIZE * 6,
	&hudSize,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Which size to use for drawing the hud.\n"),

    XP_INT_OPTION(
	"baseWarningType",
	3,
	0,
	3,
	&baseWarningType,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Which type of base warning you prefer.\n"
	"A value of 0 disables base warning.\n"
	"A value of 1 draws a time meter on a base when a ship is appearing.\n"
	"A value of 2 makes the base name flash when a ship is appearing.\n"
	"A value of 3 combines the effects of values 1 and 2.\n"),

    /* instruments */
    
    XP_BOOL_OPTION(
	"mapRadar",
	true,
	&instruments.showMapRadar,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Paint radar dots' positions on the map.\n"),

    XP_BOOL_OPTION(
	"slidingRadar",
	true,
	&instruments.showSlidingRadar,
	Set_showSlidingRadar,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"If the game is in edgewrap mode then the radar will keep your\n"
	"position on the radar in the center and raw the rest of the radar\n"
	"around it.  Note that this requires a fast graphics system.\n"),

    XP_BOOL_OPTION(
	"showShipShapes",
	true,
	&instruments.showShipShapes,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Should others' shipshapes be displayed or not.\n"),

    XP_BOOL_OPTION(
	"showMyShipShape",
	true,
	&instruments.showMyShipShape,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Should your own shipshape be displayed or not.\n"),

    XP_BOOL_OPTION(
	"ballMsgScan",
	true,
	&instruments.useBallMessageScan,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Scan messages for BALL, SAFE, COVER and POP and paint\n"
	"warning circles inside ship.\n"),

    XP_BOOL_OPTION(
	"showLivesByShip",
	false,
	&instruments.showLivesByShip,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Paint remaining lives next to ships.\n"),

    XP_BOOL_OPTION(
	"showMessages",
	true,
	&instruments.showMessages,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Should game messages appear on screen.\n"),

    XP_BOOL_OPTION(
	"showItems",
	true,
	&instruments.showItems,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Should owned items be displayed permanently on the HUD?\n"),

    XP_DOUBLE_OPTION(
	"showItemsTime",
	5.0,
	MIN_SHOW_ITEMS_TIME,
	MAX_SHOW_ITEMS_TIME,
	&showItemsTime,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"If showItems is false, the time in seconds to display item\n"
	"information on the HUD when it has changed.\n"),

    XP_BOOL_OPTION(
	"filledWorld",
	false,
	&instruments.showFilledWorld,
	Set_showFilledWorld,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Draws the walls solid, filled with one color,\n"
	"unless overridden by texture.\n"
	"Be warned that this option needs fast graphics.\n"),

    XP_BOOL_OPTION(
	"texturedWalls",
	true,
	&instruments.showTexturedWalls,
	Set_showTexturedWalls,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Allows drawing polygon bitmaps specified by the (new-style) map.\n"
	"Be warned that this needs a reasonably fast graphics system.\n"),

    XP_BOOL_OPTION(
	"outlineWorld",
	false,
	&instruments.showOutlineWorld,
	Set_showOutlineWorld,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Draws only the outline of all the wall blocks\n"
	"on block based maps.\n"),

    XP_BOOL_OPTION(
	"showDecor",
	true,
	&instruments.showDecor,
	Set_showDecor,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Should decorations be displayed on the screen and radar?\n"),

    XP_BOOL_OPTION(
	"outlineDecor",
	false,
	&instruments.showOutlineDecor,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Draws only the outline of the map decoration.\n"),

    XP_BOOL_OPTION(
	"filledDecor",
	false,
	&instruments.showFilledDecor,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Draws filled decorations.\n"),

    XP_BOOL_OPTION(
	"texturedDecor",
	false,
	&instruments.showTexturedDecor,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Draws the map decoration filled with a texture pattern.\n"),

    XP_BOOL_OPTION(
	"clientRanker",
	false,
	&instruments.useClientRanker,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Scan messages and make personal kill/death ranking.\n"),

    XP_BOOL_OPTION(
	"clockAMPM",
	false,
	&instruments.useAMPMFormatClock,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Use AMPM format for clock display instead of 24 hour format.\n"),

    /* stuff drawn on map */

    XP_INT_OPTION(
	"shotSize",
	5,
	MIN_SHOT_SIZE,
	MAX_SHOT_SIZE,
	&shot_size,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"The size of shots in pixels.\n"),

    XP_INT_OPTION(
	"teamShotSize",
	3,
	MIN_TEAMSHOT_SIZE,
	MAX_TEAMSHOT_SIZE,
	&teamshot_size,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"The size of team shots in pixels.\n"
	"Note that team shots are drawn in teamShotColor.\n"),

    XP_INT_OPTION(
	"sparkSize",
	1,
	MIN_SPARK_SIZE,
	MAX_SPARK_SIZE,
	&spark_size,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Size of sparks in pixels.\n"),

    XP_INT_OPTION(
	"backgroundPointDist",
	8,
	0,
	10,
	&map_point_distance,
	Set_backgroundPointDist,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"The distance between points in the background measured in blocks.\n"
	"These are drawn in empty map regions to keep feeling for which\n"
	"direction the ship is moving to.\n"),

    XP_INT_OPTION(
	"backgroundPointSize",
	2,
	MIN_MAP_POINT_SIZE,
	MAX_MAP_POINT_SIZE,
	&map_point_size,
	Set_backgroundPointSize,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Specifies the size of the background points.  0 means no points.\n"),

    XP_DOUBLE_OPTION(
	"scoreObjectTime",
	4.0,
	0.0,
	10.0,
	&scoreObjectTime,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"How many seconds score objects remain visible on the map.\n"),

    /* mouse stuff */
    XP_BOOL_OPTION(
	"pointerControl",
	false,
	&initialPointerControl,
	NULL,
	XP_OPTFLAG_DEFAULT,
	"Enable mouse control.  This allows ship direction control by\n"
	"moving the mouse to the left for an anti-clockwise turn and\n"
	"moving the mouse to the right for a clockwise turn.\n"
	"Also see the pointerButton options for use of the mouse buttons.\n"),

    /* message stuff */

    XP_INT_OPTION(
	"charsPerSecond",
	100,
	10,
	255,
	&charsPerSecond,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Rate at which messages appear on screen in characters per second.\n"),

    XP_INT_OPTION(
	"maxMessages",
	8,
	1,
	MAX_MSGS,
	&maxMessages,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"The maximum number of messages to display at the same time.\n"),

    XP_BOOL_OPTION(
	"reverseScroll",
	false,
	&instruments.showReverseScroll,
	Set_showReverseScroll,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Reverse scroll direction of messages.\n"),

    XP_INT_OPTION(
	"messagesToStdout",
	0,
	0,
	2,
	&messagesToStdout,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Send messages to standard output.\n"
	"0: Don't.\n"
	"1: Only player messages.\n"
	"2: Player and status messages.\n"),

    XP_BOOL_OPTION(
	"selectionAndHistory",
	true,
	&selectionAndHistory,
	NULL,
	XP_OPTFLAG_DEFAULT,
	"Provide cut&paste for the player messages and the talk window and\n"
	"a `history' for the talk window.\n"),

    XP_INT_OPTION(
	"maxLinesInHistory",
	32,
	1,
	MAX_HIST_MSGS,
	&maxLinesInHistory,
	NULL,
	XP_OPTFLAG_DEFAULT,
	"Number of your messages saved in the `history' of the talk window.\n"
	"`history' is accessible with `keyTalkCursorUp/Down'.\n"),

    /* stuff you should not have to touch */


    XP_BOOL_OPTION(
	"toggleShield",
	false,
	&toggle_shield,
	Set_toggleShield,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Are shields toggled by a keypress only?\n"),

    XP_BOOL_OPTION(
	"autoShield", /* Don auto-shield hack */
	true,
	&auto_shield,
	Set_autoShield,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Are shields lowered automatically for weapon fire?\n"),

    XP_BOOL_OPTION(
	"autoServerMotdPopup",
	false,
	&autoServerMotdPopup,
	NULL,
	XP_OPTFLAG_DEFAULT,
	"Automatically popup the MOTD of the server on startup.\n"),

    XP_DOUBLE_OPTION(
	"fuelNotify",
	500.0,
	0.0,
	1000.0,
	&fuelNotify,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"The limit when the HUD fuel bar will become visible.\n"),

    XP_DOUBLE_OPTION(
	"fuelWarning",
	200.0,
	0.0,
	1000.0,
	&fuelWarning,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"The limit when the HUD fuel bar will start flashing.\n"),

    XP_DOUBLE_OPTION(
	"fuelCritical",
	100.0,
	0.0,
	1000.0,
	&fuelCritical,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"The limit when the HUD fuel bar will flash faster.\n"),

    XP_DOUBLE_OPTION(
	"speedFactHUD",
	0.0,
	-10.0,
	+10.0,
	&hud_move_fact,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"How much to move HUD to indicate the current velocity.\n"),

    XP_DOUBLE_OPTION(
	"speedFactPTR",
	0.0,
	-10.0,
	+10.0,
	&ptr_move_fact,
    	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Uses a red line to indicate the current velocity and direction.\n"),

    XP_INT_OPTION(
	"showScoreDecimals",
	1,
	0,
	2,
	&showScoreDecimals,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"The number of decimals to use when displaying scores.\n"),

    /* kps - remove option later */
    XP_INT_OPTION(
	"receiveWindowSize",
	3,
	MIN_RECEIVE_WINDOW_SIZE,
	MAX_RECEIVE_WINDOW_SIZE,
	&receive_window_size,
	NULL,
	XP_OPTFLAG_DEFAULT,
	"Too complicated.  Keep it on 3.\n"),

    /* eye candy stuff */
    XP_BOOL_OPTION(
	"markingLights",
	false,
	&markingLights,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Should the fighters have marking lights, just like airplanes?\n"),

    /* modbanks */
    XP_STRING_OPTION(
	"modifierBank1",
	"",
	modBankStr[0], sizeof modBankStr[0],
	NULL, NULL, NULL, /* kps - add set and get functions */
	XP_OPTFLAG_DEFAULT,
	"The default weapon modifier values for the first modifier bank.\n"),

    XP_STRING_OPTION(
	"modifierBank2",
	"",
	modBankStr[1], sizeof modBankStr[1],
	NULL, NULL, NULL,
	XP_OPTFLAG_DEFAULT,
	"The default weapon modifier values for the second modifier bank.\n"),

    XP_STRING_OPTION(
	"modifierBank3",
	"",
	modBankStr[2], sizeof modBankStr[2],
	NULL, NULL, NULL,
	XP_OPTFLAG_DEFAULT,
	"The default weapon modifier values for the third modifier bank.\n"),

    XP_STRING_OPTION(
	"modifierBank4",
	"",
	modBankStr[3], sizeof modBankStr[3],
	NULL, NULL, NULL,
	XP_OPTFLAG_DEFAULT,
	"The default weapon modifier values for the fourth modifier bank.\n"),

    XP_STRING_OPTION(
	"texturePath",
	CONF_TEXTUREDIR, /* was conf_texturedir_string */
	NULL, 0,
	Set_texturePath, NULL, Get_texturePath,
	XP_OPTFLAG_DEFAULT,
	"Search path for texture files.\n"
	"This is a list of one or more directories separated by colons.\n"),

    /* kps - these should not be needed in the SDL windows client. */
#ifdef _WINDOWS
    XP_BOOL_OPTION(
	"threadedDraw",
	false,
	&threadedDraw,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Tell Windows to do the heavy BitBlt in another thread\n"),

    XP_INT_OPTION(
	"radarDivisor",
	1,
	1,
	100,
	&RadarDivisor,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Specifies how many frames between radar window updates.\n"),
#endif

    XP_STRING_OPTION(
	"clientRankFile",
	"",
	clientRankFile,
	sizeof clientRankFile,
	NULL, NULL, NULL,
	XP_OPTFLAG_DEFAULT,
	"An optional file where clientside kill/death rank is stored.\n"),

    XP_STRING_OPTION(
	"clientRankHTMLFile",
	"",
	clientRankHTMLFile,
	sizeof clientRankHTMLFile,
	NULL, NULL, NULL,
	XP_OPTFLAG_DEFAULT,
	"An optional file where clientside kill/death rank is\n"
	"published in HTML format.\n"),

    XP_STRING_OPTION(
	"clientRankHTMLNOJSFile",
	"",
	clientRankHTMLNOJSFile,
	sizeof clientRankHTMLNOJSFile,
	NULL, NULL, NULL,
	XP_OPTFLAG_DEFAULT,
	"An optional file where clientside kill/death rank is\n"
	"published in HTML format, w/o JavaScript.\n"),

#ifdef SOUND
    XP_STRING_OPTION(
	"sounds",
	CONF_SOUNDFILE,
	sounds, sizeof sounds,
	NULL, NULL, NULL,
	XP_OPTFLAG_DEFAULT,
	"Specifies the sound file.\n"),

    XP_INT_OPTION(
	"maxVolume",
	100,
	0,
	255,
	&maxVolume,
	NULL,
	XP_OPTFLAG_CONFIG_DEFAULT,
	"Specifies the volume to play sounds with.\n"),

    XP_STRING_OPTION(
	"audioServer",
	"",
	audioServer, sizeof audioServer,
	NULL, NULL, NULL,
	KEY_DUMMY,
	XP_OPTFLAG_DEFAULT,
	"Specifies the audio server to use.\n"),
#endif

};

void Store_default_options(void)
{
    STORE_OPTIONS(default_options);
}



#if 0


unsigned String_hash(const char *s)
{
    unsigned		hash = 0;

    for (; *s; s++)
	hash = (((hash >> 29) & 7) | (hash << 3)) ^ *s;
    return hash;
}


static int Find_resource(XrmDatabase db, const char *resource,
			 char *result, unsigned size, int *ind)
{
#ifndef _WINDOWS
    int			i;
    size_t		len;
    char		str_name[80],
			str_class[80],
			*str_type[10];
    XrmValue		rmValue;
    unsigned		hash = String_hash(resource);

    for (i = 0;;) {
	if (hash == options[i].hash && !strcmp(resource, options[i].name)) {
	    *ind = i;
	    break;
	}
	if (++i >= NELEM(options)) {
	    warn("BUG: Can't find option \"%s\"", resource);
	    exit(1);
	}
    }
    sprintf(str_name, "%s.%s", myName, resource);
    sprintf(str_class, "%s.%c%s", myClass, toupper(*resource), resource + 1);

    if (XrmGetResource(db, str_name, str_class, str_type, &rmValue) == True) {
	if (rmValue.addr == NULL)
	    len = 0;
	else {
	    len = MIN(rmValue.size, size - 1);
	    memcpy(result, rmValue.addr, len);
	}
	result[len] = '\0';
	return 1;
    }
    strlcpy(result, options[*ind].fallback, size);

    return 0;

#else	/* _WINDOWS */
    Config_get_resource(resource, result, size, ind);

    return 1;
#endif
}


static int Get_string_resource(XrmDatabase db,
			       const char *resource, char *result,
			       unsigned size)
{
    char		*src, *dst;
    int			ind, val;

    val = Find_resource(db, resource, result, size, &ind);
    src = dst = result;
    while ((*src & 0x7f) == *src && isgraph(*src) == 0 && *src != '\0')
	src++;

    while ((*src & 0x7f) != *src || isgraph(*src) != 0)
	*dst++ = *src++;

    *dst = '\0';

    return val;
}


static void Get_int_resource(XrmDatabase db,
			     const char *resource, int *result)
{
    int			ind;
    char		resValue[MAX_CHARS];

    Find_resource(db, resource, resValue, sizeof resValue, &ind);
    if (sscanf(resValue, "%d", result) <= 0) {
	warn("Bad value \"%s\" for option \"%s\", using default...",
	     resValue, resource);
	sscanf(options[ind].fallback, "%d", result);
    }
}


static void Get_float_resource(XrmDatabase db,
			       const char *resource, double *result)
{
    int			ind;
    double		temp_result;
    char		resValue[MAX_CHARS];

    temp_result = 0.0;
    Find_resource(db, resource, resValue, sizeof resValue, &ind);
    if (sscanf(resValue, "%lf", &temp_result) <= 0) {
	warn("Bad value \"%s\" for option \"%s\", using default...",
	     resValue, resource);
	sscanf(options[ind].fallback, "%lf", &temp_result);
    }
    *result = temp_result;
}


static void Get_bool_resource(XrmDatabase db, const char *resource,
			      bool *result)
{
    int			ind;
    char		resValue[MAX_CHARS];

    Find_resource(db, resource, resValue, sizeof resValue, &ind);
    *result = (ON(resValue) ? true : false);
}

static void Get_shipshape_resource(XrmDatabase db, char **ship_shape)
{
    char		resValue[MAX(2*MSG_LEN, PATH_MAX + 1)];

    Get_resource(db, "shipShape", resValue, sizeof resValue);
    *ship_shape = xp_strdup(resValue);
    if (*ship_shape && **ship_shape && !strchr(*ship_shape, '(' )) {
	/* so it must be the name of shipshape defined in the shipshapefile. */
	Get_resource(db, "shipShapeFile", resValue, sizeof resValue);
	if (resValue[0] != '\0') {
	    FILE *fp = fopen(resValue, "r");
	    if (!fp)
		perror(resValue);
	    else {
		char *ptr;
		char *str;
		char line[1024];
		while (fgets(line, sizeof line, fp)) {
		    if ((str = strstr(line, "(name:" )) != NULL
			|| (str = strstr(line, "(NM:" )) != NULL) {
			str = strchr(str, ':');
			while (*++str == ' ');
			if ((ptr = strchr(str, ')' )) != NULL)
			    *ptr = '\0';
			if (!strcmp(str, *ship_shape)) {
			    /* Gotcha */
			    free(*ship_shape);
			    if (ptr != NULL)
				*ptr = ')';
			    *ship_shape = xp_strdup(line);
			    break;
			}
		    }
		}
		fclose(fp);
	    }
	}
    }
}



void Parse_options(int *argcp, char **argvp)
{
    char		*ptr, *str;
    int			i, j;
    int			num;
    int			firstKeyDef;
    keys_t		key;
    KeySym		ks;

    char		resValue[MAX(2*MSG_LEN, PATH_MAX + 1)];

#ifndef _WINDOWS

    if (Get_string_resource(argDB, "display", connectParam.disp_name, MAX_DISP_LEN) == 0
	|| connectParam.disp_name[0] == '\0') {
	if ((ptr = getenv(DISPLAY_ENV)) != NULL)
	    strlcpy(connectParam.disp_name, ptr, MAX_DISP_LEN);
	else
	    strlcpy(connectParam.disp_name, DISPLAY_DEF, MAX_DISP_LEN);
    }
    if ((dpy = XOpenDisplay(connectParam.disp_name)) == NULL) {
	error("Can't open display '%s'", connectParam.disp_name);
	if (strcmp(connectParam.disp_name, "NO_X") == 0) {
	    /* user does not want X stuff.  experimental.  use at own risk. */
	    if (*connectParam.user_name)
		strlcpy(connectParam.nick_name, connectParam.user_name, MAX_NAME_LEN);
	    else
		strlcpy(connectParam.nick_name, "X", MAX_NAME_LEN);
	    connectParam.team = TEAM_NOT_SET;
	    Get_int_resource(argDB, "port", &connectParam.contact_port);
	    Get_bool_resource(argDB, "list", &xpArgs.list_servers);
	    xpArgs.text = true;
	    xpArgs.auto_connect = false;
	    XrmDestroyDatabase(argDB);
	    free(xopt);
	    return;
	}
	exit(1);
    }

    Get_string_resource(rDB, "geometry", resValue, sizeof resValue);
    geometry = xp_strdup(resValue);
#endif

    Get_shipshape_resource(rDB, &shipShape);
    Validate_shape_str(shipShape);

    Get_bool_resource(rDB, "fullColor", &fullColor);
    Get_bool_resource(rDB, "texturedObjects", &texturedObjects);
    if (!fullColor) {
	texturedObjects = false;
	instruments.showTexturedWalls = false;
    }

    Get_resource(rDB, "recordFile", resValue, sizeof resValue);
    Record_init(resValue);

    Get_resource(rDB, "texturePath", resValue, sizeof resValue);
    texturePath = xp_strdup(resValue);

    Get_int_resource(rDB, "maxFPS", &maxFPS);
    oldMaxFPS = maxFPS;

    /* Key bindings - removed */
    /* Pointer button bindings - removed */


#ifdef SOUND
    audioInit(connectParam.disp_name);
#endif /* SOUND */
}


#endif

