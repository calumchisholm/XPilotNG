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

char messages_version[] = VERSION;


message_t	*TalkMsg[MAX_MSGS], *GameMsg[MAX_MSGS];
/* store incoming messages while a cut is pending */
message_t	*TalkMsg_pending[MAX_MSGS], *GameMsg_pending[MAX_MSGS];
/* history of the talk window */
char		*HistoryMsg[MAX_HIST_MSGS];

#ifndef _WINDOWS
/* selection in talk- or draw-window */
extern selection_t selection;
extern void Delete_pending_messages(void);
#endif

bool ball_shout = false;
bool need_cover = false;


/*
 * Little less ugly message scan hack by Samaseon (ksoderbl@cc.hut.fi)
 *
 * used for:
 * - Kill/Death ratio counter, based on the original
 *   "e94_msu eKthHacks (killratio)"
 * - Mara's client ranking and base warning hacks
 */

/* if you want debug messages, use the upper one */
/*#define DP(x) x*/
#define DP(x)

static char *shottypes[] = { "a shot", NULL };
static char head_first[] = " head first";
static char *crashes[] = { "crashed", "smashed", "smacked", "was trashed",
    NULL
};
static char *obstacles[] =
    { "wall", "target", "treasure", "cannon", NULL };
static char *teamnames[] =
    { "2", "4", "0", "1", "3", "5", "6", "7", "8", "9", NULL };


/* increase if you want to look for messages with more player names. */
#define MSG_MAX_NAMES 3

/* structure to store names found in a message */
typedef struct {
    int index;
    char name[MSG_MAX_NAMES][MAX_CHARS];
} msgnames_t;

/* recursive descent parser for messages */
static bool Msg_match_fmt(char *msg, char *fmt, msgnames_t *mn)
{
    char *fp;
    int i;
    size_t len;

    DP(printf("Msg_match_fmt - fmt = '%s'\n", fmt));
    DP(printf("Msg_match_fmt - msg = '%s'\n", msg));

    /* check that msg and fmt match to % */
    fp = strstr(fmt, "%");
    if (fp == NULL) {
	/* NOTE: if msg contains stuff beyond fmt, don't care */
	if (strncmp(msg, fmt, strlen(fmt)) == 0)
	    return true;
	else
	    return false;
    }
    len = (size_t) (fp - fmt);
    if (strncmp(msg, fmt, len) != 0)
	return false;
    fmt = fp + 2;
    msg += len;

    switch (*(fp + 1)) {
	char *name;
    case 'n':			/* name */
	for (i = 0; i < num_others; i++) {
	    name = Others[i].name;
	    len = strlen(name);
	    if ((strncmp(msg, name, len) == 0)
		&& Msg_match_fmt(msg + len, fmt, mn)) {
		strncpy(mn->name[mn->index++], name, len + 1);
		return true;
	    }
	}
	break;
    case 's':			/* shot type */
	for (i = 0; shottypes[i] != NULL; i++) {
	    if (strncmp(msg, shottypes[i], strlen(shottypes[i])) == 0) {
		msg += strlen(shottypes[i]);
		return Msg_match_fmt(msg, fmt, mn);
	    }
	}
	break;
    case 'h':			/* head first or nothing */
	if (strncmp(msg, head_first, strlen(head_first)) == 0)
	    msg += strlen(head_first);
	return Msg_match_fmt(msg, fmt, mn);
    case 'o':			/* obstacle */
	for (i = 0; obstacles[i] != NULL; i++) {
	    if (strncmp(msg, obstacles[i], strlen(obstacles[i])) == 0) {
		msg += strlen(obstacles[i]);
		return Msg_match_fmt(msg, fmt, mn);
	    }
	}
	break;
    case 'c':			/* some sort of crash */
	for (i = 0; crashes[i] != NULL; i++) {
	    if (strncmp(msg, crashes[i], strlen(crashes[i])) == 0) {
		msg += strlen(crashes[i]);
		return Msg_match_fmt(msg, fmt, mn);
	    }
	}
	break;
    case 't':			/* "name" of a team */
	for (i = 0; teamnames[i] != NULL; i++) {
	    if (strncmp(msg, teamnames[i], strlen(teamnames[i])) == 0) {
		strncpy(mn->name[mn->index++], teamnames[i], 2);
		msg += strlen(teamnames[i]);
		return Msg_match_fmt(msg, fmt, mn);
	    }
	}
	break;
    default:
	break;
    }

    return false;
}

