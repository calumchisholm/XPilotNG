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

#ifndef OPTION_H
#define OPTION_H

extern void Parse_options(int *argcp, char **argvp, char *realName, int *port,
			  int *my_team, bool *text, bool *list,
			  bool *join, bool *noLocalMotd,
			  char *nickName, char *dispName, char *hostName,
			  char *shut_msg);

extern void Get_xpilotrc_file(char *, unsigned);

typedef enum {
    xp_noarg_option,
    xp_bool_option,
    xp_int_option,
    xp_double_option,
    xp_string_option,
    xp_color_option,
    xp_key_option,
} xp_option_type_t;

typedef struct xp_option xp_option_t;

typedef void (*xp_bool_option_setfunc_t)   (xp_option_t *opt, bool val);
typedef void (*xp_int_option_setfunc_t)    (xp_option_t *opt, int val);
typedef void (*xp_double_option_setfunc_t) (xp_option_t *opt, double val);
typedef bool (*xp_string_option_setfunc_t) (xp_option_t *opt, const char *val);
typedef bool (*xp_key_option_setfunc_t)    (xp_option_t *opt, const char *val);


struct xp_option {
    xp_option_type_t type;

    const char *name;
    const char *help;

    /* bool option stuff */

#define XP_BOOL_OPTION_DUMMY \
	false, NULL, NULL

    bool bool_defval;
    bool *bool_ptr;
    xp_bool_option_setfunc_t bool_setfunc;

    /* integer option stuff */

#define XP_INT_OPTION_DUMMY \
	0, 0, 0, NULL, NULL

    int int_defval;
    int int_minval;
    int int_maxval;
    int *int_ptr;
    xp_int_option_setfunc_t int_setfunc;

    /* double option stuff */

#define XP_DOUBLE_OPTION_DUMMY \
	0, 0, 0, NULL, NULL

    double dbl_defval;
    double dbl_minval;
    double dbl_maxval;
    double *dbl_ptr;
    xp_double_option_setfunc_t dbl_setfunc;

    /* string option stuff */

#define XP_STRING_OPTION_DUMMY \
	NULL, NULL, 0, NULL

    const char *str_defval;
    char *str_ptr;
    size_t str_size;
    xp_string_option_setfunc_t str_setfunc;

    /* color option stuff */
    /*color_t *color_ptr;*/

    /* key option stuff */

#define XP_KEY_OPTION_DUMMY \
	NULL, KEY_DUMMY, NULL

    const char *key_defval;
    keys_t key;
    xp_key_option_setfunc_t key_setfunc;
    /* ... */

};



/*
 * Macros for initalizing options.
 */

#if 0
#define XP_NOARG_OPTION(name, help) \
{ \
    xp_noarg_option,\
	name,\
	help,\
	XP_BOOL_OPTION_DUMMY,\
	XP_INT_OPTION_DUMMY,\
	XP_DOUBLE_OPTION_DUMMY,\
	XP_STRING_OPTION_DUMMY,\
	XP_KEY_OPTION_DUMMY,\
}
#endif

#define XP_BOOL_OPTION(name, valptr, defval, setfunc, help) \
{ \
    xp_bool_option,\
	name,\
	help,\
	defval,\
	valptr,\
	setfunc,\
	XP_INT_OPTION_DUMMY,\
	XP_DOUBLE_OPTION_DUMMY,\
	XP_STRING_OPTION_DUMMY,\
	XP_KEY_OPTION_DUMMY,\
}

#define XP_INT_OPTION(name, valptr, defval, minval, maxval, setfunc, help) \
{ \
    xp_int_option,\
	name,\
	help,\
	XP_BOOL_OPTION_DUMMY,\
	defval,\
	minval,\
	maxval,\
	valptr,\
	setfunc,\
	XP_DOUBLE_OPTION_DUMMY,\
	XP_STRING_OPTION_DUMMY,\
	XP_KEY_OPTION_DUMMY,\
}

#define XP_DOUBLE_OPTION(name, valptr, defval, minval, maxval, setfunc, help) \
{ \
    xp_double_option,\
	name,\
	help,\
	XP_BOOL_OPTION_DUMMY,\
	XP_INT_OPTION_DUMMY,\
	defval,\
	minval,\
	maxval,\
	valptr,\
	setfunc,\
	XP_STRING_OPTION_DUMMY,\
	XP_KEY_OPTION_DUMMY,\
}

#define XP_STRING_OPTION(name, valptr, size, defval, setfunc, help) \
{ \
    xp_string_option,\
	name,\
	help,\
	XP_BOOL_OPTION_DUMMY,\
	XP_INT_OPTION_DUMMY,\
	XP_DOUBLE_OPTION_DUMMY,\
	defval,\
	valptr,\
	size,\
	setfunc,\
	XP_KEY_OPTION_DUMMY,\
}

#define XP_KEY_OPTION(name, defval, key, setfunc, help) \
{ \
    xp_key_option,\
	name,\
	help,\
	XP_BOOL_OPTION_DUMMY,\
	XP_INT_OPTION_DUMMY,\
	XP_DOUBLE_OPTION_DUMMY,\
	XP_STRING_OPTION_DUMMY,\
	defval,\
	key,\
	setfunc,\
}




#endif
