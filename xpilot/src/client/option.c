/*
 * XPilotNG, an XPilot-like multiplayer space war game.
 *
 * Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *
 * Copyright (C) 2003-2004 Kristian Söderblom <kps@users.sourceforge.net>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "xpclient.h"

char option_version[] = VERSION;

int num_options = 0;
int max_options = 0;

xp_option_t *options = NULL;


unsigned String_hash(const char *s)
{
    unsigned		hash = 0;

    for (; *s; s++) {
	/* hash gives same values even if case is different */
	int c = tolower(*s);

	hash = (((hash >> 29) & 7) | (hash << 3)) ^ c;
    }

    return hash;
}

xp_option_t *Find_option(const char *name)
{
    int i;
    unsigned hash = String_hash(name);

    for (i = 0; i < num_options; i++) {
	if (hash == options[i].hash && !strcasecmp(name, options[i].name))
	    return &options[i];
    }

    return NULL;
}

static const char *Option_default_value_to_string(xp_option_t *opt)
{
    static char buf[4096];

    switch (opt->type) {
    case xp_noarg_option:
	strcpy(buf, "");
	break;
    case xp_bool_option:
	sprintf(buf, "%s", opt->bool_defval == true ? "yes" : "no");
	break;
    case xp_int_option:
	sprintf(buf, "%d", opt->int_defval);
	break;
    case xp_double_option:
	sprintf(buf, "%.3f", opt->dbl_defval);
	break;
    case xp_string_option:
	if (opt->str_defval && strlen(opt->str_defval) > 0)
	    strlcpy(buf, opt->str_defval, sizeof(buf));
	else
	    strcpy(buf, "");
	break;
    case xp_key_option:
	if (opt->key_defval && strlen(opt->key_defval) > 0)
	    strlcpy(buf, opt->key_defval, sizeof(buf));
	else
	    strcpy(buf, "");
	break;
    default:
	assert(0 && "Unknown option type");
    }
    return buf;
}


static void Print_default_value(xp_option_t *opt)
{
    const char *defval = Option_default_value_to_string(opt);

    switch (opt->type) {
    case xp_noarg_option:
	break;
    case xp_bool_option:
    case xp_int_option:
    case xp_double_option:
    case xp_string_option:
	if (strlen(defval) > 0)
	    printf("        The default value is: %s.\n", defval);
	else
	    printf("        There is no default value for this option.\n");
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
	assert(0 && "Unknown option type");
    }
}

void Usage(void)
{
    int i;

    printf("Usage: %s [-options ...] [server]\n"
	   "Where options include:\n" "\n", Program_name());
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
	   "If no server is specified on the command line, xpilot will\n"
	   "display a welcome screen where you can select a server.\n");

    exit(1);
}

static void Version(void)
{
    printf("%s %s\n", Program_name(), VERSION);
    exit(0);
}

bool Set_noarg_option(xp_option_t *opt, bool value, xp_option_origin_t origin)
{
    assert(opt);
    assert(opt->type == xp_noarg_option);
    assert(opt->noarg_ptr);

    *opt->noarg_ptr = value;
    opt->origin = origin;

    return true;
}


bool Set_bool_option(xp_option_t *opt, bool value, xp_option_origin_t origin)
{
    bool retval = true;

    assert(opt);
    assert(opt->type == xp_bool_option);
    assert(opt->bool_ptr);

    if (opt->bool_setfunc)
	retval = opt->bool_setfunc(opt, value);
    else
	*opt->bool_ptr = value;

    if (retval)
	opt->origin = origin;

    return retval;
}

