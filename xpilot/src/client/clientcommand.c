/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *  	Bjørn Stabell		 <bjoern@xpilot.org>
 *  	Ken Ronny Schouten   <ken@xpilot.org>
 *  	Bert Gijsbers		 <bert@xpilot.org>
 *  	Dick Balaska		 <dick@xpilot.org>
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

char clientcommand_version[] = VERSION;

#define NUMCOMMANDS 10
const char c_commands[NUMCOMMANDS][16] = {
    "ignore", "i",
    "ignore!", "i!",
    "unignore", "u",
    "help", "h",
    "set", "s"
};

static void print_ignorelist(void)
{
    char buffer[MAX_CHARS] = "";
    int i;
    unsigned short check = 0;

    for (i = 0; i < num_others; i++) {
	if (Others[i].ignorelevel == 1) {
	    if (strlen(buffer) + strlen(Others[i].name) + 17 > MAX_CHARS) {
		strcat(buffer, "[*Client reply*]");
		Add_message(buffer);
		buffer[0] = '\0';
	    }

	    strcat(buffer, Others[i].name);
	    strcat(buffer, " ");
	    check = 1;
	} else if (Others[i].ignorelevel == 2) {
	    if (strlen(buffer) + strlen(Others[i].name) + 18 > MAX_CHARS) {
		strcat(buffer, "[*Client reply*]");
		Add_message(buffer);
		buffer[0] = '\0';
	    }

	    strcat(buffer, "!");
	    strcat(buffer, Others[i].name);
	    strcat(buffer, " ");

	    check = 1;
	}
    }

    if (check) {
	strcat(buffer, "[*Client reply*]");
	Add_message(buffer);
    } else
	Add_message("Ignorelist is empty [*Client reply*]");
}

static void print_help(char *arg)
{
    int i;
    char message[MAX_CHARS] = "";

    Add_message(arg);

    if (arg[0] == '\0') {
	for (i = 0; i < NUMCOMMANDS; i += 2) {
	    strcat(message, c_commands[i]);
	    strcat(message, " ");
	}
	strcat(message, "[*Client reply*]");

	Add_message(message);
    } else {
	for (i = 0; i < NUMCOMMANDS; i++) {
	    if (!strcmp(arg, c_commands[i]))
		break;
	}

	switch (i) {
	case 0:		/* ignore */
	case 1:		/* i */
	    Add_message("'\\ignore <player>' ignores <player> by changing "
			"text to ***'s. [*Client reply*]");
	    Add_message("Just '\\ignore' shows list of ignored players "
			"[*Client reply*]");
	    break;
	case 2:		/* ignore! */
	case 3:		/* i! */
	    Add_message("'\\ignore! <player>' ignores <player> completely. "
			"[*Client reply*]");
	    break;
	case 4:		/* unignore */
	case 5:		/* u */
	    Add_message("'\\unignore <player>' allows messages from <player> "
			"again. [*Client reply*]");
	    break;
	case 6:		/* help */
	case 7:		/* h */
	    Add_message("'\\help <command>' shows help about <command>. "
			"Just '\\help' show avaiable commands "
			"[*Client reply*]");
	    break;
	case 8:		/* set */
	case 9:		/* s */
	    Add_message("'\\set <option> <value>' sets an option value. "
			"[*Client reply*]");
	    break;
	default:
	    Add_message("No such command [*Client reply*]");
	    break;
	}
    }
}

static void ignorePlayer(char *name, int level)
{
    other_t *other = Other_by_name(name, true);
    char buf[64 + MAX_NAME_LEN];

    if (other != NULL) {
	if (level == 1) {
	    snprintf(buf, sizeof(buf),
		     "Ignoring %s (textmask). [*Client reply*]", other->name);
	    Add_message(buf);
	} else {
	    snprintf(buf, sizeof(buf),
		     "Ignoring %s (completely). [*Client reply*]",
		     other->name);
	    Add_message(buf);
	}
	other->ignorelevel = level;
    }
}

static void unignorePlayer(char *name)
{
    other_t *other = Other_by_name(name, true);
    char buf[64 + MAX_NAME_LEN];

    if (other != NULL) {
	snprintf(buf, sizeof(buf),
		 "Stopped ignoring %s. [*Client reply*]", other->name);
	Add_message(buf);
	other->ignorelevel = 0;
    }
}

void executeCommand(char *talk_str)
{
    unsigned i;
    char command[MAX_CHARS];
    char argument[MAX_CHARS] = "";
    int command_num;

    strlcpy(command, &talk_str[1], MAX_CHARS - 1);	/* removes '\' */
    for (i = 0; i < strlen(command); i++) {
	if (command[i] == ' ')
	    break;
    }

    command[i] = '\0';

    if (i + 2 < strlen(talk_str))
	strcpy(argument, &talk_str[i + 2]);

    for (i = 0; i < NUMCOMMANDS; i++) {
	if (!strcmp(command, c_commands[i]))
	    break;
    }

    if (i == NUMCOMMANDS) {
	Add_message("Invalid clientcommand. [*Client reply*]");
	return;
    }

    command_num = i;
    switch (command_num) {
    case 0:			/* ignore */
    case 1:			/* i */
    case 2:			/* ignore! */
    case 3:			/* i! */
	if (argument[0] == '\0')	/* empty */
	    print_ignorelist();
	else
	    ignorePlayer(argument, (int) (command_num / 2 + 1));
	break;
    case 4:			/* unignore */
    case 5:			/* u */
	unignorePlayer(argument);
	break;
    case 6:			/* help */
    case 7:			/* h */
	print_help(argument);
	break;
    case 8:			/* help */
    case 9:			/* h */
	Set_command(argument);
	break;
    default:
	warn("BUG: bad command num in executeCommand()");
	break;
    }
}

void crippleTalk(char *msg)
{
    int i, msgEnd;

    for (i = strlen(msg) - 1; i > 0; i--) {
	if (msg[i - 1] == ' ' && msg[i] == '[')
	    break;
    }

    if (i == 0)
	return;

    msgEnd = i - 1;

    for (i = 0; i < msgEnd; i++) {
	if (isalpha(msg[i]) || isdigit(msg[i]))
	    msg[i] = '*';
    }
}
