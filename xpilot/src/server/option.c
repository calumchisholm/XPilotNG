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

extern int polygons[];
extern int polyc;
extern int groupc;
extern struct {int type; unsigned int hit_mask; int team;} groups[];

int *poly = polygons;
int *polypts;
int *hidptr;
int ptscount;
char *mapd;

struct polystyle pstyles[256];
struct edgestyle estyles[256];
struct bmpstyle  bstyles[256];
struct polydata  pdata[1000];

int num_pstyles, num_estyles, num_bstyles;


static int get_bmp_id(const char *s)
{
    int i;

    for (i = 0; i < num_bstyles; i++)
	if (!strcmp(bstyles[i].id, s))
	    return i;
    error("Undeclared bmpstyle %s", s);
    return 0;
}


static int get_edge_id(const char *s)
{
    int i;

    for (i = 0; i < num_estyles; i++)
	if (!strcmp(estyles[i].id, s))
	    return i;
    error("Undeclared edgestyle %s", s);
    return 0;
}


static int get_poly_id(const char *s)
{
    int i;

    for (i = 0; i < num_pstyles; i++)
	if (!strcmp(estyles[i].id, s))
	    return i;
    error("Undeclared polystyle %s", s);
    return 0;
}


static void tagstart(void *data, const char *el, const char **attr)
{
    static double scale = 1;

    if (!strcasecmp(el, "polystyle")) {
	pstyles[num_pstyles].id[sizeof(pstyles[0].id) - 1] = 0;
	pstyles[num_pstyles].color = 0;
	pstyles[num_pstyles].texture_id = 0;
	pstyles[num_pstyles].defedge_id = 0;
	pstyles[num_pstyles].flags = 0;

	while (*attr) {
	    if (!strcasecmp(*attr, "id"))
		strncpy(pstyles[num_pstyles].id, *(attr + 1),
			sizeof(pstyles[0].id - 1));
	    if (!strcasecmp(*attr, "color"))
		pstyles[num_pstyles].color = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "texture"))
		pstyles[num_pstyles].texture_id = get_bmp_id(*(attr + 1));
	    if (!strcasecmp(*attr, "defedge"))
		pstyles[num_pstyles].defedge_id = get_edge_id(*(attr + 1));
	    if (!strcasecmp(*attr, "flags"))
		pstyles[num_pstyles].flags = atoi(*(attr + 1)); /* names @!# */
	    attr += 2;
	}
	num_pstyles++;
    }

    if (!strcasecmp(el, "edgestyle")) {
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
		estyles[num_estyles].width = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "style")) /* !@# names later */
		estyles[num_estyles].width = atoi(*(attr + 1));
	    attr += 2;
	}
	num_estyles++;
    }

    if (!strcasecmp(el, "bmpstyle")) {
	bstyles[num_bstyles].flags = 0;
	bstyles[num_bstyles].filename[sizeof(bstyles[0].filename) - 1] = 0;
	bstyles[num_bstyles].id[sizeof(bstyles[0].id) - 1] = 0;
/* add checks that these are filled !@# */

	while (*attr) {
	    if (!strcasecmp(*attr, "id"))
		strncpy(bstyles[num_bstyles].id, *(attr + 1),
			sizeof(bstyles[0].id) - 1);
	    if (!strcasecmp(*attr, "filename"))
		strncpy(bstyles[num_bstyles].id, *(attr + 1),
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
	groupc++;
	groups[groupc].type = TREASURE;
	groups[groupc].team = team;
	groups[groupc].hit_mask = BALL_BIT;
    }

    if (!strcasecmp(el, "BallTarget")) {
	int team;
	while (*attr) {
	    if (!strcasecmp(*attr, "team"))
		team = atoi(*(attr + 1));
	    attr += 2;
	}
	groupc++;
	groups[groupc].type = TREASURE;
	groups[groupc].team = team;
	groups[groupc].hit_mask = NONBALL_BIT | (((NOTEAM_BIT << 1) - 1) & ~(1 << team));
    }

    if (!strcasecmp(el, "Polygon")) {
	int x, y, hidcount = 0;

	while (*attr) {
	    if (!strcasecmp(*attr, "x"))
		x = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "y"))
		y = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "hidedges"))
		hidcount = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "style"))
		pdata[polyc].style = get_poly_id(*(attr + 1));
	    attr += 2;
	}
	polyc++;
	ptscount = 0;
	*poly++ = groupc;
	polypts = poly++;
	*poly++ = hidcount;
	hidptr = poly;
	poly += hidcount + 1;
	*poly++ = x;
	*poly++ = y;
	return;
    }

    if (!strcasecmp(el, "Featurecount")) {
	int i;
	for (i = 0; i < MAX_TEAMS; i++) {
	    World.teams[i].NumMembers = 0;
	    World.teams[i].NumBases = 0;
	    World.teams[i].NumTreasures = 0;
	    World.teams[i].TreasuresDestroyed = 0;
	    World.teams[i].TreasuresLeft = 0;
	    World.teams[i].SwapperId = -1;
	}
	World.NumBases = 0;
	World.NumTreasures = 0;
	World.NumFuels = 0;
	World.NumChecks = 0;
	while (*attr) {
	    if (!strcasecmp(*attr, "bases"))
		World.NumBases = atoi(*(attr + 1));
	    else if (!strcasecmp(*attr, "balls"))
		World.NumTreasures = atoi(*(attr + 1));
	    else if (!strcasecmp(*attr, "fuels"))
		World.NumFuels = atoi(*(attr + 1));
	    else if (!strcasecmp(*attr, "checks"))
		World.NumChecks = atoi(*(attr + 1));
	    attr += 2;
	}
	if (World.NumBases > 0) {
	    if ((World.base = (base_t *) malloc(World.NumBases * sizeof(base_t))) == NULL) {
		error("Out of memory - bases");
		exit(-1);
	    }
	} else
	    error("WARNING: map has no bases!");
	if (World.NumTreasures > 0
	    && (World.treasures = (treasure_t *)
		malloc(World.NumTreasures * sizeof(treasure_t))) == NULL) {
	    error("Out of memory - treasures");
	    exit(-1);
	}
	if (World.NumFuels > 0
	    && (World.fuel = malloc(World.NumFuels * sizeof(fuel_t))) ==NULL) {
	    error("Out of memory - fuel depots");
	    exit(-1);
	}
	if (World.NumChecks > 0
	    && (World.check = malloc(World.NumChecks * sizeof(ipos))) ==NULL) {
	    error("Out of memory - checkpoints");
	    exit(-1);
	}
    }

    if (!strcasecmp(el, "Check")) {
	static int checknum;
	int x, y;

	if (checknum > World.NumChecks) {
	    error("Given checkpoint count incorrect (too small).\n");
	    exit(1);
	}
	while (*attr) {
	    if (!strcasecmp(*attr, "x"))
		x = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "y"))
		y = atoi(*(attr + 1)) * scale;
	    attr += 2;
	}
	World.check[checknum].x = x;
	World.check[checknum].y = y;
	checknum++;
    }

    if (!strcasecmp(el, "Fuel")) {
	static int fuelnum;
	int team, x, y;

	if (fuelnum >= World.NumFuels) {
	    error("Given fuel count incorrect (too small).\n");
	    exit(1);
	}
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
	World.fuel[fuelnum].clk_pos.x = x;
	World.fuel[fuelnum].clk_pos.y = y;
	World.fuel[fuelnum].fuel = START_STATION_FUEL;
	World.fuel[fuelnum].conn_mask = (unsigned)-1;
	World.fuel[fuelnum].last_change = frame_loops;
	World.fuel[fuelnum].team = TEAM_NOT_SET;
	fuelnum++;
    }

    if (!strcasecmp(el, "Base")) {
	static int basenum;
	int team, x, y, dir;

	if (basenum >= World.NumBases) {
	    error("Given base count incorrect (too small).\n");
	    exit(1);
	}
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

	World.base[basenum].pos.x = x;
	World.base[basenum].pos.y = y;
	/*
	 * The direction of the base should be so that it points
	 * up with respect to the gravity in the region.  This
	 * is fixed in Find_base_dir() when the gravity has
	 * been computed.
	 */
	World.base[basenum].dir = dir;
	if (BIT(World.rules->mode, TEAM_PLAY)) {
	    World.base[basenum].team = team;
	    World.teams[team].NumBases++;
	    if (World.teams[team].NumBases == 1)
		World.NumTeamBases++;
	} else {
	    World.base[basenum].team = TEAM_NOT_SET;
	}
	basenum++;
    }

    if (!strcasecmp(el, "Ball")) {
	static int ballnum;
	int team, x, y;

	if (ballnum >= World.NumTreasures) {
	    error("Given ball count incorrect (too small).\n");
	    exit(1);
	}
	while (*attr) {
	    if (!strcasecmp(*attr, "team"))
		team = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "x"))
		x = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "y"))
		y = atoi(*(attr + 1)) * scale;
	    attr += 2;
	}
	World.treasures[ballnum].pos.x = x;
	World.treasures[ballnum].pos.y = y;
	World.treasures[ballnum].have = false;
	World.treasures[ballnum].destroyed = 0;
	World.treasures[ballnum].team = team;
	World.teams[team].NumTreasures++;
	World.teams[team].TreasuresLeft++;
	ballnum++;
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
	int x, y, hidden = 0;
	while (*attr) {
	    if (!strcasecmp(*attr, "x"))
		x = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "y"))
		y = atoi(*(attr + 1)) * scale;
	    if (!strcasecmp(*attr, "hidden"))
		hidden = 1;
	    attr += 2;
	}
	*poly++ = x;
	*poly++ = y;
	if (hidden)
	    *hidptr++ = ptscount;
	ptscount++;
	return;
    }
    return;
}

static void tagend(void *data, const char *el)
{
    void cmdhack(void);
    if (!strcasecmp(el, "Polygon")) {
	*polypts = ptscount;
	*hidptr = INT_MAX;
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