bool Set_int_option(xp_option_t *opt, int value, xp_option_origin_t origin)
{
    bool retval = true;

    assert(opt);
    assert(opt->type == xp_int_option);
    assert(opt->int_ptr);

    if (origin == xp_option_origin_setcmd) {
	char msg[MSG_LEN];

	if (value < opt->int_minval) {
	    snprintf(msg, sizeof(msg),
		     "Minumum value for option %s is %d. [*Client reply*]",
		     opt->name, opt->int_minval);
	    Add_message(msg);
	}
	if (value > opt->int_maxval) {
	    snprintf(msg, sizeof(msg),
		     "Maximum value for option %s is %d. [*Client reply*]",
		     opt->name, opt->int_maxval);
	    Add_message(msg);
	}
    }
    else {
	if (!(value >= opt->int_minval && value <= opt->int_maxval)) {
	    warn("Bad value %d for option \"%s\", using default...",
		 value, opt->name);
	    value = opt->int_defval;
	}
    }

    LIMIT(value, opt->int_minval, opt->int_maxval);

    if (opt->int_setfunc)
	retval = opt->int_setfunc(opt, value);
    else
	*opt->int_ptr = value;

    if (retval)
	opt->origin = origin;

    return retval;
}

bool Set_double_option(xp_option_t *opt, double value,
		       xp_option_origin_t origin)
{
    bool retval = true;

    assert(opt);
    assert(opt->type == xp_double_option);
    assert(opt->dbl_ptr);

    if (origin == xp_option_origin_setcmd) {
	char msg[MSG_LEN];

	if (value < opt->dbl_minval) {
	    snprintf(msg, sizeof(msg),
		     "Minumum value for option %s is %.3f. [*Client reply*]",
		     opt->name, opt->dbl_minval);
	    Add_message(msg);
	}
	if (value > opt->dbl_maxval) {
	    snprintf(msg, sizeof(msg),
		     "Maximum value for option %s is %.3f. [*Client reply*]",
		     opt->name, opt->dbl_maxval);
	    Add_message(msg);
	}
    }
    else {
	if (!(value >= opt->dbl_minval && value <= opt->dbl_maxval)) {
	    warn("Bad value %.3f for option \"%s\", using default...",
		 value, opt->name);
	    value = opt->dbl_defval;
	}
    }

    LIMIT(value, opt->dbl_minval, opt->dbl_maxval);

    if (opt->dbl_setfunc)
	retval = opt->dbl_setfunc(opt, value);
    else
	*opt->dbl_ptr = value;

    if (retval)
	opt->origin = origin;

    return retval;
}

bool Set_string_option(xp_option_t *opt, const char *value,
		       xp_option_origin_t origin)
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
    if (opt->str_setfunc)
	retval = opt->str_setfunc(opt, value);
    else
	strlcpy(opt->str_ptr, value, opt->str_size);

    if (retval)
	opt->origin = origin;

    return retval;
}

xp_keydefs_t *keydefs = NULL;
int num_keydefs = 0;
int max_keydefs = 0;


/*
 * This function is used when platform specific code has an event where
 * the user has pressed or released the key defined by the keysym 'ks'.
 * When the key state has changed, the first call to this function should have
 * 'reset' true, then following calls related to the same event should
 * have 'reset' false. For each returned xpilot key, the calling code
 * should call some handler. The function should be called until it returns
 * KEY_DUMMY.
 */
keys_t Generic_lookup_key(xp_keysym_t ks, bool reset)
{
    keys_t ret = KEY_DUMMY;
    static int i = 0;

    if (reset)
	i = 0;

    /*
     * Variable 'i' is already initialized.
     * Use brute force linear search to find the key.
     */
    for (; i < num_keydefs; i++) {
	if (ks == keydefs[i].keysym) {
	    ret = keydefs[i].key;
	    i++;
	    break;
	}
    }

    return ret;
}

static void Store_keydef(int ks, keys_t key)
{
    int i;
    xp_keydefs_t keydef;

    /*
     * first check if pair (ks, key) already exists 
     */
    for (i = 0; i < num_keydefs; i++) {
	xp_keydefs_t *kd = &keydefs[i];

	if (kd->keysym == ks && kd->key == key) {
	    /*warn("Pair (%d, %d) exist from before", ks, (int) key);*/
	    /*
	     * already exists, no need to store 
	     */
	    return;
	}
    }

    keydef.keysym = ks;
    keydef.key = key;

    /*
     * find first KEY_DUMMY after lazy deletion 
     */
    for (i = 0; i < num_keydefs; i++) {
	xp_keydefs_t *kd = &keydefs[i];

	if (kd->key == KEY_DUMMY) {
	    assert(kd->keysym == XP_KS_UNKNOWN);
	    /*warn("Store_keydef: Found dummy at index %d", i);*/
	    *kd = keydef;
	    return;
	}
    }

    /*
     * no lazily deleted entry, ok, just store it then
     */
    STORE(xp_keydefs_t, keydefs, num_keydefs, max_keydefs, keydef);
}