/*
 * A total reset is most often done when a new match is starting.
 * If we see a total reset message we clear the statistics.
 */
static bool Msg_scan_for_total_reset(char *message)
{
    static char total_reset[] = "Total reset";

    if (strstr(message, total_reset)) {
	killratio_kills = 0;
	killratio_deaths = 0;
	killratio_totalkills = 0;
	killratio_totaldeaths = 0;
	ballstats_cashes = 0;
	ballstats_replaces = 0;
	ballstats_teamcashes = 0;
	ballstats_lostballs = 0;
	played_this_round = false;
	rounds_played = 0;
	return true;
    }

    return false;
}

static bool Msg_scan_for_replace_treasure(char *message)
{
    msgnames_t mn;

    if (!self)
	return false;

    memset(&mn, 0, sizeof(mn));
    if (Msg_match_fmt(message,
		      " < %n (team %t) has replaced the treasure >",
		      &mn)) {
	int replacer_team = atoi(mn.name[0]);
	char *replacer = mn.name[1];

	if (replacer_team == self->team) {
	    ball_shout = false;
	    if (!strcmp(replacer, self->name))
		ballstats_replaces++;
	    return true;
	}
	/*
	 * Ok, at this point we know that it was not someone in our team
	 * that replaced the treasure.
	 *
	 * If there are only 2 teams playing, and our team did not replace,
	 * it was the other team.
	 * In this case, we can clear the cover flag.
	 */
	if (num_playing_teams == 2)
	    need_cover = false;
	return true;
    }

    return false;
}

static bool Msg_scan_for_ball_destruction(char *message)
{
    msgnames_t mn;

    if (!self)
	return false;

    memset(&mn, 0, sizeof(mn));
    if (Msg_match_fmt(message,
		      " < %n's (%t) team has destroyed team %t treasure >",
		      &mn)) {
	int destroyer_team = atoi(mn.name[0]);
	int destroyed_team = atoi(mn.name[1]);
	char *destroyer = mn.name[2];

	if (destroyer_team == self->team) {
	    ballstats_teamcashes++;
	    if (!strcmp(destroyer, self->name))
		ballstats_cashes++;
	}
	if (destroyed_team == self->team)
	    ballstats_lostballs++;
	return true;
    }
    return false;
}




/* Needed by base warning hack */
static void Msg_scan_death(int id)
{
    int i;
    other_t *other;

    if (version >= 0x4F12)
	return;

    other = Other_by_id(id);
    if (!other)
	return;

    /*
     * kps - hack, we don't want to do base warning for players who
     * lost their last life. If deathtime is used for anything else
     * this must be done some other way.
     */
    if (BIT(Setup->mode, LIMITED_LIVES)	&& other->life == 0)
	return;

    for (i = 0; i < num_bases; i++) {
	if (bases[i].id == id) {
	    bases[i].deathtime = loops;
	    break;
	}
    }
}

static bool Want_msg_scan(void)
{
    int i;
    other_t *other;
    int num_playing = 0;

    /* if only player on server, let's not bother */
    if (num_others < 2)
	return false;

    /* if not playing, don't bother */
    if (!self || strchr("PW", self->mychar))
	return false;

    for (i = 0; i < num_others; i++) {
	other = &Others[i];
	/* alive and dead ships and robots are considered playing */
	if (strchr(" DR", other->mychar))
	    num_playing++;
    }

    if (num_playing > 1)
	return true;
    return false;
}


