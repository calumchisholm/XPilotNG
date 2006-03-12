/* 
 * XPilot NG, a multiplayer space war game.
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
/* Rewrite started by Karsten Siegmund - in progress */
#include "xpserver.h"

#define ROB_LOOK_AH		2

#define WITHIN(NOW,THEN,DIFF) (NOW<=THEN && (THEN-NOW)<DIFF)

/*
 * Flags for the suibot (well, default...) robots being in different modes (or moods).
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
 * Prototypes for methods of the suibot.
 */
static void Robot_suibot_round_tick(void);
static void Robot_suibot_create(player_t *pl, char *str);
static void Robot_suibot_go_home(player_t *pl);
static void Robot_suibot_play(player_t *pl);
static void Robot_suibot_set_war(player_t *pl, int victim_id);
static int Robot_suibot_war_on_player(player_t *pl);
static void Robot_suibot_message(player_t *pl, const char *str);
static void Robot_suibot_destroy(player_t *pl);
static void Robot_suibot_invite(player_t *pl, player_t *inviter);
       int Robot_suibot_setup(robot_type_t *type_ptr);




/*
 * The robot type structure for the suibot - contains the functions 
 * that will be called from robot.c
 */
static robot_type_t robot_suibot_type = {
    "suibot",
    Robot_suibot_round_tick,
    Robot_suibot_create,
    Robot_suibot_go_home,
    Robot_suibot_play,
    Robot_suibot_set_war,
    Robot_suibot_war_on_player,
    Robot_suibot_message,
    Robot_suibot_destroy,
    Robot_suibot_invite
};


/*
 * Local static variables
 */
static double	Visibility_distance;
static double	Max_enemy_distance;

/*
 * The only thing we export from this file.
 * A function to initialize the robot type structure
 * with our name and the pointers to our action routines.
 *
 * Return 0 if all is OK, anything else will ignore this
 * robot type forever.
 */
int Robot_suibot_setup(robot_type_t *type_ptr)
{
    /* Just init the type structure. */

    *type_ptr = robot_suibot_type;

    return 0;
}

/*
 * Private functions.
 */
static bool Check_robot_evade(player_t *pl, int mine_i, int ship_i);
static bool Detect_ship(player_t *pl, player_t *ship);
static int Rank_item_value(player_t *pl, enum Item itemtype);
static bool Ball_handler(player_t *pl);
static void Robot_move_randomly(player_t *pl);


/*
 * Function to cast from player structure to robot data structure.
 * This isolates casts (aka. type violations) to a few places.
 */
static robot_default_data_t *Robot_suibot_get_data(player_t *pl)
{
    return (robot_default_data_t *)pl->robot_data_ptr->private_data;
}

/*
 * A suibot is created.
 */
static void Robot_suibot_create(player_t *pl, char *str)
{
    robot_default_data_t *my_data;

    if (!(my_data = XMALLOC(robot_default_data_t, 1))) {
	error("no mem for default robot");
	End_game();
    }

    pl->turnspeed = 0; /* we play with "mouse" */
    pl->turnresistance = 0;

    my_data->robot_mode      = RM_TAKE_OFF;
    my_data->robot_count     = 0;
    my_data->robot_lock      = LOCK_NONE;
    my_data->robot_lock_id   = 0;

    my_data->longterm_mode	= 0;

    pl->robot_data_ptr->private_data = (void *)my_data;
}

/*
 * A suibot is placed on its homebase.
 */
static void Robot_suibot_go_home(player_t *pl)
{
    robot_default_data_t *my_data = Robot_suibot_get_data(pl);

    my_data->robot_mode      = RM_TAKE_OFF;
    my_data->longterm_mode   = 0;
}

/*
 * A default robot is declaring war (or resetting war).
 */
