/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
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

#include "xpserver.h"

char xpmap_version[] = VERSION;

static int Compress_map(unsigned char *map, size_t size);

static void Xpmap_treasure_to_polygon(int treasure_ind);
static void Xpmap_target_to_polygon(int target_ind);
static void Xpmap_cannon_to_polygon(int cannon_ind);
static void Xpmap_wormhole_to_polygon(int wormhole_ind);

static bool		compress_maps = true;

static void Xpmap_extra_error(int line_num)
{
    static int prev_line_num, error_count;
    const int max_error = 5;

    if (line_num > prev_line_num) {
	prev_line_num = line_num;
	if (++error_count <= max_error)
	    xpprintf("Map file contains extraneous characters on line %d\n",
		     line_num);
	else if (error_count - max_error == 1)
	    xpprintf("And so on...\n");
    }
}


static void Xpmap_missing_error(int line_num)
{
    static int prev_line_num, error_count;
    const int max_error = 5;

    if (line_num > prev_line_num) {
	prev_line_num = line_num;
	if (++error_count <= max_error)
	    xpprintf("Not enough map data on map data line %d\n", line_num);
	else if (error_count - max_error == 1)
	    xpprintf("And so on...\n");
    }
}



/*
 * Compress the map data using a simple Run Length Encoding algorithm.
 * If there is more than one consecutive byte with the same type
 * then we set the high bit of the byte and then the next byte
 * gives the number of repetitions.
 * This works well for most maps which have lots of series of the
 * same map object and is simple enough to got implemented quickly.
 */
static int Compress_map(unsigned char *map, size_t size)
{
    int			i, j, k;

    for (i = j = 0; i < (int)size; i++, j++) {
	if (i + 1 < (int)size
	    && map[i] == map[i + 1]) {
	    for (k = 2; i + k < (int)size; k++) {
		if (map[i] != map[i + k])
		    break;
		if (k == 255)
		    break;
	    }
	    map[j] = (map[i] | SETUP_COMPRESSED);
	    map[++j] = k;
	    i += k - 1;
	} else
	    map[j] = map[i];
    }
    return j;
}


static void Xpmap_setup(world_t *world)
{
    int x;
    unsigned char *map_line;
    unsigned char **map_pointer;

    world->block = (unsigned char **)
	malloc(sizeof(unsigned char *) * world->x
	       + world->x * sizeof(unsigned char) * world->y);

    if (world->block == NULL) {
	error("Couldn't allocate memory");
	exit(-1);
    }

    map_pointer = world->block;
    map_line = (unsigned char *) ((unsigned char **)map_pointer + world->x);

    for (x = 0; x < world->x; x++) {
	*map_pointer = map_line;
	map_pointer += 1;
	map_line += world->y;
    }

    /* Client will quit if it gets a nonexistent homebase, so create
     * some bases to make it happy. Of course it's impossible to play
     * with this, but at least we can tell the player what's wrong... */
    if (mapData == NULL) {
	mapData = malloc((size_t)world->NumBases + 1);
	if (mapData == NULL) {
	    error("Couldn't allocate memory");
	    exit(-1);
	}
	for (x = 0; x < world->NumBases; x++)
	    mapData[x] = '1';
	mapData[world->NumBases] = 0;
    }
    Xpmap_grok_map_data(world, mapData);
    XFREE(mapData);
    Xpmap_tags_to_internal_data(world, false);
}