static bool Msg_is_game_msg(char *message)
{
    if (message[strlen(message) - 1] == ']' || strncmp(message, " <", 2) == 0)
	return false;
    return true;
}

static void Msg_scan_game_msg(char *message)
{
    msgnames_t mn;
    char *killer = NULL, *victim = NULL, *victim2 = NULL;
    bool i_am_killer = false;
    bool i_am_victim = false;
    bool i_am_victim2 = false;
    other_t *other = NULL;

    DP(printf("MESSAGE: \"%s\"\n", message));

    /*
     * First check if it is a message indicating end of round.
     */
    if (strstr(message, "There is no Deadly Player") ||
	strstr(message, "is the Deadliest Player with") ||
	strstr(message, "are the Deadly Players with")) {

	/* Mara bmsg scan - clear flags at end of round. */
	ball_shout = false;
	need_cover = false;

	if (played_this_round) {
	    played_this_round = false;
	    rounds_played++;
	}

	roundend = true;
	killratio_totalkills += killratio_kills;
	killratio_totaldeaths += killratio_deaths;
	return;
    }

    /*
     * Now let's check if someone got killed.
     */
    memset(&mn, 0, sizeof(mn));
    /*
     * note: matched names will be in reverse order in the message names
     * struct, because the deepest recursion level knows first if the
     * parsing succeeded.
     */

    if (Msg_match_fmt(message, "%n was killed by %s from %n.", &mn)) {
	DP(printf("shot:\n"));
	killer = mn.name[0];
	victim = mn.name[1];

    } else if (Msg_match_fmt(message, "%n %c%h against a %o.", &mn)) {
	DP(printf("crashed into obstacle:\n"));
	victim = mn.name[0];

    } else if (Msg_match_fmt(message, "%n and %n crashed.", &mn)) {
	DP(printf("crash:\n"));
	victim = mn.name[1];
	victim2 = mn.name[0];

    } else if (Msg_match_fmt(message, "%n ran over %n.", &mn)) {
	DP(printf("overrun:\n"));
	killer = mn.name[1];
	victim = mn.name[0];

    } else if (Msg_match_fmt
	       (message, "%n %c%h against a %o with help from %n", &mn)) {
	DP(printf("crashed into obstacle:\n"));
	/*
	 * please fix this if you like, all helpers should get a kill
	 * (look at server/walls.c)
	 */
	killer = mn.name[0];
	victim = mn.name[1];

    } else if (Msg_match_fmt(message, "%n has committed suicide.", &mn)) {
	DP(printf("suicide:\n"));
	victim = mn.name[0];

    } else if (Msg_match_fmt(message, "%n was killed by a ball.", &mn)) {
	DP(printf("killed by ball:\n"));
	victim = mn.name[0];

    } else if (Msg_match_fmt
	       (message, "%n was killed by a ball owned by %n.", &mn)) {
	DP(printf("killed by ball:\n"));
	killer = mn.name[0];
	victim = mn.name[1];

    } else
	if (Msg_match_fmt(message, "%n succumbed to an explosion.", &mn)) {
	DP(printf("killed by explosion:\n"));
	victim = mn.name[0];

    } else
	if (Msg_match_fmt
	    (message, "%n succumbed to an explosion from %n.", &mn)) {
	DP(printf("killed by explosion:\n"));
	killer = mn.name[0];
	victim = mn.name[1];

    } else if (Msg_match_fmt
	       (message, "%n got roasted alive by %n's laser.", &mn)) {
	DP(printf("roasted alive:\n"));
	killer = mn.name[0];
	victim = mn.name[1];

    } else if (Msg_match_fmt
	       (message, "%n was hit by cannonfire.", &mn)) {
	DP(printf("hit by cannonfire:\n"));
	victim = mn.name[0];

    } else {
	/* none of the above, nothing to do */
	return;
    }


    if (killer != NULL) {
	DP(printf("Killer is %s.\n", killer));
	if (strcmp(killer, self->name) == 0)
	    i_am_killer = true;
    }

    if (victim != NULL) {
	DP(printf("Victim is %s.\n", victim));
	if (strcmp(victim, self->name) == 0)
	    i_am_victim = true;
    }

    if (victim2 != NULL) {
	DP(printf("Second victim is %s.\n", victim2));
	if (strcmp(victim2, self->name) == 0)
	    i_am_victim2 = true;
    }

    /* handle death array */
    if (victim != NULL) {
	other = Other_by_name(victim);

	/*for safety... could possibly happen with
	  loss or parser bugs =) */
	if (other != NULL)
	    Msg_scan_death(other->id);
    } else {
	DP(printf("*** [%s] was not found in the players array! ***\n",
		  victim));
    }

    if (victim2 != NULL) {
	other = Other_by_name(victim);
	if (other != NULL)
	    Msg_scan_death(other->id);
    } else {
	DP(printf("*** [%s] was not found in the players array! ***\n",
		  victim));
    }

    /* handle killratio */
    if (i_am_killer && !i_am_victim)
	killratio_kills++;

    if (i_am_victim || i_am_victim2)
	killratio_deaths++;

    if (BIT(hackedInstruments, CLIENT_RANKER)) {
	/*static char tauntstr[MAX_CHARS];
	  int kills, deaths; */

	/* handle case where there is a victim and a killer */
	if (killer != NULL && victim != NULL) {
	    if (i_am_killer && !i_am_victim) {
		Add_rank_Death(victim);
		/*if (BIT(instruments, TAUNT)) {
		  kills = Get_kills(victim);
		  deaths = Get_deaths(victim);
		  if (deaths > kills) {
		  sprintf(tauntstr, "%s: %i-%i HEHEHEHE\0",
		  victim, deaths, kills);
		  Net_talk(tauntstr);
		  }
		  } */
	    }
	    if (!i_am_killer && i_am_victim)
		Add_rank_Kill(killer);
	}
    }
}


