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

/* Robot code originally submitted by Maurice Abraham. */

#include "xpserver.h"

char robotdef_version[] = VERSION;


#define ROB_LOOK_AH		2

#define WITHIN(NOW,THEN,DIFF) (NOW<=THEN && (THEN-NOW)<DIFF)

/*
 * Flags for the default robots being in different modes (or moods).
 */
#define RM_ROBOT_IDLE         	(1 << 2)
#define RM_EVADE_LEFT         	(1 << 3)
#define RM_EVADE_RIGHT          (1 << 4)
#define RM_ROBOT_CLIMB          (1 << 5)
#define RM_HARVEST            	(1 << 6)
#define RM_ATTACK             	(1 << 7)
#define RM_TAKE_OFF           	(1 << 8)
#define RM_CANNON_KILL		(1 << 9)
#define RM_REFUEL		(1 << 10)
#define RM_NAVIGATE		(1 << 11)

/* long term modes */
#define FETCH_TREASURE		(1 << 0)
#define TARGET_KILL		(1 << 1)
#define NEED_FUEL		(1 << 2)

/*
 * Map objects a robot can fly through without damage.
 */


/*
 * Prototypes for methods of the default robot type.
 */
static void Robot_default_round_tick(void);
static void Robot_default_create(player_t *pl, char *str);
static void Robot_default_go_home(player_t *pl);
static void Robot_default_play(player_t *pl);
static void Robot_default_set_war(player_t *pl, int victim_id);
static int Robot_default_war_on_player(player_t *pl);
static void Robot_default_message(player_t *pl, const char *str);
static void Robot_default_destroy(player_t *pl);
static void Robot_default_invite(player_t *pl, player_t *inviter);
       int Robot_default_setup(robot_type_t *type_ptr);


/*
 * Local static variables
 */
static double	Visibility_distance;
static double	Max_enemy_distance;


/*
 * The robot type structure for the default robot.
 */
static robot_type_t robot_default_type = {
    "default",
    Robot_default_round_tick,
    Robot_default_create,
    Robot_default_go_home,
    Robot_default_play,
    Robot_default_set_war,
    Robot_default_war_on_player,
    Robot_default_message,
    Robot_default_destroy,
    Robot_default_invite
};


/*
 * The only thing we export from this file.
 * A function to initialize the robot type structure
 * with our name and the pointers to our action routines.
 *
 * Return 0 if all is OK, anything else will ignore this
 * robot type forever.
 */
int Robot_default_setup(robot_type_t *type_ptr)
{
    /* Not much to do for the default robot except init the type structure. */

    *type_ptr = robot_default_type;

    return 0;
}

/*
 * Private functions.
 */
static bool Check_robot_evade(player_t *pl, int mine_i, int ship_i);
static bool Check_robot_target(player_t *pl, clpos_t item_pos, int new_mode);
static bool Detect_hunt(player_t *pl, player_t *ship);
static int Rank_item_value(player_t *pl, long itemtype);
static bool Ball_handler(player_t *pl);


/*
 * Function to cast from player structure to robot data structure.
 * This isolates casts (aka. type violations) to a few places.
 */
static robot_default_data_t *Robot_default_get_data(player_t *pl)
{
    return (robot_default_data_t *)pl->robot_data_ptr->private_data;
}

/*
 * A default robot is created.
 */
static void Robot_default_create(player_t *pl, char *str)
{
    robot_default_data_t	*my_data;
    world_t *world = &World;

    if (!(my_data = (robot_default_data_t *)malloc(sizeof(*my_data)))) {
	error("no mem for default robot");
	End_game();
    }

    my_data->robot_mode      = RM_TAKE_OFF;
    my_data->robot_count     = 0;
    my_data->robot_lock      = LOCK_NONE;
    my_data->robot_lock_id   = 0;
    my_data->longterm_mode   = 0;

    if (str != NULL
	&& *str != '\0'
	&& sscanf(str, " %d %d", &my_data->attack, &my_data->defense) != 2) {
	if (str && *str) {
	    xpprintf("%s invalid parameters for default robot: \"%s\"\n", showtime(), str);
	    my_data->attack = (int)(rfrac() * 99.5f);
	    my_data->defense = 100 - my_data->attack;
	}
	LIMIT(my_data->attack, 1, 99);
	LIMIT(my_data->defense, 1, 99);
    }
    /*
     * some parameters which may be changed to be dependent upon
     * the `attack' and `defense' settings of this robot.
     */
    if (BIT(world->rules->mode, TIMING)) {
	my_data->robot_normal_speed = 10.0;
	my_data->robot_attack_speed = 25.0 + (my_data->attack / 10);
	my_data->robot_max_speed = 50.0 + (my_data->attack / 20) - (my_data->defense / 50);
    } else {
	my_data->robot_normal_speed = 6.0;
	my_data->robot_attack_speed = 15.0 + (my_data->attack / 25);
	my_data->robot_max_speed = 30.0 + (my_data->attack / 50) - (my_data->defense / 50);
    }

    pl->fuel.l3 += my_data->defense - my_data->attack + (int)((rfrac() - 0.5f) * 20);
    pl->fuel.l2 += 2 * (my_data->defense - my_data->attack) / 5 + (int)((rfrac() - 0.5f) * 8);
    pl->fuel.l1 += (my_data->defense - my_data->attack) / 5 + (int)((rfrac() - 0.5f) * 4);

    my_data->last_used_ecm	= 0;
    my_data->last_dropped_mine	= 0;
    my_data->last_fired_missile	= 0;
    my_data->last_thrown_ball	= 0;

    my_data->longterm_mode	= 0;

    pl->robot_data_ptr->private_data = (void *)my_data;
}

/*
 * A default robot is placed on its homebase.
 */
static void Robot_default_go_home(player_t *pl)
{
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    my_data->robot_mode      = RM_TAKE_OFF;
    my_data->longterm_mode   = 0;
}

/*
 * A default robot is declaring war (or resetting war).
 */
static void Robot_default_set_war(player_t *pl, int victim_id)
{
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    if (victim_id == NO_ID)
	CLR_BIT(my_data->robot_lock, LOCK_PLAYER);
    else {
	my_data->robot_lock_id = victim_id;
	SET_BIT(my_data->robot_lock, LOCK_PLAYER);
    }
}

/*
 * Return the id of the player a default robot has war against (or NO_ID).
 */
static int Robot_default_war_on_player(player_t *pl)
{
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    if (BIT(my_data->robot_lock, LOCK_PLAYER))
	return my_data->robot_lock_id;
    else
	return NO_ID;
}

/*
 * A default robot receives a message.
 */
static void Robot_default_message(player_t *pl, const char *message)
{
#if 0
    robot_default_data_t	*my_data = Robot_default_get_data(pl);
    int				len;
    char			*ptr;
    char			sender_name[MAX_NAME_LEN];
    char			msg[MSG_LEN];

    /*
     * Extract the message body and the sender name from the message string.
     */

    ptr = strrchr(message, ']');/* get end of message */
    if (!ptr) {
	return;		/* Make sure to ignore server messages, oops! */
    }
    while (*--ptr != '[');	/* skip back over receiver name */
    while (*--ptr != '[');	/* skip back over sender name */
    strlcpy(sender_name, ptr + 1, sizeof sender_name);	/* copy sender */
    len = ptr - message;	/* find read message length */
    if (message[len] == ' ') {	/* ignore the readability space */
	len--;
    }
    strlcpy(msg, message, len);	/* copy real message */
    if ((ptr = strchr(sender_name, ']')) != NULL) {
	*ptr = '\0';	/* remove the ']' separator */
    }
    printf("%s got message \"%s\" from \"%s\"\n", pl->name, msg, sender_name);
#else
    UNUSED_PARAM(pl); UNUSED_PARAM(message);
#endif
}

/*
 * A default robot is destroyed.
 */
static void Robot_default_destroy(player_t *pl)
{
    free(pl->robot_data_ptr->private_data);
    pl->robot_data_ptr->private_data = NULL;
}

/*
 * A default robot is asked to join an alliance
 */
static void Robot_default_invite(player_t *pl, player_t *inviter)
{
    int				war_id = Robot_default_war_on_player(pl);
    robot_default_data_t	*my_data = Robot_default_get_data(pl);
    int				i;
    double			limit;
    bool			we_accept = true;

    if (pl->alliance != ALLIANCE_NOT_SET) {
	/* if there is a human in our alliance, they should decide
	   let robots refuse in this case */
	for (i = 0; i < NumPlayers; i++) {
	    player_t *pl_i = Players(i);
	    if (Player_is_human(pl_i) && Players_are_allies(pl, pl_i)) {
		we_accept = false;
		break;
	    }
	}
	if (!we_accept) {
	    Refuse_alliance(pl, inviter);
	    return;
	}
    }
    limit = MAX(ABS(pl->score / MAX((my_data->attack / 10), 10)),
		my_data->defense);
    if (inviter->alliance == ALLIANCE_NOT_SET) {
	/* don't accept players we are at war with */
	if (inviter->id == war_id)
	    we_accept = false;
	/* don't accept players who are not active */
	if (!Player_is_active(inviter))
	    we_accept = false;
	/* don't accept players with scores substantially lower than ours */
	else if (inviter->score < (pl->score - limit))
	    we_accept = false;
    }
    else {
	double	avg_score = 0;
	int	member_count = Get_alliance_member_count(inviter->alliance);

	for (i = 0; i < NumPlayers; i++) {
	    player_t *pl_i = Players(i);
	    if (pl_i->alliance == inviter->alliance) {
		if (pl_i->id == war_id) {
		    we_accept = false;
		    break;
		}
		avg_score += pl_i->score;
	    }
	}
	if (we_accept) {
	    avg_score = avg_score / member_count;
	    if (avg_score < (pl->score - limit))
		we_accept = false;
	}
    }
    if (we_accept)
	Accept_alliance(pl, inviter);
    else
	Refuse_alliance(pl, inviter);
}



