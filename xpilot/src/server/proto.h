/*
 *
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

#ifndef	PROTO_H
#define	PROTO_H

#ifndef OBJECT_H
/* need player */
#include "object.h"
#endif

#ifndef LIST_H_INCLUDED
/* need list_t */
#include "list.h"
#endif

#ifndef MAP_H
/* need treasure_t */
#include "map.h"
#endif

#ifndef DEFAULTS_H
/* need optOrigin */
#include "defaults.h"
#endif

/*
 * Prototypes for cell.c
 */
void Free_cells(void);
void Alloc_cells(void);
void Cell_init_object(object *obj);
void Cell_add_object(object *obj);
void Cell_remove_object(object *obj);
void Cell_get_objects(int x, int y, int r, int max, object ***list, int *count);

/*
 * Prototypes for collision.c
 */
void Check_collision(void);
int wormXY(int x, int y);
int IsOffensiveItem(enum Item i);
int IsDefensiveItem(enum Item i);
int CountOffensiveItems(player *pl);
int CountDefensiveItems(player *pl);
int in_range_acd(double dx, double dy, double dvx, double dvy, double r);
int in_range_simple(int px, int py, int qx, int qy, int r);
int in_range_partial(double dx, double dy, double dvx, double dvy,
		     double r, DFLOAT wall_time);

/*
 * Prototypes for id.c
 */
int peek_ID(void);
int request_ID(void);
void release_ID(int id);

/*
 * Prototypes for walls.c
 */
void Walls_init(void);
void Treasure_init(void);
void Move_init(void);
void Move_object(object *obj);
void Move_player(int ind);
void Turn_player(int ind);
int is_inside(int x, int y, int hitmask, const object *obj);
int shape_is_inside(int cx, int cy, int hitmask, const object *obj,
		    const shipobj *shape, int dir);
int Polys_to_client(unsigned char **);
void Ball_line_init(void);
void Player_crash(player *pl, struct move *move, int crashtype,
		  int item_id, int pt);
void Object_crash(object *obj, struct move *move, int crashtype, int item_id);

/*
 * Prototypes for event.c
 */
int Handle_keyboard(int);
bool Team_zero_pausing_available(void);
void Pause_player(int ind, int onoff);
int Player_lock_closest(int ind, int next);
bool team_dead(int team);
void filter_mods(modifiers *mods);

/*
 * Prototypes for map.c
 */
void Free_map(void);
bool Grok_map(void);
bool Grok_map_options(void);

int Map_get_itemid(int x, int y);
void Map_set_itemid(int x, int y, int ind);
int Map_place_cannon(int cx, int cy, int dir, int team);
int Map_place_base(int cx, int cy, int dir, int team);
int Map_place_fuel(int cx, int cy, int team);
int Map_place_treasure(int cx, int cy, int team, bool empty);
int Map_place_wormhole(int cx, int cy, wormType type);
int Map_place_target(int cx, int cy, int team);
int Map_place_check(int cx, int cy, int index);
int Map_place_item_concentrator(int cx, int cy);
int Map_place_asteroid_concentrator(int cx, int cy);
int Map_place_grav(int cx, int cy, DFLOAT force, int type);

void Find_base_direction(void);
void Compute_gravity(void);
DFLOAT Wrap_findDir(DFLOAT dx, DFLOAT dy);
DFLOAT Wrap_cfindDir(int dx, int dy);
DFLOAT Wrap_length(int dx, int dy);
unsigned short Find_closest_team(int cx, int cy);

int Wildmap(
	int width,
	int height,
	char *name,
	char *author,
	char **data,
	int *width_ptr,
	int *height_ptr);

/*
 * Prototypes for xpmap.c
 */
void Xpmap_print(void);
void Xpmap_grok_map_data(void);
void Xpmap_allocate_checks(void);
void Xpmap_tags_to_internal_data(bool create_objects);
void Xpmap_find_map_object_teams(void);
void Xpmap_find_base_direction(void);
void Xpmap_blocks_to_polygons(void);


/*
 * Prototypes for xp2map.c
 */
bool isXp2MapFile(int fd);
bool parseXp2MapFile(int fd, optOrigin opt_origin);


/*
 * Prototypes for math.c
 */