static void Remove_key_from_keydefs(keys_t key)
{
    int i;

    assert(key != KEY_DUMMY);
    for (i = 0; i < num_keydefs; i++) {
	xp_keydefs_t *kd = &keydefs[i];

	/*
	 * lazy deletion 
	 */
	if (kd->key == key) {
	    /*warn("Remove_key_from_keydefs: Removing key at index %d", i);*/
	    kd->keysym = XP_KS_UNKNOWN;
	    kd->key = KEY_DUMMY;
	}
    }
}

static bool Set_key_option(xp_option_t *opt, const char *value,
			   xp_option_origin_t origin)
{
    /*bool retval = true;*/
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
    Remove_key_from_keydefs(opt->key);

    /*
     * Store the new setting.
     */
    opt->key_string = xp_safe_strdup(value);
    valcpy = xp_safe_strdup(value);
    for (str = strtok(valcpy, " \t\r\n");
	 str != NULL;
	 str = strtok(NULL, " \t\r\n")) {
	xp_keysym_t ks;

	/*
	 * You can write "none" for keys in xpilotrc to disable the key.
	 */
	if (!strcasecmp(str, "none"))
	    continue;

	ks = String_to_xp_keysym(str);
	if (ks == XP_KS_UNKNOWN) {
	    warn("Invalid keysym \"%s\" for key \"%s\".\n", str, opt->name);
	    continue;
	}

	Store_keydef(ks, opt->key);
    }

    /* in fact if we only get invalid keysyms we should return false */
    opt->origin = origin;
    xp_free(valcpy);
    return true;
}

static bool is_legal_value(xp_option_type_t type, const char *value)
{
    if (type == xp_noarg_option || type == xp_bool_option) {
	if (ON(value) || OFF(value))
	    return true;
	return false;
    }
    if (type == xp_int_option) {
	int foo;

	if (sscanf(value, "%d", &foo) <= 0)
	    return false;
	return true;
    }
    if (type == xp_double_option) {
	double foo;

	if (sscanf(value, "%lf", &foo) <= 0)
	    return false;
	return true;
    }
    return true;
}