static bool Really_empty_space(player_t *pl, int x, int y)
{
    int group, cx, cy, i, j;
    int delta = BLOCK_CLICKS / 4;
    int inside = 0, outside = 0;
    hitmask_t hitmask = NONBALL_BIT; /* kps - ok ? */

    UNUSED_PARAM(pl);
    /*
     * kps hack - check a few positions inside the block, if none of them
     * are inside, assume it is empty
     */
    cx = BLOCK_CENTER(x);
    cy = BLOCK_CENTER(y);

    for (i = -1; i <= 1; i++) {
	for (j = -1; j <= 1; j++) {
	    group = is_inside(cx + i * delta, cy + j * delta, hitmask, NULL);
	    if (group != NO_GROUP)
		inside++;
	    else
		outside++;
	}
    }

    if (inside > 0)
	return false;
    return true;


    /* blockbased:

       int	type = world->block[x][y];

    if (EMPTY_SPACE(type))
	return true;
    switch (type) {
    case FILLED:
    case REC_LU:
    case REC_LD:
    case REC_RU:
    case REC_RD:
    case FUEL:
    case TREASURE:
	return false;

    case WORMHOLE:
	if (!options.wormholeVisible
	    || world->wormHoles[Map_get_itemid(x, y)].type == WORM_OUT) {
	    return true;
	} else {
	    return false;
	}

    case TARGET:
	if (!options.targetTeamCollision
	    && BIT(world->rules->mode, TEAM_PLAY)
	    && world->targets[Map_get_itemid(x, y)].team == pl->team) {
	    return true;
	} else {
	    return false;
	}

    case CANNON:
	if (options.teamImmunity
	    && BIT(world->rules->mode, TEAM_PLAY)
	    && world->cannon[Map_get_itemid(x, y)].team == pl->team) {
	    return true;
	} else {
	    return false;
	}

    default:
	break;
    }
    return false;*/
}

static inline int decide_travel_dir(player_t *pl)
{
    world_t *world = &World;

    if (pl->velocity <= 0.2) {
	vector_t grav = World_gravity(world, pl->pos);

	return findDir(grav.x, grav.y);
    }
    return findDir(pl->vel.x, pl->vel.y);
}


static bool Check_robot_evade(player_t *pl, int mine_i, int ship_i)
{
    int				i;
    object_t			*shot;
    player_t			*ship;
    long			stop_dist;
    bool			evade;
    bool			left_ok, right_ok;
    int				safe_width;
    int				travel_dir;
    int				delta_dir;
    int				aux_dir;
    int				px[3], py[3];
    long			dist;
    vector_t			grav;
    int				gravity_dir;
    long			dx, dy;
    double			velocity;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);
    world_t *world = &World;

    safe_width = (my_data->defense / 200) * SHIP_SZ;
    /* Prevent overflow. */
    velocity = (pl->velocity <= SPEED_LIMIT) ? pl->velocity : SPEED_LIMIT;
    stop_dist =
	(long)((RES * velocity) / (MAX_PLAYER_TURNSPEED * pl->turnresistance)
	+ (velocity * velocity * pl->mass) / (2 * MAX_PLAYER_POWER)
	+ safe_width);
    /*
     * Limit the look ahead.  For very high speeds the current code
     * is ineffective and much too inefficient.
     */
    if (stop_dist > 10 * BLOCK_SZ)
	stop_dist = 10 * BLOCK_SZ;

    evade = false;

    travel_dir = decide_travel_dir(pl);

    aux_dir = MOD2(travel_dir + RES / 4, RES);
    px[0] = CLICK_TO_PIXEL(pl->pos.cx);		/* ship center x */
    py[0] = CLICK_TO_PIXEL(pl->pos.cy);		/* ship center y */
    px[1] = (int)(px[0] + safe_width * tcos(aux_dir));	/* ship left side x */
    py[1] = (int)(py[0] + safe_width * tsin(aux_dir));	/* ship left side y */
    px[2] = 2 * px[0] - px[1];	/* ship right side x */
    py[2] = 2 * py[0] - py[1];	/* ship right side y */

    left_ok = true;
    right_ok = true;

    for (dist = 0; dist < stop_dist + BLOCK_SZ / 2; dist += BLOCK_SZ / 2) {
	for (i = 0; i < 3; i++) {
	    clpos_t d;

	    dx = (long)((px[i] + dist * tcos(travel_dir)) / BLOCK_SZ);
	    dy = (long)((py[i] + dist * tsin(travel_dir)) / BLOCK_SZ);

	    dx = WRAP_XBLOCK(dx);
	    dy = WRAP_YBLOCK(dy);
	    d.cx = BLOCK_CENTER(dx);
	    d.cy = BLOCK_CENTER(dy);

	    if (!World_contains_clpos(world, d)) {
		evade = true;
		if (i == 1)
		    left_ok = false;
		if (i == 2)
		    right_ok = false;
		continue;
	    }
	    if (!Really_empty_space(pl, dx, dy)) {
		evade = true;
		if (i == 1)
		    left_ok = false;
		if (i == 2)
		    right_ok = false;
		continue;
	    }
	    /* Watch out for strong gravity */
	    grav = World_gravity(world, d);
	    if (sqr(grav.x) + sqr(grav.y) >= 0.5) {
		gravity_dir = findDir(grav.x - CLICK_TO_PIXEL(pl->pos.cx),
				      grav.y - CLICK_TO_PIXEL(pl->pos.cy));
		if (MOD2(gravity_dir - travel_dir, RES) <= RES / 4 ||
		    MOD2(gravity_dir - travel_dir, RES) >= 3 * RES / 4) {
		    evade = true;
		    if (i == 1)
			left_ok = false;
		    if (i == 2)
			right_ok = false;
		    continue;
		}
	    }
	}
    }

    if (mine_i >= 0) {
	shot = Obj[mine_i];
	aux_dir = Wrap_cfindDir(shot->pos.cx + PIXEL_TO_CLICK(shot->vel.x)
				- pl->pos.cx,
				shot->pos.cy + PIXEL_TO_CLICK(shot->vel.y)
				- pl->pos.cy);
	delta_dir = MOD2(aux_dir - travel_dir, RES);
	if (delta_dir < RES / 4) {
	    left_ok = false;
	    evade = true;
	}
	if (delta_dir > RES * 3 / 4) {
	    right_ok = false;
	    evade = true;
	}
    }
    if (ship_i >= 0) {
	ship = Players(ship_i);
	aux_dir = Wrap_cfindDir(ship->pos.cx - pl->pos.cx
				+ PIXEL_TO_CLICK(ship->vel.x * 2),
				ship->pos.cy - pl->pos.cy
				+ PIXEL_TO_CLICK(ship->vel.y * 2));
	delta_dir = MOD2(aux_dir - travel_dir, RES);
	if (delta_dir < RES / 4) {
	    left_ok = false;
	    evade = true;
	}
	if (delta_dir > RES * 3 / 4) {
	    right_ok = false;
	    evade = true;
	}
    }
    if (pl->velocity > my_data->robot_max_speed)
	evade = true;

    if (!evade)
	return false;

    delta_dir = 0;
    while (!left_ok && !right_ok && delta_dir < 7 * RES / 8) {
	delta_dir += RES / 16;

	left_ok = true;
	aux_dir = MOD2(travel_dir + delta_dir, RES);
	for (dist = 0; dist < stop_dist + BLOCK_SZ / 2; dist += BLOCK_SZ / 2) {
	    clpos_t d;

	    dx = (long)((px[0] + dist * tcos(aux_dir)) / BLOCK_SZ);
	    dy = (long)((py[0] + dist * tsin(aux_dir)) / BLOCK_SZ);

	    dx = WRAP_XBLOCK(dx);
	    dy = WRAP_YBLOCK(dy);
	    d.cx = BLOCK_CENTER(dx);
	    d.cy = BLOCK_CENTER(dy);

	    if (!World_contains_clpos(world, d)) {
		left_ok = false;
		continue;
	    }
	    if (!Really_empty_space(pl, dx, dy)) {
		left_ok = false;
		continue;
	    }
	    /* watch out for strong gravity */
	    grav = World_gravity(world, d);
	    if (sqr(grav.x) + sqr(grav.y) >= 0.5) {
		gravity_dir = findDir(grav.x - CLICK_TO_PIXEL(pl->pos.cx),
				      grav.y - CLICK_TO_PIXEL(pl->pos.cy));
		if (MOD2(gravity_dir - travel_dir, RES) <= RES / 4 ||
		    MOD2(gravity_dir - travel_dir, RES) >= 3 * RES / 4) {

		    left_ok = false;
		    continue;
		}
	    }
	}

	right_ok = true;
	aux_dir = MOD2(travel_dir - delta_dir, RES);
	for (dist = 0; dist < stop_dist + BLOCK_SZ / 2; dist += BLOCK_SZ / 2) {
	    clpos_t d;

	    dx = (long)((px[0] + dist * tcos(aux_dir)) / BLOCK_SZ);
	    dy = (long)((py[0] + dist * tsin(aux_dir)) / BLOCK_SZ);

	    dx = WRAP_XBLOCK(dx);
	    dy = WRAP_YBLOCK(dy);
	    d.cx = BLOCK_CENTER(dx);
	    d.cy = BLOCK_CENTER(dy);

	    if (!World_contains_clpos(world, d)) {
		right_ok = false;
		continue;
	    }
	    if (!Really_empty_space(pl, dx, dy)) {
		right_ok = false;
		continue;
	    }
	    /* watch out for strong gravity */
	    grav = World_gravity(world, d);
	    if (sqr(grav.x) + sqr(grav.y) >= 0.5) {
		gravity_dir = findDir(grav.x - CLICK_TO_PIXEL(pl->pos.cx),
				      grav.y - CLICK_TO_PIXEL(pl->pos.cy));
		if (MOD2(gravity_dir - travel_dir, RES) <= RES / 4 ||
		    MOD2(gravity_dir - travel_dir, RES) >= 3 * RES / 4) {

		    right_ok = false;
		    continue;
		}
	    }
	}
    }

    pl->turnspeed = MAX_PLAYER_TURNSPEED;
    pl->power = MAX_PLAYER_POWER;

    delta_dir = MOD2(pl->dir - travel_dir, RES);

    if (my_data->robot_mode != RM_EVADE_LEFT
	&& my_data->robot_mode != RM_EVADE_RIGHT) {
	if (left_ok && !right_ok)
	    my_data->robot_mode = RM_EVADE_LEFT;
	else if (right_ok && !left_ok)
	    my_data->robot_mode = RM_EVADE_RIGHT;
	else
	    my_data->robot_mode = (delta_dir < RES / 2 ?
			      RM_EVADE_LEFT : RM_EVADE_RIGHT);
    }
    /*-BA If facing the way we want to go, thrust
     *-BA If too far off, stop thrusting
     *-BA If in between, keep doing whatever we are already doing
     *-BA In all cases continue to straighten up
     */
    if (delta_dir < RES / 4 || delta_dir > 3 * RES / 4) {
	pl->turnacc = (my_data->robot_mode == RM_EVADE_LEFT ?
		       pl->turnspeed : (-pl->turnspeed));
	CLR_BIT(pl->status, THRUSTING);
    }
    else if (delta_dir < 3 * RES / 8 || delta_dir > 5 * RES / 8)
	pl->turnacc = (my_data->robot_mode == RM_EVADE_LEFT ?
		       pl->turnspeed : (-pl->turnspeed));
    else {
	pl->turnacc = 0;
	SET_BIT(pl->status, THRUSTING);
	my_data->robot_mode = (delta_dir < RES/2
			       ? RM_EVADE_LEFT : RM_EVADE_RIGHT);
    }

    return true;
}