setup_t *Xpmap_init_setup(world_t *world)
{
    int			i, x, y, team, type = -1,
			wormhole = 0,
			treasure = 0,
			target = 0,
			base = 0,
			cannon = 0;
    unsigned char	*mapdata, *mapptr;
    size_t		size, numblocks;
    setup_t		*setup;

    if (is_polygon_map && world->block) {
	free(world->block);
	world->block = NULL;
    }
    if (world->block == NULL)
	Xpmap_setup(world);

    numblocks = world->x * world->y;
    if ((mapdata = malloc(numblocks)) == NULL) {
	error("No memory for mapdata");
	return NULL;
    }
    memset(mapdata, SETUP_SPACE, numblocks);
    mapptr = mapdata;
    errno = 0;
    for (x = 0; x < world->x; x++) {
	for (y = 0; y < world->y; y++, mapptr++) {
	    type = world->block[x][y];
	    switch (type) {
	    case ACWISE_GRAV:
	    case CWISE_GRAV:
	    case POS_GRAV:
	    case NEG_GRAV:
	    case UP_GRAV:
	    case DOWN_GRAV:
	    case RIGHT_GRAV:
	    case LEFT_GRAV:
		if (!gravityVisible)
		    type = SPACE;
		break;
	    case WORMHOLE:
		if (!wormholeVisible)
		    type = SPACE;
		break;
	    case ITEM_CONCENTRATOR:
		if (!itemConcentratorVisible)
		    type = SPACE;
		break;
	    case ASTEROID_CONCENTRATOR:
		if (!asteroidConcentratorVisible)
		    type = SPACE;
		break;
	    case FRICTION:
		if (!blockFrictionVisible)
		    type = SPACE;
		else
		    type = DECOR_FILLED;
		break;
	    default:
		break;
	    }
	    switch (type) {
	    case SPACE:		*mapptr = SETUP_SPACE; break;
	    case FILLED:	*mapptr = SETUP_FILLED; break;
	    case REC_RU:	*mapptr = SETUP_REC_RU; break;
	    case REC_RD:	*mapptr = SETUP_REC_RD; break;
	    case REC_LU:	*mapptr = SETUP_REC_LU; break;
	    case REC_LD:	*mapptr = SETUP_REC_LD; break;
	    case FUEL:		*mapptr = SETUP_FUEL; break;
	    case ACWISE_GRAV:	*mapptr = SETUP_ACWISE_GRAV; break;
	    case CWISE_GRAV:	*mapptr = SETUP_CWISE_GRAV; break;
	    case POS_GRAV:	*mapptr = SETUP_POS_GRAV; break;
	    case NEG_GRAV:	*mapptr = SETUP_NEG_GRAV; break;
	    case UP_GRAV:	*mapptr = SETUP_UP_GRAV; break;
	    case DOWN_GRAV:	*mapptr = SETUP_DOWN_GRAV; break;
	    case RIGHT_GRAV:	*mapptr = SETUP_RIGHT_GRAV; break;
	    case LEFT_GRAV:	*mapptr = SETUP_LEFT_GRAV; break;
	    case ITEM_CONCENTRATOR:
		*mapptr = SETUP_ITEM_CONCENTRATOR; break;
	    case ASTEROID_CONCENTRATOR:
		*mapptr = SETUP_ASTEROID_CONCENTRATOR; break;
	    case DECOR_FILLED:	*mapptr = SETUP_DECOR_FILLED; break;
	    case DECOR_RU:	*mapptr = SETUP_DECOR_RU; break;
	    case DECOR_RD:	*mapptr = SETUP_DECOR_RD; break;
	    case DECOR_LU:	*mapptr = SETUP_DECOR_LU; break;
	    case DECOR_LD:	*mapptr = SETUP_DECOR_LD; break;
	    case WORMHOLE:
		if (wormhole >= world->NumWormholes) {
		    warn("Too many wormholes in block mapdata.");
		    *mapptr = SETUP_SPACE;
		    break;
		}
		switch (world->wormholes[wormhole++].type) {
		case WORM_NORMAL: *mapptr = SETUP_WORM_NORMAL; break;
		case WORM_IN:     *mapptr = SETUP_WORM_IN; break;
		case WORM_OUT:    *mapptr = SETUP_WORM_OUT; break;
		default:
		    error("Bad wormhole (%d,%d).", x, y);
		    free(mapdata);
		    return NULL;
		}
		break;
	    case TREASURE:
		if (treasure >= world->NumTreasures) {
		    warn("Too many treasures in block mapdata.");
		    *mapptr = SETUP_SPACE;
		    break;
		}
		team = world->treasures[treasure++].team;
		if (team == TEAM_NOT_SET)
		    team = 0;
		*mapptr = SETUP_TREASURE + team;
		break;
	    case TARGET:
		if (target >= world->NumTargets) {
		    warn("Too many targets in block mapdata.");
		    *mapptr = SETUP_SPACE;
		    break;
		}
		team = world->targets[target++].team;
		if (team == TEAM_NOT_SET)
		    team = 0;
		*mapptr = SETUP_TARGET + team;
		break;
	    case BASE:
		if (base >= world->NumBases) {
		    warn("Too many bases in block mapdata.");
		    *mapptr = SETUP_SPACE;
		    break;
		}
		team = world->bases[base].team;
		if (team == TEAM_NOT_SET)
		    team = 0;
		switch (world->bases[base++].dir) {
		case DIR_UP:    *mapptr = SETUP_BASE_UP + team; break;
		case DIR_RIGHT: *mapptr = SETUP_BASE_RIGHT + team; break;
		case DIR_DOWN:  *mapptr = SETUP_BASE_DOWN + team; break;
		case DIR_LEFT:  *mapptr = SETUP_BASE_LEFT + team; break;
		default:
		    error("Bad base at (%d,%d).", x, y);
		    free(mapdata);
		    return NULL;
		}
		break;
	    case CANNON:
		if (cannon >= world->NumCannons) {
		    warn("Too many cannons in block mapdata.");
		    *mapptr = SETUP_SPACE;
		    break;
		}
		switch (world->cannons[cannon++].dir) {
		case DIR_UP:	*mapptr = SETUP_CANNON_UP; break;
		case DIR_RIGHT:	*mapptr = SETUP_CANNON_RIGHT; break;
		case DIR_DOWN:	*mapptr = SETUP_CANNON_DOWN; break;
		case DIR_LEFT:	*mapptr = SETUP_CANNON_LEFT; break;
		default:
		    error("Bad cannon at (%d,%d).", x, y);
		    free(mapdata);
		    return NULL;
		}
		break;
	    case CHECK:
		for (i = 0; i < world->NumChecks; i++) {
		    check_t *check = Checks(world, i);
		    blpos bpos = Clpos_to_blpos(check->pos);
		    if (x != bpos.bx || y != bpos.by)
			continue;
		    *mapptr = SETUP_CHECK + i;
		    break;
		}
		if (i >= world->NumChecks) {
		    error("Bad checkpoint at (%d,%d).", x, y);
		    free(mapdata);
		    return NULL;
		}
		break;
	    default:
		error("Unknown map type (%d) at (%d,%d).", type, x, y);
		*mapptr = SETUP_SPACE;
		break;
	    }
	}
    }
    if (!compress_maps) {
	type = SETUP_MAP_UNCOMPRESSED;
	size = numblocks;
    } else {
	type = SETUP_MAP_ORDER_XY;
	size = Compress_map(mapdata, numblocks);
	if (size <= 0 || size > numblocks) {
	    warn("Map compression error (%d)", size);
	    free(mapdata);
	    return NULL;
	}
	if ((mapdata = (unsigned char *)realloc(mapdata, size)) == NULL) {
	    error("Cannot reallocate mapdata");
	    return NULL;
	}
    }

    if (!silent) {
	if (type != SETUP_MAP_UNCOMPRESSED) {
	    xpprintf("%s Block map compression ratio is %-4.2f%%\n",
		     showtime(), 100.0 * size / numblocks);
	}
    }

    if ((setup = malloc(sizeof(setup_t) + size)) == NULL) {
	error("No memory to hold oldsetup");
	free(mapdata);
	return NULL;
    }
    memset(setup, 0, sizeof(setup_t) + size);
    memcpy(setup->map_data, mapdata, size);
    free(mapdata);
    setup->setup_size = ((char *) &setup->map_data[0]
			 - (char *) setup) + size;
    setup->map_data_len = size;
    setup->map_order = type;
    setup->lives = world->rules->lives;
    setup->mode = world->rules->mode;
    setup->x = world->x;
    setup->y = world->y;
    strlcpy(setup->name, world->name, sizeof(setup->name));
    strlcpy(setup->author, world->author, sizeof(setup->author));

    return setup;
}




