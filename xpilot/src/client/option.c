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

#include "xpclient.h"

char option_version[] = VERSION;

#ifdef OPTIONHACK


int num_options = 0;
int max_options = 0;

xp_option_t *options = NULL;


xp_option_t *Find_option(const char *name)
{
    int i;

    /*
     * This could be speeded up with a hash table or just by
     * hashing the option name.
     */
    for (i = 0; i < num_options; i++) {
	if (!strcasecmp(name, options[i].name))
	    return &options[i];
    }

    return NULL;
}

static void Print_default_value(xp_option_t *opt)
{
    switch (opt->type) {
    case xp_noarg_option:
	break;
    case xp_bool_option:
	printf("        The default value is: %s.\n",
	       opt->bool_defval == true ? "True" : "False");
	break;
    case xp_int_option:
	printf("        The default value is: %d.\n", opt->int_defval);
	break;
    case xp_double_option:
	printf("        The default value is: %f.\n", opt->dbl_defval);
	break;
    case xp_string_option:
	if (opt->str_defval && strlen(opt->str_defval) > 0)
	    printf("        The default value is: %s.\n", opt->str_defval);
	else
	    printf("        There is no default value for this option.\n");
	break;
    case xp_color_option:
	assert(0 && "TODO");
	break;
    case xp_key_option:
	if (opt->key_defval && strlen(opt->key_defval) > 0)
	    printf("        The default %s: %s.\n",
		   (strchr(opt->key_defval, ' ') == NULL
		    ? "key is" : "keys are"), opt->key_defval);
	else
	    printf("        There is no default value for this option.\n");
	break;
    default:
	assert(0 && "TODO");
	break;
    }
}

void Usage(void)
{
    int i;

    printf("Usage: xpilot [-options ...] [server]\n"
	   "Where options include:\n" "\n");
    for (i = 0; i < num_options; i++) {
	xp_option_t *opt = Option_by_index(i);

	printf("    -%s %s\n", opt->name,
	       (opt->type != xp_noarg_option) ? "<value>" : "");
	if (opt->help && opt->help[0]) {
	    const char *str;
	    printf("        ");
	    for (str = opt->help; *str; str++) {
		putchar(*str);
		if (*str == '\n' && str[1])
		    printf("        ");
	    }
	    if (str[-1] != '\n')
		putchar('\n');
	}
	Print_default_value(opt);
	printf("\n");
    }
    printf("Most of these options can also be set in the .xpilotrc file\n"
	   "in your home directory.\n"
	   "Each key option may have multiple keys bound to it and\n"
	   "one key may be used by multiple key options.\n"
	   "If no server is specified then xpilot will search\n"
	   "for servers on your local network.\n"
	   "For a listing of remote servers try: telnet meta.xpilot.org 4400 \n");

    exit(1);
}


static bool Set_noarg_option(xp_option_t *opt, bool value)
{
    assert(opt);
    assert(opt->type == xp_noarg_option);
    assert(opt->noarg_ptr);

    *opt->noarg_ptr = value;
    /*
     * printf("Value of option %s is now %s.\n", opt->name, opt->noarg_ptr 
     * ? "true" : "false");
     */
    return true;
}


static bool Set_bool_option(xp_option_t *opt, bool value)
{
    bool retval = true;

    assert(opt);
    assert(opt->type == xp_bool_option);
    assert(opt->bool_ptr);

    /*
     * NOTE: this function assigns the value directly, before calling the
     * setfunc. If you have an option where you have a setfunc and you would
     * not want the value to be assinged directly, you must still provide
     * a bool pointer for this code to assign to. Note that this code
     * assumes that after the set method (opt->bool_setfunc) returns,
     * this pointer points to the new value for this option.
     */
    *opt->bool_ptr = value;

    if (opt->bool_setfunc)
	retval = opt->bool_setfunc(opt, value);

    /*
     * printf("Value of option %s is now %s.\n", opt->name, opt->bool_ptr
     * ? "true" : "false");
     */
    return retval;
}

