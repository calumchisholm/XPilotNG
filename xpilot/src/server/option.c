/* $Id$
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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

#ifdef	_WINDOWS
#include <windows.h>
#else
#include "types.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "defaults.h"
#include "error.h"
#include "click.h"

#ifdef	_WINDOWS
#include <io.h>
#define	read(__a, __b, __c)	_read(__a, __b, __c)
#endif


char option_version[] = VERSION;

#ifndef PATH_MAX
#define PATH_MAX	1023
#endif

#define	NHASH	199

valPair    *hashArray[NHASH];


/*
 * Compute a reasonable case-insensitive hash value across a character string.
 */
static unsigned int hash(const char *name)
{
    unsigned int hashVal = 0;
    unsigned char *s = (unsigned char *)name;

    while (*s) {
	char        c = *s++;

	if (isascii(c) && isalpha(c) && islower(c))
	    c = toupper(c);
	hashVal = (hashVal + c) << 1;
	while (hashVal > NHASH)
	    hashVal = (hashVal % NHASH) + (hashVal / NHASH);
    }
    return hashVal % NHASH;
}


/*
 * Allocate a new bucket for the hash table and fill in its values.
 */
static valPair *newOption(const char *name, const char *value)
{
    valPair    *tmp = (valPair *)malloc(sizeof(valPair));

    if (!tmp)
	return (valPair *) 0;

    tmp->name = (char *)malloc(strlen(name) + 1);
    tmp->value = (char *)malloc(strlen(value) + 1);

    if (!tmp->name || !tmp->value) {
	if (tmp->name)
	    free(tmp->name);
	if (tmp->value)
	    free(tmp->value);
	free(tmp);
	return (valPair *) 0;
    }
    strcpy(tmp->name, name);
    strcpy(tmp->value, value);
    return tmp;
}


/*
 * Scan through the hash table of option name-value pairs looking for an option
 * with the specified name; if found, and if override is true, change to the
 * new value; if found and override is not true, do nothing. If not found, add
 * to the hash table regardless of override.   Either way, if def is nonzero,
 * it is attached to the name-value pair - this will only happen once anyway.
 */
void addOption(const char *name, const char *value, int override, void *def)
{
    valPair    *tmp;
    int         ix = hash(name);

    for (tmp = hashArray[ix]; tmp; tmp = tmp->next)
	if (!strcasecmp(name, tmp->name)) {
	    if (override && value) {
		char       *s = (char *)malloc(strlen(value) + 1);

		if (!s)
		    return;
		free(tmp->value);
		strcpy(s, value);
		tmp->value = s;
	    }
	    if (def)
		tmp->def = def;
	    return;
	}
    if (!value)
	return;

    tmp = newOption(name, value);
    if (!tmp)
	return;
    tmp->def = def;
    tmp->next = hashArray[ix];
    hashArray[ix] = tmp;
}


/*
 * Return the value of the specified option, or (char *)0 if there is no value
 * for that option.
 */
char *getOption(const char *name)
{
    valPair    *tmp;
    int         ix = hash(name);

    for (tmp = hashArray[ix]; tmp; tmp = tmp->next)
	if (!strcasecmp(name, tmp->name))
	    return tmp->value;

    return (char *)0;
}


static char	*FileName;

#include <expat.h>
#define NOTEAM_BIT (1 << 10) /* !@# use same defines with walls.c */
#define BALL_BIT (1 << 11)
#define NONBALL_BIT (1 << 12)

static int edg[5000 * 2]; /* !@# change pointers in poly_t when realloc poss.*/
extern int polyc;
extern int num_groups;
extern struct {int type; unsigned int hit_mask; int team;} groups[];

int *edges = edg;
int *estyleptr;
int ptscount, ecount;
char *mapd;

struct polystyle pstyles[256];
struct edgestyle estyles[256] =
{{"internal", 0, 0, 0}};	/* Style 0 is always this special style */
struct bmpstyle  bstyles[256];
poly_t *pdata;

int num_pstyles, num_bstyles, num_estyles = 1; /* "Internal" edgestyle */
static int max_bases, max_balls, max_fuels, max_checks, max_polys,max_echanges;
static int current_estyle, current_group, is_decor;

