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

#include "xpserver.h"

char command_version[] = VERSION;


/*
 * Look if any player's name is exactly 'str',
 * If not, look if any player's name contains 'str'.
 * The matching is case insensitive. If there is an
 * error (no matches or several matches) NULL is returned
 * and the error code is stored in 'error' if that is not NULL
 * and a string describing the error is stored in
 * 'errorstr_p' if that is not NULL.
 */
player *Get_player_by_name(char *str, int *error, char **errorstr_p)
{
    int i;
    player *found_pl = NULL, *pl;
    size_t len = strlen(str);

    if (str == NULL || (len = strlen(str)) == 0)
	goto match_none;

    /* kps - do we want to match on id ? */

    /* Look for an exact match on player nickname. */
    for (i = 0; i < NumPlayers; i++) {
	pl = Players(i);
	if (strcasecmp(pl->name, str) == 0)
	    return pl;
    }

    /* Look if 'str' matches beginning of only one nick. */
    for (i = 0; i < NumPlayers; i++) {
	pl = Players(i);

	if (!strncasecmp(pl->name, str, len)) {
	    if (found_pl)
		goto match_several;
	    found_pl = pl;
	    break;
	}
    }
    if (found_pl)
	return found_pl;

    /*
     * Check what players' name 'str' is a substring of (case insensitively).
     */
    for (i = 0; i < NumPlayers; i++) {
	int j;
	pl = Players(i);

	for (j = 0; j < 1 + (int)strlen(pl->name) - (int)len; j++) {
	    if (!strncasecmp(pl->name + j, str, len)) {
		if (found_pl)
		    goto match_several;
		found_pl = pl;
		break;
	    }
	}
    }
    if (found_pl)
	return found_pl;

 match_none:
    if (error != NULL)
	*error = -1;
    if (errorstr_p != NULL)
	*errorstr_p = "Name does not match any player.";
    return NULL;

 match_several:
    if (error != NULL)
	*error = -2;
    if (errorstr_p != NULL)
	*errorstr_p = "Name matches several players.";
    return NULL;
}


int Get_player_index_by_name(char *name)
{
    int			i, j, len;

    if (!name || !*name) {
	return -1;
    }

    /* Id given directly */
    if (isdigit(*name)) {
	i = atoi(name);
	if ((i > 0 && i <= NUM_IDS)
	    && (j = GetInd(i)) >= 0
	    && j < NumPlayers
	    && Players(j)->id == i) {
	    return j;
	}
	else {
	    return -1;
	}
    }

    /* look for an exact match on player nickname. */
    for (i = 0; i < NumPlayers; i++) {
	if (strcasecmp(Players(i)->name, name) == 0) {
	    return i;
	}
    }

    /* now look for a partial match on both nick and realname. */
    len = strlen(name);
    for (j = -1, i = 0; i < NumPlayers; i++) {
	if (strncasecmp(Players(i)->name, name, len) == 0
	    || strncasecmp(Players(i)->realname, name, len) == 0) {
	    j = (j == -1) ? i : -2;
	}
    }

    return j;
}


static void Send_info_about_player(player * pl)
{
    int			i;

    for (i = 0; i < observerStart + NumObservers; i++) {
	player *pl_i;
	/* hack */
	if (i == NumPlayers) {
	    if (!NumObservers) {
		break;
	    } else {
		i = observerStart;
	    }
	}
	pl_i = Players(i);
	if (pl_i->conn != NULL) {
	    Send_player(pl_i->conn, pl->id);
	    Send_score(pl_i->conn, pl->id, pl->score, pl->life,
		       pl->mychar, pl->alliance);
	    Send_base(pl_i->conn, pl->id, pl->home_base);
	}
    }
}


static void Set_swapper_state(player * pl)
{
    if (BIT(pl->have, HAS_BALL))
	Detach_ball(pl, -1);
    if (BIT(World.rules->mode, LIMITED_LIVES)) {
	int i;

	for (i = 0; i < NumPlayers; i++) {
	    player *pl_i = Players(i);

	    if (!TEAM(pl, pl_i) && !BIT(pl_i->status, PAUSE)) {
		/* put team swapping player waiting mode. */
		if (pl->mychar == ' ')
		    pl->mychar = 'W';
		pl->prev_life = pl->life = 0;
		SET_BIT(pl->status, GAME_OVER | PLAYING);
		CLR_BIT(pl->status, SELF_DESTRUCT);
		pl->count = -1;
		break;
	    }
	}
    }
}


#define CMD_RESULT_SUCCESS		0
#define CMD_RESULT_ERROR		(-1)
#define CMD_RESULT_NOT_OPERATOR		(-2)
#define CMD_RESULT_NO_NAME		(-3)


