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


typedef bool (*xp_bool_option_setfunc_t)   (xp_option_t *opt, bool val);
typedef bool (*xp_int_option_setfunc_t)    (xp_option_t *opt, int val);
typedef bool (*xp_double_option_setfunc_t) (xp_option_t *opt, double val);
typedef bool (*xp_string_option_setfunc_t) (xp_option_t *opt, const char *val);
typedef char *(*xp_string_option_getfunc_t)(xp_option_t *opt);
typedef bool (*xp_key_binding_callback_t)  (keys_t key, const char *str);


/*
 * NOTE: DON'T ACCESS THIS STRUCTURE DIRECTLY, USE THE INITIALIZER MACROS,
 * AND OTHER ACCESS FUNCTIONS.
 */
struct xp_option {
    xp_option_type_t type;

    const char *name;
    const char *help;

    /* noarg option stuff */

#define XP_NOARG_OPTION_DUMMY \
	NULL

    bool *noarg_ptr;

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
	NULL, NULL, 0, NULL, NULL

    const char *str_defval;
    char *str_ptr;
    size_t str_size;
    xp_string_option_setfunc_t str_setfunc;
    xp_string_option_getfunc_t str_getfunc;

    /* color option stuff */
    /*color_t *color_ptr;*/

    /* key option stuff */

#define XP_KEY_OPTION_DUMMY \
	NULL, NULL, KEY_DUMMY

    const char *key_defval;
    char *key_string;
    keys_t key;

    /* ... */

};


/* number of options in global option array */
extern int num_options;
#ifdef OPTIONHACK
extern xp_option_t *options;
#endif


extern void Parse_options(int *argcp, char **argvp);

extern void Get_xpilotrc_file(char *, unsigned);

extern bool Set_option(const char *name, const char *value);
extern xp_option_t *Find_option(const char *name);
extern void Set_command(const char *command);
extern void Get_command(const char *command);

extern void Usage(void);
extern const char *Get_keyHelpString(keys_t key);
extern const char *Get_keyResourceString(keys_t key);
extern const char *Option_value_to_string(xp_option_t *opt);

/*
 * Client implementations should call this function to set a callback for
 * handling key option bindings.
 */
extern void Set_key_binding_callback(xp_key_binding_callback_t callback);


void Store_option(xp_option_t *);

#define STORE_OPTIONS(option_array) \
{ \
    int ii; \
    for (ii = 0; ii < NELEM(option_array); ii++) \
	Store_option(& (option_array) [ii]); \
} \

#ifdef OPTIONHACK
static inline const char *Option_get_name(xp_option_t *opt)
{
    assert(opt);
    return opt->name;
}

static inline keys_t Option_get_key(xp_option_t *opt)
{
    assert(opt);
    return opt->key;
}

static inline xp_option_t *Option_by_index(int ind)
{
    if (ind < 0 || ind >= num_options)
	return NULL;
    return &options[ind];
}
#endif




/*
 * Macros for initalizing options.
 */

#define XP_NOARG_OPTION(name, valptr, help) \
{ \
    xp_noarg_option,\
	name,\
	help,\
	valptr,\
	XP_BOOL_OPTION_DUMMY,\
	XP_INT_OPTION_DUMMY,\
	XP_DOUBLE_OPTION_DUMMY,\
	XP_STRING_OPTION_DUMMY,\
	XP_KEY_OPTION_DUMMY,\
}

#define XP_BOOL_OPTION(name, defval, valptr, setfunc, help) \
{ \
    xp_bool_option,\
	name,\
	help,\
	XP_NOARG_OPTION_DUMMY,\
	defval,\
	valptr,\
	setfunc,\
	XP_INT_OPTION_DUMMY,\
	XP_DOUBLE_OPTION_DUMMY,\
	XP_STRING_OPTION_DUMMY,\
	XP_KEY_OPTION_DUMMY,\
}