static void Robot_check_new_modifiers(player_t *pl, modifiers_t mods)
{
    world_t *world = &World;

    if (!BIT(world->rules->mode, ALLOW_NUKES))
	mods.nuclear = 0;
    if (!BIT(world->rules->mode, ALLOW_CLUSTERS))
	CLR_BIT(mods.warhead, CLUSTER);
    if (!BIT(world->rules->mode, ALLOW_MODIFIERS)) {
	mods.velocity =
	mods.mini =
	mods.spread =
	mods.power = 0;
	CLR_BIT(mods.warhead, IMPLOSION);
    }
    if (!BIT(world->rules->mode, ALLOW_LASER_MODIFIERS))
	mods.laser = 0;
    pl->mods = mods;
}

static void Choose_weapon_modifier(player_t *pl, int weapon_type)
{
    int				stock, min;
    modifiers_t			mods;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    CLEAR_MODS(mods);

    switch (weapon_type) {
    case HAS_TRACTOR_BEAM:
	Robot_check_new_modifiers(pl, mods);
	return;

    case HAS_LASER:
	/*
	 * Robots choose non-damage laser settings occasionally.
	 */
	if ((my_data->robot_count % 4) == 0)
	    mods.laser = (int)(rfrac() * (MODS_LASER_MAX + 1));
	Robot_check_new_modifiers(pl, mods);
	return;

    case OBJ_SHOT:
	/*
	 * Robots usually use wide beam shots, however they may narrow
	 * the beam occasionally.
	 */
	mods.spread = 0;
	if ((my_data->robot_count % 4) == 0)
	    mods.spread = (int)(rfrac() * (MODS_SPREAD_MAX + 1));
	Robot_check_new_modifiers(pl, mods);
	return;

    case OBJ_MINE:
	stock = pl->item[ITEM_MINE];
	min = options.nukeMinMines;
	break;

    case OBJ_SMART_SHOT:
    case OBJ_HEAT_SHOT:
    case OBJ_TORPEDO:
	stock = pl->item[ITEM_MISSILE];
	min = options.nukeMinSmarts;
	if ((my_data->robot_count % 4) == 0)
	    mods.power = (int)(rfrac() * (MODS_POWER_MAX + 1));
	break;

    default:
	return;
    }

    if (stock >= min) {
	/*
	 * More aggressive robots will choose to use nuclear weapons, this
	 * means you can safely approach wimpy robots... perhaps.
	 */
	if ((my_data->robot_count % 100) <= my_data->attack) {
	    SET_BIT(mods.nuclear, NUCLEAR);
	    if (stock > min && (stock < (2 * min)
				|| (my_data->robot_count % 2) == 0))
		    SET_BIT(mods.nuclear, FULLNUCLEAR);
	}
    }

    if (pl->fuel.sum > pl->fuel.l3) {
	if ((my_data->robot_count % 2) == 0) {
	    if ((my_data->robot_count % 8) == 0)
		mods.velocity = (int)(rfrac() * MODS_VELOCITY_MAX) + 1;
	    SET_BIT(mods.warhead, CLUSTER);
	}
    }
    else if ((my_data->robot_count % 3) == 0)
	SET_BIT(mods.warhead, IMPLOSION);

    /*
     * Robot may change to use mini device setting occasionally.
     */
    if ((my_data->robot_count % 10) == 0) {
	mods.mini = (int)(rfrac() * (MODS_MINI_MAX + 1));
	mods.spread = (int)(rfrac() * (MODS_SPREAD_MAX + 1));
    }

    Robot_check_new_modifiers(pl, mods);
}