static int Cmd_addr(char *arg, player *pl, int oper, char *msg);
static int Cmd_advance(char *arg, player *pl, int oper, char *msg);
static int Cmd_ally(char *arg, player *pl, int oper, char *msg);
/*static int Cmd_auth(char *arg, player *pl, int oper, char *msg);*/
static int Cmd_get(char *arg, player *pl, int oper, char *msg);
static int Cmd_help(char *arg, player *pl, int oper, char *msg);
static int Cmd_kick(char *arg, player *pl, int oper, char *msg);
static int Cmd_lock(char *arg, player *pl, int oper, char *msg);
static int Cmd_mutezero(char *arg, player *pl, int oper, char *msg);
static int Cmd_nuke(char *arg, player *pl, int oper, char *msg);
static int Cmd_op(char *arg, player *pl, int oper, char *msg);
static int Cmd_password(char *arg, player *pl, int oper, char *msg);
static int Cmd_pause(char *arg, player *pl, int oper, char *msg);
static int Cmd_queue(char *arg, player *pl, int oper, char *msg);
static int Cmd_reset(char *arg, player *pl, int oper, char *msg);
static int Cmd_set(char *arg, player *pl, int oper, char *msg);
/*static int Cmd_setpass(char *arg, player *pl, int oper, char *msg);*/
static int Cmd_stats(char *arg, player *pl, int oper, char *msg);
static int Cmd_team(char *arg, player *pl, int oper, char *msg);
static int Cmd_version(char *arg, player *pl, int oper, char *msg);
static int Cmd_zerolock(char *arg, player *pl, int oper, char *msg);


typedef struct {
    const char		*name;
    const char		*abbrev;
    const char		*help;
    int			oper_only;
    int			(*cmd)(char *arg, player *pl, int oper, char *msg);
} Command_info;


/*
 * A list of all the commands sorted alphabetically.
 */
static Command_info commands[] = {
    {
	"addr",
	"addr",
	"/addr <player name or ID number>. Show IP-address of player.  "
	"(operator)",
	1,
	Cmd_addr
    },
    {
	"advance",
	"ad",
	"/advance <name of player in the queue>. "
	"Move the player to the front of the queue.  (operator)",
	1,
	Cmd_advance
    },
    {
	"ally",
	"al",
	"/ally {invite|cancel|refuse|accept|leave|list} [<player name>]. "
	"Manages alliances and invitations for them.",
	0,
	Cmd_ally
    },
#if 0
    {
	"auth",
	"au",
	"/auth <password>.  Use this command if your nick is "
	"password-protected. ",
	0,
	Cmd_auth
    },
#endif
    {
	"get",
	"g",
	"/get <option>.  Gets a server option.",
	0,
	Cmd_get
    },
    {
	"help",
	"h",
	"Print command list.  /help <command> gives more info.",
	0,
	Cmd_help
    },
    {
	"kick",
	"k",
	"/kick <player name or ID number>.  Remove a player from game.  "
	"(operator)",
	1,
	Cmd_kick
    },
    {
	"lock",
	"l",
	"Just /lock tells lock status.  /lock 1 locks, /lock 0 unlocks.  "
	"(operator)",
	0,      /* checked in the function */
	Cmd_lock
    },
    {
	"mutezero",
	"m",
	"Just /mute 1 mutes, /mute 0 unmutes team zero.  (operator)",
	0,      /* checked in the function */
	Cmd_mutezero
    },
    {
	"nuke",
	"n",
	"/nuke [player name]. Nuke player's score. (operator)",
	1,
	Cmd_nuke
    },
    {
	"op",
	"o",
	"/op <command> [player name or ID number]. Operator commands.  "
	"(operator)",
	1,
	Cmd_op
    },
    {
	"password",
	"pas",
	"/password <string>.  If string matches -password option "
	"gives operator status.",
	0,
	Cmd_password
    },
    {
	"pause",
	"pau",
	"/pause <player name or ID number>.  Pauses player.  (operator)",
	1,
	Cmd_pause
    },
    {
	"queue",
	"q",
	"/queue.  Show the names of players waiting to enter.",
	0,
	Cmd_queue
    },
    {
	"reset",
	"r",
	"Just /reset re-starts the round. "
	"/reset.  Resets all scores to 0.  (operator)",
	1,
	Cmd_reset
    },
    {
	"set",
	"s",
	"/set <option> <value>.  Sets a server option.  (operator)",
	1,
	Cmd_set
    },
#if 0
    {
	"setpass",
	"setp",
	"/setpass <new pw> <new pw> [old pw]. "
	"Protects your nick with a password. ",
	0,
	Cmd_setpass
    },
#endif
    {
	"stats",
	"st",
	"/stats [player name or ID number].  Show player ranking info.",
	0,
	Cmd_stats
    },
    {
	"team",
	"t",
	"/team <team number> [name] swaps you to given team. "
	"Can be used with full teams too.",
	0,
	Cmd_team
    },
    {
	"version",
	"v",
	"Print server version.",
	0,
	Cmd_version
    },
    {
	"zerolock",
	"z",
	"Just /zerolock 1 locks, /zerolock 0 unlocks team zero.  (operator)",
	0,      /* checked in the function */
	Cmd_zerolock
    },
};


/*
 * cmd parameter has no leading slash.
 */