int ON(char *optval);
int OFF(char *optval);
int mod(int x, int y);
int f2i(DFLOAT f);
DFLOAT findDir(DFLOAT x, DFLOAT y);
DFLOAT rfrac(void);
void Make_table(void);


/*
 * Prototypes for cmdline.c
 */
void tuner_none(void);
void tuner_dummy(void);
bool Init_options(void);
void Free_options(void);
void Timing_setup(void);

/*
 * Prototypes for play.c
 */
void Thrust(int ind);
void Record_shove(player *pl, player *pusher, long time);
void Delta_mv(object *ship, object *obj);
void Delta_mv_elastic(object *obj1, object *obj2);
void Obj_repel(object *obj1, object *obj2, int repel_dist);
void Item_damage(int ind, DFLOAT prob);
void Tank_handle_detach(player*);
void Add_fuel(pl_fuel_t*, long);
void Update_tanks(pl_fuel_t *);
void Place_item(int type, int ind);
int Choose_random_item(void);
void Tractor_beam(int ind);
void General_tractor_beam(int ind, int cx, int cy,
			  int items, int target, bool pressor);
void Place_mine(int ind);
void Place_moving_mine(int ind);
void Place_general_mine(int ind, unsigned short team, long status,
			int cx, int cy, DFLOAT vx, DFLOAT vy, modifiers mods);
void Detonate_mines(int ind);
char *Describe_shot(int type, long status, modifiers mods, int hit);
void Fire_ecm(int ind);
void Fire_general_ecm(int ind, unsigned short team, int cx, int cy);
void Connector_force(int ind);
void Fire_shot(int ind, int type, int dir);
void Fire_general_shot(int ind, unsigned short team, bool cannon,
		       int cx, int cy, int type, int dir,
		       modifiers mods, int target);
void Fire_normal_shots(int ind);
void Fire_main_shot(int ind, int type, int dir);
void Fire_shot(int ind, int type, int dir);
void Fire_left_shot(int ind, int type, int dir, int gun);
void Fire_right_shot(int ind, int type, int dir, int gun);
void Fire_left_rshot(int ind, int type, int dir, int gun);
void Fire_right_rshot(int ind, int type, int dir, int gun);
void Make_treasure_ball(int treasure);
int Punish_team(int ind, int t_destroyed, int cx, int cy);
void Ball_hits_goal(ballobject *ball, int group);
void Ball_is_replaced(ballobject *ball);
void Ball_is_destroyed(ballobject *ball);
bool Balltarget_hitfunc(struct group *group, struct move *move);
int Cannon_hitmask(cannon_t *cannon);
void Cannon_restore_on_map(int ind);
void Cannon_remove_from_map(int ind);
bool Cannon_hitfunc(struct group *group, struct move *move);
int Target_hitmask(target_t *targ);
void Target_init(void);
void Target_restore_on_map(int ind);
void Target_remove_from_map(int ind);
bool Target_hitfunc(struct group *group, struct move *move);
void Wormhole_remove_from_map(int ind);
void Team_immunity_init(void);
void Groups_init(void);
void Delete_shot(int ind);
void Fire_laser(int ind);
void Fire_general_laser(int ind, unsigned short team, int cx, int cy, int dir,
			modifiers mods);
void Do_deflector(int ind);
void Do_transporter(int ind);
void Do_general_transporter(int ind, int cx, int cy, int target,
			    int *item, long *amount);
void do_hyperjump(player *pl);
void do_lose_item(int ind);
void Move_smart_shot(int ind);
void Move_mine(int ind);
void Make_debris(
	    /* pos.x, pos.y   */ int    cx,          int   cy,
	    /* vel.x, vel.y   */ DFLOAT  velx,       DFLOAT vely,
	    /* owner id       */ int    id,
	    /* owner team     */ unsigned short team,
	    /* type           */ int    type,
	    /* mass           */ DFLOAT  mass,
	    /* status         */ long   status,
	    /* color          */ int    color,
	    /* radius         */ int    radius,
	    /* num debris     */ int    num_debris,
	    /* min,max dir    */ int    min_dir,    int    max_dir,
	    /* min,max speed  */ DFLOAT min_speed,  DFLOAT max_speed,
	    /* min,max life   */ DFLOAT min_life,   DFLOAT max_life
	    );