static bool Check_robot_target(player_t *pl, clpos_t item_pos, int new_mode)
{
    player_t			*ship;
    long			item_dist;
    int				item_dir;
    int				travel_dir;
    int				delta_dir;
    long			dx, dy;
    long			dist;
    bool			clear_path;
    bool			slowing;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);
    world_t *world = &World;

    dx = CLICK_TO_PIXEL(item_pos.cx - pl->pos.cx), dx = WRAP_DX(dx);
    dy = CLICK_TO_PIXEL(item_pos.cy - pl->pos.cy), dy = WRAP_DY(dy);

    item_dist = LENGTH(dy, dx);

    if (dx == 0 && dy == 0) {
	vector_t grav = World_gravity(world, pl->pos);
	item_dir = findDir(grav.x, grav.y);
	item_dir = MOD2(item_dir + RES/2, RES);
    } else
	item_dir = findDir((double)dx, (double)dy);

    if (new_mode == RM_REFUEL)
	item_dist = item_dist - 90;

    clear_path = true;

    for (dist = 0; clear_path && dist < item_dist; dist += BLOCK_SZ / 2) {

	dx = (long)((CLICK_TO_PIXEL(pl->pos.cx)
		     + dist * tcos(item_dir)) / BLOCK_SZ);
	dy = (long)((CLICK_TO_PIXEL(pl->pos.cy)
		     + dist * tsin(item_dir)) / BLOCK_SZ);

	dx = WRAP_XBLOCK(dx);
	dy = WRAP_YBLOCK(dy);
	if (dx < 0 || dx >= world->x || dy < 0 || dy >= world->y) {
	    clear_path = false;
	    continue;
	}
	if (!Really_empty_space(pl, dx, dy)) {
	    clear_path = false;
	    continue;
	}
    }

    if (new_mode == RM_CANNON_KILL)
	item_dist -= 4 * BLOCK_SZ;

    if (!clear_path && new_mode != RM_NAVIGATE)
	return false;

    travel_dir = decide_travel_dir(pl);

    pl->turnspeed = MAX_PLAYER_TURNSPEED / 2;
    pl->power = (BIT(world->rules->mode, TIMING) ?
		 MAX_PLAYER_POWER :
		 MAX_PLAYER_POWER / 2);

    delta_dir = MOD2(item_dir - travel_dir, RES);
    if (delta_dir >= RES/4 && delta_dir <= 3*RES/4) {

	if (new_mode == RM_HARVEST ||
	    (new_mode == RM_NAVIGATE &&
		(clear_path || dist > 8 * BLOCK_SZ)))
	    /* reverse direction of travel */
	    item_dir = MOD2(travel_dir + (delta_dir > RES / 2
					    ? -5 * RES / 8
					    : 5 * RES / 8), RES);
	pl->turnspeed = MAX_PLAYER_TURNSPEED;
	slowing = true;

	if (pl->item[ITEM_MINE] && item_dist < 8 * BLOCK_SZ) {
	    Choose_weapon_modifier(pl, OBJ_MINE);
	    if (BIT(world->rules->mode, TIMING))
		Place_mine(pl);
	    else
		Place_moving_mine(pl);
	    new_mode = (rfrac() < 0.5f) ? RM_EVADE_RIGHT : RM_EVADE_LEFT;
	}
    } else if (new_mode == RM_CANNON_KILL && item_dist <= 0) {

	/* too close, so move away */
	pl->turnspeed = MAX_PLAYER_TURNSPEED;
	item_dir = MOD2(item_dir + RES / 2, RES);
	slowing = true;
    } else
	slowing = false;

    if (new_mode == RM_NAVIGATE && !clear_path) {
	if (dist <= 8 * BLOCK_SZ && dist > 4 * BLOCK_SZ)
	    item_dir = MOD2(item_dir + (delta_dir > RES / 2
					? -3 * RES / 4 : 3 * RES / 4), RES);
	else if (dist <= 4 * BLOCK_SZ)
	    item_dir = MOD2(item_dir + RES / 2, RES);
	pl->turnspeed = MAX_PLAYER_TURNSPEED;
	slowing = true;
    }

    delta_dir = MOD2(item_dir - pl->dir, RES);

    if (delta_dir > RES / 8 && delta_dir < 7 * RES / 8)
	pl->turnspeed = MAX_PLAYER_TURNSPEED;
    else if (delta_dir > RES / 16 && delta_dir < 15 * RES / 16)
	pl->turnspeed = MAX_PLAYER_TURNSPEED;
    else if (delta_dir > RES / 64 && delta_dir < 63 * RES / 64)
	pl->turnspeed = MAX_PLAYER_TURNSPEED;
    else
	pl->turnspeed = 0.0;

    pl->turnacc = (delta_dir < RES / 2 ? pl->turnspeed : (-pl->turnspeed));

    if (slowing || BIT(pl->used, HAS_SHIELD))
	SET_BIT(pl->status, THRUSTING);
    else if (item_dist < 0)
	CLR_BIT(pl->status, THRUSTING);
    else if (item_dist < 3*BLOCK_SZ && new_mode != RM_HARVEST) {

	if (pl->velocity < my_data->robot_normal_speed / 2)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > my_data->robot_normal_speed)
	    CLR_BIT(pl->status, THRUSTING);

    } else if ((new_mode != RM_ATTACK
		&& new_mode != RM_NAVIGATE)
	    || item_dist < 8*BLOCK_SZ
	    || (new_mode == RM_NAVIGATE
		&& delta_dir > 3 * RES / 8
		&& delta_dir < 5 * RES / 8)) {

	if (pl->velocity < 2*my_data->robot_normal_speed)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > 3*my_data->robot_normal_speed)
	    CLR_BIT(pl->status, THRUSTING);

    } else if (new_mode == RM_ATTACK
	    || (new_mode == RM_NAVIGATE
		&& (dist < 12 * BLOCK_SZ
		    || (delta_dir > RES / 8
			&& delta_dir < 7 * RES / 8)))) {

	if (pl->velocity < my_data->robot_attack_speed / 2)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > my_data->robot_attack_speed)
	    CLR_BIT(pl->status, THRUSTING);
    } else if (clear_path
	    && (delta_dir < RES / 8
		|| delta_dir > 7 * RES / 8)
	    && item_dist > 18 * BLOCK_SZ) {
	if (pl->velocity
	    < my_data->robot_max_speed - my_data->robot_normal_speed)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > my_data->robot_max_speed)
	    CLR_BIT(pl->status, THRUSTING);
    } else {
	if (pl->velocity < my_data->robot_attack_speed)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity
	    > my_data->robot_max_speed - my_data->robot_normal_speed)
	    CLR_BIT(pl->status, THRUSTING);
    }

    if (new_mode == RM_ATTACK
	|| (BIT(world->rules->mode, TIMING)
	    && new_mode == RM_NAVIGATE)) {
	if (pl->item[ITEM_ECM] > 0
	    && item_dist < ECM_DISTANCE / 4)
	    Fire_ecm(pl);
	else if (pl->item[ITEM_TRANSPORTER] > 0
		 && item_dist < TRANSPORTER_DISTANCE
		 && pl->fuel.sum > -ED_TRANSPORTER)
	    Do_transporter(pl);
	else if (pl->item[ITEM_LASER] > pl->num_pulses
		 && pl->fuel.sum + ED_LASER > pl->fuel.l3
		 && new_mode == RM_ATTACK) {
	    if (BIT(my_data->robot_lock, LOCK_PLAYER)
		&& Player_is_active(Player_by_id(my_data->robot_lock_id)))
		ship = Player_by_id(my_data->robot_lock_id);
	    else if (BIT(pl->lock.tagged, LOCK_PLAYER))
		ship = Player_by_id(pl->lock.pl_id);
	    else
		ship = NULL;

	    if (ship && Player_is_active(ship)) {

		double	x2, y2, x3, y3, x4, y4, x5, y5;
		double	ship_dist, dir3, dir4, dir5;
		clpos_t m_gun;

		m_gun = Ship_get_m_gun_clpos(pl->ship, pl->dir);
		x2 = CLICK_TO_PIXEL(pl->pos.cx) + pl->vel.x
		    + CLICK_TO_PIXEL(m_gun.cx);
		y2 = CLICK_TO_PIXEL(pl->pos.cy) + pl->vel.y
		    + CLICK_TO_PIXEL(m_gun.cy);
		x3 = CLICK_TO_PIXEL(ship->pos.cx) + ship->vel.x;
		y3 = CLICK_TO_PIXEL(ship->pos.cy) + ship->vel.y;

		ship_dist = Wrap_length(PIXEL_TO_CLICK(x3 - x2),
					PIXEL_TO_CLICK(y3 - y2)) / CLICK;

		/* kps -
		   changed PULSE_LIFE(pl->item[ITEM_LASER]) to options.pulseLife */
		if (ship_dist < options.pulseSpeed * options.pulseLife + SHIP_SZ) {
		    dir3 = Wrap_findDir(x3 - x2, y3 - y2);
		    x4 = x3 + tcos(MOD2((int)(dir3 - RES/4), RES)) * SHIP_SZ;
		    y4 = y3 + tsin(MOD2((int)(dir3 - RES/4), RES)) * SHIP_SZ;
		    x5 = x3 + tcos(MOD2((int)(dir3 + RES/4), RES)) * SHIP_SZ;
		    y5 = y3 + tsin(MOD2((int)(dir3 + RES/4), RES)) * SHIP_SZ;
		    dir4 = Wrap_findDir(x4 - x2, y4 - y2);
		    dir5 = Wrap_findDir(x5 - x2, y5 - y2);
		    if ((dir4 > dir5)
			? (pl->dir >= dir4 || pl->dir <= dir5)
			: (pl->dir >= dir4 && pl->dir <= dir5))
			SET_BIT(pl->used, HAS_LASER);
		}
	    }
	}
	else if (BIT(pl->have, HAS_TRACTOR_BEAM)) {
	    CLR_BIT(pl->used, HAS_TRACTOR_BEAM);
	    pl->tractor_is_pressor = false;

	    if (BIT(pl->lock.tagged, LOCK_PLAYER)
		&& pl->fuel.sum > pl->fuel.l3
		&& pl->lock.distance
		   < TRACTOR_MAX_RANGE(pl->item[ITEM_TRACTOR_BEAM])) {

		double xvd, yvd, vel;
		long dir;
		int away;

		ship = Player_by_id(pl->lock.pl_id);
		xvd = ship->vel.x - pl->vel.x;
		yvd = ship->vel.y - pl->vel.y;
		vel = LENGTH(xvd, yvd);
		dir = (long)(Wrap_cfindDir(pl->pos.cx - ship->pos.cx,
					   pl->pos.cy - ship->pos.cy)
			     - findDir(xvd, yvd));
		dir = MOD2(dir, RES);
		away = (dir >= RES/4 && dir <= 3*RES/4);

		/*
		 * vel  - The relative velocity of ship to us.
		 * away - Heading away from us?
		 */
		if (pl->velocity <= my_data->robot_normal_speed) {
		    if (pl->lock.distance < (SHIP_SZ * 4)
			|| (!away && vel > my_data->robot_attack_speed)) {
			SET_BIT(pl->used, HAS_TRACTOR_BEAM);
			pl->tractor_is_pressor = true;
		    } else if (away
			       && vel < my_data->robot_max_speed
			       && vel > my_data->robot_normal_speed)
			SET_BIT(pl->used, HAS_TRACTOR_BEAM);
		}
		if (BIT(pl->used, HAS_TRACTOR_BEAM))
		    SET_BIT(pl->lock.tagged, LOCK_VISIBLE);
	    }
	}
	if (BIT(pl->used, HAS_LASER)) {
	    pl->turnacc = 0.0;
	    Choose_weapon_modifier(pl, HAS_LASER);
	}
	/*-BA Be more agressive, esp if lots of ammo
	 * else if ((my_data->robot_count % 10) == 0
	 * && pl->item[ITEM_MISSILE] > 0)
	 */
	else if ((my_data->robot_count % 10) < pl->item[ITEM_MISSILE]
		  && !WITHIN(my_data->robot_count,
			     my_data->last_fired_missile,10)) {
	    int type;

	    switch (my_data->robot_count % 5) {
	    case 0: case 1: case 2:	type = OBJ_SMART_SHOT; break;
	    case 3:			type = OBJ_HEAT_SHOT; break;
	    default:			type = OBJ_TORPEDO; break;
	    }
	    if (Detect_hunt(pl, Player_by_id(pl->lock.pl_id))
		&& !pl->visibility[GetInd(pl->lock.pl_id)].canSee)
		type = OBJ_HEAT_SHOT;
	    if (type == OBJ_SMART_SHOT && !options.allowSmartMissiles)
		type = OBJ_HEAT_SHOT;
	    Choose_weapon_modifier(pl, type);
	    Fire_shot(pl, type, pl->dir);
	    if (type == OBJ_HEAT_SHOT)
		CLR_BIT(pl->status, THRUSTING);
	    my_data->last_fired_missile=my_data->robot_count;
	}
	else if ((my_data->robot_count % 2) == 0
		   && item_dist < Visibility_distance
		   /*&& BIT(my_data->robot_lock, LOCK_PLAYER)*/){
	    if ((int)(rfrac() * 64) < pl->item[ITEM_MISSILE] ) {
		Choose_weapon_modifier(pl, OBJ_SMART_SHOT);
		Fire_shot(pl, OBJ_SMART_SHOT, pl->dir);
		my_data->last_fired_missile=my_data->robot_count;
	    } else {
		if ((new_mode == RM_ATTACK && clear_path)
		    || (my_data->robot_count % 50) == 0) {
		    Choose_weapon_modifier(pl, OBJ_SHOT);
		    Fire_normal_shots(pl);
		}
	    }
	}
	/*-BA Be more agressive, esp if lots of ammo
	 * if ((my_data->robot_count % 32) == 0)
	 */
	else if ((my_data->robot_count % 32) < pl->item[ITEM_MINE]
		  && !WITHIN(my_data->robot_count,
			     my_data->last_dropped_mine, 10)) {
	    if (pl->fuel.sum > pl->fuel.l3) {
		Choose_weapon_modifier(pl, OBJ_MINE);
		Place_mine(pl);
	    } else /*if (pl->fuel.sum < pl->fuel.l2)*/ {
		Place_mine(pl);
		CLR_BIT(pl->used, HAS_CLOAKING_DEVICE);
	    }
	    my_data->last_dropped_mine=my_data->robot_count;
	}
    }
    if (new_mode == RM_CANNON_KILL && !slowing) {
	if ((my_data->robot_count % 2) == 0
	    && item_dist < Visibility_distance
	    && clear_path) {
	    Choose_weapon_modifier(pl, OBJ_SHOT);
	    Fire_normal_shots(pl);
	}
    }
    my_data->robot_mode = new_mode;
    return true;
}


static bool Check_robot_hunt(player_t *pl)
{
    player_t			*ship;
    int				ship_dir;
    int				travel_dir;
    int				delta_dir;
    int				adj_dir;
    int				toofast, tooslow;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    if (!BIT(my_data->robot_lock, LOCK_PLAYER)
	|| my_data->robot_lock_id == pl->id)
	return false;
    if (pl->fuel.sum < pl->fuel.l3 /*MAX_PLAYER_FUEL/2*/)
	return false;
    ship = Player_by_id(my_data->robot_lock_id);
    if (!Detect_hunt(pl, ship))
	return false;

    ship_dir = Wrap_cfindDir(ship->pos.cx - pl->pos.cx,
			     ship->pos.cy - pl->pos.cy);

    travel_dir = decide_travel_dir(pl);

    delta_dir = MOD2(ship_dir - travel_dir, RES);
    tooslow = (pl->velocity < my_data->robot_attack_speed/2);
    toofast = (pl->velocity > my_data->robot_attack_speed);

    if (!tooslow && !toofast
	&& (delta_dir <= RES/16 || delta_dir >= 15*RES/16)) {

	pl->turnacc = 0;
	CLR_BIT(pl->status, THRUSTING);
	my_data->robot_mode = RM_ROBOT_IDLE;
	return true;
    }

    adj_dir = (delta_dir<RES/2 ? RES/4 : (-RES/4));

    if (tooslow) adj_dir = adj_dir/2;	/* point forwards more */
    if (toofast) adj_dir = 3*adj_dir/2;	/* point backwards more */

    adj_dir = MOD2(travel_dir + adj_dir, RES);
    delta_dir = MOD2(adj_dir - pl->dir, RES);

    if (delta_dir>=RES/16 && delta_dir<=15*RES/16) {
	pl->turnspeed = MAX_PLAYER_TURNSPEED/4;
	pl->turnacc = (delta_dir<RES/2 ? pl->turnspeed : (-pl->turnspeed));
    }

    if (delta_dir<RES/8 || delta_dir>7*RES/8)
	SET_BIT(pl->status, THRUSTING);
    else
	CLR_BIT(pl->status, THRUSTING);

    my_data->robot_mode = RM_ROBOT_IDLE;
    return true;
}

static bool Detect_hunt(player_t *pl, player_t *ship)
{
    double distance;

    if (!Player_is_playing(ship))
	return false;		/* can't go after non-playing ships */

    if (BIT(ship->used, HAS_PHASING_DEVICE))
	return false;		/* can't do anything with phased ships */

    if (pl->visibility[GetInd(ship->id)].canSee)
	return true;		/* trivial */

    /* can't see it, so it must be cloaked
	maybe we can detect it's presence from other clues? */

    distance = Wrap_length(ship->pos.cx - pl->pos.cx,
			   ship->pos.cy - pl->pos.cy) / CLICK;
    if (distance > Visibility_distance)
	return false;		/* can't detect ships beyond visual range */

    if (BIT(ship->status, THRUSTING) && options.cloakedExhaust)
	return true;

    if (BIT(ship->used, HAS_SHOT|
			HAS_LASER|
			HAS_REFUEL|
			HAS_REPAIR|
			HAS_CONNECTOR|
			HAS_TRACTOR_BEAM))
	return true;

    if (BIT(ship->have, HAS_BALL))
	return true;

    return false;		/* the sky seems clear.. */
}