/*
 * Grok block based map data.
 *
 * Create world->block using mapData.
 * Free mapData.
 */
void Xpmap_grok_map_data(world_t *world, char *map_data)
{
    int x, y, c;
    char *s;
    blpos blk;

    x = -1;
    y = world->y - 1;

    s = map_data;
    while (y >= 0) {

	x++;

	if (extraBorder && (x == 0 || x == world->x - 1
	    || y == 0 || y == world->y - 1)) {
	    if (x >= world->x) {
		x = -1;
		y--;
		continue;
	    } else
		/* make extra border of solid rock */
		c = 'x';
	}
	else {
	    c = *s;
	    if (c == '\0' || c == EOF) {
		if (x < world->x) {
		    /* not enough map data on this line */
		    if (!silent)
			Xpmap_missing_error(world->y - y);
		    c = ' ';
		} else
		    c = '\n';
	    } else {
		if (c == '\n' && x < world->x) {
		    /* not enough map data on this line */
		    if (!silent)
			Xpmap_missing_error(world->y - y);
		    c = ' ';
		} else
		    s++;
	    }
	}
	if (x >= world->x || c == '\n') {
	    y--; x = -1;
	    if (c != '\n') {			/* Get rest of line */
		if (!silent)
		    Xpmap_extra_error(world->y - y);
		while (c != '\n' && c != EOF)
		    c = *s++;
	    }
	    continue;
	}
	blk.bx = x;
	blk.by = y;
	World_set_block(world, blk, c);
    }
}


void Xpmap_allocate_checks(world_t *world)
{
    int x, y;

    if (!BIT(world->rules->mode, TIMING))
	return;

    /* count the checks */
    for (y = 0; y < world->y; y++) {
	for (x = 0; x < world->x; x++) {
	    switch (world->block[x][y]) {
	    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	    case 'Y': case 'Z':
		world->NumChecks++;
		break;
	    default:
		break;
	    }
	}
    }

    if ((world->checks = malloc(OLD_MAX_CHECKS * sizeof(check_t))) == NULL) {
	error("Out of memory - checks");
	exit(-1);
    }
}


/*
 * Determining which team these belong to is done later,
 * in Find_closest_team().
 */
static void Xpmap_place_cannon(world_t *world, blpos blk, int dir, bool create)
{
    clpos pos;

    switch (dir) {
    case DIR_UP:
	pos.cx = (blk.bx + 0.5) * BLOCK_CLICKS;
	pos.cy = (blk.by + 0.333) * BLOCK_CLICKS;
	break;
    case DIR_LEFT:
	pos.cx = (blk.bx + 0.667) * BLOCK_CLICKS;
	pos.cy = (blk.by + 0.5) * BLOCK_CLICKS;
	break;
    case DIR_RIGHT:
	pos.cx = (blk.bx + 0.333) * BLOCK_CLICKS;
	pos.cy = (blk.by + 0.5) * BLOCK_CLICKS;
	break;
    case DIR_DOWN:
	pos.cx = (blk.bx + 0.5) * BLOCK_CLICKS;
	pos.cy = (blk.by + 0.667) * BLOCK_CLICKS;
	break;
    default:
 	/* can't happen */
	assert(0 && "Unknown cannon direction.");
	break;
    }

    World_set_block(world, blk, CANNON);
    if (create)
	World_place_cannon(world, pos, dir, TEAM_NOT_SET);
}