static int get_bmp_id(const char *s)
{
    int i;

    for (i = 0; i < num_bstyles; i++)
	if (!strcmp(bstyles[i].id, s))
	    return i;
    errno = 0;
    error("Undeclared bmpstyle %s", s);
    return 0;
}


static int get_edge_id(const char *s)
{
    int i;

    for (i = 0; i < num_estyles; i++)
	if (!strcmp(estyles[i].id, s))
	    return i;
    errno = 0;
    error("Undeclared edgestyle %s", s);
    return -1;
}


static int get_poly_id(const char *s)
{
    int i;

    for (i = 0; i < num_pstyles; i++)
	if (!strcmp(pstyles[i].id, s))
	    return i;
    errno = 0;
    error("Undeclared polystyle %s", s);
    return 0;
}


#define STORE(T,P,N,M,V)						\
    if (N >= M && ((M <= 0)						\
	? (P = (T *) malloc((M = 1) * sizeof(*P)))			\
	: (P = (T *) realloc(P, (M += M) * sizeof(*P)))) == NULL) {	\
	error("No memory");						\
	exit(1);							\
    } else								\
	(P[N++] = V)
/* !@# add a final realloc later to free wasted memory */


static void tagstart(void *data, const char *el, const char **attr)
{
    static double scale = 1;

    if (!strcasecmp(el, "Polystyle")) {
	pstyles[num_pstyles].id[sizeof(pstyles[0].id) - 1] = 0;
	pstyles[num_pstyles].color = 0;
	pstyles[num_pstyles].texture_id = 0;
	pstyles[num_pstyles].defedge_id = 0;
	pstyles[num_pstyles].flags = 0;

	while (*attr) {
	    if (!strcasecmp(*attr, "id"))
		strncpy(pstyles[num_pstyles].id, *(attr + 1),
			sizeof(pstyles[0].id) - 1);
	    if (!strcasecmp(*attr, "color"))
		pstyles[num_pstyles].color = strtol(*(attr + 1), NULL, 16);
	    if (!strcasecmp(*attr, "texture"))
		pstyles[num_pstyles].texture_id = get_bmp_id(*(attr + 1));
	    if (!strcasecmp(*attr, "defedge"))
		pstyles[num_pstyles].defedge_id = get_edge_id(*(attr + 1));
	    if (!strcasecmp(*attr, "flags"))
		pstyles[num_pstyles].flags = atoi(*(attr + 1)); /* names @!# */
	    attr += 2;
	}
	if (pstyles[num_pstyles].defedge_id == 0) {
	    errno = 0;
	    error("Polygon default edgestyle cannot be omitted or set "
		  "to 'internal'!");
	    exit(1);
	}
	num_pstyles++;
    }

    if (!strcasecmp(el, "Edgestyle")) {
	estyles[num_estyles].id[sizeof(estyles[0].id) - 1] = 0;
	estyles[num_estyles].width = 0;
	estyles[num_estyles].color = 0;
	estyles[num_estyles].style = 0;
	while (*attr) {
	    if (!strcasecmp(*attr, "id"))
		strncpy(estyles[num_estyles].id, *(attr + 1),
			sizeof(estyles[0].id) - 1);
	    if (!strcasecmp(*attr, "width"))
		estyles[num_estyles].width = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "color"))
		estyles[num_estyles].color = strtol(*(attr + 1), NULL, 16);
	    if (!strcasecmp(*attr, "style")) /* !@# names later */
		estyles[num_estyles].style = atoi(*(attr + 1));
	    attr += 2;
	}
	num_estyles++;
    }

    if (!strcasecmp(el, "Bmpstyle")) {
	bstyles[num_bstyles].flags = 0;
	bstyles[num_bstyles].filename[sizeof(bstyles[0].filename) - 1] = 0;
	bstyles[num_bstyles].id[sizeof(bstyles[0].id) - 1] = 0;
/* add checks that these are filled !@# */

	while (*attr) {
	    if (!strcasecmp(*attr, "id"))
		strncpy(bstyles[num_bstyles].id, *(attr + 1),
			sizeof(bstyles[0].id) - 1);
	    if (!strcasecmp(*attr, "filename"))
		strncpy(bstyles[num_bstyles].filename, *(attr + 1),
			sizeof(bstyles[0].filename) - 1);
	    if (!strcasecmp(*attr, "scalable"))
		if (!strcasecmp(*(attr + 1), "yes"))
		    bstyles[num_bstyles].flags |= 1;
	    attr += 2;
	}
	num_bstyles++;
    }

    if (!strcasecmp(el, "Scale")) {
	if (!*attr || strcasecmp(*attr, "value"))
	    error("Invalid Scale");
	else
	    scale = atof(*(attr + 1));
    }

    if (!strcasecmp(el, "BallArea")) {
	int team;
	current_group = ++num_groups;
	groups[current_group].type = TREASURE;
	groups[current_group].team = team;
	groups[current_group].hit_mask = BALL_BIT;
    }

    if (!strcasecmp(el, "BallTarget")) {
	int team;
	while (*attr) {
	    if (!strcasecmp(*attr, "team"))
		team = atoi(*(attr + 1));
	    attr += 2;
	}
	current_group = ++num_groups;
	groups[current_group].type = TREASURE;
	groups[current_group].team = team;
	groups[current_group].hit_mask = NONBALL_BIT | (((NOTEAM_BIT << 1) - 1) & ~(1 << team));
    }

    if (!strcasecmp(el, "Decor"))
	is_decor = 1;

    if (!strcasecmp(el, "Polygon")) {
	int x, y, style = -1;
	poly_t t;

	while (*attr) {
	    if (!strcasecmp(*attr, "x"))
		x = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "y"))
		y = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "style"))
		style = get_poly_id(*(attr + 1));
	    attr += 2;
	}
	if (style == -1) {
	    errno = 0;
	    error("Currently you must give polygon style, no default");
	    exit(1);
	}
	ptscount = 0;
	t.x = x;
	t.y = y;
	t.group = current_group;
	t.edges = edges;
	t.style = style;
	t.estyles_start = ecount;
	t.is_decor = is_decor;
	current_estyle = pstyles[style].defedge_id;
	STORE(poly_t, pdata, polyc, max_polys, t);
	return;
    }

    if (!strcasecmp(el, "Check")) {
	ipos t;
	int x, y;

	while (*attr) {
	    if (!strcasecmp(*attr, "x"))
		x = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "y"))
		y = atoi(*(attr + 1)) * scale;
	    attr += 2;
	}
	t.x = x;
	t.y = y;
	STORE(ipos, World.check, World.NumChecks, max_checks, t);
    }

    if (!strcasecmp(el, "Fuel")) {
	fuel_t t;
	int team, x, y;

	team = TEAM_NOT_SET;
	while (*attr) {
	    if (!strcasecmp(*attr, "team"))
		team = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "x"))
		x = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "y"))
		y = atoi(*(attr + 1)) * scale;
	    attr += 2;
	}
	t.clk_pos.x = x;
	t.clk_pos.y = y;
	t.fuel = START_STATION_FUEL;
	t.conn_mask = (unsigned)-1;
	t.last_change = frame_loops;
	t.team = team;
	STORE(fuel_t, World.fuel, World.NumFuels, max_fuels, t);
    }

    if (!strcasecmp(el, "Base")) {
	base_t	t;
	int	team, x, y, dir;

	while (*attr) {
	    if (!strcasecmp(*attr, "team"))
		team = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "x"))
		x = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "y"))
		y = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "dir"))
		dir = atoi(*(attr + 1));
	    attr += 2;
	}
	if (team < 0 || team >= MAX_TEAMS) {
	    error("Illegal team number in base tag.\n");
	    exit(1);
	}

	t.pos.x = x;
	t.pos.y = y;
	t.dir = dir;
	if (BIT(World.rules->mode, TEAM_PLAY)) {
	    t.team = team;
	    World.teams[team].NumBases++;
	    if (World.teams[team].NumBases == 1)
		World.NumTeamBases++;
	} else {
	    t.team = TEAM_NOT_SET;
	}
	STORE(base_t, World.base, World.NumBases, max_bases, t);
    }

    if (!strcasecmp(el, "Ball")) {
    	treasure_t t;
	int team, x, y;

	while (*attr) {
	    if (!strcasecmp(*attr, "team"))
		team = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "x"))
		x = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "y"))
		y = atoi(*(attr + 1)) * scale;
	    attr += 2;
	}
	t.pos.x = x;
	t.pos.y = y;
	t.have = false;
	t.destroyed = 0;
	t.team = team;
	World.teams[team].NumTreasures++;
	World.teams[team].TreasuresLeft++;
	STORE(treasure_t, World.treasures, World.NumTreasures, max_balls, t);
    }

    if (!strcasecmp(el, "Option")) {
	const char *name, *value;
	while (*attr) {
	    if (!strcasecmp(*attr, "name"))
		name = *(attr + 1);
	    if (!strcasecmp(*attr, "value"))
		value = *(attr + 1);
	    attr += 2;
	}
	addOption(name, value, 0, NULL);
	return;
    }

    if (!strcmp(el, "Offset")) {
	int x, y, edgestyle = -1;
	while (*attr) {
	    if (!strcasecmp(*attr, "x"))
		x = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "y"))
		y = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "style"))
		edgestyle = get_edge_id(*(attr + 1));
	    attr += 2;
	}
	*edges++ = x;
	*edges++ = y;
	if (edgestyle != -1 && edgestyle != current_estyle) {
	    STORE(int, estyleptr, ecount, max_echanges, ptscount);
	    STORE(int, estyleptr, ecount, max_echanges, edgestyle);
	    current_estyle = edgestyle;
	}
	ptscount++;
	return;
    }
    return;
}