void Handle_player_command(player *pl, char *cmd_orig)
{
    int			i, result;
    char		*args;
    char		msg[MSG_LEN];
    char		cmd[MAX_CHARS];

    /* Make a copy so teamZeroPausing hack can at least use constant strings
     * when calling this function. Maybe fix it in nicer ways later. */
    strlcpy(cmd, cmd_orig, sizeof(cmd));
    cmd[sizeof(cmd) - 1] = 0;

    if (!*cmd) {
	strlcpy(msg,
		"No command given.  Type /help for help.  [*Server reply*]",
		sizeof(msg));
	Set_player_message(pl, msg);
	return;
    }

    args = strchr(cmd + 1, ' ');
    if (!args) {
	/* point to end of string. */
	args = cmd + strlen(cmd);
    }
    else {
	/* zero terminate cmd and advance 1 byte. */
	*args++ = '\0';
	while (isspace(*args))
	    args++;
    }

    /*
     * Command matching changed so that setpass command can be
     * given.
     */
    for (i = 0; i < NELEM(commands); i++) {
	size_t len1 = strlen(commands[i].abbrev);
	size_t len2 = strlen(cmd);

	if (!strncasecmp(cmd, commands[i].name,
			 MAX(len1, len2))) {
	    break;
	}
    }

    if (i == NELEM(commands)) {
	sprintf(msg, "Unknown command '%s'.  [*Server reply*]", cmd);
	Set_player_message(pl, msg);
	return;
    }
#if 0 /* kps - recording related stuff too obscure */
    else if (!pl->isoperator && (commands[i].operOnly || rplayback && !playback && commands[i].number != PASSWORD_CMD)) {
	i = NO_CMD;
	sprintf(msg, "You need operator status to use this command.");
    }
#endif

    msg[0] = '\0';
    result = (*commands[i].cmd)(args, pl, pl->isoperator, msg);
    switch (result) {
    case CMD_RESULT_SUCCESS:
	break;

    case CMD_RESULT_ERROR:
	if (msg[0] == '\0') {
	    strcpy(msg, "Error.");
	}
	break;

    case CMD_RESULT_NOT_OPERATOR:
	if (msg[0] == '\0') {
	    strlcpy(msg,
		    "You need operator status to use this command.",
		    sizeof(msg));
	}
	break;

    case CMD_RESULT_NO_NAME:
	if (msg[0] == '\0') {
	    strlcpy(msg,
		    "You must give a player name as an argument.",
		    sizeof(msg));
	}
	break;

    default:
	strcpy(msg, "Bug.");
	break;
    }

    if (msg[0]) {
	strlcat(msg, " [*Server reply*]", sizeof(msg));
	Set_player_message(pl, msg);
    }
}



static int Cmd_addr(char *arg, player *pl, int oper, char *msg)
{
    player *pl2 = NULL;
    char *errorstr;

    if (!oper)
	return CMD_RESULT_NOT_OPERATOR;

    if (!arg || !*arg)
	return CMD_RESULT_NO_NAME;

    pl2 = Get_player_by_name(arg, NULL, &errorstr);
    if (pl2) {
	const char *addr = Player_get_addr(pl2);
	if (addr == NULL)
	    sprintf(msg, "Unable to get address for %s.", pl2->name);
	else
	    sprintf(msg, "%s plays from: %s.", pl2->name, addr);
    } else {
	strcpy(msg, errorstr);
	return CMD_RESULT_ERROR;
    }

    return CMD_RESULT_SUCCESS;
}


/*
 * The queue system from the original server is not replicated
 * during playback. Therefore interactions with it in the
 * recording can cause problems (at least different message
 * lengths in acks from client). It would be possible to work
 * around this, but not implemented now. Currently queue and advance
 * commands are disabled during recording.
 */
static int Cmd_advance(char *arg, player *pl, int oper, char *msg)
{
    int			result;

    if (!oper) {
	return CMD_RESULT_NOT_OPERATOR;
    }

    if (record || playback) {
	strcpy(msg, "Command currently disabled during recording for "
	       "technical reasons.");
	return CMD_RESULT_ERROR;
    }

    if (!arg || !*arg) {
	return CMD_RESULT_NO_NAME;
    }

    result = Queue_advance_player(arg, msg);

    if (result < 0) {
	return CMD_RESULT_ERROR;
    }

    return CMD_RESULT_SUCCESS;
}


static int Cmd_ally(char *arg, player *pl, int oper, char *msg)
{
    char		*command;
    int			result = CMD_RESULT_SUCCESS;
    static const char	usage[] = "Usage: "
			"/ally {invite|cancel|refuse|accept|leave|list} "
			"[<player name>]";
    static const char *cmds[] = {
	"invite",
	"cancel",
	"refuse",
	"accept",
	"leave",
	"list",
    };
    enum AllyCmds {
	AllyInvite  = 0,
	AllyCancel  = 1,
	AllyRefuse  = 2,
	AllyAccept  = 3,
	AllyLeave   = 4,
	AllyList    = 5,
	NumAllyCmds = 6
    };
    int			i, cmd;

    if (!BIT(World.rules->mode, ALLIANCES)) {
	strlcpy(msg, "Alliances are not allowed.", MSG_LEN);
	result = CMD_RESULT_ERROR;
    }
    else if (!arg || !(command = strtok(arg, " \t"))) {
	strlcpy(msg, usage, MSG_LEN);
	result = CMD_RESULT_ERROR;
    }
    else {
	if ((arg = strtok(NULL, "")) != NULL) {
	    while (*arg == ' ') {
		++arg;
	    }
	}
	cmd = -1;
	for (i = 0; i < NumAllyCmds; i++) {
	    if (!strncasecmp(cmds[i], command, strlen(command))) {
		cmd = (cmd == -1) ? i : (-2);
	    }
	}
	if (cmd < 0) {
	    strlcpy(msg, usage, MSG_LEN);
	    result = CMD_RESULT_ERROR;
	}
	else if (arg) {
	    /* a name is specified */
	    char *errorstr;
	    player *pl2 = Get_player_by_name(arg, NULL, &errorstr);
	    if (pl2) {
		if (cmd == AllyInvite)
		    Invite_player(pl, pl2);
		else if (cmd == AllyRefuse)
		    Refuse_alliance(pl, pl2);
		else if (cmd == AllyAccept)
		    Accept_alliance(pl, pl2);
		else {
		    strlcpy(msg, usage, MSG_LEN);
		    result = CMD_RESULT_ERROR;
		}
	    } else {
		strcpy(msg, errorstr);
		result = CMD_RESULT_ERROR;
	    }
	} else {
	    /* no player name is specified */
	    if (cmd == AllyCancel) {
		Cancel_invitation(pl);
	    }
	    else if (cmd == AllyRefuse) {
		Refuse_all_alliances(pl);
	    }
	    else if (cmd == AllyAccept) {
		Accept_all_alliances(pl);
	    }
	    else if (cmd == AllyLeave) {
		Leave_alliance(pl);
	    }
	    else if (cmd == AllyList) {
		Alliance_player_list(pl);
	    }
	    else {
		strlcpy(msg, usage, MSG_LEN);
		result = CMD_RESULT_ERROR;
	    }
	}
    }
    return result;
}