/*
 * The direction of the base should be so that it points
 * up with respect to the gravity in the region.  This
 * is fixed in Find_base_dir() when the gravity has
 * been computed.
 */
static clpos Xpmap_get_clpos(blpos blk)
{
    clpos pos;

    pos.cx = BLOCK_CENTER(blk.bx);
    pos.cy = BLOCK_CENTER(blk.by);

    return pos;
}

static void Xpmap_place_base(world_t *world, blpos blk, int team, bool create)
{
    World_set_block(world, blk, BASE);
    if (create)
	World_place_base(world, Xpmap_get_clpos(blk), DIR_UP, team);
}

static void Xpmap_place_fuel(world_t *world, blpos blk, bool create)
{
    World_set_block(world, blk, FUEL);
    if (create)
	World_place_fuel(world, Xpmap_get_clpos(blk), TEAM_NOT_SET);
}

static void Xpmap_place_treasure(world_t *world, blpos blk, bool empty,
				 bool create)
{
    World_set_block(world, blk, TREASURE);
    if (create)
	World_place_treasure(world, Xpmap_get_clpos(blk), TEAM_NOT_SET, empty);
}

static void Xpmap_place_wormhole(world_t *world, blpos blk, wormType type,
				 bool create)
{
    World_set_block(world, blk, WORMHOLE);
    if (create)
	World_place_wormhole(world, Xpmap_get_clpos(blk), type);
}

static void Xpmap_place_target(world_t *world, blpos blk, bool create)
{
    World_set_block(world, blk, TARGET);
    if (create)
	World_place_target(world, Xpmap_get_clpos(blk), TEAM_NOT_SET);
}

static void Xpmap_place_check(world_t *world, blpos blk, int ind, bool create)
{
    if (!BIT(world->rules->mode, TIMING)) {
	World_set_block(world, blk, SPACE);
	return;
    }

    World_set_block(world, blk, CHECK);
    if (create)
	World_place_check(world, Xpmap_get_clpos(blk), ind);
}

static void Xpmap_place_item_concentrator(world_t *world, blpos blk,
					  bool create)
{
    World_set_block(world, blk, ITEM_CONCENTRATOR);
    if (create)
	World_place_item_concentrator(world, Xpmap_get_clpos(blk));
}

static void Xpmap_place_asteroid_concentrator(world_t *world, blpos blk,
					      bool create)
{
    World_set_block(world, blk, ASTEROID_CONCENTRATOR);
    if (create)
	World_place_asteroid_concentrator(world, Xpmap_get_clpos(blk));
}

static void Xpmap_place_grav(world_t *world, blpos blk,
			     double force, int type, bool create)
{
    World_set_block(world, blk, type);
    if (create)
	World_place_grav(world, Xpmap_get_clpos(blk), force, type);
}

static void Xpmap_place_block(world_t *world, blpos blk, int type)
{
    World_set_block(world, blk, type);
}

/*
 * Change read tags to internal data, create objects if 'create' is true.
 */
void Xpmap_tags_to_internal_data(world_t *world, bool create)
{
    int x, y;
    char c;

    for (x = 0; x < world->x; x++) {

	for (y = 0; y < world->y; y++) {

	    blpos blk;

	    blk.bx = x;
	    blk.by = y;

	    c = world->block[x][y];
	    
	    switch (c) {
	    case ' ':
	    case '.':
	    default:
		Xpmap_place_block(world, blk, SPACE);
		break;
		
	    case 'x':
		Xpmap_place_block(world, blk, FILLED);
		break;
	    case 's':
		Xpmap_place_block(world, blk, REC_LU);
		break;
	    case 'a':
		Xpmap_place_block(world, blk, REC_RU);
		break;
	    case 'w':
		Xpmap_place_block(world, blk, REC_LD);
		break;
	    case 'q':
		Xpmap_place_block(world, blk, REC_RD);
		break;
		    
	    case 'r':
		Xpmap_place_cannon(world, blk, DIR_UP, create);
		break;
	    case 'd':
		Xpmap_place_cannon(world, blk, DIR_LEFT, create);
		break;
	    case 'f':
		Xpmap_place_cannon(world, blk, DIR_RIGHT, create);
		break;
	    case 'c':
		Xpmap_place_cannon(world, blk, DIR_DOWN, create);
		break;

	    case '#':
		Xpmap_place_fuel(world, blk, create);
		break;
	    case '*':
		Xpmap_place_treasure(world, blk, false, create);
		break;
	    case '^':
		Xpmap_place_treasure(world, blk, true, create);
		break;
	    case '!':
		Xpmap_place_target(world, blk, create);
		break;
	    case '%':
		Xpmap_place_item_concentrator(world, blk, create);
		break;
	    case '&':
		Xpmap_place_asteroid_concentrator(world, blk, create);
		break;
	    case '$':
		Xpmap_place_block(world, blk, BASE_ATTRACTOR);
		break;
	    case '_':
		Xpmap_place_base(world, blk, TEAM_NOT_SET, create);
		break;
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
		Xpmap_place_base(world, blk, (int) (c - '0'), create);
		break;
		
	    case '+':
		Xpmap_place_grav(world, blk, -GRAVS_POWER, POS_GRAV, create);
		break;
	    case '-':
		Xpmap_place_grav(world, blk, GRAVS_POWER, NEG_GRAV, create);
		break;
	    case '>':
		Xpmap_place_grav(world, blk, GRAVS_POWER, CWISE_GRAV, create);
		break;
	    case '<':
		Xpmap_place_grav(world, blk, -GRAVS_POWER, ACWISE_GRAV,
				 create);
		break;
	    case 'i':
		Xpmap_place_grav(world, blk, GRAVS_POWER, UP_GRAV, create);
		break;
	    case 'm':
		Xpmap_place_grav(world, blk, -GRAVS_POWER, DOWN_GRAV, create);
		break;
	    case 'k':
		Xpmap_place_grav(world, blk, GRAVS_POWER, RIGHT_GRAV, create);
		break;
	    case 'j':
		Xpmap_place_grav(world, blk, -GRAVS_POWER, LEFT_GRAV, create);
		break;
		
	    case '@':
		Xpmap_place_wormhole(world, blk, WORM_NORMAL, create);
		break;
	    case '(':
		Xpmap_place_wormhole(world, blk, WORM_IN, create);
		break;
	    case ')':
		Xpmap_place_wormhole(world, blk, WORM_OUT, create);
		break;
		
	    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	    case 'Y': case 'Z':
		Xpmap_place_check(world, blk, (int) (c - 'A'), create);
		break;
		
	    case 'z':
		Xpmap_place_block(world, blk, FRICTION);
		break;
		
	    case 'b':
		Xpmap_place_block(world, blk, DECOR_FILLED);
		break;
	    case 'h':
		Xpmap_place_block(world, blk, DECOR_LU);
		break;
	    case 'g':
		Xpmap_place_block(world, blk, DECOR_RU);
		break;
	    case 'y':
		Xpmap_place_block(world, blk, DECOR_LD);
		break;
	    case 't':
		Xpmap_place_block(world, blk, DECOR_RD);
		break;
	    }
	}
    }
}


