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

char rules_version[] = VERSION;


#define MAX_FUEL                10000
#define MAX_WIDEANGLE           99
#define MAX_REARSHOT            99
#define MAX_CLOAK               99
#define MAX_SENSOR              99
#define MAX_TRANSPORTER         99
#define MAX_MINE                99
#define MAX_MISSILE             99
#define MAX_ECM                 99
#define MAX_ARMOR		99
#define MAX_EMERGENCY_THRUST    99
#define MAX_AUTOPILOT           99
#define MAX_EMERGENCY_SHIELD    99
#define MAX_DEFLECTOR           99
#define MAX_MIRROR		99
#define MAX_PHASING             99
#define MAX_HYPERJUMP           99
#define MAX_LASER		99
#define MAX_TRACTOR_BEAM	99

long	KILLING_SHOTS = (OBJ_SHOT|OBJ_CANNON_SHOT|OBJ_SMART_SHOT
			 |OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_PULSE);
long	DEF_BITS = 0;
long	KILL_BITS = (THRUSTING|PLAYING|KILLED|SELF_DESTRUCT|WARPING/*|WARPED*/);
long	DEF_HAVE =
	(HAS_SHIELD|HAS_COMPASS|HAS_REFUEL|HAS_REPAIR|HAS_CONNECTOR
	|HAS_SHOT|HAS_LASER);
long	DEF_USED = (HAS_SHIELD|HAS_COMPASS);
long	USED_KILL =
	(HAS_REFUEL|HAS_REPAIR|HAS_CONNECTOR|HAS_SHOT|HAS_LASER|HAS_ARMOR
	|HAS_TRACTOR_BEAM|HAS_CLOAKING_DEVICE|HAS_PHASING_DEVICE
	|HAS_DEFLECTOR|HAS_MIRROR|HAS_EMERGENCY_SHIELD|HAS_EMERGENCY_THRUST);



/*
 * Convert between probability for something to happen a given second on a
 * given block, to chance for such an event to happen on any block this tick.
 */
static void Set_item_chance(int item)
{
    world_t *world = &World;
    double	max = itemProbMult * maxItemDensity * world->x * world->y;
    double	sum = 0;
    int		i, num = 0;

    if (itemProbMult * world->items[item].prob > 0) {
	world->items[item].chance = (int)(1.0
	    / (itemProbMult * world->items[item].prob
	       * world->x * world->y * FPS));
	world->items[item].chance = MAX(world->items[item].chance, 1);
    } else
	world->items[item].chance = 0;

    if (max > 0) {
	if (max < 1)
	    world->items[item].max = 1;
	else
	    world->items[item].max = (int)max;
    } else
	world->items[item].max = 0;

    if (!BIT(CANNON_USE_ITEM, 1U << item)) {
	world->items[item].cannonprob = 0;
	return;
    }
    for (i = 0; i < NUM_ITEMS; i++) {
	if (world->items[i].prob > 0
	    && BIT(CANNON_USE_ITEM, 1U << i)) {
	    sum += world->items[i].prob;
	    num++;
	}
    }
    if (num)
	world->items[item].cannonprob = world->items[item].prob
				       * (num / sum)
				       * (maxItemDensity / 0.00012);
    else
	world->items[item].cannonprob = 0;
}


/*
 * An item probability has been changed during game play.
 * Update the world->items structure and test if there are more items
 * in the world than wanted for the new item probabilities.
 * This function is also called when itemProbMult or maxItemDensity changes.
 */
void Tune_item_probs(void)
{
    int			i, j, excess;
    world_t *world = &World;

    for (i = 0; i < NUM_ITEMS; i++) {
	Set_item_chance(i);
	excess = world->items[i].num - world->items[i].max;
	if (excess > 0) {
	    for (j = 0; j < NumObjs; j++) {
		object *obj = Obj[j];
		if (obj->type == OBJ_ITEM) {
		    if (obj->info == i) {
			Delete_shot(j);
			j--;
			if (--excess == 0)
			    break;
		    }
		}
	    }
	}
    }
}

void Tune_asteroid_prob(void)
{
    world_t *world = &World;
    double	max = maxAsteroidDensity * world->x * world->y;

    if (world->asteroids.prob > 0) {
	world->asteroids.chance = (int)(1.0
			/ (world->asteroids.prob * world->x * world->y * FPS));
	world->asteroids.chance = MAX(world->asteroids.chance, 1);
    } else
	world->asteroids.chance = 0;

    if (max > 0) {
	if (max < 1)
	    world->asteroids.max = 1;
	else
	    world->asteroids.max = (int)max;
    } else
	world->asteroids.max = 0;

    /* superfluous asteroids are handled by Asteroid_update() */

    /* Tune asteroid concentrator parameters */
    LIMIT(asteroidConcentratorRadius, 1, world->diagonal);
    LIMIT(asteroidConcentratorProb, 0.0, 1.0);
}