/*
 * Checks if the message is in angle brackets, that is,
 * starts with " < " and ends with ">"
 */
static bool Msg_is_in_angle_brackets(char *message)
{
    if (strncmp(message, " < ", 3))
	return false;
    if (message[strlen(message) - 1] != '>')
	return false;
    return true;
}

static void Msg_scan_angle_bracketed_msg(char *message, bool want_scan)
{
    /* let's scan for total reset even if not playing */
    if (Msg_scan_for_total_reset(message))
	return;
    if (!want_scan)
	return;
    if (Msg_scan_for_ball_destruction(message))
	return;
    if (Msg_scan_for_replace_treasure(message))
	return;
}

/* Mara's ball message scan */
static msg_bms_t Msg_do_bms(char *message, char *bracket)
{
    static char ball_text1[] = "BALL";
    static char ball_text2[] = "Ball";
    static char ball_text3[] = "VAKK";
    static char ball_text4[] = "B A L L";
    static char ball_text5[] = "ball";
    static char safe_text1[] = "SAFE";
    static char safe_text2[] = "Safe";
    static char safe_text3[] = "safe";
    static char safe_text4[] = "S A F E";
    static char cover_text1[] = "COVER";
    static char cover_text2[] = "Cover";
    static char cover_text3[] = "cover";
    static char pop_text1[] = "POP";
    static char pop_text2[] = "Pop";
    static char pop_text3[] = "pop";

    /* for safety */
    if (bracket == NULL)
	return BmsNone;

    /* We don't want to scan the player name */
    *bracket = '\0';

    /*check safe b4 ball */
    if (strstr(message, safe_text1) ||
	strstr(message, safe_text2) ||
	strstr(message, safe_text3) ||
	strstr(message, safe_text4)) {
	ball_shout = false;
	*bracket = '[';
	return BmsSafe;
    }

    if (strstr(message, cover_text1) ||
	strstr(message, cover_text2) ||
	strstr(message, cover_text3)) {
	need_cover = true;
	*bracket = '[';
	return BmsCover;
    }

    if (strstr(message, pop_text1) ||
	strstr(message, pop_text2) ||
	strstr(message, pop_text3)) {
	need_cover = false;
	*bracket = '[';
	return BmsPop;
    }

    if (strstr(message, ball_text1) ||
	strstr(message, ball_text2) ||
	strstr(message, ball_text3) ||
	strstr(message, ball_text4) ||
	strstr(message, ball_text5)) {
	ball_shout = true;
	*bracket = '[';
	return BmsBall;
    }

    *bracket = '[';
    return BmsNone;
}