void Xpmap_find_map_object_teams(world_t *world)
{
    int i;

    /*
     * Determine which team a treasure belongs to.
     */
    if (BIT(world->rules->mode, TEAM_PLAY)) {
	int team = TEAM_NOT_SET;

	for (i = 0; i < world->NumTreasures; i++) {
	    treasure_t *treasure = Treasures(world, i);

	    team = Find_closest_team(world, treasure->pos);
	    treasure->team = team;
	    if (team == TEAM_NOT_SET)
		warn("Couldn't find a matching team for the treasure.");
	    else {
		world->teams[team].NumTreasures++;
		if (!treasure->empty)
		    world->teams[team].TreasuresLeft++;
		else
		    world->teams[team].NumEmptyTreasures++;
	    }
	}

	for (i = 0; i < world->NumTargets; i++) {
	    target_t *targ = Targets(world, i);

	    team = Find_closest_team(world, targ->pos);
	    if (team == TEAM_NOT_SET)
		warn("Couldn't find a matching team for the target.");
	    targ->team = team;
	}

	if (teamCannons) {
	    for (i = 0; i < world->NumCannons; i++) {
		cannon_t *cannon = Cannons(world, i);

		team = Find_closest_team(world, cannon->pos);
		if (team == TEAM_NOT_SET)
		    warn("Couldn't find a matching team for the cannon.");
		cannon->team = team;
	    }
	}
	
	for (i = 0; i < world->NumFuels; i++) {
	    fuel_t *fs = Fuels(world, i);

	    team = Find_closest_team(world, fs->pos);
	    if (team == TEAM_NOT_SET)
		warn("Couldn't find a matching team for fuelstation.");
	    fs->team = team;
	}
    }
}


/*
 * Find the correct direction of the base, according to the gravity in
 * the base region.
 *
 * If a base attractor is adjacent to a base then the base will point
 * to the attractor.
 */