static bool Set_int_option(xp_option_t *opt, int value)
{
    bool retval = true;

    assert(opt);
    assert(opt->type == xp_int_option);
    assert(opt->int_ptr);

    if (!(value >= opt->int_minval && value <= opt->int_maxval)) {
	warn("Bad value %d for option \"%s\", using default...",
	     value, opt->name);
	value = opt->int_defval;
    }

    LIMIT(value, opt->int_minval, opt->int_maxval);

    *opt->int_ptr = value;

    if (opt->int_setfunc)
	retval = opt->int_setfunc(opt, value);

    /*
     * printf("Value of option %s is now %d.\n", opt->name,
     * *opt->int_ptr); 
     */
    return retval;
}

static bool Set_double_option(xp_option_t *opt, double value)
{
    bool retval = true;

    assert(opt);
    assert(opt->type == xp_double_option);
    assert(opt->dbl_ptr);

    if (!(value >= opt->dbl_minval && value <= opt->dbl_maxval)) {
	warn("Bad value %.3f for option \"%s\", using default...",
	     value, opt->name);
	value = opt->dbl_defval;
    }

    LIMIT(value, opt->dbl_minval, opt->dbl_maxval);

    *opt->dbl_ptr = value;

    if (opt->dbl_setfunc)
	retval = opt->dbl_setfunc(opt, value);

    /*
     * printf("Value of option %s is now %.3f.\n", opt->name,
     * *opt->dbl_ptr); 
     */
    return retval;
}

static bool Set_string_option(xp_option_t *opt, const char *value)
{
    bool retval = true;

    assert(opt);
    assert(opt->type == xp_string_option);
    assert(opt->str_ptr || (opt->str_setfunc && opt->str_getfunc));
    assert(value);		/* allow NULL ? */

    /*
     * The reason string options don't assume a static area is that that
     * would not allow dynamically allocated strings of arbitrary size.
     */
    if (opt->str_ptr)
	strlcpy(opt->str_ptr, value, opt->str_size);

    if (opt->str_setfunc)
	retval = opt->str_setfunc(opt, value);

    /*
     * if (opt->str_ptr) printf("Value of option %s is now \"%s\".\n",
     * opt->name, opt->str_ptr); else printf("Value of option %s is now
     * \"%s\".\n", opt->name, opt->str_getfunc(opt)); 
     */
    return retval;
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
    XP_KS_KP_PRINT, /* Print Screen */

} xp_keysym_t;

typedef struct {
    xp_keysym_t ks;
    keys_t key;
} xp_keydefs_t;

xp_keydefs_t *xpkeydefs = NULL;
int num_xpkeydefs = 0;
int max_xpkeydefs = 0;

void Store_xpkeydef(xp_keysym_t ks, keys_t key)
{
    int i;
    xp_keydefs_t xpkeydef;

    /*
     * first check if pair (ks, key) already exists 
     */
    for (i = 0; i < num_xpkeydefs; i++) {
	xp_keydefs_t *kd = &xpkeydefs[i];

	if (kd->ks == ks && kd->key == key) {
	    warn("Pair (%d, %d) exist from before", (int) ks, (int) key);
	    /*
	     * already exists, no need to store 
	     */
	    return;
	}
    }

    xpkeydef.ks = ks;
    xpkeydef.key = key;

    /*
     * find first KEY_DUMMY after lazy deletion 
     */
    for (i = 0; i < num_xpkeydefs; i++) {
	xp_keydefs_t *kd = &xpkeydefs[i];

	if (kd->key == KEY_DUMMY) {
	    assert(kd->ks == XP_KS_UNKNOWN);
	    warn("Store_xpkeydef: Found dummy at index %d", i);
	    *kd = xpkeydef;
	    return;
	}
    }

    /*
     * ok just store it then 
     */
    STORE(xp_keydefs_t, xpkeydefs, num_xpkeydefs, max_xpkeydefs, xpkeydef);
}