/*
 * Determine how important an item is to a given player.
 * Return one of the following 3 values:
 */
#define ROBOT_MUST_HAVE_ITEM	2	/* must have */
#define ROBOT_HANDY_ITEM	1	/* handy */
#define ROBOT_IGNORE_ITEM	0	/* ignore */
/*
 */
static int Rank_item_value(player_t *pl, long itemtype)
{
    world_t *world = &World;

    if (itemtype == ITEM_AUTOPILOT)
	return ROBOT_IGNORE_ITEM;		/* never useful for robots */
    if (pl->item[itemtype] >= world->items[itemtype].limit)
	return ROBOT_IGNORE_ITEM;		/* already full */
    if ((IsDefensiveItem(itemtype)
	 && CountDefensiveItems(pl) >= options.maxDefensiveItems)
	|| (IsOffensiveItem(itemtype)
	 && CountOffensiveItems(pl) >= options.maxOffensiveItems))
	return ROBOT_IGNORE_ITEM;
    if (itemtype == ITEM_FUEL) {
	if (pl->fuel.sum >= pl->fuel.max * 0.90)
	    return ROBOT_IGNORE_ITEM;		/* already (almost) full */
	else if ((pl->fuel.sum < (BIT(world->rules->mode, TIMING))
		  ? pl->fuel.l1
		  : pl->fuel.l2))
	    return ROBOT_MUST_HAVE_ITEM;		/* ahh fuel at last */
	else
	    return ROBOT_HANDY_ITEM;
    }
    if (BIT(world->rules->mode, TIMING)) {
	switch (itemtype) {
	case ITEM_AFTERBURNER:	/* the more speed the better */
	case ITEM_EMERGENCY_THRUST:	/* makes you go really fast */
	case ITEM_TRANSPORTER:	/* steal fuel when you overtake someone */
	case ITEM_REARSHOT:	/* shoot competitors behind you */
	case ITEM_MINE:		/* blows others off the track */
	case ITEM_ECM:		/* blinded players smash into walls */
	    return ROBOT_MUST_HAVE_ITEM;
	case ITEM_CLOAK:	/* not important in racemode */
	case ITEM_LASER:	/* cost too much fuel */
	case ITEM_MISSILE:	/* likely to hit self */
	case ITEM_SENSOR:	/* who cares about seeing others? */
	case ITEM_TANK:		/* makes you heavier */
	case ITEM_TRACTOR_BEAM:	/* pushes/pulls owner off the track too */
	case ITEM_HYPERJUMP:	/* likely to end up in wrong place */
	case ITEM_PHASING:	/* robots don't know how to use them yet */
	case ITEM_DEFLECTOR:	/* cost too much fuel */
	case ITEM_ARMOR:	/* makes you heavier */
	    return ROBOT_IGNORE_ITEM;
	default:		/* unknown */
	    warn("Rank_item_value: unknown item.");
	    return ROBOT_IGNORE_ITEM;
	}
    } else {
	switch (itemtype) {
	case ITEM_EMERGENCY_SHIELD:
	case ITEM_DEFLECTOR:
	case ITEM_ARMOR:
	    if (BIT(pl->have, HAS_SHIELD))
		return ROBOT_HANDY_ITEM;
	    else
		return ROBOT_MUST_HAVE_ITEM;

	case ITEM_REARSHOT:
	case ITEM_WIDEANGLE:
	    if (options.maxPlayerShots <= 0
		|| options.shotLife <= 0
		|| !options.playerKillings)
		return ROBOT_HANDY_ITEM;
	    else
		return ROBOT_MUST_HAVE_ITEM;

	case ITEM_MISSILE:
	    if (options.maxPlayerShots <= 0
		|| options.shotLife <= 0
		|| !options.playerKillings)
		return ROBOT_IGNORE_ITEM;
	    else
		return ROBOT_MUST_HAVE_ITEM;

	case ITEM_MINE:
	case ITEM_CLOAK:
	    return ROBOT_MUST_HAVE_ITEM;

	case ITEM_LASER:
	    if (options.playerKillings)
		return ROBOT_MUST_HAVE_ITEM;
	    else
		return ROBOT_HANDY_ITEM;

	case ITEM_PHASING:	/* robots don't know how to use them yet */
	    return ROBOT_IGNORE_ITEM;

	default:
	    break;
	}
    }
    return ROBOT_HANDY_ITEM;
}

static bool Ball_handler(player_t *pl)
{
    int		i,
		closest_tr = -1,
		closest_ntr = -1,
		dist,
		closest_tr_dist = INT_MAX,
		closest_ntr_dist = INT_MAX,
		bdir,
		tdir;
    bool	clear_path = true;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);
    world_t *world = &World;

    for (i = 0; i < world->NumTreasures; i++) {
	treasure_t *treasure = Treasures(world, i);

	if ((BIT(pl->have, HAS_BALL) || pl->ball)
	    && treasure->team == pl->team) {
	    dist = Wrap_length(treasure->pos.cx - pl->pos.cx,
			       treasure->pos.cy - pl->pos.cy) / CLICK;
	    if (dist < closest_tr_dist) {
		closest_tr = i;
		closest_tr_dist = dist;
	    }
	} else if (treasure->team != pl->team
		   && Teams(world, treasure->team)->NumMembers > 0
		   && !BIT(pl->have, HAS_BALL)
		   && !pl->ball
		   && treasure->have) {
	    dist = Wrap_length(treasure->pos.cx - pl->pos.cx,
			       treasure->pos.cy - pl->pos.cy) / CLICK;
	    if (dist < closest_ntr_dist) {
		closest_ntr = i;
		closest_ntr_dist = dist;
	    }
	}
    }
    if (BIT(pl->have, HAS_BALL) || pl->ball) {
	ballobject_t *ball = NULL;
	blkpos_t bbpos;
	int dist_np = INT_MAX;
	int xdist, ydist;
	int dx, dy;
	treasure_t *closest_treasure;

	if (pl->ball)
	    ball = pl->ball;
	else {
	    for (i = 0; i < NumObjs; i++) {
		if (BIT(Obj[i]->type, OBJ_BALL) && Obj[i]->id == pl->id) {
		    ball = BALL_PTR(Obj[i]);
		    break;
		}
	    }
	}
	for (i = 0; i < NumPlayers; i++) {
	    player_t *pl_i = Players(i);
	    dist = LENGTH(ball->pos.cx - pl_i->pos.cx,
			  ball->pos.cy - pl_i->pos.cy) / CLICK;
	    if (pl_i->id != pl->id
		&& Player_is_active(pl_i)
		&& dist < dist_np)
		dist_np = dist;
	}
	closest_treasure = Treasures(world, closest_tr);
	bdir = findDir(ball->vel.x, ball->vel.y);
	tdir = Wrap_cfindDir(closest_treasure->pos.cx - ball->pos.cx,
			     closest_treasure->pos.cy - ball->pos.cy);
	bbpos = Clpos_to_blkpos(ball->pos);
	xdist = (closest_treasure->pos.cx / BLOCK_CLICKS) - bbpos.bx;
	ydist = (closest_treasure->pos.cy / BLOCK_CLICKS) - bbpos.by;
	for (dist = 0;
	     clear_path && dist < (closest_tr_dist - BLOCK_SZ);
	     dist += BLOCK_SZ / 2) {
	    double fraction = (double)dist / closest_tr_dist;
	    dx = (int)((fraction * xdist) + bbpos.bx);
	    dy = (int)((fraction * ydist) + bbpos.by);

	    dx = WRAP_XBLOCK(dx);
	    dy = WRAP_YBLOCK(dy);
	    if (dx < 0 || dx >= world->x || dy < 0 || dy >= world->y) {
		clear_path = false;
		continue;
	    }
	    if (!BIT(1U << world->block[dx][dy], SPACE_BLOCKS)) {
		clear_path = false;
		continue;
	    }
	}
	if (tdir == bdir
	    && dist_np > closest_tr_dist
	    && clear_path
	    && sqr(ball->vel.x) + sqr(ball->vel.y) > 60) {
	    Detach_ball(pl, NULL);
	    CLR_BIT(pl->used, HAS_CONNECTOR);
	    my_data->last_thrown_ball = my_data->robot_count;
	    CLR_BIT(my_data->longterm_mode, FETCH_TREASURE);
	} else {
	    SET_BIT(my_data->longterm_mode, FETCH_TREASURE);
	    return (Check_robot_target(pl,
				       world->treasures[closest_tr].pos,
				       RM_NAVIGATE));
	}
    } else {
	int	ball_dist;
	int	closest_ball_dist = closest_ntr_dist;
	int	closest_ball = -1;

	for (i = 0; i < NumObjs; i++) {
	    if (Obj[i]->type == OBJ_BALL) {
		ballobject_t *ball = BALL_IND(i);

		if ((ball->id == NO_ID)
		    ? (ball->owner != NO_ID)
		    : (Player_by_id(ball->id)->team != pl->team)) {
		    ball_dist = LENGTH(pl->pos.cx - ball->pos.cx,
				       pl->pos.cy - ball->pos.cy) / CLICK;
		    if (ball_dist < closest_ball_dist) {
			closest_ball_dist = ball_dist;
			closest_ball = i;
		    }
		}
	    }
	}
	if (closest_ball == -1
	    && closest_ntr_dist < (my_data->robot_count / 10) * BLOCK_SZ) {
	    SET_BIT(my_data->longterm_mode, FETCH_TREASURE);
	    return (Check_robot_target(pl,
				       world->treasures[closest_ntr].pos,
				       RM_NAVIGATE));
	} else if (closest_ball_dist < (my_data->robot_count / 10) * BLOCK_SZ
		   && closest_ball_dist > options.ballConnectorLength) {
	    SET_BIT(my_data->longterm_mode, FETCH_TREASURE);
	    return (Check_robot_target(pl,
				       Obj[closest_ball]->pos,
				       RM_NAVIGATE));
	}
    }
    return false;
}