static void Robot_suibot_set_war(player_t *pl, int victim_id)
{
    robot_default_data_t *my_data = Robot_suibot_get_data(pl);

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
static int Robot_suibot_war_on_player(player_t *pl)
{
    robot_default_data_t *my_data = Robot_suibot_get_data(pl);

    if (BIT(my_data->robot_lock, LOCK_PLAYER))
	return my_data->robot_lock_id;
    else
	return NO_ID;
}

/*
 * A default robot receives a message.
 */
static void Robot_suibot_message(player_t *pl, const char *message)
{
    UNUSED_PARAM(pl); UNUSED_PARAM(message);
}

/*
 * A default robot is destroyed.
 */
static void Robot_suibot_destroy(player_t *pl)
{
    XFREE(pl->robot_data_ptr->private_data);
}

/*
 * A default robot is asked to join an alliance
 */
static void Robot_suibot_invite(player_t *pl, player_t *inviter)
{
    int war_id = Robot_suibot_war_on_player(pl), i;
    robot_default_data_t *my_data = Robot_suibot_get_data(pl);
    double limit;
    bool we_accept = true;

    if (pl->alliance != ALLIANCE_NOT_SET) {
	/* if there is a human in our alliance, they should decide
	   let robots refuse in this case */
	for (i = 0; i < NumPlayers; i++) {
	    player_t *pl_i = Player_by_index(i);

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
    limit = MAX(ABS( Get_Score(pl) / MAX((my_data->attack / 10), 10)),
		my_data->defense);
    if (inviter->alliance == ALLIANCE_NOT_SET) {
	/* don't accept players we are at war with */
	if (inviter->id == war_id)
	    we_accept = false;
	/* don't accept players who are not active */
	if (!Player_is_active(inviter))
	    we_accept = false;
	/* don't accept players with scores substantially lower than ours */
	else if ( Get_Score(inviter) < ( Get_Score(pl) - limit))
	    we_accept = false;
    }
    else {
	double avg_score = 0;
	int member_count = Get_alliance_member_count(inviter->alliance);

	for (i = 0; i < NumPlayers; i++) {
	    player_t *pl_i = Player_by_index(i);
	    if (pl_i->alliance == inviter->alliance) {
		if (pl_i->id == war_id) {
		    we_accept = false;
		    break;
		}
		avg_score +=  Get_Score(pl_i);
	    }
	}
	if (we_accept) {
	    avg_score = avg_score / member_count;
	    if (avg_score < ( Get_Score(pl) - limit))
		we_accept = false;
	}
    }
    if (we_accept)
	Accept_alliance(pl, inviter);
    else
	Refuse_alliance(pl, inviter);
}

static inline int decide_travel_dir(player_t *pl)
{
    double gdir;

    if (pl->velocity <= 0.2) {
	vector_t grav = World_gravity(pl->pos);

	gdir = findDir(grav.x, grav.y);
    } else
	gdir = findDir(pl->vel.x, pl->vel.y);

    return MOD2((int) (gdir + 0.5), RES);
}


static void Robot_take_off_from_base(player_t *pl);

static void Robot_take_off_from_base(player_t *pl)
{ 
  robot_default_data_t  *my_data = Robot_suibot_get_data(pl);
  /*Function that could do specific things when robot takes off*/  
  
  Robot_move_randomly(pl);

  if((rfrac())<0.1 ){
    my_data->robot_mode = RM_ATTACK;
  }
}


/*KS: let robot "play mouse" */
static void Robot_set_pointing_direction(player_t *pl,int direction);

static void Robot_set_pointing_direction(player_t *pl,int direction)
{ 
  robot_default_data_t	*my_data = Robot_suibot_get_data(pl);
  int turnvel;     

    if(pl->turnspeed != 0 | pl->turnresistance != 0) End_game();


   turnvel = (direction - pl->dir);
   pl->turnvel = turnvel;
}

/*KS: describe func here.....*/
/* void Obj_pos_set(object *obj, int cx, int cy); */

/* void Obj_pos_set(object *obj, int cx, int cy){ */
/*     struct _objposition		*pos = (struct _objposition *)&obj->pos; */



/*     pos->cx = cx; */
/*     pos->x = CLICK_TO_PIXEL(cx); */
/*     pos->bx = pos->x / BLOCK_SZ; */
/*     pos->cy = cy; */
/*     pos->y = CLICK_TO_PIXEL(cy); */
/*     pos->by = pos->y / BLOCK_SZ; */
/*     obj->prevpos.x = obj->pos.x; */
/*     obj->prevpos.y = obj->pos.y; */

/* } */

struct collans {
    int line;
    int point;
    clvec_t moved;
};

static bool Wall_in_between_points(int cx1, int cy1, int cx2, int cy2);
static bool Wall_in_between_points(int cx1, int cy1, int cx2, int cy2){ /* Wall between two given points?*/
  
  struct collans answer;
  move_t mv;
  mv.delta.cx = WRAP_DCX(cx2 - cx1);
  mv.delta.cy = WRAP_DCY(cy2 - cy1);
  mv.start.cx = WRAP_XCLICK(cx1);
  mv.start.cy = WRAP_YCLICK(cy1);
  mv.obj   = NULL;
  mv.hitmask  = NONBALL_BIT;

  while (mv.delta.cx || mv.delta.cy) {
    Move_point(&mv, &answer);
    if (answer.line != -1)
      return true; //answer.line;
    mv.start.cx = WRAP_XCLICK(mv.start.cx + answer.moved.cx);
    mv.start.cy = WRAP_YCLICK(mv.start.cy + answer.moved.cy);
    mv.delta.cx -= answer.moved.cx;
    mv.delta.cy -= answer.moved.cy;
  }
  return false;

}


bool Robot_evade_shot(player_t *pl);

struct dangerous_shot_data{
  double hit_time;
  double sqdistance;
};

bool Robot_evade_shot(player_t *pl){

/*     struct dangerous_shot_data *shotsarray; */
  int j;
  object_t *shot, **obj_list;
  int  obj_count;
  long killing_shots;
  //  player_t *opponent;
  
      killing_shots = KILLING_SHOTS;
    if (options.treasureCollisionMayKill)
        killing_shots |= OBJ_BALL;
    if (options.wreckageCollisionMayKill)
        killing_shots |= OBJ_WRECKAGE;
    if (options.asteroidCollisionMayKill)
        killing_shots |= OBJ_ASTEROID;
    if (!options.allowShields)
	killing_shots |=  OBJ_PLAYER;

    robot_default_data_t *my_data = Robot_suibot_get_data(pl);

    const int                   max_objs = 1000;
    int shot_dist;
    double time_shot_closest, shortest_hit_time;
    double delta_velx, delta_vely, delta_x, delta_y, sqdistance;
    double sqship_sz;
    Cell_get_objects(pl->pos, (int)(Visibility_distance / BLOCK_SZ),
		     max_objs, &obj_list, &obj_count);
    

    shortest_hit_time=10000;
    int closest_shot = -1;

    for (j = 0; j < obj_count; j++) { /*for .. obj_count*/

	shot = obj_list[j];

	/* Get rid of most objects */
	if (!BIT(shot->type, killing_shots ))
	    continue;

	delta_velx=( shot->vel.x -  pl->vel.x );
	delta_vely=( shot->vel.y -  pl->vel.y );
	delta_x=   WRAP_DCX( shot->pos.cx - pl->pos.cx );
	delta_y=   WRAP_DCY( shot->pos.cy - pl->pos.cy );

	/* prevent possible division by 0 */
	if(delta_velx == 0 || delta_vely == 0) 
	  continue;
	/* get time of "hit" from deviation of distance function */


	time_shot_closest = 
	  -( delta_x * delta_velx + delta_y * delta_vely) /
	  ((sqr(delta_velx) + sqr(delta_vely))); 


	if((time_shot_closest < 0) || (time_shot_closest > 400))
          /*option instead of fixed value: options.dodgetime))*/
	  continue;
	
	sqdistance = 
	  (sqr(delta_velx) + sqr(delta_vely)) * sqr(time_shot_closest)  +
	  2 * (delta_velx * delta_x + delta_vely * delta_y) * time_shot_closest +
	  sqr(delta_x) + sqr(delta_y);


	sqship_sz = sqr(1.1 * PIXEL_TO_CLICK(SHIP_SZ));

		if(sqdistance > sqship_sz)
	  continue;

		if(Wall_in_between_points(
				  pl->pos.cx + time_shot_closest * pl->vel.x, 
				  pl->pos.cy + time_shot_closest * pl->vel.y, 
				  shot->pos.cx + time_shot_closest * shot->vel.x,
				  shot->pos.cy + time_shot_closest * shot->vel.y
				  ))
	 continue;

	if(shortest_hit_time > time_shot_closest){
	  shortest_hit_time = time_shot_closest;
	  closest_shot=j;
	}
    }

    if((closest_shot == -1)) {return false; }


    shot = obj_list[closest_shot];
	delta_velx=( shot->vel.x -  pl->vel.x );
	delta_vely=( shot->vel.y -  pl->vel.y );
	delta_x=   WRAP_DCX( shot->pos.cx - pl->pos.cx );
	delta_y=   WRAP_DCY( shot->pos.cy - pl->pos.cy );


    double direction_pl,direction_evade1,direction_evade2;
    double norm_vel= sqrt(sqr(delta_velx)+sqr(delta_vely));
    double norm_xy = sqrt(sqr(delta_x)+sqr(delta_y));
    double evade_x = -(delta_velx / norm_vel  + delta_x / norm_xy);
    double evade_y = -(delta_vely / norm_vel  + delta_y / norm_xy);

    direction_evade1 = findDir(evade_x, evade_y );

  Robot_set_pointing_direction(pl, direction_evade1);
    Thrust(pl, true);

    return true;

}


void Robot_move_randomly(player_t *pl){
  int direction;

  /* Move randomly */
  if(rfrac()<0.25) 
    pl->turnvel = ((rfrac()*RES)- RES / 2)*0.3;

  if(pl->velocity > options.maxUnshieldedWallBounceSpeed){ /* not too fast...*/
                            
    direction= (int)findDir(-pl->vel.x,-pl->vel.y);
    Robot_set_pointing_direction(pl, direction);
    Thrust(pl, true);
    return;
  }
  
  /* Fire, too */
  if((rfrac())>0.98 ){
        Fire_normal_shots(pl);  
  }
  /* Sometimes thrust */
  if((rfrac())>0.7 )
    {  
      Thrust(pl, true); 
    } 
  else{
    Thrust(pl, false);
    }
  
} 

void Robot_pop_ball(player_t *pl,ballobject_t *ball){
int direction; 

 double delta_velx, delta_vely, delta_x, delta_y;
 double time_ball_closest;

	delta_velx=( ball->vel.x -  pl->vel.x );
	delta_vely=( ball->vel.y -  pl->vel.y );
	delta_x=   WRAP_DCX( ball->pos.cx - pl->pos.cx );
	delta_y=   WRAP_DCY( ball->pos.cy - pl->pos.cy );

	/* prevent possible division by 0 */
	if(delta_velx == 0 || delta_vely == 0) 
	  time_ball_closest =0;
	else 
	time_ball_closest = 
	  -( delta_x * delta_velx + delta_y * delta_vely) /
	  ((sqr(delta_velx) + sqr(delta_vely))); 

    direction=(int)(Wrap_cfindDir(ball->pos.cx - pl->pos.cx 
				  + delta_velx * abs(time_ball_closest) / 2,
				  ball->pos.cy - pl->pos.cy  
				  + delta_vely * abs(time_ball_closest) / 2));
Robot_set_pointing_direction(pl, direction);
    Thrust(pl, true);
    return;
  }



void Robot_suicide_player(player_t *pl, player_t *pl_to_suicide);
void Robot_suicide_player(player_t *pl, player_t *pl_to_suicide){/*Suicide_player*/
  
  int                         dx,dy;
  double                     tmp_a,tmp_b,tmp_c,direction,t1,t,t2;

  // check if we can fire or have to wait because of repeat rate
  // if (frame_time <= pl->shot_time + options.fireRepeatRate - timeStep + 1e-3)
  dx = WRAP_DCX(pl_to_suicide->pos.cx - pl->pos.cx)/CLICK;
  dy = WRAP_DCY(pl_to_suicide->pos.cy - pl->pos.cy)/CLICK;
  
  /*Find direction, where a shot will hit a ship with constant velocity*/
  /* use tmp_vars to try to keep it readable */
  tmp_a=dx*(pl_to_suicide->vel.x - pl->vel.x)+dy*(pl_to_suicide->vel.y - pl->vel.y);
  
  tmp_b= sqr(pl_to_suicide->vel.x - pl->vel.x) + sqr(pl_to_suicide->vel.y - pl->vel.y) 
         - sqr(options.shotSpeed);
  
  tmp_c = (sqr(tmp_a) - tmp_b*(sqr(dx)+sqr(dy)));
  
 if( tmp_c >= 0) { /* square-root only if number positive*/
   tmp_c = sqrt(tmp_c);

    t1 = (-tmp_a - tmp_c)/tmp_b;
    t2 = (-tmp_a + tmp_c)/tmp_b;
    t = 0;
    
    /* t (=time) must be greater than 0, but as small as possible... 
     if problem can't be solved point where (??)*/
 
if (t1 >= 0 && t2 >= 0) { 
   if (t1 > t2){ t = t2;} else {t = t1;}
   Fire_normal_shots(pl);
 }
 else if( t2 >= 0 ) {t = t2; Fire_normal_shots(pl);} 
 else if( t1 >= 0 ) {t = t1; Fire_normal_shots(pl);}
 else {t = LENGTH(dx, dy)/options.shotSpeed;
 if(rfrac() >0.5){ Fire_normal_shots(pl);}
 }

 }else{t1=-1;t2=-1;
/*  t=sqrt(sqr(dx)+sqr(dy))/options.shotSpeed;   */
 t = -tmp_a/tmp_b;
 if (t <0){t = sqrt(sqr(dx)+sqr(dy))/options.shotSpeed;  /*printf("And t negative\n");*/ }
 /*  printf ("Sqrt of negative, no solution!\n"); */  
 /* sqrt(neg.number) -> no solution*/


 if(rfrac() >0.4){ Fire_normal_shots(pl);}
 }
 

 dx = dx + t * (pl_to_suicide->vel.x - pl->vel.x) /*+ pl_to_suicide->acc.x * t * sqr(sqr(rfrac()))*/;
 dy = dy + t * (pl_to_suicide->vel.y - pl->vel.y) /*+ pl_to_suicide->acc.y * t * sqr(sqr(rfrac()))*/;

 /* actually, it would be accleration * sqr(time); but this is much too much for even relatively
    small times */
 /* slightly bias shooting towads where a player thrusts */
 if(rfrac() > 0.75){
   dx = dx + pl_to_suicide->acc.x * t * sqr(sqr(rfrac()));
   dy = dy + pl_to_suicide->acc.y * t * sqr(sqr(rfrac()));
 }

    //    printf("t %f sqr t %f\n", t, sqr(t));
/*     printf("nch:dx: %i dy: %i\n",dx,dy);   */
   
    /*    if(dy == 0){direction =  RES * ((dx>=0) ? 0 : 0.5 ); }
     else{
    direction =  RES /(6.283185307) * acos(dx / sqrt(sqr(dx)+sqr(dy)));
    }
     if(dy <0){direction = -direction;} 
    */
    direction= (int)findDir(dx, dy);

     if(rfrac() > 0.75){ direction +=  ((rfrac()-0.5 )*10);} 
  


  Robot_set_pointing_direction(pl, (int)(abs(direction+0.5)));
     Thrust(pl, true); 

}

/*KS: end*/



/*
 * Calculate minimum of length of hypotenuse in triangle with sides
 * 'dcx' and 'dcy' and 'min', taking into account wrapping.
 * Unit is clicks.
 */
static inline double Wrap_length_min(double dcx, double dcy, double min)
{
    double len;

    dcx = WRAP_DCX(dcx), dcx = ABS(dcx);
    if (dcx >= min)
	return min;
    dcy = WRAP_DCY(dcy), dcy = ABS(dcy);
    if (dcy >= min)
	return min;

    len = LENGTH(dcx, dcy);

    return MIN(len, min);
}


static void Robotdef_fire_laser(player_t *pl)
{
    robot_default_data_t *my_data = Robot_suibot_get_data(pl);
    double x2, y2, x3, y3, x4, y4, x5, y5;
    double ship_dist, dir3, dir4, dir5;
    clpos_t m_gun;
    player_t *ship;

    if (BIT(my_data->robot_lock, LOCK_PLAYER)
	&& Player_is_active(Player_by_id(my_data->robot_lock_id)))
	ship = Player_by_id(my_data->robot_lock_id);
    else if (BIT(pl->lock.tagged, LOCK_PLAYER))
	ship = Player_by_id(pl->lock.pl_id);
    else
	return;

    /* kps - this should be Player_is_alive() ? */
    if (!Player_is_active(ship))
	return;

    m_gun = Ship_get_m_gun_clpos(pl->ship, pl->dir);
    x2 = CLICK_TO_PIXEL(pl->pos.cx) + pl->vel.x
	+ CLICK_TO_PIXEL(m_gun.cx);
    y2 = CLICK_TO_PIXEL(pl->pos.cy) + pl->vel.y
	+ CLICK_TO_PIXEL(m_gun.cy);
    x3 = CLICK_TO_PIXEL(ship->pos.cx) + ship->vel.x;
    y3 = CLICK_TO_PIXEL(ship->pos.cy) + ship->vel.y;

    ship_dist = Wrap_length(PIXEL_TO_CLICK(x3 - x2),
			    PIXEL_TO_CLICK(y3 - y2)) / CLICK;

    if (ship_dist >= options.pulseSpeed * options.pulseLife + SHIP_SZ)
	return;

    dir3 = Wrap_findDir(x3 - x2, y3 - y2);
    Robot_set_pointing_direction(pl, dir3);

	SET_BIT(pl->used, HAS_LASER);
}

static bool Detect_ship(player_t *pl, player_t *ship)
{
    double distance;

    /* can't go after non-playing ships */
    if (!Player_is_alive(ship))
	return false;

    /* can't do anything with phased ships */
    if (Player_is_phasing(ship))
	return false;

    /* trivial */
    if (pl->visibility[GetInd(ship->id)].canSee)
	return true;

    /*
     * can't see it, so it must be cloaked
     * maybe we can detect it's presence from other clues?
     */
    distance = Wrap_length(ship->pos.cx - pl->pos.cx,
			   ship->pos.cy - pl->pos.cy) / CLICK;
    /* can't detect ships beyond visual range */
    if (distance > Visibility_distance)
	return false;

    if (Player_is_thrusting(ship)
	&& options.cloakedExhaust)
	return true;

    if (BIT(ship->used, HAS_SHOT)
	|| BIT(ship->used, HAS_LASER)
	|| Player_is_refueling(ship)
	|| Player_is_repairing(ship)
	|| Player_uses_connector(ship)
	|| Player_uses_tractor_beam(ship))
	return true;

    if (BIT(ship->have, HAS_BALL))
	return true;

    /* the sky seems clear.. */
    return false;
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
static int Rank_item_value(player_t *pl, enum Item itemtype)
{
    robot_default_data_t *my_data = Robot_suibot_get_data(pl);

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
		  ? my_data->fuel_l1
		  : my_data->fuel_l2))
	    return ROBOT_MUST_HAVE_ITEM;		/* ahh fuel at last */
	else
	    return ROBOT_HANDY_ITEM;
    }
    if (BIT(world->rules->mode, TIMING)) {
	switch (itemtype) {
	case ITEM_FUEL:		/* less refuel stops */
	case ITEM_REARSHOT:	/* shoot competitors behind you */
	case ITEM_AFTERBURNER:	/* the more speed the better */
	case ITEM_TRANSPORTER:	/* steal fuel when you overtake someone */
	case ITEM_MINE:		/* blows others off the track */
	case ITEM_ECM:		/* blinded players smash into walls */
	case ITEM_EMERGENCY_THRUST:	/* makes you go really fast */
	case ITEM_EMERGENCY_SHIELD:	/* could be useful when ramming */
	    return ROBOT_MUST_HAVE_ITEM;
	case ITEM_WIDEANGLE:	/* not important in racemode */
	case ITEM_CLOAK:	/* not important in racemode */
	case ITEM_SENSOR:	/* who cares about seeing others? */
	case ITEM_TANK:		/* makes you heavier */
	case ITEM_MISSILE:	/* likely to hit self */
	case ITEM_LASER:	/* cost too much fuel */
	case ITEM_TRACTOR_BEAM:	/* pushes/pulls owner off the track too */
	case ITEM_AUTOPILOT:	/* probably not useful */
	case ITEM_DEFLECTOR:	/* cost too much fuel */
	case ITEM_HYPERJUMP:	/* likely to end up in wrong place */
	case ITEM_PHASING:	/* robots don't know how to use them yet */
	case ITEM_MIRROR:	/* not important in racemode */
	case ITEM_ARMOR:	/* makes you heavier */
	    return ROBOT_IGNORE_ITEM;
	default:		/* unknown */
	    warn("Rank_item_value: unknown item %ld.", itemtype);
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
		|| !options.allowPlayerKilling)
		return ROBOT_HANDY_ITEM;
	    else
		return ROBOT_MUST_HAVE_ITEM;

	case ITEM_MISSILE:
	    if (options.maxPlayerShots <= 0
		|| options.shotLife <= 0
		|| !options.allowPlayerKilling)
		return ROBOT_IGNORE_ITEM;
	    else
		return ROBOT_MUST_HAVE_ITEM;

	case ITEM_MINE:
	case ITEM_CLOAK:
	    return ROBOT_MUST_HAVE_ITEM;

	case ITEM_LASER:
	    if (options.allowPlayerKilling)
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

static void Robot_suibot_play(player_t *pl)
{
  player_t *ship;
  int direction;
  double distance, ship_dist, enemy_dist, speed, x_speed, y_speed;
  int item_dist, mine_dist, item_i, mine_i;
  int j, ship_i, item_imp, enemy_i, shoot_time;
  bool harvest_checked, evade_checked, navigate_checked;
  robot_default_data_t *my_data = Robot_suibot_get_data(pl);
  
  /*KS: mods*/
  double ship_dist_closest;
  player_t *closest_opponent;
  closest_opponent= NULL;
  const int maxdist =  1200; /* maximum distance from which to try to pop ball*/
  double ball_dist;
  
  pl->turnspeed = 0;
  pl->turnacc = 0;
  pl->power = MAX_PLAYER_POWER;
  
  if(my_data->robot_mode == RM_TAKE_OFF){
    Robot_take_off_from_base(pl);
    return;
  }
  
  if(Robot_evade_shot(pl)){
    return;
  }
  //    Thrust(pl, false);
  
  ship_dist_closest= 2* World.hypotenuse;
  for (ship_i = 0; ship_i < NumPlayers; ship_i++) {
   player_t *ship = Player_by_index(ship_i); 
    ship_dist = 
      CLICK_TO_PIXEL((int)
	(Wrap_length((pl->pos.cx - ship->pos.cx),
	       (pl->pos.cy - ship->pos.cy))));
    
    if(BIT(ship->have, HAS_BALL ))
      ship_dist = ship_dist/3.0;
    /* Player with ball is considered as "much closer" */
    /* this is rather arbitrary 
       the reasoning goes: dont try to attack player 
       with ball who is really far off
       while some other player is really really close*/
    
    if ((ship->id != pl->id)
	&& Player_is_alive(ship)
	&& ship_dist < ship_dist_closest 
	&& (pl->team != ship->team) 
      	&& ((!BIT(ship->used, HAS_SHIELD))
	    || Wrap_length(ship->pos.cx - pl->pos.cx,
			   ship->pos.cy - pl->pos.cy) < 8000)
	)
      {
	ship_dist_closest = ship_dist;
	closest_opponent = ship; 
      }
  }
  
 if(ship_dist_closest <  maxdist && ! closest_opponent){ /* if not true, there's a bug */ 
   warn(" Robotdef.c: opponent very close, but variable empty!\n");
   //   sprintf (msg,"I hit a bug! Chasing a non-existant opponent! [%s]",pl->name);
   //Set_message(msg);
   return;
 }

 if(ship_dist_closest <  maxdist && BIT(closest_opponent->used, HAS_SHIELD)){
   direction = Wrap_cfindDir(-closest_opponent->pos.cx + pl->pos.cx,
			     -closest_opponent->pos.cy + pl->pos.cy);
   Robot_set_pointing_direction(pl, (int)(abs(direction+0.5)));
   Thrust(pl, true); 
   return;
 }
  

  
  
  /* Closest_ball - should be function, but how do I return a ball + a dist?*/

 ballobject_t *closest_ball, *ball;
 object_t **obj_list, *object;
 //robot_default_data_t *my_data = Robot_suibot_get_data(pl);

 double     closest_ball_dist;
 int     i;
 int     obj_count;
 const int                   max_objs = 1000;
 
 ball_dist = 2 * maxdist;
 closest_ball_dist= 2* maxdist;
 closest_ball=NULL;
 
 Cell_get_objects(pl->pos, (int)(Visibility_distance / BLOCK_SZ),
		  max_objs, &obj_list, &obj_count);
 
 for (i = 0; i < obj_count; i++) { /*for .. obj_count*/
   object = obj_list[i];
   if (object->type == OBJ_BALL) {
     ball= BALL_PTR(object);
     ball_dist = Wrap_length(pl->pos.cx - ball->pos.cx,
			     pl->pos.cy - ball->pos.cy) / CLICK;
     if (ball_dist < closest_ball_dist) {
       closest_ball_dist = ball_dist;
       closest_ball = ball;
     }
   }
 }    
 ball =  closest_ball;
 
 if(ball){
   if (Wall_in_between_points((pl->pos.cx),(pl->pos.cy),(ball->pos.cx),
			      (ball->pos.cy)))
     ball_dist = 2*  World.hypotenuse; 
 }
 



 if(ship_dist_closest <  maxdist
    && ship_dist_closest < (2.5 * ball_dist)
    && (Wall_in_between_points((pl->pos.cx),(pl->pos.cy),(closest_opponent->pos.cx),
			       (closest_opponent->pos.cy)) == 0)
    && (!BIT(pl->have, HAS_BALL))	) {
   Robot_suicide_player(pl,closest_opponent);
   return;
 }
 /*else{
   if(ship_dist_closest >=  maxdist) {printf("Too far away\n");} else
   if(!closest_opponent)             {printf("No opponent\n");} else
   if((Wall_in_between_points((pl->pos.cx),(pl->pos.cy),(closest_opponent->pos.cx),
   (closest_opponent->pos.cy)))) {printf("Wall!\n");} else
   if(BIT(pl->have, HAS_BALL)){ printf("Has ball!\n");} else
   printf("ERROR\n");
   }*/
 

 if( ball 
     && ball_dist < maxdist 
          && Wrap_length(ball->pos.cx - ball->ball_treasure->pos.cx,
     		    ball->pos.cy - ball->ball_treasure->pos.cy
     		    ) > 10000
     ){
   Robot_pop_ball(pl, ball);
   return;
}

/* Helps to get stuck on walls less frequently
   10 propably only works ok with framerate of 50 fps 
   plan: a) make value depend on framerate,
         b) use better algorithm than that
*/

if((pl->last_wall_touch + 11) >= frame_loops) {
   direction= (int)findDir(pl->vel.x,pl->vel.y);
   Robot_set_pointing_direction(pl, direction);

    Thrust(pl, false);
  return;
}
 if((pl->last_wall_touch + 14) >= frame_loops){
   direction= (int)findDir(pl->vel.x,pl->vel.y);
   Robot_set_pointing_direction(pl, direction);
   if(!Wall_in_between_points(pl->pos.cx, pl->pos.cy, 
                              pl->pos.cx + pl->vel.x * 5,
                              pl->pos.cy + pl->vel.y * 5)){
	Thrust(pl, true);
	}

 }


/* nothing sensible to to at the moment */

    Robot_move_randomly(pl);
}

/*KS: mods end*/


/*
 * This is called each round.
 * It allows us to adjust our file local parameters.
 */
static void Robot_suibot_round_tick(void)
{
    double min_visibility = 256.0;
    double min_enemy_distance = 512.0;

    /* reduce visibility when there are a lot of robots. */
    Visibility_distance = min_visibility
	+ (((VISIBILITY_DISTANCE - min_visibility)
	    * (NUM_IDS - NumRobots)) / NUM_IDS);

    /* limit distance to allowable enemies. */
    Max_enemy_distance = world->hypotenuse;
    if (world->hypotenuse > Visibility_distance)
	Max_enemy_distance = min_enemy_distance
	    + (((world->hypotenuse - min_enemy_distance)
		* (NUM_IDS - NumRobots)) / NUM_IDS);
}