static void Remove_key_from_xpkeydefs(keys_t key)
{
    int i;

    assert(key != KEY_DUMMY);
    for (i = 0; i < num_xpkeydefs; i++) {
	xp_keydefs_t *kd = &xpkeydefs[i];

	/*
	 * lazy deletion 
	 */
	if (kd->key == key) {
	    warn("Remove_key_from_xpkeydefs: Removing key at index %d", i);
	    kd->ks = XP_KS_UNKNOWN;
	    kd->key = KEY_DUMMY;
	}
    }
}

typedef struct {
    const char *name;
    xp_keysym_t ks;
} xp_key_t;

static xp_key_t xpkeys[] = {
    {"BackSpace",	XP_KS_BACKSPACE},
    {"Tab",		XP_KS_TAB},
    {"Return",		XP_KS_RETURN},
    {"Pause",		XP_KS_PAUSE},
    {"Scroll_Lock",	XP_KS_SCROLLOCK},
    {"Escape",		XP_KS_ESCAPE},
    {"Delete",		XP_KS_DELETE},
    {"Home",		XP_KS_HOME},
    {"Left",		XP_KS_LEFT},
    {"Up",		XP_KS_UP},
    {"Right",		XP_KS_RIGHT},
    {"Down",		XP_KS_HOME},
    {"Page_Up",		XP_KS_PAGEUP},
    {"Page_Down",	XP_KS_PAGEDOWN},
    {"End",		XP_KS_END},
    {"Insert",		XP_KS_INSERT},
    {"Num_Lock",	XP_KS_NUMLOCK},
    {"KP_Enter",	XP_KS_KP_ENTER},
    {"KP_Multiply",	XP_KS_KP_MULTIPLY},
    {"KP_Add",		XP_KS_KP_PLUS},
    {"KP_Subtract",	XP_KS_KP_MINUS},
    {"KP_Decimal",	XP_KS_KP_PERIOD},
    {"KP_Divide",	XP_KS_KP_DIVIDE},
    {"KP_Insert",	XP_KS_KP_INSERT},
    {"KP_Delete",	XP_KS_KP_DELETE},
    {"KP_0",		XP_KS_KP0},
    {"KP_1",		XP_KS_KP1},
    {"KP_2",		XP_KS_KP2},
    {"KP_3",		XP_KS_KP3},
    {"KP_4",		XP_KS_KP4},
    {"KP_5",		XP_KS_KP5},
    {"KP_6",		XP_KS_KP6},
    {"KP_7",		XP_KS_KP7},
    {"KP_8",		XP_KS_KP8},
    {"KP_9",		XP_KS_KP9},
    {"F1",		XP_KS_F1},
    {"F2",		XP_KS_F2},
    {"F3",		XP_KS_F3},
    {"F4",		XP_KS_F4},
    {"F5",		XP_KS_F5},
    {"F6",		XP_KS_F6},
    {"F7",		XP_KS_F7},
    {"F8",		XP_KS_F8},
    {"F9",		XP_KS_F9},
    {"F10",		XP_KS_F10},
    {"F11",		XP_KS_F11},
    {"F12",		XP_KS_F12},
    {"Shift_L",		XP_KS_LSHIFT},
    {"Shift_R",		XP_KS_RSHIFT},
    {"Control_L",	XP_KS_LCTRL},
    {"Control_R",	XP_KS_RCTRL},
    {"Caps_Lock",	XP_KS_CAPSLOCK},
    {"space",		XP_KS_SPACE},
    {"apostrophe",	XP_KS_QUOTE},
    {"quoteright",	XP_KS_QUOTE},
    {"comma",		XP_KS_COMMA},
    {"plus",		XP_KS_PLUS},
    {"minus",		XP_KS_MINUS},
    {"period",		XP_KS_PERIOD},
    {"slash",		XP_KS_SLASH},
    {"semicolon",	XP_KS_SEMICOLON},
    {"equal",		XP_KS_EQUALS},
    {"0",		XP_KS_0},
    {"1",		XP_KS_1},
    {"2",		XP_KS_2},
    {"3",		XP_KS_3},
    {"4",		XP_KS_4},
    {"5",		XP_KS_5},
    {"6",		XP_KS_6},
    {"7",		XP_KS_7},
    {"8",		XP_KS_8},
    {"9",		XP_KS_9},
    {"a",		XP_KS_a},
    {"b",		XP_KS_b},
    {"c",		XP_KS_c},
    {"d",		XP_KS_d},
    {"e",		XP_KS_e},
    {"f",		XP_KS_f},
    {"g",		XP_KS_g},
    {"h",		XP_KS_h},
    {"i",		XP_KS_i},
    {"j",		XP_KS_j},
    {"k",		XP_KS_k},
    {"l",		XP_KS_l},
    {"m",		XP_KS_m},
    {"n",		XP_KS_n},
    {"o",		XP_KS_o},
    {"p",		XP_KS_p},
    {"q",		XP_KS_q},
    {"r",		XP_KS_r},
    {"s",		XP_KS_s},
    {"t",		XP_KS_t},
    {"u",		XP_KS_u},
    {"v",		XP_KS_v},
    {"w",		XP_KS_w},
    {"x",		XP_KS_x},
    {"y",		XP_KS_y},
    {"z",		XP_KS_z},
    {"bracketleft",	XP_KS_LEFTBRACKET},
    {"backslash",	XP_KS_BACKSLASH},
    {"bracketright",	XP_KS_RIGHTBRACKET},
    {"grave",		XP_KS_BACKQUOTE},
    {"quoteleft",	XP_KS_BACKQUOTE},

    /*
     * kps additions 
     */
    {"Prior",		XP_KS_PAGEUP},
    {"Next",		XP_KS_PAGEDOWN},
    {"Print",		XP_KS_KP_PRINT},
    {"Linefeed",	XP_KS_RETURN},
    {"Select",		XP_KS_UP},
    

};