bool Set_option(const char *name, const char *value, xp_option_origin_t origin)
{
    xp_option_t *opt;

    opt = Find_option(name);
    if (!opt)
	/* unknown */
	return false;

    if (!is_legal_value(opt->type, value)) {
	if (origin != xp_option_origin_setcmd)
	    warn("Bad value \"%s\" for option %s.", value, opt->name);
	else {
	    char msg[MSG_LEN];
	
	    snprintf(msg, sizeof(msg),
		     "Bad value \"%s\" for option %s. [*Client reply*]",
		     value, opt->name);
	    Add_message(msg);
	}
	return false;
    }

    switch (opt->type) {
    case xp_noarg_option:
	return Set_noarg_option(opt, ON(value) ? true : false, origin);
    case xp_bool_option:
	return Set_bool_option(opt, ON(value) ? true : false, origin);
    case xp_int_option:
	return Set_int_option(opt, atoi(value), origin);
    case xp_double_option:
	return Set_double_option(opt, atof(value), origin);
    case xp_string_option:
	return Set_string_option(opt, value, origin);
    case xp_key_option:
	return Set_key_option(opt, value, origin);
    default:
	assert(0 && "TODO");
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
    char msg[MSG_LEN];

    assert(args);

    valcpy = xp_safe_strdup(args);

    name = strtok(valcpy, " \t\r\n");
    value = strtok(NULL, "");

    opt = Find_option(name);

    if (!opt) {
	snprintf(msg, sizeof(msg),
		 "Unknown option \"%s\". [*Client reply*]", name);
	Add_message(msg);
	goto out;
    }

    if (!value) {
	Add_message("Set command needs an option and a value. "
		    "[*Client reply*]");
	goto out;
    }
    else {
	const char *newvalue;
	const char *nm = Option_get_name(opt);

	Set_option(name, value, xp_option_origin_setcmd);

	newvalue = Option_value_to_string(opt);
	snprintf(msg, sizeof(msg),
		 "The value of %s is now %s. [*Client reply*]",
		 nm, newvalue);
	Add_message(msg);
    }

 out:
    xp_free(valcpy);
}

const char *Option_value_to_string(xp_option_t *opt)
{
    static char buf[MSG_LEN];

    switch (opt->type) {
    case xp_noarg_option:
	sprintf(buf, "%s", *opt->noarg_ptr == true ? "yes" : "no");
	break;
    case xp_bool_option:
	sprintf(buf, "%s", *opt->bool_ptr == true ? "yes" : "no");
	break;
    case xp_int_option:
	sprintf(buf, "%d", *opt->int_ptr);
	break;
    case xp_double_option:
	sprintf(buf, "%.3f", *opt->dbl_ptr);
	break;
    case xp_string_option:
	/*
	 * Assertion in Store_option guarantees one of these is not NULL. 
	 */
	if (opt->str_getfunc)
	    return opt->str_getfunc(opt);
	else
	    return opt->str_ptr;
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
		     "The value of %s is %s. [*Client reply*]", nm, val);
	else
	    snprintf(msg, sizeof(msg),
		     "The option %s has no value. [*Client reply*]", nm);
	Add_message(msg);
    } else {
	snprintf(msg, sizeof(msg),
		 "No client option named \"%s\". [*Client reply*]", name);
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

    /* Find_option() needs the hash value. */
    opt->hash = String_hash(opt->name);

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
	Set_noarg_option(opt, false, xp_option_origin_default);
	break;
    case xp_bool_option:
	Set_bool_option(opt, opt->bool_defval, xp_option_origin_default);
	break;
    case xp_int_option:
	Set_int_option(opt, opt->int_defval, xp_option_origin_default);
	break;
    case xp_double_option:
	Set_double_option(opt, opt->dbl_defval, xp_option_origin_default);
	break;
    case xp_string_option:
	assert(opt->str_defval);
	assert(opt->str_ptr || (opt->str_setfunc && opt->str_getfunc));
	Set_string_option(opt, opt->str_defval, xp_option_origin_default);
	break;
    case xp_key_option:
	assert(opt->key_defval);
	assert(opt->key != KEY_DUMMY);
	Set_key_option(opt, opt->key_defval, xp_option_origin_default);
	break;
    default:
	warn("Could not set default value for option %s", opt->name);
	break;
    }

}

/*
<SynrG> kps: would be nice if not only it saved options known to other clients, but also comments in the original
*/


typedef struct xpilotrc_line {
    xp_option_t *opt;
    const char *comment;
} xpilotrc_line_t;

static xpilotrc_line_t	*xpilotrc_lines = NULL;
static int num_xpilotrc_lines = 0, max_xpilotrc_lines = 0;
static int num_ok_options = 0;