static int Robot_default_play_check_map(player_t *pl)
{
    int				j;
    int				cannon_i, fuel_i, target_i;
    int				dx, dy;
    int				distance, cannon_dist, fuel_dist, target_dist;
    bool			fuel_checked;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);
    world_t *world = &World;

    fuel_checked = false;

    cannon_i = -1;
    cannon_dist = Visibility_distance;
    fuel_i = -1;
    fuel_dist = Visibility_distance;
    target_i = -1;
    target_dist = Visibility_distance;

    for (j = 0; j < world->NumFuels; j++) {
	fuel_t *fs = Fuels(world, j);

	if (fs->fuel < 100.0)
	    continue;

	if (BIT(world->rules->mode, TEAM_PLAY)
	    && options.teamFuel
	    && fs->team != pl->team)
	    continue;

	if ((dx = (CLICK_TO_PIXEL(fs->pos.cx - pl->pos.cx)),
		dx = WRAP_DX(dx), ABS(dx)) < fuel_dist
	    && (dy = (CLICK_TO_PIXEL(fs->pos.cy - pl->pos.cy)),
		dy = WRAP_DY(dy), ABS(dy)) < fuel_dist
	    && (distance = LENGTH(dx, dy)) < fuel_dist) {
	    blkpos_t bpos = Clpos_to_blkpos(fs->pos);

	    if (World_get_block(world, bpos) == FUEL) {
		fuel_i = j;
		fuel_dist = distance;
	    }
	}
    }

    for (j = 0; j < world->NumTargets; j++) {
	target_t *targ = Targets(world, j);

	/* Ignore dead or owned targets */
	if (targ->dead_ticks > 0
	    || pl->team == targ->team
	    || Teams(world, targ->team)->NumMembers == 0)
	    continue;

	if ((dx = CLICK_TO_PIXEL(targ->pos.cx - pl->pos.cx),
		dx = WRAP_DX(dx), ABS(dx)) < target_dist
	    && (dy = CLICK_TO_PIXEL(targ->pos.cy - pl->pos.cy),
		dy = WRAP_DY(dy), ABS(dy)) < target_dist
	    && (distance = LENGTH(dx, dy)) < target_dist) {
	    target_i = j;
	    target_dist = distance;
	}
    }

    if (fuel_i >= 0
	&& (target_dist > fuel_dist
	    || !BIT(world->rules->mode, TEAM_PLAY))
	&& BIT(my_data->longterm_mode, NEED_FUEL)) {

	fuel_checked = true;
	SET_BIT(pl->used, HAS_REFUEL);
	pl->fs = fuel_i;

	if (Check_robot_target(pl, Fuels(world, fuel_i)->pos, RM_REFUEL))
	    return 1;
    }
    if (target_i >= 0) {
	SET_BIT(my_data->longterm_mode, TARGET_KILL);
	if (Check_robot_target(pl, Targets(world, target_i)->pos, RM_CANNON_KILL))
	    return 1;

	CLR_BIT(my_data->longterm_mode, TARGET_KILL);
    }

    for (j = 0; j < world->NumCannons; j++) {
	cannon_t *cannon = Cannons(world, j);

	if (cannon->dead_ticks > 0)
	    continue;

	if (BIT(world->rules->mode, TEAM_PLAY)
	    && cannon->team == pl->team)
	    continue;

	if ((dx = CLICK_TO_PIXEL(cannon->pos.cx - pl->pos.cx),
		dx = WRAP_DX(dx), ABS(dx)) < cannon_dist
	    && (dy = CLICK_TO_PIXEL(cannon->pos.cy - pl->pos.cy),
		dy = WRAP_DY(dy), ABS(dy)) < cannon_dist
	    && (distance = LENGTH(dx, dy)) < cannon_dist) {
	    cannon_i = j;
	    cannon_dist = distance;
	}
    }

    if (cannon_i >= 0) {
	cannon_t *cannon = Cannons(world, cannon_i);
	clpos_t d = cannon->pos;

	d.cx += (BLOCK_CLICKS * 0.1 * tcos(cannon->dir));
	d.cy += (BLOCK_CLICKS * 0.1 * tsin(cannon->dir));

	if (Check_robot_target(pl, d, RM_CANNON_KILL))
	    return 1;
    }

    if (fuel_i >= 0
	&& !fuel_checked
	&& BIT(my_data->longterm_mode, NEED_FUEL)) {

	SET_BIT(pl->used, HAS_REFUEL);
	pl->fs = fuel_i;

	if (Check_robot_target(pl, Fuels(world, fuel_i)->pos, RM_REFUEL))
	    return 1;
    }

    return 0;
}

static void Robot_default_play_check_objects(player_t *pl,
					     int *item_i, int *item_dist,
					     int *item_imp,
					     int *mine_i, int *mine_dist)
{
    int				j;
    object_t			*shot, **obj_list;
    int				distance, obj_count;
    int				dx, dy;
    int				shield_range;
    long			killing_shots;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);
    world_t *world = &World;

    /*-BA Neural overload - if NumObjs too high, only consider
     *-BA max_objs many objects - improves performance under nukes
     *-BA 1000 is a fairly arbitrary choice.  If you wish to tune it,
     *-BA take into account the following.  A 4 mine cluster nuke produces
     *-BA about 4000 short lived objects.  An 8 mine cluster nuke produces
     *-BA about 14000 short lived objects.  By default, there is a limit
     *-BA of about 16000 objects.  Each player/robot produces between
     *-BA 20 and 40 objects just thrusting, and up to perhaps another 100
     *-BA by firing.  If the number is set too low the robots just fly
     *-BA around with thier shields on looking stupid and not doing
     *-BA much.  If too high, your system will slow down too much when
     *-BA the cluster nukes start going off.
     */
    const int                   max_objs = 1000;

    killing_shots = KILLING_SHOTS;
    if (options.treasureCollisionMayKill)
	killing_shots |= OBJ_BALL;
    if (options.wreckageCollisionMayKill)
	killing_shots |= OBJ_WRECKAGE;
    if (options.asteroidCollisionMayKill)
	killing_shots |= OBJ_ASTEROID;

    Cell_get_objects(pl->pos, (int)(Visibility_distance / BLOCK_SZ), max_objs,
		     &obj_list, &obj_count);

    for (j = 0; j < obj_count; j++) {

	shot = obj_list[j];

	/* Get rid of the most common object types first for speed. */
	if (BIT(shot->type, OBJ_DEBRIS|OBJ_SPARK))
	    continue;

	dx = CLICK_TO_PIXEL(shot->pos.cx - pl->pos.cx);
	dy = CLICK_TO_PIXEL(shot->pos.cy - pl->pos.cy);
	dx = WRAP_DX(dx);
	dy = WRAP_DX(dy);

	if (BIT(shot->type, OBJ_BALL)
	    && !WITHIN(my_data->last_thrown_ball,
		       my_data->robot_count,
		       3 * FPS))
	    SET_BIT(pl->used, HAS_CONNECTOR);

	/* Ignore shots and laser pulses if shields already up
	   - nothing else to do anyway */
	if (BIT(shot->type, OBJ_SHOT|OBJ_CANNON_SHOT|OBJ_PULSE)
	    && BIT(pl->used, HAS_SHIELD))
	    continue;

	/*-BA This code shouldn't be executed for `friendly` shots
	 *-BA Moved down 2 paragraphs
	 *	if (BIT(shot->type, OBJ_SMART_SHOT|OBJ_HEAT_SHOT|OBJ_MINE)) {
	 *	    fx = shot->pos.x - pl->pos.x;
	 *	    fy = shot->pos.y - pl->pos.y;
	 *	    if ((dx = fx, dx = WRAP_DX(dx), ABS(dx)) < mine_dist
	 *		&& (dy = fy, dy = WRAP_DY(dy), ABS(dy)) < mine_dist
	 *		&& (distance = LENGTH(dx, dy)) < mine_dist) {
	 *		mine_i = j;
	 *		mine_dist = distance;
	 *	    }
	 *	    if ((dx = fx + (shot->vel.x - pl->vel.x) * ROB_LOOK_AH,
	 *		    dx = WRAP_DX(dx), ABS(dx)) < mine_dist
	 *		&& (dy = fy + (shot->vel.y - pl->vel.y) * ROB_LOOK_AH,
	 *		    dy = WRAP_DY(dy), ABS(dy)) < mine_dist
	 *		&& (distance = LENGTH(dx, dy)) < mine_dist) {
	 *		mine_i = j;
	 *		mine_dist = distance;
	 *	    }
	 *	}
	 */

	/*
	 * The only thing left to do regarding objects is to check if
	 * this robot needs to put up shields to protect against objects.
	 */
	if (!BIT(shot->type, killing_shots)) {

	    /* Find closest item */
	    if (BIT(shot->type, OBJ_ITEM)) {
		if (ABS(dx) < *item_dist
		    && ABS(dy) < *item_dist) {
		    int imp;

		    if (BIT(shot->status, RANDOM_ITEM))
			/* It doesn't know what it is, so get it if it can */
			imp = ROBOT_HANDY_ITEM;
		    else
			imp = Rank_item_value(pl, obj_list[j]->info);

		    if (imp > ROBOT_IGNORE_ITEM && imp >= *item_imp) {
			*item_imp = imp;
			*item_dist = LENGTH(dx, dy);
			*item_i = j;
		    }
		}
	    }

	    continue;
	}

	/* Any shot of team members excluding self are passive. */
	if (Team_immune(shot->id, pl->id))
	    continue;

	/* Self shots may be passive too... */
	if (shot->id == pl->id
	    && options.selfImmunity)
	    continue;

	/* Own non-reflected laser pulses too. */
	if (BIT(shot->type, OBJ_PULSE)) {
	    pulseobject_t *pulse = PULSE_PTR(shot);

	    if (pulse->id == pl->id
		&& !pulse->refl)
		continue;
	}

	/* Find nearest missile/mine */
	if (BIT(shot->type, OBJ_TORPEDO|OBJ_SMART_SHOT|OBJ_ASTEROID
			    |OBJ_HEAT_SHOT|OBJ_BALL|OBJ_CANNON_SHOT)
	    || (BIT(shot->type, OBJ_SHOT)
		&& !BIT(world->rules->mode, TIMING)
		&& shot->id != pl->id
		&& shot->id != NO_ID)
	    || (BIT(shot->type, OBJ_MINE)
		&& shot->id != pl->id)
	    || (BIT(shot->type, OBJ_WRECKAGE)
		&& !BIT(world->rules->mode, TIMING))) {
	    if (ABS(dx) < *mine_dist
		&&  ABS(dy) < *mine_dist
		&& (distance = LENGTH(dx, dy)) < *mine_dist) {
		*mine_i = j;
		*mine_dist = distance;
	    }
	    if ((dx = ((CLICK_TO_PIXEL(shot->pos.cx - pl->pos.cx))
			     + (shot->vel.x - pl->vel.x)),
		    dx = WRAP_DX(dx), ABS(dx)) < *mine_dist
		&& (dy = ((CLICK_TO_PIXEL(shot->pos.cy - pl->pos.cy))
				 + (shot->vel.y - pl->vel.y)),
		    dy = WRAP_DY(dy), ABS(dy)) < *mine_dist
		&& (distance = LENGTH(dx, dy)) < *mine_dist) {
		*mine_i = j;
		*mine_dist = distance;
	    }
	}

	shield_range = 21 + SHIP_SZ + shot->pl_range;

	if ((dx = (CLICK_TO_PIXEL(shot->pos.cx)
		   + shot->vel.x
		   - (CLICK_TO_PIXEL(pl->pos.cx) + pl->vel.x)),
		dx = WRAP_DX(dx),
		ABS(dx)) < shield_range
	    && (dy = (CLICK_TO_PIXEL(shot->pos.cy) + shot->vel.y
		      - (CLICK_TO_PIXEL(pl->pos.cy) + pl->vel.y)),
		dy = WRAP_DY(dy),
		ABS(dy)) < shield_range
	    && sqr(dx) + sqr(dy) <= sqr(shield_range)
	    && (int)(rfrac() * 100) <
	       (85 + (my_data->defense / 7) - (my_data->attack / 50))) {
	    SET_BIT(pl->used, HAS_SHIELD);
	    if (!options.cloakedShield)
		CLR_BIT(pl->used, HAS_CLOAKING_DEVICE);
	    SET_BIT(pl->status, THRUSTING);

	    if (BIT(shot->type, OBJ_TORPEDO|OBJ_SMART_SHOT|OBJ_ASTEROID
				|OBJ_HEAT_SHOT|OBJ_MINE)
		&& (pl->fuel.sum < pl->fuel.l3
		    || !BIT(pl->have, HAS_SHIELD))) {
	      if (Initiate_hyperjump(pl))
		  break;
	    }
	}
	if (BIT(shot->type, OBJ_SMART_SHOT)) {
	    if (*mine_dist < ECM_DISTANCE / 4)
		Fire_ecm(pl);
	}
	if (BIT(shot->type, OBJ_MINE)) {
	    if (*mine_dist < ECM_DISTANCE / 2)
		Fire_ecm(pl);
	}
	if (BIT(shot->type, OBJ_HEAT_SHOT)) {
	    CLR_BIT(pl->status, THRUSTING);
	    if (pl->fuel.sum < pl->fuel.l3
		&& pl->fuel.sum > pl->fuel.l1
		&& pl->fuel.num_tanks > 0)
		Tank_handle_detach(pl);
	}
	if (BIT(shot->type, OBJ_ASTEROID)) {
	    int delta_dir = 0;
	    if (*mine_dist > (WIRE_PTR(shot)->size == 1 ? 2 : 4) * BLOCK_SZ
		&& *mine_dist < 8 * BLOCK_SZ
		&& (delta_dir = (pl->dir
				 - Wrap_cfindDir(shot->pos.cx - pl->pos.cx,
						 shot->pos.cy - pl->pos.cy))
		    < WIRE_PTR(shot)->size * (RES / 10)
		    || delta_dir > RES - WIRE_PTR(shot)->size * (RES / 10)))
		SET_BIT(pl->used, HAS_SHOT);
	}
    }

    /* Convert *item_i from index in local obj_list[] to index in Obj[] */
    if (*item_i >= 0) {
	for (j=0; (j < NumObjs) && (Obj[j]->id != obj_list[*item_i]->id); j++);
	if (j >= NumObjs)
	    /* Perhaps an error should be printed, too? */
	    *item_i = -1;
	else
	    *item_i = j;
    }

}