xp_keysym_t String_to_xp_keysym(const char *name)
{
    int i;

    for (i = 0; i < NELEM(xpkeys); i++) {
	xp_key_t *k = &xpkeys[i];
	if (!strcmp(k->name, name))
	    return k->ks;
	if (!strcasecmp(k->name, name)) {
	    warn("Correct case of key \"%s\" is \"%s\"", name, k->name);
	    return k->ks;
	}
    }

    return XP_KS_UNKNOWN;
}

static bool Set_key_option(xp_option_t *opt, const char *value)
{
    bool retval = true;
    char *str, *valcpy;

    assert(opt);
    assert(opt->name);
    assert(opt->type == xp_key_option);
    assert(opt->key != KEY_DUMMY);
    assert(value);

    /*
     * warn("Setting key option %s to \"%s\"", opt->name, value); 
     */

    /*
     * First remove the old setting.
     */
    if (opt->key_string)
	xp_free(opt->key_string);
    Remove_key_from_xpkeydefs(opt->key);

    /*
     * Store the new setting.
     */
    opt->key_string = xp_safe_strdup(value);
    valcpy = xp_safe_strdup(value);
    for (str = strtok(valcpy, " \t\r\n");
	 str != NULL;
	 str = strtok(NULL, " \t\r\n")) {
	xp_keysym_t ks = String_to_xp_keysym(str);

	if (ks == XP_KS_UNKNOWN) {
	    warn("Invalid keysym \"%s\" for key \"%s\".\n", str, opt->name);
	    continue;
	}

	/*
	 * kps - here we should count how many succesful bindings we've done,
	 * and if no successful bindings was done, the old setting should
	 * be restored.
	 */
	Store_xpkeydef(ks, opt->key);
    }

    xp_free(valcpy);
    return true;
}

#if 0
static xp_key_binding_callback_t key_binding_callback = NULL;

void Set_key_binding_callback(xp_key_binding_callback_t callback)
{
    key_binding_callback = callback;
}