static void Parse_xpilotrc_line(const char *line)
{
    char *l = xp_safe_strdup(line);
    char *first, *second, *colon, *name, *value = NULL, *comment;
    xpilotrc_line_t t;
    xp_option_t *opt;
    int i;

    memset(&t, 0, sizeof(xpilotrc_line_t));

    /*warn("line is \"%s\"", line);*/

    /* everything after semicolon is comment, ignore it. */
    comment = strchr(l, ';');
    if (comment) {
	/*warn("found comment on line \"%s\"\n", line);*/
	t.comment = xp_safe_strdup(comment);
	*comment = '\0';
    }

    first = strtok(l, " \t");
    if (!first) {
	/* comment line or empty line */
	/*warn("line \"%s\" has comment or is empty.", line);*/
	goto out;
    }

    if (!(strncasecmp(first, "xpilot.", 7) == 0
	  || strncasecmp(first, "xpilot*", 7) == 0)) {
	/* consider it a comment */
	t.comment = xp_safe_strdup(line);
	goto out;
    }

    /* line starts with "xpilot." or "xpilot*" */
    first += strlen("xpilot.");
    /*warn("of token remains \"%s\"", first);*/
    /* get rid of colon if one is found */
    colon = strchr(first, ':');
    if (colon) {
	/*
	 * There might be stuff after the colon, e.g. if line is
	 * xpilot.wallColor:2
	 */
	if (strlen(colon) > 1) {
	    value = colon + 1;
	    /*warn("value is \"%s\"\n", value);*/
	}
	*colon = '\0';
    }
    /*warn("of token remains \"%s\"", first);*/
    /* now first should point to the option name */
    name = first;
    opt = Find_option(name);
    if (!opt) {
	/* this client doesn't know about this option */
	/* warn("Parse_xpilotrc_line: Unknown option \"%s\"\n", first); */
	/* let's just store the line, treat it as a comment */
	t.comment = xp_safe_strdup(line);
	goto out;
    }

    /* did we see this before ? */
    for (i = 0; i < num_xpilotrc_lines; i++) {
	xpilotrc_line_t *x = &xpilotrc_lines[i];
	
	if (x->opt == opt) {
	    /* same option defined several times in xpilotrc */
	    warn("WARNING: Xpilotrc line %d:", num_xpilotrc_lines + 1);
	    warn("Option %s previously given on line %d, ignoring new value.",
		 name, i + 1);
	    /* treat as comment */
	    t.comment = xp_safe_strdup(line);
	    goto out;
	}
    }

    /* ok let's see if a valid value was given */

    /* maybe colon wasn't in first token */
    if (!value) {
	if (!colon) {
	    /*
	     * line may be like "xpilot.foobar :<something>", then the colon
	     * wasn't found in the first token
	     */
	    second = strtok(NULL, " \t");
	    /*warn("second is \"%s\"", second);*/
	    if (second == NULL || *second != ':') {
		/* no colon on line, not ok */
		warn("WARNING: Xpilotrc line %d:", num_xpilotrc_lines + 1);
		warn("Line has no colon after option name, ignoring.");
		/* treat as comment */
		t.comment = xp_safe_strdup(line);
		goto out;
	    }
	    colon = second;
	    /*warn("colon: \"%s\"", colon);*/
	    if (strlen(colon) > 1)
		/* e.g xpilot.wallColor :2 */
		value = colon + 1;
	    else
		/* e.g xpilot.wallColor : 2 */
		value = strtok(NULL, "");
	}
	else {
	    /* line was like xpilot.wallColor: value */
	    value = strtok(NULL, "");
	}
    }

    if (!value) {
	warn("WARNING: Xpilotrc line %d:", num_xpilotrc_lines + 1);
	warn("No value given for option %s, ignoring.", name);
	/* treat as comment */
	t.comment = xp_safe_strdup(line);
	goto out;
    }

    /* remove leading whitespace */
    while (isspace(*value))
	value++;

    /*warn("option is %s, value \"%s\"", name, value);*/

    if (!Set_option(name, value, xp_option_origin_xpilotrc)) {
	warn("WARNING: Xpilotrc line %d:", num_xpilotrc_lines + 1);
	warn("Failed to set option %s value \"%s\", ignoring.", name, value);
	/* treat as comment */
	t.comment = xp_safe_strdup(line);
	goto out;
    }

    t.opt = opt;
    num_ok_options++;

 out:
    STORE(xpilotrc_line_t,
	  xpilotrc_lines, num_xpilotrc_lines, max_xpilotrc_lines, t);
    xp_free(l);
}


static inline bool is_noarg_option(const char *name)
{
    xp_option_t *opt = Find_option(name);

    if (!opt || opt->type != xp_noarg_option)
	return false;
    return true;
}