/*
 * Postprocess a change command for the number of items per pack.
 */
void Tune_item_packs(void)
{
    world_t *world = &World;

    world->items[ITEM_MINE].max_per_pack = maxMinesPerPack;
    world->items[ITEM_MISSILE].max_per_pack = maxMissilesPerPack;
}


/*
 * Initializes special items.
 * First parameter is type,
 * second and third parameters are minimum and maximum number
 * of elements one item gives when picked up by a ship.
 */
static void Init_item(int item, int minpp, int maxpp)
{
    world_t *world = &World;

    world->items[item].num = 0;

    world->items[item].min_per_pack = minpp;
    world->items[item].max_per_pack = maxpp;

    Set_item_chance(item);
}


/*
 * Give (or remove) capabilities of the ships depending upon
 * the availability of initial items.
 * Limit the initial resources between minimum and maximum possible values.
 */
void Set_initial_resources(void)
{
    int			i;
    world_t *world = &World;

    LIMIT(world->items[ITEM_FUEL].limit, 0, MAX_FUEL);
    LIMIT(world->items[ITEM_WIDEANGLE].limit, 0, MAX_WIDEANGLE);
    LIMIT(world->items[ITEM_REARSHOT].limit, 0, MAX_REARSHOT);
    LIMIT(world->items[ITEM_AFTERBURNER].limit, 0, MAX_AFTERBURNER);
    LIMIT(world->items[ITEM_CLOAK].limit, 0, MAX_CLOAK);
    LIMIT(world->items[ITEM_SENSOR].limit, 0, MAX_SENSOR);
    LIMIT(world->items[ITEM_TRANSPORTER].limit, 0, MAX_TRANSPORTER);
    LIMIT(world->items[ITEM_TANK].limit, 0, MAX_TANKS);
    LIMIT(world->items[ITEM_MINE].limit, 0, MAX_MINE);
    LIMIT(world->items[ITEM_MISSILE].limit, 0, MAX_MISSILE);
    LIMIT(world->items[ITEM_ECM].limit, 0, MAX_ECM);
    LIMIT(world->items[ITEM_LASER].limit, 0, MAX_LASER);
    LIMIT(world->items[ITEM_EMERGENCY_THRUST].limit, 0, MAX_EMERGENCY_THRUST);
    LIMIT(world->items[ITEM_TRACTOR_BEAM].limit, 0, MAX_TRACTOR_BEAM);
    LIMIT(world->items[ITEM_AUTOPILOT].limit, 0, MAX_AUTOPILOT);
    LIMIT(world->items[ITEM_EMERGENCY_SHIELD].limit, 0, MAX_EMERGENCY_SHIELD);
    LIMIT(world->items[ITEM_DEFLECTOR].limit, 0, MAX_DEFLECTOR);
    LIMIT(world->items[ITEM_PHASING].limit, 0, MAX_PHASING);
    LIMIT(world->items[ITEM_HYPERJUMP].limit, 0, MAX_HYPERJUMP);
    LIMIT(world->items[ITEM_MIRROR].limit, 0, MAX_MIRROR);
    LIMIT(world->items[ITEM_ARMOR].limit, 0, MAX_ARMOR);

    for (i = 0; i < NUM_ITEMS; i++)
	LIMIT(world->items[i].initial, 0, world->items[i].limit);

    CLR_BIT(DEF_HAVE,
	HAS_CLOAKING_DEVICE |
	HAS_EMERGENCY_THRUST |
	HAS_EMERGENCY_SHIELD |
	HAS_PHASING_DEVICE |
	HAS_TRACTOR_BEAM |
	HAS_AUTOPILOT |
	HAS_DEFLECTOR |
	HAS_MIRROR |
	HAS_ARMOR);

    if (world->items[ITEM_CLOAK].initial > 0)
	SET_BIT(DEF_HAVE, HAS_CLOAKING_DEVICE);
    if (world->items[ITEM_EMERGENCY_THRUST].initial > 0)
	SET_BIT(DEF_HAVE, HAS_EMERGENCY_THRUST);
    if (world->items[ITEM_EMERGENCY_SHIELD].initial > 0)
	SET_BIT(DEF_HAVE, HAS_EMERGENCY_SHIELD);
    if (world->items[ITEM_PHASING].initial > 0)
	SET_BIT(DEF_HAVE, HAS_PHASING_DEVICE);
    if (world->items[ITEM_TRACTOR_BEAM].initial > 0)
	SET_BIT(DEF_HAVE, HAS_TRACTOR_BEAM);
    if (world->items[ITEM_AUTOPILOT].initial > 0)
	SET_BIT(DEF_HAVE, HAS_AUTOPILOT);
    if (world->items[ITEM_DEFLECTOR].initial > 0)
	SET_BIT(DEF_HAVE, HAS_DEFLECTOR);
    if (world->items[ITEM_MIRROR].initial > 0)
	SET_BIT(DEF_HAVE, HAS_MIRROR);
    if (world->items[ITEM_ARMOR].initial > 0)
	SET_BIT(DEF_HAVE, HAS_ARMOR);
}