#if 0
static int Cmd_auth(char *arg, player *pl, int oper, char *msg)
{
    int r, i = -1;

    if (!allowPlayerPasswords) {
	strcpy(msg, "Player passwords are disabled on this server.");
	return CMD_RESULT_ERROR;
    }

    if (!*pl->auth_nick) {
	strcpy(msg, "You're already authenticated or your nick isn't "
	       "password-protected.");
	return CMD_RESULT_ERROR;
    }

    if (!arg || !*arg) {
	strcpy(msg, "Need a password.");
	return CMD_RESULT_ERROR;
    }

    while (*arg == ' ')
	arg++;
    if (!*arg) {
	strcpy(msg, "Need a password.");
	return CMD_RESULT_ERROR;
    }
    r = Check_player_password(pl->auth_nick, arg);
    if (r & (PASSWD_WRONG | PASSWD_ERROR)) {
	char *reason = NULL, *reason_p = NULL;
	if (r & PASSWD_ERROR)
	    reason_p = "Couldn't check password";
	else
	    reason_p = "Wrong password";
	if (reason_p)
	    warn("Authentication failed (%s -> %s): %s.",
		 pl->name, pl->auth_nick, reason_p);
	sprintf(msg, "Authentication (->%s) failed: %s.",
		pl->auth_nick, reason ? reason : reason_p);
	return CMD_RESULT_ERROR;
    }

    sprintf(msg, "\"%s\" successfully authenticated (%s).",
	    pl->name, pl->auth_nick);
    warn(msg);
    Set_message(msg);

    Queue_kick(pl->auth_nick);

    for (i = 0; i < NumPlayers; i++) {
	player *pl_i = Players(i);
	if (pl != pl_i &&
	    !strcasecmp(pl_i->auth_nick, pl->auth_nick))
	{
	    sprintf(msg, "%s has been kicked out (nick collision).",
		    pl_i->name);
	    if (pl_i->conn == NULL)
		Delete_player(pl_i);
	    else
		Destroy_connection(pl_i->conn,
				   "kicked out (someone else authenticated "
				   "for the same nick)");
	    warn(msg);
	    Set_message(msg);
	}
    }

    /* kps - what to do about the ranking stuff here ? */
    /*Rank_save_score(pl);*/
    Conn_change_nick(pl->conn, pl->auth_nick);
    strcpy(pl->name, pl->auth_nick);
    /*Rank_get_saved_score(pl);*/
    Send_info_about_player(pl);
    pl->auth_nick[0] = 0;

    *msg = 0;

    return CMD_RESULT_SUCCESS;
}
#endif

static int Cmd_get(char *arg, player *pl, int oper, char *msg)
{
    char value[MAX_CHARS];
    int i;

    if (!arg || !*arg) {
	strcpy(msg, "Usage: /get option.");
	return CMD_RESULT_ERROR;
    }

    if (!strcasecmp(arg, "password") ||
	!strcasecmp(arg, "mapData")) {
	strcpy(msg, "Cannot retrieve that option.");
	return CMD_RESULT_ERROR;
    }

    i = Get_option_value(arg, value, sizeof(value));

    switch (i) {
    case 1:
	sprintf(msg, "The value of %s is %s.", arg, value);
	return CMD_RESULT_SUCCESS;
    case -2:
	sprintf(msg, "No option named %s.", arg);
	break;
    default:
	strcpy(msg, "Generic error.");
	break;
    }

    return CMD_RESULT_ERROR;
}


static int Cmd_help(char *arg, player *pl, int oper, char *msg)
{
    int			i;

    if (!*arg) {
	strcpy(msg, "Commands: ");
	for(i = 0; i < NELEM(commands); i++) {
	    strcat(msg, commands[i].name);
	    strcat(msg, " ");
	}
    }
    else {
	for (i = 0; i < NELEM(commands); i++) {
	    if (!strncasecmp(arg, commands[i].name,
			     strlen(commands[i].abbrev))) {
		break;
	    }
	}
	if (i == NELEM(commands)) {
	    sprintf(msg, "No help for nonexistent command '%s'.", arg);
	}
	else {
	    strcpy(msg, commands[i].help);
	}
    }

    return CMD_RESULT_SUCCESS;
}