static bool Set_key_option(xp_option_t *opt, const char *value)
{
    bool retval = true;
    char *str, *valcpy;

    assert(opt);
    assert(opt->name);
    assert(opt->type == xp_key_option);
    assert(value);

    valcpy = xp_safe_strdup(value);
    if (opt->key_string)
	xp_free(opt->key_string);
    opt->key_string = xp_safe_strdup(value);

    /*
     * No setfunc supported, rather a key binding callback is used.
     */
    for (str = strtok(valcpy, " \t\r\n");
	 str != NULL;
	 str = strtok(NULL, " \t\r\n")) {
	bool ok;
	/*
	 * The platform specific code must register a callback
	 * for us to bind keys.
	 */
	assert(key_binding_callback != NULL);
	ok = (*key_binding_callback) (opt->key, str);
	if (!ok)
	    warn("Invalid keysym \"%s\" for key \"%s\".\n",
		 str, opt->name);
	/*
	 * else printf("Keysym \"%s\" was successfully bound to key
	 * \"%s\".\n", str, opt->name); 
	 */
    }

    xp_free(valcpy);
    /* not currently returning false even if keysyms are invalid */
    return retval;
}
#endif

static bool is_legal_value(xp_option_type_t type, const char *value)
{
    if (type == xp_noarg_option || type == xp_bool_option) {
	if (ON(value) || OFF(value))
	    return true;
	return false;
    }
    if (type == xp_int_option) {
	int foo;

	if (!sscanf(value, "%d", &foo))
	    return false;
	return true;
    }
    if (type == xp_double_option) {
	double foo;

	if (!sscanf(value, "%lf", &foo))
	    return false;
	return true;
    }
    return true;
}

/*
 * This could also be used from a client '\set' command, e.g.
 * "\set scalefactor 1.5"
 */
/*
 * returns true if ok 
 */
bool Set_option(const char *name, const char *value)
{
    xp_option_t *opt;

    opt = Find_option(name);
    if (!opt) {
	warn("Could not find option \"%s\"\n", name);
	return false;
    }

    if (!is_legal_value(opt->type, value)) {
	warn("Bad value \"%s\" for option \"%s\"", value, opt->name);
	return false;
    }

    switch (opt->type) {
    case xp_noarg_option:
	return Set_noarg_option(opt, ON(value) ? true : false);
    case xp_bool_option:
	return Set_bool_option(opt, ON(value) ? true : false);
    case xp_int_option:
	return Set_int_option(opt, atoi(value));
    case xp_double_option:
	return Set_double_option(opt, atof(value));
    case xp_string_option:
	return Set_string_option(opt, value);
    case xp_key_option:
	return Set_key_option(opt, value);
    default:
	warn("FOO");
	assert(0);
    }
    return false;
}


/*
 * kps - these commands need some fine tuning. 
 * TODO - unset a value, i.e. set it to empty 
 */
/*
 * Handler for \set client command.
 */
void Set_command(const char *args)
{
    char *name, *value, *valcpy;
    xp_option_t *opt;

    assert(args);

    valcpy = xp_safe_strdup(args);

    name = strtok(valcpy, " \t\r\n");
    value = strtok(NULL, "");

    opt = Find_option(name);

    if (opt && value) {
	const char *newvalue;
	const char *nm = Option_get_name(opt);
	char msg[MSG_LEN];

	Set_option(name, value);

	newvalue = Option_value_to_string(opt);
	snprintf(msg, sizeof(msg),
		 "The value of %s is now \"%s\". [*Client reply*]",
		 nm, newvalue);
	Add_message(msg);
    } else {
	Add_message("Boring... [*Client reply*]");
	/*
	 * usage, e.g. return false 
	 */
    }

    xp_free(valcpy);
}

const char *Option_value_to_string(xp_option_t *opt)
{
    static char buf[MSG_LEN];

    switch (opt->type) {
    case xp_noarg_option:
	sprintf(buf, "%s", *opt->noarg_ptr == true ? "true" : "false");
	break;
    case xp_bool_option:
	sprintf(buf, "%s", *opt->bool_ptr == true ? "true" : "false");
	break;
    case xp_int_option:
	sprintf(buf, "%d", *opt->int_ptr);
	break;
    case xp_double_option:
	sprintf(buf, "%.3lf", *opt->dbl_ptr);
	break;
    case xp_string_option:
	/*
	 * Assertion in Store_option guarantees one of these is not NULL. 
	 */
	if (opt->str_ptr)
	    return opt->str_ptr;
	else
	    return opt->str_getfunc(opt);
    case xp_key_option:
	assert(opt->key_string);
	return opt->key_string;
    default:
	assert(0 && "Unknown option type");
    }
    return buf;
}