void Set_misc_item_limits(void)
{
    world_t *world = &World;

    LIMIT(dropItemOnKillProb, 0.0, 1.0);
    LIMIT(detonateItemOnKillProb, 0.0, 1.0);
    LIMIT(movingItemProb, 0.0, 1.0);
    LIMIT(randomItemProb, 0.0, 1.0);
    LIMIT(destroyItemInCollisionProb, 0.0, 1.0);

    LIMIT(itemConcentratorRadius, 1, world->diagonal);
    LIMIT(itemConcentratorProb, 0.0, 1.0);

    LIMIT(asteroidItemProb, 0.0, 1.0);

    if (asteroidMaxItems < 0)
	asteroidMaxItems = 0;
}


/*
 * First time initialization of all global item stuff.
 */
void Set_world_items(void)
{
    Init_item(ITEM_FUEL, 0, 0);
    Init_item(ITEM_TANK, 1, 1);
    Init_item(ITEM_ECM, 1, 1);
    Init_item(ITEM_ARMOR, 1, 1);
    Init_item(ITEM_MINE, 1, maxMinesPerPack);
    Init_item(ITEM_MISSILE, 1, maxMissilesPerPack);
    Init_item(ITEM_CLOAK, 1, 1);
    Init_item(ITEM_SENSOR, 1, 1);
    Init_item(ITEM_WIDEANGLE, 1, 1);
    Init_item(ITEM_REARSHOT, 1, 1);
    Init_item(ITEM_AFTERBURNER, 1, 1);
    Init_item(ITEM_TRANSPORTER, 1, 1);
    Init_item(ITEM_MIRROR, 1, 1);
    Init_item(ITEM_DEFLECTOR, 1, 1);
    Init_item(ITEM_HYPERJUMP, 1, 1);
    Init_item(ITEM_PHASING, 1, 1);
    Init_item(ITEM_LASER, 1, 1);
    Init_item(ITEM_EMERGENCY_THRUST, 1, 1);
    Init_item(ITEM_EMERGENCY_SHIELD, 1, 1);
    Init_item(ITEM_TRACTOR_BEAM, 1, 1);
    Init_item(ITEM_AUTOPILOT, 1, 1);

    Set_misc_item_limits();

    Set_initial_resources();
}


void Set_world_rules(void)
{
    world_t *world = &World;
    static rules_t rules;

    rules.mode =
      ((crashWithPlayer ? CRASH_WITH_PLAYER : 0)
       | (bounceWithPlayer ? BOUNCE_WITH_PLAYER : 0)
       | (playerKillings ? PLAYER_KILLINGS : 0)
       | (playerShielding ? PLAYER_SHIELDING : 0)
       | (limitedVisibility ? LIMITED_VISIBILITY : 0)
       | (limitedLives ? LIMITED_LIVES : 0)
       | (teamPlay ? TEAM_PLAY : 0)
       | (allowAlliances ? ALLIANCES : 0)
       | (timing ? TIMING : 0)
       | (allowNukes ? ALLOW_NUKES : 0)
       | (allowClusters ? ALLOW_CLUSTERS : 0)
       | (allowModifiers ? ALLOW_MODIFIERS : 0)
       | (allowLaserModifiers ? ALLOW_LASER_MODIFIERS : 0)
       | (edgeWrap ? WRAP_PLAY : 0));
    rules.lives = worldLives;
    world->rules = &rules;

    if (BIT(world->rules->mode, TEAM_PLAY))
	CLR_BIT(world->rules->mode, ALLIANCES);

    if (!BIT(world->rules->mode, PLAYER_KILLINGS))
	CLR_BIT(KILLING_SHOTS,
		OBJ_SHOT|OBJ_CANNON_SHOT|OBJ_SMART_SHOT
		|OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_PULSE);

    if (!BIT(world->rules->mode, PLAYER_SHIELDING))
	CLR_BIT(DEF_HAVE, HAS_SHIELD);

    DEF_USED &= DEF_HAVE;
}

void Set_world_asteroids(void)
{
    world_t *world = &World;

    world->asteroids.num = 0;
    Tune_asteroid_prob();
}