static void tagend(void *data, const char *el)
{
    void cmdhack(void);
    if (!strcasecmp(el, "Decor"))
	is_decor = 0;
    if (!strcasecmp(el, "BallArea") || !strcasecmp(el, "BallTarget"))
	current_group = 0;
    if (!strcasecmp(el, "Polygon")) {
	pdata[polyc - 1].num_points = ptscount;
	pdata[polyc - 1].num_echanges = ecount -pdata[polyc - 1].estyles_start;
	STORE(int, estyleptr, ecount, max_echanges, INT_MAX);
    }
    if (!strcasecmp(el, "GeneralOptions")) {
	cmdhack(); /* !@# */
	parseOptions();
	Grok_map();
    }
    return;
}


int Load_lines(int fd)
{
    char buff[8192];
    int len;
    XML_Parser p = XML_ParserCreate(NULL);

    if (!p) {
	error("Creating Expat instance for map parsing failed.\n");
	exit(1);
    }
    XML_SetElementHandler(p, tagstart, tagend);
    do {
	len = read(fd, buff, 8192);
	if (len < 0) {
	    error("Error reading map!");
	    return false;
	}
	if (!XML_Parse(p, buff, len, !len)) {
	    errno = 0;
	    error("Parse error reading map at line %d:\n%s\n",
		  XML_GetCurrentLineNumber(p),
		  XML_ErrorString(XML_GetErrorCode(p)));
	    exit(1);
	}
    } while (len);
    return 1;
}