/*
 * Handler for \get client command.
 */
void Get_command(const char *args)
{
    char *name, *valcpy;
    xp_option_t *opt;
    char msg[MSG_LEN];

    assert(args);

    valcpy = xp_safe_strdup(args);

    name = strtok(valcpy, " \t\r\n");
    opt = Find_option(name);

    if (opt) {
	const char *val = Option_value_to_string(opt);
	const char *nm = Option_get_name(opt);
	if (val && strlen(val) > 0)
	    snprintf(msg, sizeof(msg),
		     "The value of %s is \"%s\". [*Client reply*]", nm, val);
	else
	    sprintf(msg, "The option %s has no value. [*Client reply*]", nm);
	Add_message(msg);
    } else {
	sprintf(msg, "No client option named \"%s\". [*Client reply*]",	name);
	Add_message(msg);
    }

    xp_free(valcpy);
}

/*
 * NOTE: Store option assumes the passed pointers will remain valid.
 */
void Store_option(xp_option_t *opt)
{
    xp_option_t option;

    assert(opt->name);
    assert(strlen(opt->name) > 0);
    assert(opt->help);
    assert(strlen(opt->help) > 0);

    /*
     * Let's not allow several options with the same name 
     */
    if (Find_option(opt->name) != NULL) {
	warn("Trying to store duplicate option \"%s\"", opt->name);
	assert(0);
    }

    /*
     * Check that default value is in range 
     * NOTE: these assertions will hold also for options of other types 
     */
    assert(opt->int_defval >= opt->int_minval);
    assert(opt->int_defval <= opt->int_maxval);
    assert(opt->dbl_defval >= opt->dbl_minval);
    assert(opt->dbl_defval <= opt->dbl_maxval);

    memcpy(&option, opt, sizeof(xp_option_t));

    STORE(xp_option_t, options, num_options, max_options, option);

    opt = Find_option(opt->name);
    assert(opt);

    /* Set the default value. */
    switch (opt->type) {
    case xp_noarg_option:
	Set_noarg_option(opt, false);
	break;
    case xp_bool_option:
	Set_bool_option(opt, opt->bool_defval);
	break;
    case xp_int_option:
	Set_int_option(opt, opt->int_defval);
	break;
    case xp_double_option:
	Set_double_option(opt, opt->dbl_defval);
	break;
    case xp_string_option:
	assert(opt->str_defval);
	assert(opt->str_ptr || (opt->str_setfunc && opt->str_getfunc));
	Set_string_option(opt, opt->str_defval);
	break;
    case xp_key_option:
	assert(opt->key_defval);
	assert(opt->key != KEY_DUMMY);
	Set_key_option(opt, opt->key_defval);
	break;
    default:
	warn("Could not set default value for option %s", opt->name);
	break;
    }

}



static void Parse_xpilotrc_line(const char *line)
{
    char *s;

    /* printf("parsing xpilotrc line \"%s\"\n", line); */
    /*
     * Ignore lines that don't start with xpilot. or
     * xpilot*
     */
    if (!(strncasecmp(line, "xpilot.", 7) == 0
	  || strncasecmp(line, "xpilot*", 7) == 0))
	/*
	 * not interested 
	 */
	return;

    /* printf("-> line is now \"%s\"\n", line); */
    line += 7;
    /* printf("-> line is now \"%s\"\n", line); */
    if (!(s = strchr(line, ':'))) {
	/* no colon on line with xpilot. or xpilot* */
	/* warn("line missing colon"); */
	return;
    }

    /*
     * Zero the colon, advance to next char, remove leading whitespace
     * from option value.
     */
    *s++ = '\0';
    while (isspace(*s))
	s++;

    Set_option(line, s);
}


static inline bool is_noarg_option(const char *name)
{
    xp_option_t *opt = Find_option(name);

    if (!opt || opt->type != xp_noarg_option)
	return false;
    return true;
}