static int Cmd_kick(char *arg, player *pl, int oper, char *msg)
{
    player		*kicked_pl;
    char		*errorstr;

    if (!oper)
	return CMD_RESULT_NOT_OPERATOR;

    if (!arg || !*arg)
	return CMD_RESULT_NO_NAME;

    kicked_pl = Get_player_by_name(arg, NULL, &errorstr);
    if (kicked_pl) {
	sprintf(msg, "%s kicked %s out! [*Server notice*]",
		pl->name, kicked_pl->name);
	if (kicked_pl->conn == NULL)
	    Delete_player(kicked_pl);
	else
	    Destroy_connection(kicked_pl->conn, "kicked out");
	Set_message(msg);
	strcpy(msg, "");
	return CMD_RESULT_SUCCESS;
    }
    strcpy(msg, errorstr);

    return CMD_RESULT_ERROR;
}


static int Cmd_lock(char *arg, player *pl, int oper, char *msg)
{
    int			new_lock;

    if (!arg || !*arg) {
	sprintf(msg, "The game is currently %s.",
		game_lock ? "locked" : "unlocked");
	return CMD_RESULT_SUCCESS;
    }

    if (!oper) {
	return CMD_RESULT_NOT_OPERATOR;
    }

    if (!strcmp(arg, "1")) {
	new_lock = 1;
    }
    else if (!strcmp(arg, "0")) {
	new_lock = 0;
    }
    else {
	sprintf(msg, "Invalid argument '%s'.  Specify either 0 or 1.", arg);
	return CMD_RESULT_ERROR;
    }

    if (new_lock == game_lock) {
	sprintf(msg, "Game is already %s.",
		game_lock ? "locked" : "unlocked");
    }
    else {
	game_lock = new_lock;
	sprintf(msg, " < The game has been %s by %s! >",
		game_lock ? "locked" : "unlocked",
		pl->name);
	Set_message(msg);
	strcpy(msg, "");
    }

    return CMD_RESULT_SUCCESS;
}


static int Cmd_mutezero(char *arg, player *pl, int oper, char *msg)
{
    int			new_mute;

    if (!teamZeroPausing) {
	sprintf(msg, "There is no team zero pausing here (set in mapfile).");
	return CMD_RESULT_ERROR;
    }

    if (!arg || !*arg) {
	sprintf(msg, "Team zero is currently %s.",
		mute_zero ? "muted" : "unmuted");
	return CMD_RESULT_SUCCESS;
    }

    if (!oper) {
	return CMD_RESULT_NOT_OPERATOR;
    }

    if (!strcmp(arg, "1")) {
	new_mute = 1;
    }
    else if (!strcmp(arg, "0")) {
	new_mute = 0;
    }
    else {
	sprintf(msg, "Invalid argument '%s'.  Specify either 0 or 1.", arg);
	return CMD_RESULT_ERROR;
    }

    if (new_mute == mute_zero) {
	sprintf(msg, "Team zero is already %s.",
		mute_zero ? "muted" : "unmuted");
    }
    else {
	mute_zero = new_mute;
	sprintf(msg, " < Team zero has been %s by %s! >",
		mute_zero ? "muted" : "unmuted",
		pl->name);
	Set_message(msg);
	strcpy(msg, "");
    }

    return CMD_RESULT_SUCCESS;
}


static int Cmd_nuke(char *arg, player *pl, int oper, char *msg)
{
    RankInfo *rank;
    int ind;

    if (!oper) {
	return CMD_RESULT_NOT_OPERATOR;
    }

    if (!arg || !*arg) {
	return CMD_RESULT_NO_NAME;
    }

    ind = Get_player_index_by_name(arg);

    /* hopefully this will help some weird issues */
    if (ind >= 0)
	rank = Rank_get_by_name(Players(ind)->name);
    else
	rank = Rank_get_by_name(arg);

    if (!rank) {
	sprintf(msg,"Name does not match any player.");
	return CMD_RESULT_ERROR;
    }

    if (ind >= 0) {
	Players(ind)->score = 0;
    }

    sprintf(msg, "Nuked %s.", rank->entry.nick);

    Rank_nuke_score(rank);

    return CMD_RESULT_SUCCESS;
}

/* kps - this one is a bit obscure, maybe clean it up a bit ? */
static int Cmd_op(char *arg, player *pl, int oper, char *msg)
{
    player *issuer = pl;
    char *origarg = arg;
    char *name;
    int cmd, priv;

    if (!oper)
	return CMD_RESULT_NOT_OPERATOR;

    if (!arg || (*arg != '+' && *arg != '-')) {
	sprintf(msg, "Usage: /op {+|-}[nlo]+ <player name>");
	return CMD_RESULT_ERROR;
    }

    name = strpbrk(arg, " \t");
    if (name) {
	char *errorstr;

	*name++ = '\0';
	while (isspace(*name))
	    name++;

	pl = Get_player_by_name(name, NULL, &errorstr);
	if (!pl) {
	    strcpy(msg, errorstr);
	    return CMD_RESULT_ERROR;
	}
    }

    priv = 0;
    cmd = *arg;
    arg++;
    while (*arg) {
	switch (*arg) {
	case 'n':
	    priv |= PRIV_NOAUTOKICK;
	    break;
	case 'l':
	    priv |= PRIV_AUTOKICKLAST;
	    break;
	case 'o':
	    if (cmd == '+')
		pl->isoperator = 1;
	    else
		pl->isoperator = 0;
	    break;
	default:
	    sprintf(msg, "Invalid operator command '%c'.", *arg);
	    return CMD_RESULT_ERROR;
	}
	arg++;
    }
    if (cmd == '+') {
	pl->privs |= priv;
    } else {
	pl->privs &= ~priv;
    }
    if (pl != issuer) {
	sprintf(msg, "%s executed '/op %s' on you. [*Server notice*]",
		issuer->name, origarg);
	Set_player_message(pl, msg);
    }
    sprintf(msg, "Executed '/op %s' on %s", origarg, pl->name);

    return CMD_RESULT_SUCCESS;
}


