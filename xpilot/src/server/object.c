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

/*
 * This file deals with low-level object structure manipulations.
 */

#include "xpserver.h"

char object_version[] = VERSION;


/*
 * Global variables
 */
int			ObjCount = 0;
int			NumPulses = 0;
int			NumEcms = 0;
int			NumTransporters = 0;
object_t		*Obj[MAX_TOTAL_SHOTS];
ecm_t			*Ecms[MAX_TOTAL_ECMS];
trans_t			*Transporters[MAX_TOTAL_TRANSPORTERS];


static void Object_incr_count(void)
{
    ObjCount++;
}

static void Object_decr_count(void)
{
    ObjCount--;
}

object_t *Object_allocate(void)
{
    object_t	*obj = OBJ_PTR(NULL);

    if (ObjCount < MAX_TOTAL_SHOTS) {
	obj = Obj[ObjCount];
	Object_incr_count();

	obj->type = OBJ_DEBRIS;
	obj->life = 0;
    }

    return obj;
}

void Object_free_ind(int ind)
{
    if ((0 <= ind) && (ind < ObjCount) && (ObjCount <= MAX_TOTAL_SHOTS)) {
	object_t *obj = Obj[ind];
	Object_decr_count();
	Obj[ind] = Obj[ObjCount];
	Obj[ObjCount] = obj;
    } else
	warn("Cannot free object %d, when count = %d, and total = %d !",
	     ind, ObjCount, MAX_TOTAL_SHOTS);
}

void Object_free_ptr(object_t *obj)
{
    int		i;

    for (i = ObjCount - 1; i >= 0; i--) {
	if (Obj[i] == obj) {
	    Object_free_ind(i);
	    break;
	}
    }
    if (i < 0)
	warn("Could NOT free object!");
}

static anyobject_t *objArray;

void Alloc_shots(world_t *world, int number)
{
    anyobject_t		*x;
    int			i;

    x = (anyobject_t *) calloc((size_t)number, sizeof(anyobject_t));
    if (!x) {
	error("Not enough memory for shots.");
	exit(1);
    }

    objArray = x;
    for (i = 0; i < number; i++) {
	Obj[i] = &(x->obj);
	MINE_PTR(Obj[i])->owner = NO_ID;
	Cell_init_object(world, Obj[i]);
	x++;
    }
}

void Free_shots(world_t *world)
{
    UNUSED_PARAM(world);
    XFREE(objArray);
}


/* kps debug hack */
const char *Object_typename(object_t *obj)
{
    int type;

    if (!obj)
	return "none";

    type = obj->type;

    if (type == OBJ_PLAYER)
	return "OBJ_PLAYER";
    if (type == OBJ_DEBRIS)
	return "OBJ_DEBRIS";
    if (type == OBJ_SPARK)
	return "OBJ_SPARK";
    if (type == OBJ_BALL)
	return "OBJ_BALL";
    if (type == OBJ_SHOT)
	return "OBJ_SHOT";
    if (type == OBJ_SMART_SHOT)
	return "OBJ_SMART_SHOT";
    if (type == OBJ_MINE)
	return "OBJ_MINE";
    if (type == OBJ_TORPEDO)
	return "OBJ_TORPEDO";
    if (type == OBJ_HEAT_SHOT)
	return "OBJ_HEAT_SHOT";
    if (type == OBJ_PULSE)
	return "OBJ_PULSE";
    if (type == OBJ_ITEM)
	return "OBJ_ITEM";
    if (type == OBJ_WRECKAGE)
	return "OBJ_WRECKAGE";
    if (type == OBJ_ASTEROID)
	return "OBJ_ASTEROID";
    if (type == OBJ_CANNON_SHOT)
	return "OBJ_CANNON_SHOT";
    return "unknown type";
}
