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


#if 0
bool power_setfunc(const char *name, const char *value, void *private)
{
    double dval;

    assert(value);
    dval = atof(value);
    if (dval < MIN_PLAYER_POWER || dval > MAX_PLAYER_POWER)
	return false;
    power = dval;
    warn("power is now %f\n", power);
    return true;
}
#endif

int num_options = 0;
int max_options = 0;

#if 0
typedef struct {
    const char		*name;		/* option name */
    const char		*noArg;		/* value for non-argument options */
    const char		*fallback;	/* default value */
    keys_t		key;		/* key if not KEY_DUMMY */
    const char		*help;		/* user help (multiline) */
    unsigned		hash;		/* option name hashed. */
    cl_option_setfunc_t	setfunc;
    void		*private;	/* passed to set function */
} cl_option_t;
#endif

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


/*
 * NOTE: Store option assumes the passed pointers will remain valid.
 */
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

/*
 * This could also be used from a client '\set' command, e.g.
 * "\set scalefactor 1.5"
 */

void Set_option(const char *name, const char *value)
{
#if 0
    xp_option_t *opt;
    bool set_ok;

    opt = Find_option(name);

    if (!opt) {
	/*Store_option(name, value);
	  opt = Find_option(name);*/
	return false;
    }

    set_ok = opt->setfunc(opt->name, value, opt->private);

    return set_ok;
#endif
}


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

#if 0
    /* kps - do before Parse_options */
    {
	int i;

	for (i = 0; i < NELEM(default_options); i++)
	    Store_option_struct(&default_options[i]);
    }
#endif


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

void Usage(void)
{

}

#endif