static int Cmd_password(char *arg, player *pl, int oper, char *msg)
{
    if (!password || !arg || strcmp(arg, password)) {
	strcpy(msg, "Wrong.");
	if (pl->isoperator && pl->rectype != 2) {
	    NumOperators--;
	    pl->isoperator = 0;
	    strcat(msg, "  You lost operator status.");
	}
    }
    else {
	if (!pl->isoperator && pl->rectype != 2) {
	    NumOperators++;
	    pl->isoperator = 1;
	    pl->privs |= PRIV_AUTOKICKLAST;
	}
	strcpy(msg, "You got operator status.");
    }
    return CMD_RESULT_SUCCESS;
}


static int Cmd_pause(char *arg, player *pl, int oper, char *msg)
{
    /*int			i;*/
    char *errorstr;
    player *pl2;

    if (!oper)
	return CMD_RESULT_NOT_OPERATOR;

    if (!arg || !*arg)
	return CMD_RESULT_NO_NAME;

    pl2 = Get_player_by_name(arg, NULL, &errorstr);
    if (!pl2) {
	strcpy(msg, errorstr);
	return CMD_RESULT_ERROR;
    }

    if (pl2->conn != NULL) {
	if (Player_is_playing(pl2))
	    Kill_player(pl2, false);
	if (Team_zero_pausing_available()) {
	    sprintf(msg, "%s was pause-swapped by %s.", pl2->name, pl->name);
	    Handle_player_command(pl2, "team 0");
	} else {
	    Pause_player(pl2, true);
	    sprintf(msg, "%s was paused by %s.", pl2->name, pl->name);
	}
	Set_message(msg);
	strcpy(msg, "");
    } else {
	sprintf(msg, "Robots can't be paused.");
	return CMD_RESULT_ERROR;
    }

    return CMD_RESULT_SUCCESS;
}


static int Cmd_queue(char *arg, player *pl, int oper, char *msg)
{
    int			result;

    if (record || playback) {
	strcpy(msg, "Command currently disabled during recording for "
	       "technical reasons.");
	return CMD_RESULT_ERROR;
    }

    result = Queue_show_list(msg);

    if (result < 0)
	return CMD_RESULT_ERROR;

    return CMD_RESULT_SUCCESS;
}


static int Cmd_reset(char *arg, player *pl, int oper, char *msg)
{
    int			i;

    if (!oper)
	return CMD_RESULT_NOT_OPERATOR;

    if (arg && !strcasecmp(arg, "all")) {
	for (i = NumPlayers - 1; i >= 0; i--) {
	    Rank_SetScore(Players(i), 0);
	}
	for (i = 0; i < MAX_TEAMS; i++)
	    World.teams[i].score = 0;
	Reset_all_players();
	if (gameDuration == -1)
	    gameDuration = 0;
	roundsPlayed = 0;

	sprintf(msg, " < Total reset by %s! >", pl->name);
	Set_message(msg);
	strcpy(msg, "");
    }
    else {
	Reset_all_players();
	if (gameDuration == -1)
	    gameDuration = 0;

	sprintf(msg, " < Round reset by %s! >", pl->name);
	Set_message(msg);
	strcpy(msg, "");
    }

    return CMD_RESULT_SUCCESS;
}


static int Cmd_stats(char *arg, player *pl, int oper, char *msg)
{
    char *errorstr;
    player *pl2;

    if (!arg || !*arg)
	return CMD_RESULT_NO_NAME;

    pl2 = Get_player_by_name(arg, NULL, &errorstr);
    if (!pl2) {
	strcpy(msg, errorstr);
	return CMD_RESULT_ERROR;
    }

    if (pl2->conn == NULL) {
	sprintf(msg, "Robots don't have ranking stats.");
	return CMD_RESULT_ERROR;
    }

    Rank_get_stats(pl2, msg);

    return CMD_RESULT_SUCCESS;
}