static void Robot_default_play(player_t *pl)
{
    player_t			*ship;
    double			distance, ship_dist,
				enemy_dist,
				speed, x_speed, y_speed;
    int				item_dist, mine_dist;
    int				item_i, mine_i;
    int				j, ship_i, item_imp, enemy_i;
    bool			harvest_checked;
    bool			evade_checked;
    bool			navigate_checked;
    int				shoot_time;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);
    world_t *world = &World;

    if (my_data->robot_count <= 0)
	my_data->robot_count = 1000 + (int)(rfrac() * 32);

    my_data->robot_count--;

    CLR_BIT(pl->used, HAS_SHOT | HAS_SHIELD | HAS_CLOAKING_DEVICE | HAS_LASER);
    if (BIT(pl->have, HAS_EMERGENCY_SHIELD)
	&& !BIT(pl->used, HAS_EMERGENCY_SHIELD))
	Emergency_shield(pl, true);

    harvest_checked = false;
    evade_checked = false;
    navigate_checked = false;

    mine_i = -1;
    mine_dist = SHIP_SZ + 200;
    item_i = -1;
    item_dist = Visibility_distance;
    item_imp = ROBOT_IGNORE_ITEM;

    if (BIT(pl->have, HAS_CLOAKING_DEVICE) && pl->fuel.sum > pl->fuel.l2)
	SET_BIT(pl->used, HAS_CLOAKING_DEVICE);

    if (BIT(pl->have, HAS_EMERGENCY_THRUST)
	&& !BIT(pl->used, HAS_EMERGENCY_THRUST))
	Emergency_thrust(pl, true);

    if (BIT(pl->have, HAS_DEFLECTOR) && !BIT(world->rules->mode, TIMING))
	Deflector(pl, true);

    if (pl->fuel.sum <= (BIT(world->rules->mode, TIMING) ? 0 : pl->fuel.l1))
	Player_self_destruct(pl, true);
    else
	Player_self_destruct(pl, false);

    /* blinded by ECM. since we're not supposed to see anything,
       put up shields and return */
    if (pl->damaged > 0) {
	SET_BIT(pl->used, HAS_SHIELD);
	if (!options.cloakedShield)
	    CLR_BIT(pl->used, HAS_CLOAKING_DEVICE);
	return;
    }

    if (pl->fuel.sum < pl->fuel.max * 0.80) {
	for (j = 0; j < world->NumFuels; j++) {
	    fuel_t *fs = Fuels(world, j);

	    if (BIT(world->rules->mode, TEAM_PLAY)
		&& options.teamFuel
		&& fs->team != pl->team)
		continue;

	    if ((Wrap_length(pl->pos.cx - fs->pos.cx,
			     pl->pos.cy - fs->pos.cy) <= 90.0 * CLICK)
		&& fs->fuel > REFUEL_RATE * timeStep) {
		pl->fs = j;
		SET_BIT(pl->used, HAS_REFUEL);
		break;
	    } else
		CLR_BIT(pl->used, HAS_REFUEL);
	}
    }

    /* don't turn NEED_FUEL off until refueling stops */
    if (pl->fuel.sum < (BIT(world->rules->mode, TIMING) ?
			pl->fuel.l1 : pl->fuel.l3))
	SET_BIT(my_data->longterm_mode, NEED_FUEL);
    else if (!BIT(pl->used, HAS_REFUEL))
	CLR_BIT(my_data->longterm_mode, NEED_FUEL);

    if (BIT(world->rules->mode, TEAM_PLAY)) {
	for (j = 0; j < world->NumTargets; j++) {
	    target_t *targ = Targets(world, j);

	    if (targ->team == pl->team
		&& targ->damage < TARGET_DAMAGE
		&& targ->dead_ticks >= 0) {

		if (Wrap_length(pl->pos.cx - targ->pos.cx,
				pl->pos.cy - targ->pos.cy) <= 90.0 * CLICK) {
		    pl->repair_target = j;
		    SET_BIT(pl->used, HAS_REPAIR);
		    break;
		}
	    }
	}
    }

    Robot_default_play_check_objects(pl,
				     &item_i, &item_dist, &item_imp,
				     &mine_i, &mine_dist);

    /* Note: Only take time to navigate if not being shot at */
    /* KK: it seems that this 'Check_robot_navigate' function caused
	the infamous 'robot stuck under wall' bug, so I commented it out */
    /* KK: ps. I tried to change that function, but I don't grok it */
    /*if (!(BIT(pl->used, HAS_SHIELD) && BIT(pl->status, THRUSTING))
	&& Check_robot_navigate(ind, &evade_checked)) {
	if (options.playerShielding == 0
	    && options.playerStartsShielded != 0
	    && BIT(pl->have, HAS_SHIELD)) {
	    SET_BIT(pl->used, HAS_SHIELD);
	}
	return;
    }*/
    /* KK: unfortunately, this introduced a new bug. robots with large
	shipshapes don't take off from their bases. here's an attempt to
	fix it */
    if (QUICK_LENGTH(pl->pos.cx - pl->home_base->pos.cx,
		     pl->pos.cy - pl->home_base->pos.cy) < BLOCK_CLICKS)
	SET_BIT(pl->status, THRUSTING);

    ship_i = -1;
    ship_dist = SHIP_SZ * 6;
    enemy_i = -1;
    if (pl->fuel.sum > pl->fuel.l3)
	enemy_dist = (BIT(world->rules->mode, LIMITED_VISIBILITY) ?
		      MAX(pl->fuel.sum * ENERGY_RANGE_FACTOR,
			  Visibility_distance)
		      : Max_enemy_distance);
    else
	enemy_dist = Visibility_distance;

    if (BIT(pl->used, HAS_SHIELD))
	ship_dist = 0;

    if (BIT(my_data->robot_lock, LOCK_PLAYER)) {
	ship = Player_by_id(my_data->robot_lock_id);
	j = GetInd(ship->id);

	if (Player_is_active(ship)
	    && Detect_hunt(pl, ship)) {

	    if (BIT(my_data->robot_lock, LOCK_PLAYER)
		&& my_data->robot_lock_id == ship->id) {
		my_data->lock_last_seen = my_data->robot_count;
		my_data->lock_last_pos.x = CLICK_TO_PIXEL(ship->pos.cx);
		my_data->lock_last_pos.y = CLICK_TO_PIXEL(ship->pos.cy);
	    }

	    distance = Wrap_length(ship->pos.cx - pl->pos.cx,
				   ship->pos.cy - pl->pos.cy) / CLICK;

	    if (distance < ship_dist) {
		ship_i = GetInd(my_data->robot_lock_id);
		ship_dist = distance;
	    }

	    if (distance < enemy_dist) {
		enemy_i = j;
		enemy_dist = distance;
	    }
	}
    }

    if (ship_i == -1 || enemy_i == -1) {

	for (j = 0; j < NumPlayers; j++) {

	    ship = Players(j);
	    if (j == GetInd(pl->id)
		|| !Player_is_active(ship)
		|| Team_immune(pl->id, ship->id))
		continue;

	    if (!Detect_hunt(pl, ship))
		continue;

	    distance = Wrap_length(ship->pos.cx - pl->pos.cx,
				   ship->pos.cy - pl->pos.cy) / CLICK;

	    if (distance < ship_dist) {
		ship_i = j;
		ship_dist = distance;
	    }

	    if (!BIT(my_data->robot_lock, LOCK_PLAYER)) {
		if ((my_data->robot_count % 3) == 0
		    && ((my_data->robot_count % 100) < my_data->attack)
		    && distance < enemy_dist) {
		    enemy_i    = j;
		    enemy_dist = distance;
		}
	    }
	}
    }

    if (ship_dist < 3*SHIP_SZ && BIT(pl->have, HAS_SHIELD)) {
	SET_BIT(pl->used, HAS_SHIELD);
	if (!options.cloakedShield)
	   CLR_BIT(pl->used, HAS_CLOAKING_DEVICE);
    }

    if (ship_dist <= 10*BLOCK_SZ && pl->fuel.sum <= pl->fuel.l3
	&& !BIT(world->rules->mode, TIMING)) {
	if (Initiate_hyperjump(pl))
	    return;
    }

    if (ship_i != -1
	&& BIT(my_data->robot_lock, LOCK_PLAYER)
	&& my_data->robot_lock_id == Players(ship_i)->id) {
	ship_i = -1; /* don't avoid target */
    }

    if (enemy_i >= 0) {
	ship = Players(enemy_i);
	if (!BIT(pl->lock.tagged, LOCK_PLAYER)
	    || (enemy_dist < pl->lock.distance/2
		&& (BIT(world->rules->mode, TIMING) ?
		    (ship->check >= pl->check
		     && ship->round >= pl->round) : 1))
	    || (enemy_dist < pl->lock.distance*2
		&& BIT(world->rules->mode, TEAM_PLAY)
		&& BIT(ship->have, HAS_BALL))
	    || ship->score > Player_by_id(pl->lock.pl_id)->score) {
	    pl->lock.pl_id = ship->id;
	    SET_BIT(pl->lock.tagged, LOCK_PLAYER);
	    pl->lock.distance = enemy_dist;
	    Compute_sensor_range(pl);
	}
    }

    if (BIT(pl->lock.tagged, LOCK_PLAYER)) {
	int delta_dir;
	ship = Player_by_id(pl->lock.pl_id);
	delta_dir = (int)(pl->dir - Wrap_cfindDir(ship->pos.cx - pl->pos.cx,
						  ship->pos.cy - pl->pos.cy));
	delta_dir = MOD2(delta_dir, RES);
	if (!Player_is_active(ship)
	    || (BIT(my_data->robot_lock, LOCK_PLAYER)
		&& my_data->robot_lock_id != pl->lock.pl_id
		&& Player_is_active(Player_by_id(my_data->robot_lock_id)))
	    || !Detect_hunt(pl, ship)
	    || (pl->fuel.sum <= pl->fuel.l3
		&& !BIT(world->rules->mode, TIMING))
	    || (BIT(world->rules->mode, TIMING)
		&& (delta_dir < 3 * RES / 4 || delta_dir > RES / 4))
	    || Team_immune(pl->id, ship->id)) {
	    /* unset the player lock */
	    CLR_BIT(pl->lock.tagged, LOCK_PLAYER);
	    pl->lock.pl_id = 1;
	    pl->lock.distance = 0;
	}
    }
    if (!evade_checked) {
	if (Check_robot_evade(pl, mine_i, ship_i)) {
	    if (options.playerShielding == 0
		&& options.playerStartsShielded != 0
		&& BIT(pl->have, HAS_SHIELD)) {
		SET_BIT(pl->used, HAS_SHIELD);
		if (!options.cloakedShield)
		    CLR_BIT(pl->used, HAS_CLOAKING_DEVICE);
	    }
	    else if (options.maxShieldedWallBounceSpeed >
		    options.maxUnshieldedWallBounceSpeed
		&& BIT(pl->have, HAS_SHIELD)) {
		SET_BIT(pl->used, HAS_SHIELD);
		if (!options.cloakedShield)
		    CLR_BIT(pl->used, HAS_CLOAKING_DEVICE);
	    }
	    return;
	}
    }
    if (BIT(world->rules->mode, TIMING) && !navigate_checked) {
	int delta_dir;
	if (item_i >= 0) {
	    delta_dir =
		(int)(pl->dir
		      - Wrap_cfindDir(Obj[item_i]->pos.cx - pl->pos.cx,
				      Obj[item_i]->pos.cy - pl->pos.cy));
	    delta_dir = MOD2(delta_dir, RES);
	} else {
	    delta_dir = RES;
	    item_imp = ROBOT_IGNORE_ITEM;
	}
	if ((item_imp == ROBOT_MUST_HAVE_ITEM && item_dist > 4 * BLOCK_SZ)
	    || (item_imp == ROBOT_HANDY_ITEM && item_dist > 2 * BLOCK_SZ)
	    || (item_imp == ROBOT_IGNORE_ITEM)
	    || (delta_dir < 3 * RES / 4 && delta_dir > RES / 4)) {
	    navigate_checked = true;
	    if (Check_robot_target(pl, Checks(world, pl->check)->pos,
				   RM_NAVIGATE))
		return;
	}
    }
    if (item_i >= 0
	&& 3*enemy_dist > 2*item_dist
	&& item_dist < 12*BLOCK_SZ
	&& !BIT(my_data->longterm_mode, FETCH_TREASURE)
	&& (!BIT(my_data->longterm_mode, NEED_FUEL)
	    || Obj[item_i]->info == ITEM_FUEL
	    || Obj[item_i]->info == ITEM_TANK)) {

	if (item_imp != ROBOT_IGNORE_ITEM) {
	    clpos_t d = Obj[item_i]->pos;

	    harvest_checked = true;
	    d.cx += (long)(Obj[item_i]->vel.x
			   * (ABS(d.cx - pl->pos.cx) /
			      my_data->robot_normal_speed));
	    d.cy += (long)(Obj[item_i]->vel.y
			   * (ABS(d.cy - pl->pos.cy) /
			      my_data->robot_normal_speed));
	    if (Check_robot_target(pl, d, RM_HARVEST))
		return;
	}
    }
    if (BIT(pl->lock.tagged, LOCK_PLAYER) &&
	Detect_hunt(pl, Player_by_id(pl->lock.pl_id))) {
	clpos_t d;

	ship = Player_by_id(pl->lock.pl_id);
	shoot_time = (int)(pl->lock.distance / (options.shotSpeed + 1));
	d.cx = (long)(ship->pos.cx + ship->vel.x * shoot_time * CLICK);
	d.cy = (long)(ship->pos.cy + ship->vel.y * shoot_time * CLICK);
	/*-BA Also allow for our own momentum. */
	d.cx -= (long)(pl->vel.x * shoot_time * CLICK);
	d.cy -= (long)(pl->vel.y * shoot_time * CLICK);

	if (Check_robot_target(pl, d, RM_ATTACK)
	    && !BIT(my_data->longterm_mode, FETCH_TREASURE
					    |TARGET_KILL
					    |NEED_FUEL))
	    return;
    }
    if (BIT(world->rules->mode, TEAM_PLAY)
	&& world->NumTreasures > 0
	&& world->teams[pl->team].NumTreasures > 0
	&& !navigate_checked
	&& !BIT(my_data->longterm_mode, TARGET_KILL|NEED_FUEL)) {
	navigate_checked = true;
	if (Ball_handler(pl))
	    return;
    }
    if (item_i >= 0
	&& !harvest_checked
	&& item_dist < 12*BLOCK_SZ) {

	if (item_imp != ROBOT_IGNORE_ITEM) {
	    clpos_t d = Obj[item_i]->pos;

	    d.cx += (long)(Obj[item_i]->vel.x
			   * (ABS(d.cx - pl->pos.cx) /
			      my_data->robot_normal_speed));
	    d.cy += (long)(Obj[item_i]->vel.y
			   * (ABS(d.cy - pl->pos.cy) /
			      my_data->robot_normal_speed));

	    if (Check_robot_target(pl, d, RM_HARVEST))
		return;
	}
    }

    if (Check_robot_hunt(pl)) {
	if (options.playerShielding == 0
	    && options.playerStartsShielded != 0
	    && BIT(pl->have, HAS_SHIELD)) {
	    SET_BIT(pl->used, HAS_SHIELD);
	    if (!options.cloakedShield)
		CLR_BIT(pl->used, HAS_CLOAKING_DEVICE);
	}
	return;
    }

    if (Robot_default_play_check_map(pl) == 1)
	return;

    if (options.playerShielding == 0
	&& options.playerStartsShielded != 0
	&& BIT(pl->have, HAS_SHIELD)) {
	SET_BIT(pl->used, HAS_SHIELD);
	if (!options.cloakedShield)
	    CLR_BIT(pl->used, HAS_CLOAKING_DEVICE);
    }

    x_speed = pl->vel.x - 2 * World_gravity(world, pl->pos).x;
    y_speed = pl->vel.y - 2 * World_gravity(world, pl->pos).y;

    if (y_speed < (-my_data->robot_normal_speed)
	|| (my_data->robot_count % 64) < 32) {

	my_data->robot_mode = RM_ROBOT_CLIMB;
	pl->turnspeed = MAX_PLAYER_TURNSPEED / 2;
	pl->power = MAX_PLAYER_POWER / 2;
	if (ABS(pl->dir - RES / 4) > RES / 16)
	    pl->turnacc = (pl->dir < RES / 4
			   || pl->dir >= 3 * RES / 4
			   ? pl->turnspeed : (-pl->turnspeed));
	else
	    pl->turnacc = 0.0;

	if (y_speed < my_data->robot_normal_speed / 2
	    && pl->velocity < my_data->robot_attack_speed)
	    SET_BIT(pl->status, THRUSTING);
	else if (y_speed > my_data->robot_normal_speed)
	    CLR_BIT(pl->status, THRUSTING);
	return;
    }
    my_data->robot_mode = RM_ROBOT_IDLE;
    pl->turnspeed = MAX_PLAYER_TURNSPEED / 2;
    pl->turnacc = 0;
    pl->power = MAX_PLAYER_POWER / 2;
    CLR_BIT(pl->status, THRUSTING);
    speed = LENGTH(x_speed, y_speed);
    if (speed < my_data->robot_normal_speed / 2)
	SET_BIT(pl->status, THRUSTING);
    else if (speed > my_data->robot_normal_speed)
	CLR_BIT(pl->status, THRUSTING);
}


/*
 * This is called each round.
 * It allows us to adjust our file local parameters.
 */
static void Robot_default_round_tick(void)
{
    double		min_visibility = 256.0;
    double		min_enemy_distance = 512.0;
    world_t *world = &World;

    /* reduce visibility when there are a lot of robots. */
    Visibility_distance = min_visibility
	+ (((VISIBILITY_DISTANCE - min_visibility)
	    * (NUM_IDS - NumRobots)) / NUM_IDS);

    /* limit distance to allowable enemies. */
    Max_enemy_distance = world->hypotenuse;
    if (world->hypotenuse > Visibility_distance) {
	Max_enemy_distance = min_enemy_distance
	    + (((world->hypotenuse - min_enemy_distance)
		* (NUM_IDS - NumRobots)) / NUM_IDS);
    }
}
