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


static inline xp_option_t *Option_by_index(int ind)
{
    if (ind < 0 || ind >= num_options)
	return NULL;
    return &options[ind];
}

static inline xp_option_t *Find_option(const char *name)
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
	break;
    case xp_color_option:
	assert(0 && "TODO");
	break;
    case xp_key_option:
	if (opt->key_defval && strlen(opt->key_defval) > 0)
	    printf("        The default %s: %s.\n", 
		   (strchr(opt->key_defval, ' ') == NULL
		    ? "key is"
		    : "keys are"),
		   opt->key_defval);
	break;
    default:
	assert(0 && "TODO");
	break;
    }
}

void Usage(void)
{
    int			i;

    printf(
"Usage: xpilot [-options ...] [server]\n"
"Where options include:\n"
"\n"
	  );
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
#if 0
	if (opt->fallback && opt->fallback[0]) {
	    printf("        The default %s: %s.\n",
		   (opt->key == KEY_DUMMY)
		       ? "value is"
		       : (strchr(opt->fallback, ' ') == NULL)
			   ? "key is"
			   : "keys are",
		   opt->fallback);
	}
#endif

	printf("\n");
    }
    printf(
"Most of these options can also be set in the .xpilotrc file\n"
"in your home directory.\n"
"Each key option may have multiple keys bound to it and\n"
"one key may be used by multiple key options.\n"
"If no server is specified then xpilot will search\n"
"for servers on your local network.\n"
"For a listing of remote servers try: telnet meta.xpilot.org 4400 \n"
	  );

    exit(1);
}


static void Set_bool_option(xp_option_t *opt, bool value)
{
    assert(opt);
    assert(opt->type == xp_bool_option);

    if (opt->bool_setfunc)
	opt->bool_setfunc(opt, value);
    else
	*opt->bool_ptr = value;

    printf("Value of option %s is now %s\n", opt->name,
	   *opt->bool_ptr ? "true" : "false");
}

static void Set_int_option(xp_option_t *opt, int value)
{
    assert(opt);
    assert(opt->type == xp_int_option);

    LIMIT(value, opt->int_minval, opt->int_maxval);

    if (opt->int_setfunc)
	opt->int_setfunc(opt, value);
    else
	*opt->int_ptr = value;

    printf("Value of option %s is now %d\n", opt->name, *opt->int_ptr);
}

static void Set_double_option(xp_option_t *opt, double value)
{
    assert(opt);
    assert(opt->type == xp_double_option);

    LIMIT(value, opt->dbl_minval, opt->dbl_maxval);

    if (opt->dbl_setfunc)
	opt->dbl_setfunc(opt, value);
    else
	*opt->dbl_ptr = value;

    printf("Value of option %s is now %f\n", opt->name, *opt->dbl_ptr);
}

static void Set_string_option(xp_option_t *opt, const char *value)
{
    assert(opt);
    assert(opt->type == xp_string_option);

    if (opt->str_setfunc)
	opt->str_setfunc(opt, value);
    else
	strlcpy(opt->str_ptr, value, opt->str_size);

    printf("Value of option %s is now \"%s\"\n", opt->name, *opt->str_ptr);
}

/*
 * This could also be used from a client '\set' command, e.g.
 * "\set scalefactor 1.5"
 */
void Set_option(const char *name, const char *value)
{
    xp_option_t *opt;

    opt = Find_option(name);

    if (!opt) {
	warn("Could not find option \"%s\"\n", name);
	return;
    }

    switch (opt->type) {
    case xp_bool_option:
	Set_bool_option(opt, ON(value) ? true : false);
	break;
    case xp_int_option:
	Set_int_option(opt, atoi(value));
	break;
    case xp_double_option:
	Set_double_option(opt, atof(value));
	break;
    default:
	warn("FOO");
    }

#if 0

    bool set_ok;



    if (!opt) {
	/*Store_option(name, value);
	  opt = Find_option(name);*/
	return false;
    }

    set_ok = opt->setfunc(opt->name, value, opt->private);

    return set_ok;
#endif
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

    /* Let's not allow several options with the same name */
    assert(Find_option(opt->name) == NULL);

    memcpy(&option, opt, sizeof(xp_option_t));

    STORE(xp_option_t, options, num_options, max_options, option);

    opt = Find_option(opt->name);
    assert(opt);

    /* Set the default value */
    switch (opt->type) {
    case xp_bool_option:
	Set_bool_option(opt, opt->bool_defval);
	break;
    case xp_int_option:
	Set_int_option(opt, opt->int_defval);
	break;
    case xp_double_option:
	Set_double_option(opt, opt->dbl_defval);
	break;
    default:
	warn("FOO");
    }

}



#if 0
static void Store_option(const char *name,
			 const char *value_to_set,
			 const char *help,
			 xp_option_setfunc_t setfunc,
			 void *private)
     
{
    xp_option_t option;

    assert(name);
    assert(strlen(name) > 0);

    /* Let's not allow several options with the same name */
    assert(Find_option(name) == NULL);

    option.name = name;
    option.help = help;
    option.setfunc = setfunc;
    option.private = private;

    STORE(xp_option_t, options, num_options, max_options, option);

    /*
     * If no setfunc, value can't be set.
     */
    if (option.setfunc) {
	bool set_ok;
	set_ok = option.setfunc(name, value_to_set, private);
	/* Setting the default value must succeed */
	if (!set_ok) {
	    warn("Setting default value for option %s failed.", name);
	    assert(0 && "Setting option default value must succeed.");
	}
    }
}

static void Store_option_struct(xp_option_t *opt)
{
    Store_option(opt->name,
		 opt->fallback,
		 opt->help,
		 opt->setfunc,
		 opt->private);
}
#endif



static void Parse_xpilotrc_line(const char *line)
{
    char *s;

    /*printf("parsing xpilotrc line \"%s\"\n", line);*/
    /*
     * Ignore lines that don't start with xpilot. or
     * xpilot*
     */
    if (!(strncasecmp(line, "xpilot.", 7) == 0
	  || strncasecmp(line, "xpilot*", 7) == 0))
	/* not interested */
	return;

    /*printf("-> line is now \"%s\"\n", line);*/
    line += 7;
    /*printf("-> line is now \"%s\"\n", line);*/
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


void Parse_options(int *argcp, char **argvp, char *realName, int *port,
		   int *my_team, bool *text, bool *list,
		   bool *join, bool *noLocalMotd,
		   char *nickName, char *dispName, char *hostName,
		   char *shut_msg)
{
    char path[PATH_MAX + 1];
    char buf[BUFSIZ];
    FILE *fp;

    /*
     * Create data structure holding all options we know of and their values.
     */
    /*warn("numoptions: %d", NELEM(options));*/

    /*
     * Read options from xpilotrc.
     */
    Get_xpilotrc_file(path, sizeof(path));
    warn("Using xpilotrc file %s\n", path);
    if (strlen(path) > 0
	&& ((fp = fopen(path, "r")) != NULL)) {
	while (fgets(buf, sizeof buf, fp)) {
	    char *cp;
	    /* kps - remove NL and CR, does this work in windows ? */
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



    
}



char *Get_keyHelpString(keys_t key)
{
    int			i;
    char		*nl;
    static char		buf[MAX_CHARS];

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
    int			i;

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
    const char		*home = getenv("HOME");
    const char		*defaultFile = ".xpilotrc";
    const char		*optionalFile = getenv("XPILOTRC");

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