void Xpmap_find_base_direction(world_t *world)
{
    int	i;

    for (i = 0; i < world->NumBases; i++) {
	base_t *base = Bases(world, i);
	int x, y, dir, att;
	vector gravity = World_gravity(base->pos);

	if (gravity.x == 0.0 && gravity.y == 0.0)
	    /*
	     * Undefined direction
	     * Should be set to direction of gravity!
	     */
	    dir = DIR_UP;
	else {
	    dir = findDir(-gravity.x, -gravity.y);
	    dir = ((dir + RES/8) / (RES/4)) * (RES/4);	/* round it */
	    dir = MOD2(dir, RES);
	}
	att = -1;

	x = CLICK_TO_BLOCK(base->pos.cx);
	y = CLICK_TO_BLOCK(base->pos.cy);

	/* First check upwards attractor */
	if (y == world->y - 1 && world->block[x][0] == BASE_ATTRACTOR
	    && BIT(world->rules->mode, WRAP_PLAY)) {
	    if (att == -1 || dir == DIR_UP)
		att = DIR_UP;
	}
	if (y < world->y - 1 && world->block[x][y + 1] == BASE_ATTRACTOR) {
	    if (att == -1 || dir == DIR_UP)
		att = DIR_UP;
	}

	/* then downwards */
	if (y == 0 && world->block[x][world->y-1] == BASE_ATTRACTOR
	    && BIT(world->rules->mode, WRAP_PLAY)) {
	    if (att == -1 || dir == DIR_DOWN)
		att = DIR_DOWN;
	}
	if (y > 0 && world->block[x][y - 1] == BASE_ATTRACTOR) {
	    if (att == -1 || dir == DIR_DOWN)
		att = DIR_DOWN;
	}

	/* then rightwards */
	if (x == world->x - 1 && world->block[0][y] == BASE_ATTRACTOR
	    && BIT(world->rules->mode, WRAP_PLAY)) {
	    if (att == -1 || dir == DIR_RIGHT)
		att = DIR_RIGHT;
	}
	if (x < world->x - 1 && world->block[x + 1][y] == BASE_ATTRACTOR) {
	    if (att == -1 || dir == DIR_RIGHT)
		att = DIR_RIGHT;
	}

	/* then leftwards */
	if (x == 0 && world->block[world->x-1][y] == BASE_ATTRACTOR
	    && BIT(world->rules->mode, WRAP_PLAY)) {
	    if (att == -1 || dir == DIR_LEFT)
		att = DIR_LEFT;
	}
	if (x > 0 && world->block[x - 1][y] == BASE_ATTRACTOR) {
	    if (att == -1 || dir == DIR_LEFT)
		att = DIR_LEFT;
	}

	if (att != -1)
	    dir = att;
	base->dir = dir;
    }
    for (i = 0; i < world->x; i++) {
	int j;
	for (j = 0; j < world->y; j++) {
	    blpos blk;
	    blk.bx = i;
	    blk.by = j;
	    if (world->block[i][j] == BASE_ATTRACTOR)
		World_set_block(world, blk, SPACE);
	}
    }
}


/*
 * The following functions is for converting the block based map data
 * to polygons.
 */

/* number of vertices in polygon */
#define N (2 + 12)
static void Xpmap_treasure_to_polygon(int treasure_ind)
{
    int cx, cy, i, r, n;
    double angle;
    int polystyle, edgestyle;
    world_t *world = &World;
    treasure_t *treasure = Treasures(world, treasure_ind);
    clpos pos[N + 1];

    polystyle = P_get_poly_id("treasure_ps");
    edgestyle = P_get_edge_id("treasure_es");

    cx = treasure->pos.cx - BLOCK_CLICKS / 2;
    cy = treasure->pos.cy - BLOCK_CLICKS / 2;

    pos[0].cx = cx;
    pos[0].cy = cy;
    pos[1].cx = cx + (BLOCK_CLICKS - 1);
    pos[1].cy = cy;

    cx = treasure->pos.cx;
    cy = treasure->pos.cy;
    /* -1 is to ensure the vertices are inside the block */
    r = (BLOCK_CLICKS / 2) - 1;
    /* number of points in half circle */
    n = N - 2;

    for (i = 0; i < n; i++) {
	angle = (((double)i)/(n - 1)) * PI;
	pos[i + 2].cx = cx + r * cos(angle);
	pos[i + 2].cy = cy + r * sin(angle);
    }

    pos[N] = pos[0];

    /* create balltarget */
    P_start_balltarget(treasure->team, treasure_ind);
    P_start_polygon(pos[0], polystyle);
    for (i = 1; i <= N; i++)
	P_vertex(pos[i], edgestyle); 
    P_end_polygon();
    P_end_balltarget();

    /* create ballarea */
    P_start_ballarea();
    P_start_polygon(pos[0], polystyle);
    for (i = 1; i <= N; i++)
	P_vertex(pos[i], edgestyle); 
    P_end_polygon();
    P_end_ballarea();
}
#undef N


static void Xpmap_block_polygon(clpos bpos, int polystyle, int edgestyle)
{
    clpos pos[5];
    int i;

    bpos.cx = CLICK_TO_BLOCK(bpos.cx) * BLOCK_CLICKS;
    bpos.cy = CLICK_TO_BLOCK(bpos.cy) * BLOCK_CLICKS;

    pos[0].cx = bpos.cx;
    pos[0].cy = bpos.cy;
    pos[1].cx = bpos.cx + (BLOCK_CLICKS - 1);
    pos[1].cy = bpos.cy;
    pos[2].cx = bpos.cx + (BLOCK_CLICKS - 1);
    pos[2].cy = bpos.cy + (BLOCK_CLICKS - 1);
    pos[3].cx = bpos.cx;
    pos[3].cy = bpos.cy + (BLOCK_CLICKS - 1);
    pos[4] = pos[0];

    P_start_polygon(pos[0], polystyle);
    for (i = 1; i <= 4; i++)
	P_vertex(pos[i], edgestyle); 
    P_end_polygon();
}


static void Xpmap_target_to_polygon(int target_ind)
{
    int ps, es;
    world_t *world = &World;
    target_t *targ = Targets(world, target_ind);

    ps = P_get_poly_id("target_ps");
    es = P_get_edge_id("target_es");

    /* create target polygon */
    P_start_target(target_ind);
    Xpmap_block_polygon(targ->pos, ps, es);
    P_end_target();
}