/*
 * Parse a file containing defaults (and possibly a map).
 */
static bool parseOpenFile(FILE *ifile)
{
    int		fd;

    fd = fileno(ifile);

    return Load_lines(fd);
}

static int copyFilename(const char *file)
{
    if (FileName) {
	free(FileName);
    }
    FileName = strdup(file);
    return (FileName != 0);
}


static FILE *fileOpen(const char *file)
{
    FILE *fp = fopen(file, "r");
    if (fp ) {
	if (!copyFilename(file)) {
	    fclose(fp);
	    fp = NULL;
	}
    }
    return fp;
}


static void fileClose(FILE *fp)
{
    fclose(fp);
    if (FileName) {
	free(FileName);
	FileName = NULL;
    }
}


/*
 * Test if filename has the XPilot map extension.
 */
static int hasMapExtension(const char *filename)
{
    int fnlen = strlen(filename);
    if (fnlen > 3 && !strcmp(&filename[fnlen - 4], ".xp2")) {
	return 1;
    }
    if (fnlen > 4 && !strcmp(&filename[fnlen - 4], ".map")) {
	return 1;
    }
    return 0;
}


/*
 * Test if filename has a directory component.
 */
static int hasDirectoryPrefix(const char *filename)
{
    static const char	sep = '/';
    return (strchr(filename, sep) != NULL);
}


/*
 * Combine a directory and a file.
 * Returns new path as dynamically allocated memory.
 */