static int Cmd_team(char *arg, player *pl, int oper, char *msg)
{
    int			i;
    int			team;
    int			swap_allowed;
    char		*arg2;

    /*
     * Assume nothing will be said or done.
     */
    msg[0] = '\0';
    swap_allowed = false;
    team = pl->team;

    if (!BIT(World.rules->mode, TEAM_PLAY))
	sprintf(msg, "No team play going on.");
    else if (pl->team >= MAX_TEAMS)
	sprintf(msg, "You do not currently have a team.");
    else if (!arg)
	sprintf(msg, "No team specified.");
    else if (!isdigit(*arg))
	sprintf(msg, "Invalid team specification.");
    else {
	team = strtoul(arg, &arg2, 0);
	if (arg2 && *arg2) {
	    char *errorstr;
	    if (!pl->isoperator) {
		sprintf(msg,
			"You need operator status to swap other players.");
		return CMD_RESULT_NOT_OPERATOR;
	    }
	    while (isspace(*arg2))
		arg2++;
	    pl = Get_player_by_name(arg2, NULL, &errorstr);
	    if (!pl) {
		strcpy(msg, errorstr);
		return CMD_RESULT_ERROR;
	    }
	}

	for (i = 0; i < MAX_TEAMS ; i++) {
	    /* Can't queue to two teams at once. */
	    if (World.teams[i].SwapperId == pl->id)
		World.teams[i].SwapperId = -1;
	}

	if (teamZeroPausing && game_lock && pl->team == 0)
	    sprintf(msg, "Playing teams are locked.");
	else if (teamZeroPausing && lock_zero && (team == 0))
	    sprintf(msg, "Team zero is locked.");
	else if (team < 0 || team >= MAX_TEAMS)
	    sprintf(msg, "Team %d is not a valid team.", team);
	else if (team == pl->team)
	    sprintf(msg, "You already are on team %d.", team);
	else if (World.teams[team].NumBases == 0)
	    sprintf(msg, "There are no bases for team %d on this map.", team);
	else if (reserveRobotTeam && team == robotTeam)
	    sprintf(msg, "You cannot join the robot team on this server.");
	else
	    swap_allowed = true;
    }
    if (!swap_allowed)
	return CMD_RESULT_ERROR;

    if (World.teams[team].NumBases > World.teams[team].NumMembers) {
	sprintf(msg, "%s has swapped to team %d.", pl->name, team);
	Set_message(msg);
	World.teams[pl->team].NumMembers--;
	TEAM_SCORE(pl->team, -(pl->score));
	pl->team = team;
	World.teams[pl->team].NumMembers++;
	TEAM_SCORE(pl->team, pl->score);
	Set_swapper_state(pl);
	Pick_startpos(pl);
	Send_info_about_player(pl);
	strcpy(msg, "");

	if (teamZeroPausing)
	    Pause_player(pl, pl->team == 0);

	return CMD_RESULT_SUCCESS;
    }

    i = World.teams[pl->team].SwapperId;
    while (i != -1) {
	if ((i = Player_by_id(i)->team) != team)
	    i = World.teams[i].SwapperId;
	else {
	    /* Found a cycle, now change the teams */
	    int xbase = pl->home_base, xteam = pl->team, xbase2, xteam2;
	    player *pl2 = pl;
	    do {
		pl2 = Player_by_id(World.teams[xteam].SwapperId);
		World.teams[xteam].SwapperId = -1;
		xbase2 = pl2->home_base;
		xteam2 = pl2->team;
		pl2->team = xteam;
		pl2->home_base = xbase;
		TEAM_SCORE(xteam2, -(pl2->score));
		TEAM_SCORE(pl2->team, pl2->score);
		Set_swapper_state(pl2);
		/*
		 * kps - previously this was Pause_player(pl2->id, ..)
		 * which was buggy, since Pause_player took a player
		 * index as argument. (1)
		 */
		if (teamZeroPausing)
		    Pause_player(pl2, pl2->team == 0);
		Send_info_about_player(pl2);
		/* This can send a huge amount of data if several
		 * players swap. Unfortunately all player data, even
		 * shipshape, has to be resent to change the team of
		 * a player. This should probably be changed somehow
		 * to prevent disturbing other players. */
		xbase = xbase2;
		xteam = xteam2;
	    } while (xteam != team);
	    xteam = pl->team;
	    pl->team = team;
	    pl->home_base = xbase;
	    TEAM_SCORE(xteam, -(pl->score));
	    TEAM_SCORE(pl->team, pl->score);
	    Set_swapper_state(pl);
	    /* kps - same bug was here as above (1) */
	    if (teamZeroPausing)
		Pause_player(pl, pl->team == 0);
	    Send_info_about_player(pl);
	    sprintf(msg, "Some players swapped teams.");
	    Set_message(msg);
	    strcpy(msg, "");
	    return CMD_RESULT_SUCCESS;
	}
    }
    /* Swap a paused player away from the full team */
    if (!teamZeroPausing || team != 0) {
	for (i = NumPlayers - 1; i >= 0; i--) {
	    player *pl2 = Players(i);
	    if (pl2->conn != NULL && BIT(pl2->status, PAUSE)
		&& (pl2->team == team)) {
		pl2->team = pl->team;
		pl->team = team;
		team = pl2->home_base;
		pl2->home_base = pl->home_base;
		pl->home_base = team;
		TEAM_SCORE(pl2->team, -(pl->score));
		TEAM_SCORE(pl->team, -(pl2->score));
		TEAM_SCORE(pl2->team, pl2->score);
		TEAM_SCORE(pl->team, pl->score);
		Set_swapper_state(pl2);
		Set_swapper_state(pl);
		Send_info_about_player(pl2);
		Send_info_about_player(pl);
		sprintf(msg, "%s has swapped with paused %s.",
			pl->name, pl2->name);
		Set_message(msg);
		strcpy(msg, "");
		return CMD_RESULT_SUCCESS;
	    }
	}
    }
    sprintf(msg,"You are queued for swap to team %d.", team);
    World.teams[team].SwapperId = pl->id;
    return CMD_RESULT_SUCCESS;
}