#define XP_INT_OPTION(name, defval, minval, maxval, valptr, setfunc, help) \
{ \
    xp_int_option,\
	name,\
	help,\
	XP_NOARG_OPTION_DUMMY,\
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

#define XP_DOUBLE_OPTION(name, defval, minval, maxval, valptr, setfunc, help) \
{ \
    xp_double_option,\
	name,\
	help,\
	XP_NOARG_OPTION_DUMMY,\
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

#define XP_STRING_OPTION(name, defval, valptr, size, setfunc, getfunc, help) \
{ \
    xp_string_option,\
	name,\
	help,\
	XP_NOARG_OPTION_DUMMY,\
	XP_BOOL_OPTION_DUMMY,\
	XP_INT_OPTION_DUMMY,\
	XP_DOUBLE_OPTION_DUMMY,\
	defval,\
	valptr,\
	size,\
	setfunc,\
	getfunc,\
	XP_KEY_OPTION_DUMMY,\
}

#define XP_KEY_OPTION(name, defval, key, help) \
{ \
    xp_key_option,\
	name,\
	help,\
	XP_NOARG_OPTION_DUMMY,\
	XP_BOOL_OPTION_DUMMY,\
	XP_INT_OPTION_DUMMY,\
	XP_DOUBLE_OPTION_DUMMY,\
	XP_STRING_OPTION_DUMMY,\
	defval,\
	NULL,\
	key,\
}








typedef enum {
    XP_KS_UNKNOWN = 0,
    XP_KS_BACKSPACE,
    XP_KS_TAB,
    XP_KS_RETURN,
    XP_KS_PAUSE,
    XP_KS_SCROLLOCK,
    XP_KS_ESCAPE,
    XP_KS_DELETE,
    XP_KS_LEFT,
    XP_KS_UP,
    XP_KS_RIGHT,
    XP_KS_HOME,
    XP_KS_PAGEUP,
    XP_KS_PAGEDOWN,
    XP_KS_END,
    XP_KS_INSERT,
    XP_KS_NUMLOCK,
    XP_KS_KP_ENTER,
    XP_KS_KP_MULTIPLY,
    XP_KS_KP_PLUS,
    XP_KS_KP_MINUS,
    XP_KS_KP_PERIOD,
    XP_KS_KP_DIVIDE,
    XP_KS_KP0,
    XP_KS_KP1,
    XP_KS_KP2,
    XP_KS_KP3,
    XP_KS_KP4,
    XP_KS_KP5,
    XP_KS_KP6,
    XP_KS_KP7,
    XP_KS_KP8,
    XP_KS_KP9,
    XP_KS_F1,
    XP_KS_F2,
    XP_KS_F3,
    XP_KS_F4,
    XP_KS_F5,
    XP_KS_F6,
    XP_KS_F7,
    XP_KS_F8,
    XP_KS_F9,
    XP_KS_F10,
    XP_KS_F11,
    XP_KS_F12,
    XP_KS_LSHIFT,
    XP_KS_RSHIFT,
    XP_KS_LCTRL,
    XP_KS_RCTRL,
    XP_KS_CAPSLOCK,
    XP_KS_SPACE,
    XP_KS_QUOTE,
    XP_KS_COMMA,
    XP_KS_PLUS,
    XP_KS_MINUS,
    XP_KS_PERIOD,
    XP_KS_SLASH,
    XP_KS_0,
    XP_KS_1,
    XP_KS_2,
    XP_KS_3,
    XP_KS_4,
    XP_KS_5,
    XP_KS_6,
    XP_KS_7,
    XP_KS_8,
    XP_KS_9,
    XP_KS_SEMICOLON,
    XP_KS_EQUALS,
    XP_KS_a,
    XP_KS_b,
    XP_KS_c,
    XP_KS_d,
    XP_KS_e,
    XP_KS_f,
    XP_KS_g,
    XP_KS_h,
    XP_KS_i,
    XP_KS_j,
    XP_KS_k,
    XP_KS_l,
    XP_KS_m,
    XP_KS_n,
    XP_KS_o,
    XP_KS_p,
    XP_KS_q,
    XP_KS_r,
    XP_KS_s,
    XP_KS_t,
    XP_KS_u,
    XP_KS_v,
    XP_KS_w,
    XP_KS_x,
    XP_KS_y,
    XP_KS_z,
    XP_KS_LEFTBRACKET,
    XP_KS_BACKSLASH,
    XP_KS_RIGHTBRACKET,
    XP_KS_BACKQUOTE,

    /*
     * kps additions 
     */
    XP_KS_KP_INSERT,
    XP_KS_KP_DELETE,
    XP_KS_PRINT, /* Print Screen */
    XP_KS_SECTION,

} xp_keysym_t;

typedef struct {
    xp_keysym_t ks;
    keys_t key;
} xp_keydefs_t;

extern keys_t Generic_lookup_key(xp_keysym_t ks, bool reset);

#endif