/*
 * Checks if the message is sent by someone in your team.
 * In 'bracket' we will store info about where the
 * player name starts so the bms does can ignore that.
 */
static bool Msg_is_from_our_team(char *message, char **bracket)
{
    other_t *other;
    static char buf[MAX_CHARS + 8];
    size_t bufstrlen, len;
    int i;

    if (self == NULL)
	return false;

    len = strlen(message);
    for (i = 0; i < num_others; i++) {
	other = &Others[i];
	if (other->team != self->team)
	    continue;

	/* first check if someone in your team sent the message for all */
	sprintf(buf, "[%s]", other->name);
	bufstrlen = strlen(buf);
	if (len < bufstrlen)
	    continue;
	if (!strcmp(&message[len - bufstrlen], buf)) {
	    *bracket = &message[len - bufstrlen];
	    return true;
	}

	/* if not, check if it was sent to your team only */
	sprintf(buf, "[%s]:[%d]", other->name, other->team);
	bufstrlen = strlen(buf);
	if (len < bufstrlen)
	    continue;
	if (!strcmp(&message[len - bufstrlen], buf)) {
	    *bracket = &message[len - bufstrlen];
	    return true;
	}
    }
    return false;
}


/*
 * add an incoming talk/game message.
 * however, buffer new messages if there is a pending selection.
 * Add_pending_messages() will be called later in Talk_cut_from_messages().
 */