static void Xpmap_cannon_polygon(cannon_t *cannon,
				 int polystyle, int edgestyle)
{
    clpos pos[4], cpos = cannon->pos;
    int i;

    pos[0] = cannon->pos;

    cpos.cx = CLICK_TO_BLOCK(cpos.cx) * BLOCK_CLICKS;
    cpos.cy = CLICK_TO_BLOCK(cpos.cy) * BLOCK_CLICKS;

    switch (cannon->dir) {
    case DIR_RIGHT:
	pos[1].cx = cpos.cx;
	pos[1].cy = cpos.cy + (BLOCK_CLICKS - 1);
	pos[2].cx = cpos.cx;
	pos[2].cy = cpos.cy;
	break;
    case DIR_UP:
	pos[1].cx = cpos.cx;
	pos[1].cy = cpos.cy;
	pos[2].cx = cpos.cx + (BLOCK_CLICKS - 1);
	pos[2].cy = cpos.cy;
	break;
    case DIR_LEFT:
	pos[1].cx = cpos.cx + (BLOCK_CLICKS - 1);
	pos[1].cy = cpos.cy;
	pos[2].cx = cpos.cx + (BLOCK_CLICKS - 1);
	pos[2].cy = cpos.cy + (BLOCK_CLICKS - 1);
	break;
    case DIR_DOWN:
	pos[1].cx = cpos.cx + (BLOCK_CLICKS - 1);
	pos[1].cy = cpos.cy + (BLOCK_CLICKS - 1);
	pos[2].cx = cpos.cx;
	pos[2].cy = cpos.cy + (BLOCK_CLICKS - 1);
	break;
    default:
 	/* can't happen */
	assert(0 && "Unknown cannon direction.");
	break;
    }
    pos[3] = pos[0];

    P_start_polygon(pos[0], polystyle);
    for (i = 1; i <= 3; i++)
	P_vertex(pos[i], edgestyle); 
    P_end_polygon();
}


static void Xpmap_cannon_to_polygon(int cannon_ind)
{
    world_t *world = &World;
    int ps, es;
    cannon_t *cannon = Cannons(world, cannon_ind);

    ps = P_get_poly_id("cannon_ps");
    es = P_get_edge_id("cannon_es");

    P_start_cannon(cannon_ind);
    Xpmap_cannon_polygon(cannon, ps, es);
    P_end_cannon();
}

#define N 12
static void Xpmap_wormhole_to_polygon(int wormhole_ind)
{
    world_t *world = &World;
    int ps, es, i, r;
    double angle;
    wormhole_t *wormhole = Wormholes(world, wormhole_ind);
    clpos pos[N + 1], wpos;

    /* don't make a polygon for an out wormhole */
    if (wormhole->type == WORM_OUT)
	return;

    ps = P_get_poly_id("wormhole_ps");
    es = P_get_edge_id("wormhole_es");

    wpos = wormhole->pos;
    r = (BLOCK_CLICKS / 2) - 1;

    for (i = 0; i < N; i++) {
	angle = (((double)i)/ N) * 2 * PI;
	pos[i].cx = wpos.cx + r * cos(angle);
	pos[i].cy = wpos.cy + r * sin(angle);
    }
    pos[N] = pos[0];

    P_start_wormhole(wormhole_ind);
    P_start_polygon(pos[0], ps);
    for (i = 1; i <= N; i++)
	P_vertex(pos[i], es); 
    P_end_polygon();
    P_end_wormhole();
}


/*
 * Add a wall polygon
 *
 * The polygon consists of a start block and and endblock and possibly
 * some full wall/fuel blocks in between. A total number of numblocks
 * blocks are part of the polygon and must be 1 or more. If numblocks
 * is one, the startblock and endblock are the same block.
 *
 * The block coordinates of the first block is (bx, by)
 *
 * The polygon will have 3 or 4 vertices.
 *
 * Idea: first assume the polygon is a rectangle, then move
 * the vertices depending on the start and end blocks.
 *
 * The vertex index:
 * 0: upper left vertex
 * 1: lower left vertex
 * 2: lower right vertex
 * 3: upper right vertex
 * 4: upper left vertex, second time
 */
static void Xpmap_wall_poly(int bx, int by,
			    int startblock, int endblock, int numblocks,
			    int polystyle, int edgestyle)
{
    int i;
    clpos pos[5]; /* positions of vertices */

    if (numblocks < 1)
	return;

    /* first assume we have a rectangle */
    /* kps - use -1 if you don't wan't the polygon corners to
     * overlap other polygons
     */
    pos[0].cx = bx * BLOCK_CLICKS;
    pos[0].cy = (by + 1) * BLOCK_CLICKS - 1;
    pos[1].cx = bx * BLOCK_CLICKS;
    pos[1].cy = by * BLOCK_CLICKS;
    pos[2].cx = (bx + numblocks) * BLOCK_CLICKS - 1;
    pos[2].cy = by * BLOCK_CLICKS;
    pos[3].cx = (bx + numblocks) * BLOCK_CLICKS - 1;
    pos[3].cy = (by + 1) * BLOCK_CLICKS - 1;
    
    /* move the vertices depending on the startblock and endblock */
    switch (startblock) {
    case FILLED:
    case REC_LU:
    case REC_LD:
    case FUEL:
	/* no need to move the leftmost 2 vertices */
	break;
    case REC_RU:
	/* move lower left vertex to the right */
	pos[1].cx += (BLOCK_CLICKS - 1);
	break;
    case REC_RD:
	/* move upper left vertex to the right */
	pos[0].cx += (BLOCK_CLICKS - 1);
	break;
    default:
	return;
    }
    
    switch (endblock) {
    case FILLED:
    case FUEL:
    case REC_RU:
    case REC_RD:
	/* no need to move the rightmost 2 vertices */
	break;
    case REC_LU:
	pos[2].cx -= (BLOCK_CLICKS - 1);
	break;
    case REC_LD:
	pos[3].cx -= (BLOCK_CLICKS - 1);
	break;
    default:
	return;
    }

    /*
     * Since we want to form a closed loop of line segments, the
     * last vertex must equal the first.
     */
    pos[4] = pos[0];

    P_start_polygon(pos[0], polystyle);
    for (i = 1; i <= 4; i++)
	P_vertex(pos[i], edgestyle); 
    P_end_polygon();
}