static char *fileJoin(const char *dir, const char *file)
{
    static const char	sep = '/';
    char		*path;

    path = (char *) malloc(strlen(dir) + 1 + strlen(file) + 1);
    if (path) {
	sprintf(path, "%s%c%s", dir, sep, file);
    }
    return path;
}


/*
 * Combine a file and a filename extension.
 * Returns new path as dynamically allocated memory.
 */
static char *fileAddExtension(const char *file, const char *ext)
{
    char		*path;

    path = (char *) malloc(strlen(file) + strlen(ext) + 1);
    if (path) {
	sprintf(path, "%s%s", file, ext);
    }
    return path;
}


#if defined(COMPRESSED_MAPS)
static int	usePclose;


static int isCompressed(const char *filename)
{
    int fnlen = strlen(filename);
    int celen = strlen(Conf_zcat_ext());
    if (fnlen > celen && !strcmp(&filename[fnlen - celen], Conf_zcat_ext())) {
	return 1;
    }
    return 0;
}


static void closeCompressedFile(FILE *fp)
{
    if (usePclose) {
	pclose(fp);
	usePclose = 0;
	if (FileName) {
	    free(FileName);
	    FileName = NULL;
	}
    } else {
	fileClose(fp);
    }
}


static FILE *openCompressedFile(const char *filename)
{
    FILE		*fp = NULL;
    char		*cmdline = NULL;
    char		*newname = NULL;

    usePclose = 0;
    if (!isCompressed(filename)) {
	if (access(filename, 4) == 0) {
	    return fileOpen(filename);
	}
	newname = fileAddExtension(filename, Conf_zcat_ext());
	if (!newname) {
	    return NULL;
	}
	filename = newname;
    }
    if (access(filename, 4) == 0) {
	cmdline = (char *) malloc(strlen(Conf_zcat_format()) + strlen(filename) + 1);
	if (cmdline) {
	    sprintf(cmdline, Conf_zcat_format(), filename);
	    fp = popen(cmdline, "r");
	    if (fp) {
		usePclose = 1;
		if (!copyFilename(filename)) {
		    closeCompressedFile(fp);
		    fp = NULL;
		}
	    }
	}
    }
    if (newname) free(newname);
    if (cmdline) free(cmdline);
    return fp;
}

#else

static int isCompressed(const char *filename)
{
    return 0;
}

static void closeCompressedFile(FILE *fp)
{
    fileClose(fp);
}

static FILE *openCompressedFile(const char *filename)
{
    return fileOpen(filename);
}
#endif

/*
 * Open a map file.
 * Filename argument need not contain map filename extension
 * or compress filename extension.
 * The search order should be:
 *      filename
 *      filename.gz              if COMPRESSED_MAPS is true
 *      filename.xp2
 *      filename.xp2.gz          if COMPRESSED_MAPS is true
 *      filename.map
 *      filename.map.gz          if COMPRESSED_MAPS is true
 *      MAPDIR filename
 *      MAPDIR filename.gz       if COMPRESSED_MAPS is true
 *      MAPDIR filename.xp2
 *      MAPDIR filename.xp2.gz   if COMPRESSED_MAPS is true
 *      MAPDIR filename.map
 *      MAPDIR filename.map.gz   if COMPRESSED_MAPS is true
 */
static FILE *openMapFile(const char *filename)
{
    FILE		*fp = NULL;
    char		*newname;
    char		*newpath;

    fp = openCompressedFile(filename);
    if (fp) {
	return fp;
    }
    if (!isCompressed(filename)) {
	if (!hasMapExtension(filename)) {
	    newname = fileAddExtension(filename, ".xp2");
	    fp = openCompressedFile(newname);
	    free(newname);
	    if (fp) {
		return fp;
	    }
	    newname = fileAddExtension(filename, ".map");
	    fp = openCompressedFile(newname);
	    free(newname);
	    if (fp) {
		return fp;
	    }
	}
    }
    if (!hasDirectoryPrefix(filename)) {
	newpath = fileJoin(Conf_mapdir(), filename);
	if (!newpath) {
	    return NULL;
	}
	if (hasDirectoryPrefix(newpath)) {
	    /* call recursively. */
	    fp = openMapFile(newpath);
	}
	free(newpath);
	if (fp) {
	    return fp;
	}
    }
    return NULL;
}


static void closeMapFile(FILE *fp)
{
    closeCompressedFile(fp);
}