void Parse_options(int *argcp, char **argvp)
{
    char path[PATH_MAX + 1];
    char buf[BUFSIZ];
    FILE *fp;
    int arg_ind, num_remaining_args, num_servers = 0;

    /*
     * Read options from xpilotrc.
     */
    Get_xpilotrc_file(path, sizeof(path));
    warn("Using xpilotrc file %s\n", path);
    if (strlen(path) > 0 && ((fp = fopen(path, "r")) != NULL)) {
	while (fgets(buf, sizeof buf, fp)) {
	    char *cp;
	    /*
	     * kps - remove NL and CR, does this work in windows ? 
	     */
	    cp = strchr(buf, '\n');
	    if (cp)
		*cp = '\0';
	    cp = strchr(buf, '\r');
	    if (cp)
		*cp = '\0';
	    Parse_xpilotrc_line(buf);
	}
	fclose(fp);
    }

    /*
     * Here we step trough argc - 1 arguments, leaving
     * only the arguments that might be server names.
     */
    arg_ind = 1;
    num_remaining_args = *argcp - 1;

    while (num_remaining_args > 0) {
	if (argvp[arg_ind][0] == '-') {
	    char *arg = &argvp[arg_ind][1];

	    /* Add GNU style option support e.g. --wallcolor=1 ??? */
	    if (is_noarg_option(arg)) {
		int i;
		Set_option(arg, "true");
		num_remaining_args--;
		for (i = 0; i < num_remaining_args; i++)
		    argvp[arg_ind + i] = argvp[arg_ind + i + 1];
	    } else {
		bool ok = false;

		if (num_remaining_args >= 2) {
		    ok = Set_option(arg, argvp[arg_ind + 1]);
		    if (ok) {
			int i;
			num_remaining_args -= 2;
			for (i = 0; i < num_remaining_args; i++)
			    argvp[arg_ind + i] = argvp[arg_ind + i + 2];
		    }
		}

		if (!ok) {
		    warn("Unknown or incomplete option '%s'", argvp[arg_ind]);
		    warn("Type: %s -help to see a list of options", argvp[0]);
		    exit(1);
		}
	    }
	} else {
	    /* assume this is a server name. */
	    arg_ind++;
	    num_remaining_args--;
	    num_servers++;
	}
    }

    /*
     * The remaining args are assumed to be names of servers to try to contact.
     * + 1 is for the program name.
     */
    *argcp = num_servers + 1;

    if (xpArgs.help)
	Usage();

    if (xpArgs.version) {
	puts(TITLE);
	exit(0);
    }

}



const char *Get_keyHelpString(keys_t key)
{
    int i;
    char *nl;
    static char buf[MAX_CHARS];

    for (i = 0; i < num_options; i++) {
	xp_option_t *opt = Option_by_index(i);
	if (opt->key == key) {
	    strlcpy(buf, opt->help, sizeof buf);
	    if ((nl = strchr(buf, '\n')) != NULL)
		*nl = '\0';
	    return buf;
	}
    }

    return NULL;
}


const char *Get_keyResourceString(keys_t key)
{
    int i;

    for (i = 0; i < num_options; i++) {
	xp_option_t *opt = Option_by_index(i);
	if (opt->key == key)
	    return opt->name;
    }

    return NULL;
}

void defaultCleanup(void)
{
}

#ifndef _WINDOWS
void Get_xpilotrc_file(char *path, unsigned size)
{
    const char *home = getenv("HOME");
    const char *defaultFile = ".xpilotrc";
    const char *optionalFile = getenv("XPILOTRC");

    if (optionalFile != NULL)
	strlcpy(path, optionalFile, size);
    else if (home != NULL) {
	strlcpy(path, home, size);
	strlcat(path, "/", size);
	strlcat(path, defaultFile, size);
    } else
	strlcpy(path, "", size);
}
#else
void Get_xpilotrc_file(char *path, unsigned size)
{
    /* kps - some windows pro implement this */
#error "Function Get_xpilotrc_file() not implemented."
}
#endif /* _WINDOWS */

#endif