void Make_wreckage(
	    /* pos.x, pos.y   */ int    cx,         int    cy,
	    /* vel.x, vel.y   */ DFLOAT velx,       DFLOAT vely,
	    /* owner id       */ int    id,
	    /* owner team     */ unsigned short team,
	    /* min,max mass   */ DFLOAT min_mass,   DFLOAT max_mass,
	    /* total mass     */ DFLOAT total_mass,
	    /* status         */ long   status,
	    /* color          */ int    color,
	    /* max wreckage   */ int    max_wreckage,
	    /* min,max dir    */ int    min_dir,    int    max_dir,
	    /* min,max speed  */ DFLOAT min_speed,  DFLOAT max_speed,
	    /* min,max life   */ DFLOAT min_life,   DFLOAT max_life
	    );
void Make_item(int cx, int cy,
	       int vx, int vy,
	       int item, int num_per_pack,
	       long status);
void Explode(int ind);
void Explode_fighter(int ind);
void Throw_items(int ind);
void Detonate_items(int ind);
void add_temp_wormholes(int xin, int yin, int xout, int yout);
void remove_temp_wormhole(int ind);


/*
 * Prototypes for asteroid.c
 */
void Break_asteroid(int ind);
void Asteroid_update(void);
#ifdef LIST_H_INCLUDED
list_t Asteroid_get_list(void);
#endif


/*
 * Prototypes for cannon.c
 */
void Cannon_init(int ind);
void Cannon_add_item(int ind, int item, int amount);
void Cannon_throw_items(int ind);
void Cannon_check_defense(int ind);
void Cannon_check_fire(int ind);
void Cannon_dies(int ind, player *pl);

/*
 * Prototypes for command.c
 */
void Handle_player_command(player *pl, char *cmd);
int Get_player_index_by_name(char *name);

/*
 * Prototypes for player.c
 */
void Pick_startpos(int ind);
void Go_home(int ind);
void Compute_sensor_range(player *);
void Player_add_tank(int ind, long tank_fuel);
void Player_remove_tank(int ind, int which_tank);
void Player_hit_armor(int ind);
void Player_used_kill(int ind);
int Init_player(int ind, shipobj *ship);
void Alloc_players(int number);
void Free_players(void);
void Update_score_table(void);
void Reset_all_players(void);
void Check_team_members(int);
void Compute_game_status(void);
void Delete_player(int ind);
void Detach_ball(int ind, int ball);
void Kill_player(int ind);
void Player_death_reset(int ind);
void Team_game_over(int winning_team, const char *reason);
void Individual_game_over(int winner);
void Race_game_over(void);
int Team_immune(int id1, int id2);

/*
 * Prototypes for robot.c
 */
void Parse_robot_file(void);
void Robot_init(void);
void Robot_delete(int ind, int kicked);
void Robot_destroy(int ind);
void Robot_update(void);
void Robot_invite(int ind, int inv_ind);
void Robot_war(int ind, int killer);
void Robot_reset_war(int ind);
int Robot_war_on_player(int ind);
void Robot_go_home(int ind);
void Robot_program(int ind, int victim_id);
void Robot_message(int ind, const char *message);

/*
 * Prototypes for rules.c
 */
void Tune_item_probs(void);
void Tune_item_packs(void);
void Set_initial_resources(void);
void Set_world_items(void);
void Set_world_rules(void);
void Set_world_asteroids(void);
void Set_misc_item_limits(void);
void Tune_asteroid_prob(void);

/*
 * Prototypes for server.c
 */
int End_game(void);
int Pick_team(int pick_for_type);
void Server_info(char *str, unsigned max_size);
void Log_game(const char *heading);
void Game_Over(void);
void Server_log_admin_message(int ind, const char *str);
int plock_server(int onoff);
void Main_loop(void);


/*
 * Prototypes for contact.c
 */
void Contact_cleanup(void);
int Contact_init(void);
void Contact(int fd, void *arg);
void Queue_kick(const char *nick);
void Queue_loop(void);
int Queue_advance_player(char *name, char *msg);
int Queue_show_list(char *msg);
void Set_deny_hosts(void);

/*
 * Prototypes for metaserver.c
 */
void Meta_send(char *mesg, int len);
int Meta_from(char *addr, int port);
void Meta_gone(void);
void Meta_init(void);
void Meta_update(int change);

/*
 * Prototypes for frame.c
 */