static int Cmd_set(char *arg, player *pl, int oper, char *msg)
{
    int			i;
    char		*option;
    char		*value;

    if (!oper) {
	return CMD_RESULT_NOT_OPERATOR;
    }

    if (!arg
	|| !(option = strtok(arg, " "))
	|| !(value = strtok(NULL, " "))) {

	sprintf(msg, "Usage: /set option value.");
	return CMD_RESULT_ERROR;
    }

    i = Tune_option(option, value);
    if (i == 1) {
	if (!strcasecmp(option, "password"))
	    sprintf(msg, "Operation successful.");
	else {
	    char value[MAX_CHARS];
	    Get_option_value(option, value, sizeof(value));
	    sprintf(msg, " < Option %s set to %s by %s. >",
		    option, value, pl->name);
	    Set_message(msg);
	    strcpy(msg, "");

	    return CMD_RESULT_SUCCESS;
	}
    }
    else if (i == 0) {
	sprintf(msg, "Invalid value.");
    }
    else if (i == -1) {
	sprintf(msg, "This option cannot be changed at runtime.");
    }
    else if (i == -2) {
	sprintf(msg, "No option named '%s'.", option);
    }
    else {
	sprintf(msg, "Error.");
    }

    return CMD_RESULT_ERROR;
}

#if 0
static int Cmd_setpass(char *arg, player *pl, int oper, char *msg)
{
    char *new_pw, *new_pw2, *old_pw;
    int r, new = 0;

    if (!allowPlayerPasswords) {
	strcpy(msg, "Player passwords are disabled on this server.");
	return CMD_RESULT_ERROR;
    }

    if (pl->name[strlen(pl->name)-1] == PROT_EXT) {
	strcpy(msg, "You cannot set a password for your current nick.");
	return CMD_RESULT_ERROR;
    }

    if (!arg || !*arg) {
	strcpy(msg, "Need at least two arguments.");
	return CMD_RESULT_ERROR;
    }

    new_pw = strtok(arg, " ");
    new_pw2 = strtok(NULL, " ");
    old_pw = strtok(NULL, " ");

    if (!new_pw || strlen(new_pw) < MIN_PASS_LEN) {
	sprintf(msg, "Minimum password lenght allowed is %d.", MIN_PASS_LEN);
	return CMD_RESULT_ERROR;
    }
    if (strlen(new_pw) > MAX_PASS_LEN) {
	sprintf(msg, "Maximum password lenght allowed is %d.", MAX_PASS_LEN);
	return CMD_RESULT_ERROR;
    }
    if (!new_pw2) {
	strcpy(msg, "Please specify new password twice.");
	return CMD_RESULT_ERROR;
    }
    if (strcmp(new_pw, new_pw2)) {
	strcpy(msg, "Passwords don't match. Try again.");
	return CMD_RESULT_ERROR;
    }
    if (old_pw && !strcmp(old_pw, new_pw)) {
	strcpy(msg, "New and old password are the same. Nothing changed.");
	return CMD_RESULT_ERROR;
    }

    r = Check_player_password(pl->name, old_pw ? old_pw : "");
    switch (r) {
    case PASSWD_NONE:
	new = 1;

    case PASSWD_OK:
	r = Set_player_password(pl->name, new_pw, new);
	if (!r) {
	    strcpy(msg, "Okay.");
	} else {
	    warn("Command \"/setpass\": Error setting password for "
		 "player \"%s\".", pl->name);
	    strcpy(msg, r == -1 ?
		   "Server error." :
		   "Sorry, no more player passwords allowed. Limit reached!");
	    return CMD_RESULT_ERROR;
	}
	break;

    case PASSWD_WRONG:
	if (old_pw && old_pw[0]) {
	    strcpy(msg, "Old password is wrong. Nothing changed.");
	} else {
	    strcpy(msg, "Need old password as third argument.");
	}
	return CMD_RESULT_ERROR;

    case PASSWD_ERROR:
	warn("Command \"/setpass\": Couldn't check password of "
	     "player \"%s\".", pl->name);
	strcpy(msg, "Server error.");
	return CMD_RESULT_ERROR;
    }

    return CMD_RESULT_SUCCESS;
}
#endif

static int Cmd_version(char *arg, player *pl, int oper, char *msg)
{
    sprintf(msg, "XPilot version %s.", VERSION);
    return CMD_RESULT_SUCCESS;
}


static int Cmd_zerolock(char *arg, player *pl, int oper, char *msg)
{
    int			new_lock;

    if (!teamZeroPausing) {
	sprintf(msg, "There is no team zero pausing here (set in mapfile).");
	return CMD_RESULT_ERROR;
	}

	if (!arg || !*arg) {
	sprintf(msg, "Team zero is currently %s.",
		lock_zero ? "locked" : "unlocked");
	return CMD_RESULT_SUCCESS;
    }

    if (!oper) {
	return CMD_RESULT_NOT_OPERATOR;
    }

    if (!strcmp(arg, "1")) {
	new_lock = 1;
    }
    else if (!strcmp(arg, "0")) {
	new_lock = 0;
    }
    else {
	sprintf(msg, "Invalid argument '%s'.  Specify either 0 or 1.", arg);
	return CMD_RESULT_ERROR;
    }

    if (new_lock == lock_zero) {
	sprintf(msg, "Team zero is already %s.",
		lock_zero ? "locked" : "unlocked");
    }
    else {
	lock_zero = new_lock;
	sprintf(msg, " < Team zero has been %s by %s! >",
		lock_zero ? "locked" : "unlocked",
		pl->name);
	Set_message(msg);
	strcpy(msg, "");
    }

    return CMD_RESULT_SUCCESS;
}