void Add_message(char *message)
{
    int			i;
    size_t		len;
    message_t		*tmp, **msg_set;
    bool		is_game_msg = false, want_scan = false;
    msg_bms_t		bmsinfo = BmsNone;
    char		*bracket = NULL;

#ifndef _WINDOWS
    bool		is_drawn_talk_message	= false; /* not pending */
    int			last_msg_index;
    bool		show_reverse_scroll	= false;
    bool		scrolling		= false; /* really moving */

    show_reverse_scroll = BIT(instruments, SHOW_REVERSE_SCROLL);

    is_game_msg = Msg_is_game_msg(message);
    if (!is_game_msg) {
	if (selectionAndHistory && selection.draw.state == SEL_PENDING) {
	    /* the buffer for the pending messages */
	    msg_set = TalkMsg_pending;
	} else {
	    msg_set = TalkMsg;
	    is_drawn_talk_message = true;
	}
    } else {
	if (selectionAndHistory && selection.draw.state == SEL_PENDING)
	    msg_set = GameMsg_pending;
	else
	    msg_set = GameMsg;
    }
#else
    is_game_msg = Msg_is_game_msg(message);
    if (!is_game_msg)
	msg_set = TalkMsg;
    else
	msg_set = GameMsg;
#endif

    want_scan = Want_msg_scan();
    if (is_game_msg && want_scan)
	Msg_scan_game_msg(message);

    else if (Msg_is_in_angle_brackets(message))
	Msg_scan_angle_bracketed_msg(message, want_scan);

    else if (BIT(hackedInstruments, BALL_MSG_SCAN)
	     && !is_game_msg
	     && BIT(Setup->mode, TEAM_PLAY)
	     && want_scan
	     && Msg_is_from_our_team(message, &bracket))
	bmsinfo = Msg_do_bms(message, bracket);

#ifndef _WINDOWS
    if (selectionAndHistory && is_drawn_talk_message) {
	/* how many talk messages */
        last_msg_index = 0;
        while (last_msg_index < maxMessages
		&& TalkMsg[last_msg_index]->len != 0) {
            last_msg_index++;
        }
        last_msg_index--; /* make it an index */

	/*
	 * if show_reverse_scroll, it will really _scroll if there were
	 * already maxMessages (one will be added now)
	 */
	if (show_reverse_scroll && last_msg_index == maxMessages - 1) {
	    scrolling = true;
	}

	/*
	 * keep the emphasizing (`jumping' from talk window to talk messages)
	 */
	if (selection.keep_emphasizing) {
	    selection.keep_emphasizing = false;
	    selection.talk.state = SEL_NONE;
	    selection.draw.state = SEL_EMPHASIZED;
	    if (!show_reverse_scroll) {
		selection.draw.y1 = -1;
		selection.draw.y2 = -1;
	    } else if (show_reverse_scroll) {
		selection.draw.y1 = last_msg_index + 1;
		selection.draw.y2 = last_msg_index + 1;
	    }
	} /* talk window emphasized */
    } /* talk messages */
#endif

    tmp = msg_set[maxMessages - 1];
    for (i = maxMessages - 1; i > 0; i--)
	msg_set[i] = msg_set[i - 1];

    msg_set[0] = tmp;

    msg_set[0]->lifeTime = MSG_LIFE_TIME;
    strlcpy(msg_set[0]->txt, message, MSG_LEN);
    len = strlen(message);
    msg_set[0]->len = len;
    msg_set[0]->bmsinfo = bmsinfo;

#ifndef _WINDOWS
    /*
     * scroll also the emphasizing
     */
    if (selectionAndHistory && is_drawn_talk_message
	  && selection.draw.state == SEL_EMPHASIZED ) {

	if ((scrolling && selection.draw.y2 == 0)
	      || (!show_reverse_scroll && selection.draw.y1
		  == maxMessages - 1)) {
	    /*
	     * the emphasizing vanishes, as it's `last' line
	     * is `scrolled away'
	     */
	    selection.draw.state = SEL_SELECTED;
	} else {
	    if (scrolling) {
		selection.draw.y2--;
		if ( selection.draw.y1 == 0) {
		    selection.draw.x1 = 0;
		} else {
		    selection.draw.y1--;
		}
	    } else if (!show_reverse_scroll) {
		selection.draw.y1++;
		if (selection.draw.y2 == maxMessages - 1) {
		    selection.draw.x2 = msg_set[selection.draw.y2]->len - 1;
		} else {
		    selection.draw.y2++;
		}
	    }
	}
    }
#endif

    msg_set[0]->pixelLen = XTextWidth(messageFont, msg_set[0]->txt,
				      msg_set[0]->len);

    /* Print messages to standard output.
     */
    if (messagesToStdout == 2 ||
	(messagesToStdout == 1 &&
	 message[0] &&
	 message[strlen(message)-1] == ']')) {

	xpprintf("%s\n", message);
    }
}


#ifndef _WINDOWS
/*
 * clear the buffer for the pending messages
 */
void Delete_pending_messages(void)
{
    message_t* msg;
    int i;
    if (!selectionAndHistory)
	return;

    for (i = 0; i < maxMessages; i++) {
	msg = TalkMsg_pending[i];
	if (msg->len > 0) {
            msg->txt[0] = '\0';
            msg->len = 0;
	}
	msg = GameMsg_pending[i];
	if (msg->len > 0) {
            msg->txt[0] = '\0';
            msg->len = 0;
	}
    }
}


/*
 * after a pending cut has been completed,
 * add the (buffered) messages which were coming in meanwhile.
 */
void Add_pending_messages(void)
{
    int			i;

    if (!selectionAndHistory)
	return;
    /* just through all messages */
    for (i = maxMessages-1; i >= 0; i--) {
	if (TalkMsg_pending[i]->len > 0) {
	    Add_message(TalkMsg_pending[i]->txt);
	}
	if (GameMsg_pending[i]->len > 0) {
	    Add_message(GameMsg_pending[i]->txt);
	}
    }
    Delete_pending_messages();
}
#endif