static void Xpmap_walls_to_polygons(world_t *world)
{
    int x, y, x0 = 0;
    int numblocks = 0;
    int inside = false;
    int startblock = 0, endblock = 0, block;
    int maxblocks = POLYGON_MAX_OFFSET / BLOCK_CLICKS;
    int ps, es;

    ps = P_get_poly_id("wall_ps");
    es = P_get_edge_id("wall_es");

    /*
     * x, FILLED = solid wall
     * s, REC_LU = wall triangle pointing left and up 
     * a, REC_RU = wall triangle pointing right and up 
     * w, REC_LD = wall triangle pointing left and down
     * q, REC_RD = wall triangle pointing right and down
     * #, FUEL   = fuel block
     */

    for (y = world->y - 1; y >= 0; y--) {
	for (x = 0; x < world->x; x++) {
	    block = world->block[x][y];

	    if (!inside) {
		switch (block) {
		case FILLED:
		case REC_RU:
		case REC_RD:
		case FUEL:
		    x0 = x;
		    startblock = endblock = block;
		    inside = true;
		    numblocks = 1;
		    break;

		case REC_LU:
		case REC_LD:
		    Xpmap_wall_poly(x, y, block, block, 1, ps, es);
		    break;
		default:
		    break;
		}
	    } else {

		switch (block) {
		case FILLED:
		case FUEL:
		    numblocks++;
		    endblock = block;
		    break;

		case REC_RU:
		case REC_RD:
		    /* old polygon ends */
		    Xpmap_wall_poly(x0, y, startblock, endblock,
				    numblocks, ps, es);
		    /* and a new one starts */
		    x0 = x;
		    startblock = endblock = block;
		    numblocks = 1;
		    break;

		case REC_LU:
		case REC_LD:
		    numblocks++;
		    endblock = block;
		    Xpmap_wall_poly(x0, y, startblock, endblock,
				    numblocks, ps, es);
		    inside = false;
		    break;

		default:
		    /* none of the above, polygon ends */
		    Xpmap_wall_poly(x0, y, startblock, endblock,
				    numblocks, ps, es);
		    inside = false;
		    break;
		}
	    }

	    /*
	     * We don't want the polygon to have offsets
	     * that is too big
	     */
	    if (inside && numblocks == maxblocks) {
		Xpmap_wall_poly(x0, y, startblock, endblock,
				numblocks, ps, es);
		inside = false;
	    }

	}

	/* end of row */
	if (inside) {
	    Xpmap_wall_poly(x0, y, startblock, endblock,
			    numblocks, ps, es);
	    inside = false;
	}
    }
}


void Xpmap_blocks_to_polygons(world_t *world)
{
    int i;

    /* create edgestyles and polystyles */
    P_edgestyle("wall_es", -1, 0x2244EE, 0);
    P_polystyle("wall_ps", 0x0033AA, 0, P_get_edge_id("wall_es"), 0);

    P_edgestyle("treasure_es", -1, 0xFF0000, 0);
    P_polystyle("treasure_ps", 0xFF0000, 0, P_get_edge_id("treasure_es"), 0);

    P_edgestyle("target_es", 3, 0xFF7700, 0);
    P_polystyle("target_ps", 0xFF7700, 3, P_get_edge_id("target_es"), 0);

    P_edgestyle("cannon_es", 3, 0xFFFFFF, 0);
    P_polystyle("cannon_ps", 0xFFFFFF, 2, P_get_edge_id("cannon_es"), 0);

    P_edgestyle("wormhole_es", 2, 0x00FFFF, 0);
    P_polystyle("wormhole_ps", 0x00FFFF, 2, P_get_edge_id("wormhole_es"), 0);

    Xpmap_walls_to_polygons(world);

    if (polygonMode)
	is_polygon_map = true;

    for (i = 0; i < world->NumTreasures; i++)
	Xpmap_treasure_to_polygon(i);

    for (i = 0; i < world->NumTargets; i++)
	Xpmap_target_to_polygon(i);

    for (i = 0; i < world->NumCannons; i++)
	Xpmap_cannon_to_polygon(i);

    for (i = 0; i < world->NumWormholes; i++)
	Xpmap_wormhole_to_polygon(i);

    /*xpprintf("Created %d polygons.\n", num_polys);*/
}