void Frame_update(void);
void Set_message(const char *message);
void Set_player_message(player *pl, const char *message);

/*
 * Prototypes for update.c
 */
void Update_radar_target(int);
void Update_objects(void);
void Autopilot(int ind, int on);
void Cloak(int ind, int on);
void Deflector(int ind, int on);
void Emergency_thrust(int ind, int on);
void Emergency_shield(int ind, int on);
void Phasing(int ind, int on);

/*
 * Prototypes for option.c
 */
void Options_parse(void);
void Options_free(void);
bool Convert_string_to_int(const char *value_str, int *int_ptr);
bool Convert_string_to_float(const char *value_str, DFLOAT *float_ptr);
bool Convert_string_to_bool(const char *value_str, bool *bool_ptr);
void Convert_list_to_string(list_t list, char **string);
void Convert_string_to_list(const char *value, list_t *list_ptr);

/*
 * Prototypes for parser.c
 */
int Parser_list_option(int *index, char *buf);
bool Parser(int argc, char **argv);
int Tune_option(char *name, char *val);
int Get_option_value(const char *name, char *value, unsigned size);

/*
 * Prototypes for fileparser.c
 */
bool parseDefaultsFile(const char *filename);
bool parsePasswordFile(const char *filename);
bool parseMapFile(const char *filename);
void expandKeyword(const char *keyword);

/*
 * Prototypes for laser.c
 */
void Laser_pulse_collision(void);
void Laser_pulse_hits_player(int ind, pulseobject *pulse);

/*
 * Prototypes for alliance.c
 */
int Invite_player(int ind, int ally_ind);
int Cancel_invitation(int ind);
int Refuse_alliance(int ind, int ally_ind);
int Refuse_all_alliances(int ind);
int Accept_alliance(int ind, int ally_ind);
int Accept_all_alliances(int ind);
int Get_alliance_member_count(int id);
void Player_join_alliance(int ind, int ally_ind);
void Dissolve_all_alliances(void);
int Leave_alliance(int ind);
void Alliance_player_list(int ind);

/*
 * Prototypes for object.c
 */
object *Object_allocate(void);
void Object_free_ind(int ind);
void Object_free_ptr(object *obj);
void Alloc_shots(int number);
void Free_shots(void);
char *Object_typename(object *obj);

/*
 * Prototypes for polygon.c
 */
void P_edgestyle(char *id, int width, int color, int style);
void P_polystyle(char *id, int color, int texture_id, int defedge_id,
		 int flags);
void P_bmpstyle(char *id, char *filename, int flags);
void P_start_polygon(int cx, int cy, int style);
void P_offset(int offcx, int offcy, int edgestyle);
void P_vertex(int cx, int cy, int edgestyle);
void P_end_polygon(void);
int P_start_ballarea(void);
void P_end_ballarea(void);
int P_start_balltarget(int team);
void P_end_balltarget(void);
int P_start_target(int team, int ind);
void P_end_target(void);
int P_start_cannon(int team, int ind);
void P_end_cannon(void);
int P_start_wormhole(int ind);
void P_end_wormhole(void);
void P_start_decor(void);
void P_end_decor(void);
int P_start_frictionarea(void);
void P_end_frictionarea(void);
int P_get_bmp_id(const char *s);
int P_get_edge_id(const char *s);
int P_get_poly_id(const char *s);
void P_grouphack(int type, void (*f)(int));
void P_set_hitmask(int group, int hitmask);

/*
 * Prototypes for showtime.c
 */
char *showtime(void);

/*
 * Prototypes for srecord.c
 */
void Init_recording(void);
void Handle_recording_buffers(void);

/*
 * Prototypes for rank.c
 */
void Rank_init_saved_scores(void);
void Rank_get_saved_score(player *pl);
void Rank_save_data(void);
void Rank_web_scores(void);
void Rank_save_score(const player *pl);
void Rank_show_standings(void);
void Rank_kill(player *pl);
void Rank_lost_ball(player *pl);
void Rank_cashed_ball(player *pl);
void Rank_won_ball(player *pl);
void Rank_saved_ball(player *pl);
void Rank_death(player *pl);
void Rank_add_score(player *pl, DFLOAT points);
void Rank_set_score(player *pl, DFLOAT points);
void Rank_fire_shot(player *pl);

#endif
