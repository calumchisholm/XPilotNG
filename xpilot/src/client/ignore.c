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

#define NUMCOMMANDS 8
const char c_commands[NUMCOMMANDS][16] = {
    "ignore", "i",
    "ignore!", "i!",
    "unignore", "u",
    "help", "h"
};

extern other_t *Others;
extern int num_others;

void Add_message(char *message);

void print_ignorelist(void)
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
    } else {
	Add_message("Ignorelist is empty [*Client reply*]");
    }
}

void print_help(char *arg)
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
	    Add_message
		("'\\ignore <player>' ignores <player> by changing text to ***'s. [*Client reply*]");
	    Add_message
		("Just '\\ignore' shows list of ignored players [*Client reply*]");
	    break;
	case 2:		/* ignore! */
	case 3:		/* i! */
	    Add_message
		("'\\ignore! <player>' ignores <player> completely. [*Client reply*]");
	    break;
	case 4:		/* unignore */
	case 5:		/* u */
	    Add_message
		("'\\unignore <player>' allows messages from <player> again. [*Client reply*]");
	    break;
	case 6:		/* help */
	case 7:		/* h */
	    Add_message
		("'\\help <command>' shows help about <command>. Just '\\help' show avaiable commands [*Client reply *]");
	    break;
	default:
	    Add_message("No such command [*Client reply*]");
	    break;
	}
    }
}

void ignorePlayer(char *name, short level)
{
    int i, count = 0;
    other_t *other;

    other = Other_by_name(name);
    if (other == NULL) {
	for (i = 0; i < num_others; i++) {
	    if (!strncasecmp(Others[i].name, name, strlen(name))) {
		other = &Others[i];
		count++;
	    }
	}

	switch (count) {
	case 0:
	    Add_message("No such player. [*Client reply*]");
	    return;
	    break;
	case 1:
	    if (level == 1)
		Add_message
		    ("Ignoring player (textmask). [*Client reply*]");
	    else
		Add_message
		    ("Ignoring player (completely). [*Client reply*]");
	    other->ignorelevel = level;
	    return;
	    break;
	default:
	    Add_message("Ambigious. [*Client reply*]");
	    return;
	    break;
	}
    } else {
	other->ignorelevel = level;
    }
}

void unignorePlayer(char *name)
{
    int i, count = 0;
    other_t *other;

    other = Other_by_name(name);
    if (other == NULL) {
	for (i = 0; i < num_others; i++) {
	    if (!strncasecmp(Others[i].name, name, strlen(name))
		&& Others[i].ignorelevel) {
		other = &Others[i];
		count++;
	    }
	}

	switch (count) {
	case 0:
	    Add_message("No such player. [*Client reply*]");
	    return;
	    break;
	case 1:
	    Add_message("Stopped ignoring player. [*Client reply*]");
	    other->ignorelevel = 0;
	    return;
	    break;
	default:
	    Add_message("Ambigious. [*Client reply*]");
	    return;
	    break;
	}
    } else {
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

    if (i + 2 < strlen(talk_str)) {
	strcpy(argument, &talk_str[i + 2]);
    }

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
	if (argument[0] == '\0') {	/* empty */
	    print_ignorelist();
	} else {
	    ignorePlayer(argument, (short) (command_num / 2 + 1));
	}
	break;
    case 4:			/* unignore */
    case 5:			/* u */
	unignorePlayer(argument);
	break;
    case 6:			/* help */
    case 7:			/* h */
	print_help(argument);
	break;
    }
}

void crippleTalk(char *msg)
{
    int i, j;
    int msgEnd;
    unsigned short ballFound = 0;

    for (i = strlen(msg) - 1; i > 0; i--) {
	if (msg[i - 1] == ' ' && msg[i] == '[')
	    break;
    }

    if (i == 0)
	return;

    msgEnd = i - 1;

    for (i = 0; i < msgEnd; i++) {
	if (!strncasecmp(&msg[i], "ball", 4)) {
	    ballFound = 1;
	    break;
	}
    }

    for (i = 0; i < msgEnd; i++) {
	if (!strncasecmp(&msg[i], "ball", 4)
	    || !strncasecmp(&msg[i], "safe", 4)) {
	    i += 3;
	} else if (ballFound) {
	    for (j = 0; j < num_others; j++) {
		if (!strncasecmp
		    (&msg[i], Others[j].name, strlen(Others[j].name))) {
		    i += strlen(Others[j].name) - 1;
		    break;
		}
	    }

	    if (j == num_others && (isalpha(msg[i]) || isdigit(msg[i])))
		msg[i] = '*';

	} else {
	    if (isalpha(msg[i]) || isdigit(msg[i])) {
		msg[i] = '*';
	    }
	}
    }
}