int Xpilotrc_read(const char *path)
{
    char buf[4096]; /* long enough max line length in xpilotrc? */
    FILE *fp;

    assert(path);
    if (strlen(path) == 0) {
	warn("Xpilotrc_read: Zero length filename.");
	return -1;
    }

    fp = fopen(path, "r");
    if (fp == NULL) {
	error("Xpilotrc_read: Failed to open file \"%s\"", path);
	return -2;
    }

    warn("Reading options from xpilotrc file %s.\n", path);

    while (fgets(buf, sizeof buf, fp)) {
	char *cp;

	cp = strchr(buf, '\n');
	if (cp)
	    *cp = '\0';
	cp = strchr(buf, '\r');
	if (cp)
	    *cp = '\0';
	Parse_xpilotrc_line(buf);
    }

    /*warn("Total number of nonempty lines in xpilotrc: %d",
      num_xpilotrc_lines);
      warn("Number of options set: %d\n", num_ok_options);*/

    fclose(fp);

    return 0;
}


#if 0
/*
 * Find a key in keydefs[].
 * On success set output pointer to index into keydefs[] and return true.
 * On failure return false.
 */
static int Config_find_key(keys_t key, int start, int end, int *key_index)
{
    int			i;

    for (i = start; i < end; i++) {
	if (keydefs[i].key == key) {
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
    for (i = 0; i < num_keydefs; i++) {
	ks = keydefs[i].keysym;
	key = keydefs[i].key;

	/* try and see if we have already saved this key. */
	if (Config_find_key(key, 0, i, &j) == true)
	    /* yes, saved this one before.  skip it now. */
	    continue;

	if ((str = XKeysymToString(ks)) == NULL)
	    continue;

	if ((res = Get_keyResourceString(key)) != NULL) {
	    strlcpy(buf, str, sizeof(buf));
	    /* find all other keysyms which map to the same key. */
	    j = i;
	    while (Config_find_key(key, j + 1, num_keydefs, &j) == true) {
		ks = keydefs[j].keysym;
		if ((str = XKeysymToString(ks)) != NULL) {
		    strlcat(buf, " ", sizeof(buf));
		    strlcat(buf, str, sizeof(buf));
		}
	    }
	    Config_save_resource(fp, res, buf);
	}
    }
}
#endif

#define TABSIZE 8
static void Xpilotrc_write_line(FILE *fp, xpilotrc_line_t *lp)
{
    char buf[4096];
    xp_option_t *opt = lp->opt;
    int len, numtabs, i;

    strcpy(buf, "");

    if (opt) {
	const char *value;

	snprintf(buf, sizeof(buf), "xpilot.%s:", opt->name);
	len = (int) strlen(buf);
	/* assume tabs are max size of TABSIZE */
	numtabs = ((5 * TABSIZE - 1) - len) / TABSIZE;
	for (i = 0; i < numtabs; i++)
	    strlcat(buf, "\t", sizeof(buf));
	value = Option_value_to_string(opt);
	if (value)
	    strlcat(buf, value, sizeof(buf));
    }

    if (lp->comment)
	strlcat(buf, lp->comment, sizeof(buf));

    fprintf(fp, "%s\n", buf);
}
#undef TABSIZE

int Xpilotrc_write(const char *path)
{
    FILE *fp;
    int i;

    assert(path);
    if (strlen(path) == 0) {
	warn("Xpilotrc_write: Zero length filename.");
	return -1;
    }

    fp = fopen(path, "w");
    if (fp == NULL) {
	error("Xpilotrc_write: Failed to open file \"%s\"", path);
	return -2;
    }

    /* make sure all options are in the xpilotrc */
    for (i = 0; i < num_options; i++) {
	xp_option_t *opt = Option_by_index(i);
	xp_option_origin_t origin;
	const char *os;

	/* Let's not save these */
	if (Option_get_type(opt) == xp_noarg_option)
	    continue;

	origin = Option_get_origin(opt);
#if 0
	switch (origin) {
	case xp_option_origin_default:  os = "default";  break;
	case xp_option_origin_cmdline:  os = "cmdline";  break;
	case xp_option_origin_env:      os = "env";      break;
	case xp_option_origin_xpilotrc: os = "xpilotrc"; break;
	case xp_option_origin_config:   os = "config";   break;
	case xp_option_origin_setcmd:   os = "setcmd";   break;
	default:                        os = "unknown";  break;
	}
	warn("option %s origin is %s.", Option_get_name(opt), os);
#endif

	if (origin == xp_option_origin_xpilotrc)
	    /* exists already in the xpilotrc data struct */
	    continue;

	
    }

    for (i = 0; i < num_xpilotrc_lines; i++) {
	xpilotrc_line_t *lp = &xpilotrc_lines[i];

	Xpilotrc_write_line(fp, lp);
    }

    fclose(fp);

#if 0
    int			i;
    FILE		*fp = NULL;
    char		buf[512];

    char		oldfile[PATH_MAX + 1],
			newfile[PATH_MAX + 1];

    if ((fp = fopen(oldfile, "r")) != NULL) {
	while (fgets(buf, sizeof buf, fp))
	    Xpilotrc_add(buf);
	fclose(fp);
    }
    sprintf(newfile, "%s.new", oldfile);
    unlink(newfile);
    if ((fp = fopen(newfile, "w")) == NULL) {
	Config_save_failed("Can't open file to save to.", strptr);
	return 1;
    }

    Config_save_comment(fp,
			";\n"
			"; Keys\n"
			";\n"
			"; The X Window System program xev can be used to\n"
			"; find out the names of keyboard keys.\n"
			";\n");
    Config_save_keys(fp);

#ifndef _WINDOWS
    Xpilotrc_end(fp);
    fclose(fp);
    sprintf(newfile, "%s.bak", oldfile);
    rename(oldfile, newfile);
    unlink(oldfile);
    sprintf(newfile, "%s.new", oldfile);
    rename(newfile, oldfile);
#endif
#endif


    return 0;
}
 

void Parse_options(int *argcp, char **argvp)
{
    int arg_ind, num_remaining_args, num_servers = 0, i;
    char path[PATH_MAX + 1];

    Xpilotrc_get_filename(path, sizeof(path));
    Xpilotrc_read(path);

    /*
     * Here we step trough argc - 1 arguments, leaving
     * only the arguments that might be server names.
     */
    arg_ind = 1;
    num_remaining_args = *argcp - 1;

    while (num_remaining_args > 0) {
	if (argvp[arg_ind][0] == '-') {
	    char *arg = &argvp[arg_ind][1];

	    /*
	     * kps -
	     * Incomplete GNU style option support, this only works for
	     * options with no argument, e.g. --version
	     * A complete implementation should also support option given
	     * like this:
	     * --option=value
	     */
	    if (arg[0] == '-')
		arg++;

	    if (is_noarg_option(arg)) {
		Set_option(arg, "true", xp_option_origin_cmdline);
		num_remaining_args--;
		for (i = 0; i < num_remaining_args; i++)
		    argvp[arg_ind + i] = argvp[arg_ind + i + 1];
	    } else {
		bool ok = false;

		if (num_remaining_args >= 2) {
		    ok = Set_option(arg, argvp[arg_ind + 1],
				    xp_option_origin_cmdline);
		    if (ok) {
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
    for (i = num_servers + 1; i < *argcp; i++)
	argvp[i] = NULL;
    *argcp = num_servers + 1;

    if (xpArgs.help)
	Usage();

    if (xpArgs.version)
	Version();

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

#ifndef _WINDOWS
void Xpilotrc_get_filename(char *path, size_t size)
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
void Xpilotrc_get_filename(char *path, size_t size)
{
    strlcpy(path, "xpilotrc.txt", size);
}
#endif /* _WINDOWS */





#if 0

static int Get_string_resource(XrmDatabase db,
			       const char *resource, char *result,
			       unsigned size)
{
    char		*src, *dst;
    int			ind, val;

    val = Find_resource(db, resource, result, size, &ind);
    src = dst = result;
    while ((*src & 0x7f) == *src && !isgraph(*src) && *src != '\0')
	src++;

    while ((*src & 0x7f) != *src || isgraph(*src))
	*dst++ = *src++;

    *dst = '\0';

    return val;
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