static FILE *openDefaultsFile(const char *filename)
{
    return fileOpen(filename);
}


static void closeDefaultsFile(FILE *fp)
{
    fileClose(fp);
}


/*
 * Parse a file containing defaults.
 */
bool parseDefaultsFile(const char *filename)
{
    FILE       *ifile;
    bool	result;

    if ((ifile = openDefaultsFile(filename)) == NULL) {
	return false;
    }
    result = parseOpenFile(ifile);
    closeDefaultsFile(ifile);

    return true;
}


/*
 * Parse a file containing a map.
 */
bool parseMapFile(const char *filename)
{
    FILE       *ifile;
    bool	result;

    if ((ifile = openMapFile(filename)) == NULL) {
	return false;
    }
    result = parseOpenFile(ifile);
    closeMapFile(ifile);

    return true;
}


/*
 * Go through the hash table looking for name-value pairs that have defaults
 * assigned to them.   Process the defaults and, if possible, set the
 * associated variables.
 */
void parseOptions(void)
{
    int         i;
    valPair    *tmp, *next;
    char       *fpsstr;
    optionDesc *desc;

    /*
     * This must be done in order that FPS will return the eventual
     * frames per second for computing valSec and valPerSec.
     */
    if ((fpsstr = getOption("framesPerSecond")) != NULL)
	framesPerSecond = atoi(fpsstr);
    if (FPS <= 0) {
	errno = 0;
	error("Can't run with %d frames per second, should be positive\n",
	    FPS);
	End_game();
    }

    for (i = 0; i < NHASH; i++)
	for (tmp = hashArray[i]; tmp; tmp = tmp->next) {
	    /* Does it have a default?   (If so, get a pointer to it) */
	    if ((desc = (optionDesc *)tmp->def) != NULL) {
		if (desc->variable) {
		    switch (desc->type) {

		    case valVoid:
			break;

		    case valInt:
			{
			    int        *ptr = (int *)desc->variable;

			    *ptr = atoi(tmp->value);
			    break;
			}

		    case valReal:
			{
			    DFLOAT     *ptr = (DFLOAT *)desc->variable;

			    *ptr = atof(tmp->value);
			    break;
			}

		    case valBool:
			{
			    bool	*ptr = (bool *)desc->variable;

			    if (!strcasecmp(tmp->value, "yes")
				|| !strcasecmp(tmp->value, "on")
				|| !strcasecmp(tmp->value, "true"))
				*ptr = true;
			    else if (!strcasecmp(tmp->value, "no")
				     || !strcasecmp(tmp->value, "off")
				     || !strcasecmp(tmp->value, "false"))
				*ptr = false;
			    else {
				error("Invalid boolean value for %s - %s\n",
				      desc->name, tmp->value);
			    }
			    break;
			}

		    case valIPos:
			{
			    ipos       *ptr = (ipos *)desc->variable;
			    char       *s;

			    s = strchr(tmp->value, ',');
			    if (!s) {
				error("Invalid coordinate pair for %s - %s\n",
				      desc->name, tmp->value);
				break;
			    }
			    ptr->x = atoi(tmp->value);
			    ptr->y = atoi(++s);
			    break;
			}

		    case valString:
			{
			    char      **ptr = (char **)desc->variable;

			    *ptr = tmp->value;
			    break;
			}

		    case valSec:
			{
			    int		*ptr = (int *)desc->variable;

			    *ptr = (int)(atof(tmp->value) * FPS);
			    break;
			}

		    case valPerSec:
			{
			    DFLOAT	*ptr = (DFLOAT *)desc->variable;

			    *ptr = (DFLOAT)(atof(tmp->value) / FPS);
			    break;
			}
		    }
		}
	    }
	}

    for (i = 0; i < NHASH; i++) {
	for (tmp = hashArray[i]; tmp; tmp = next) {
	    free(tmp->name);
	    /* free(tmp->value); */
	    next = tmp->next;
	    memset((void *)tmp, 0, sizeof(*tmp));
	    free(tmp);
	}
    }
}


#ifdef	_WINDOWS
/* clear the hashArray in case we're restarted */
void	FreeOptions()
{
	int		i;
    /* valPair    *tmp, *next; */

	for (i=0; i<NHASH; i++) {
	}

}
#endif
