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

#ifndef SERVER_H
#define SERVER_H

#ifndef OBJECT_H
# include "object.h"
#endif

#ifndef LIST_H
# include "list.h"
#endif

#ifndef MAP_H
# include "map.h"
#endif

#ifndef DEFAULTS_H
# include "defaults.h"
#endif

#ifndef WALLS_H
# include "walls.h"
#endif

/*
 * When using this, add a final realloc later to free wasted memory
 */
#define STORE(T,P,N,M,V)						\
    if (N >= M && ((M <= 0)						\
	? (P = (T *) malloc((M = 1) * sizeof(*P)))			\
	: (P = (T *) realloc(P, (M += M) * sizeof(*P)))) == NULL) {	\
	warn("No memory");						\
	exit(1);							\
    } else								\
	(P[N++] = V)

typedef struct {
    char	owner[80];
    char	host[80];
} server_t;

/*
 * Global data.
 */

#define FPS		options.framesPerSecond
#define NumObjs		(ObjCount + 0)
#define MAX_SPECTATORS	8

extern object_t		*Obj[];
extern ecm_t		*Ecms[];
extern trans_t		*Transporters[];
extern long		frame_loops;
extern long		frame_loops_slow;
extern double		frame_time;
extern int		spectatorStart;
extern int		NumPlayers;
extern int		NumSpectators;
extern int		NumOperators;
extern int		NumPseudoPlayers;
extern int		NumQueuedPlayers;
extern int		ObjCount;
extern int		NumEcms;
extern int		NumTransporters;
extern int		NumAlliances;
extern int		NumRobots;
extern int		login_in_progress;
extern char		ShutdownReason[];
extern sock_t		contactSocket;
extern time_t		serverTime;
extern server_t		Server;
extern char		*serverAddr;
extern long		DEF_BITS, KILL_BITS, DEF_HAVE, DEF_USED, USED_KILL;
extern int		ShutdownServer, ShutdownDelay;
extern long		main_loops;
extern int		mapRule;
extern bool		teamAssign;
extern int		tagItPlayerId;
extern bool		updateScores;
extern bool		allowPlayerPasswords;
extern int		game_lock;
extern int		mute_baseless;
extern time_t		gameOverTime;
extern double		friction;
extern int		roundtime;
extern int		roundsPlayed;
extern long		KILLING_SHOTS;
extern unsigned		SPACE_BLOCKS;
extern double		timeStep;
extern double		timePerFrame;
extern double		ecmSizeFactor;
extern char		*playerPasswordsFileName;
extern int		playerPasswordsFileSizeLimit;
extern double		coriolisCosine, coriolisSine;

extern struct options {
    int		maxRobots;
    int		minRobots;
    char	*robotFile;
    int		robotsTalk;
    int		robotsLeave;
    int		robotLeaveLife;
    double	robotLeaveScore;
    double	robotLeaveRatio;
    int		robotTeam;
    bool	restrictRobots;
    bool	reserveRobotTeam;
    int 	robotTicksPerSecond;
    list_t	expandList;		/* Predefined settings. */
    double	shotMass;
    double	shipMass;
    double	shotSpeed;
    double	gravity;
    double	ballMass;
    double	minItemMass;
    int		maxPlayerShots;
    double	shotLife;
    double	pulseSpeed;
    double	pulseLength;
    double	pulseLife;
    bool	shotsGravity;
    bool	shotHitFuelDrainUsesKineticEnergy;
    double	fireRepeatRate;
    double	laserRepeatRate;
    bool	Log;
    bool	silent;
    bool	RawMode;
    bool	NoQuit;
    bool	logRobots;
    int		framesPerSecond;
    char	*mapFileName;
    char	*mapData;
    int		mapWidth;
    int		mapHeight;
    char	*mapName;
    char	*mapAuthor;
    int 	contactPort;
    char	*serverHost;
    char	*greeting;
    bool	crashWithPlayer;
    bool	bounceWithPlayer;
    bool	playerKillings;
    bool	playerShielding;
    bool	playerStartsShielded;
    bool	shotsWallBounce;
    bool	ballsWallBounce;
    bool	ballCollisions;
    bool	ballSparkCollisions;
    bool	minesWallBounce;
    bool	itemsWallBounce;
    bool	missilesWallBounce;
    bool	sparksWallBounce;
    bool	debrisWallBounce;
    bool	asteroidsWallBounce;
    bool	pulsesWallBounce;
    bool	cloakedExhaust;
    bool	cloakedShield;
    bool	ecmsReprogramMines;
    bool	ecmsReprogramRobots;
    double	maxObjectWallBounceSpeed;
    double	maxShieldedWallBounceSpeed;
    double	maxUnshieldedWallBounceSpeed;
    double	playerWallBrakeFactor;
    double	objectWallBrakeFactor;
    double	objectWallBounceLifeFactor;
    double	wallBounceFuelDrainMult;
    double	wallBounceDestroyItemProb;
    double	afterburnerPowerMult;

    bool	limitedVisibility;
    double	minVisibilityDistance;
    double	maxVisibilityDistance;
    bool	limitedLives;
    int		worldLives;
    bool	endOfRoundReset;
    int		resetOnHuman;
    bool	allowAlliances;
    bool	announceAlliances;
    bool	teamPlay;
    bool	teamFuel;
    bool	teamCannons;
    int		cannonSmartness;
    bool	cannonsUseItems;
    bool	cannonsDefend;
    bool	cannonFlak;
    double	cannonDeadTicks;
    bool	keepShots;
    bool	teamImmunity;
    bool	teamShareScore;
    bool	tagGame;
    bool	timing;
    bool	ballrace;
    bool	ballrace_connect;
    bool	edgeWrap;
    bool	edgeBounce;
    bool	extraBorder;
    ipos_t	gravityPoint;
    double	gravityAngle;
    bool	gravityPointSource;
    bool	gravityClockwise;
    bool	gravityAnticlockwise;
    bool	gravityVisible;
    bool	wormholeVisible;
    bool	itemConcentratorVisible;
    bool	asteroidConcentratorVisible;
    double	wormholeStableTicks;
    char	*defaultsFileName;
    char	*passwordFileName;
    int		nukeMinSmarts;
    int		nukeMinMines;
    double	nukeClusterDamage;
    double	nukeDebrisLife;
    double	mineFuseTicks;
    double	mineLife;
    double	minMineSpeed;
    double	missileLife;
    double	baseMineRange;
    double	mineShotDetonateDistance;

    double	shotKillScoreMult;
    double	torpedoKillScoreMult;
    double	smartKillScoreMult;
    double	heatKillScoreMult;
    double	clusterKillScoreMult;
    double	laserKillScoreMult;
    double	tankKillScoreMult;
    double	runoverKillScoreMult;
    double	ballKillScoreMult;
    double	explosionKillScoreMult;
    double	shoveKillScoreMult;
    double	crashScoreMult;
    double	mineScoreMult;
    double	selfKillScoreMult;
    double	selfDestructScoreMult;
    double	unownedKillScoreMult;
    double	tagItKillScoreMult;
    double	tagKillItScoreMult;
    double	asteroidPoints;
    double	cannonPoints;
    double	asteroidMaxScore;
    double	cannonMaxScore;

    double	destroyItemInCollisionProb;
    bool 	allowSmartMissiles;
    bool 	allowHeatSeekers;
    bool 	allowTorpedoes;
    bool 	allowNukes;
    bool	allowClusters;
    bool	allowModifiers;
    bool	allowLaserModifiers;
    bool	allowShipShapes;

    bool	shieldedItemPickup;
    bool	shieldedMining;
    bool	laserIsStunGun;
    bool	targetKillTeam;
    bool	targetSync;
    double	targetDeadTicks;
    bool	reportToMetaServer;
    bool	searchDomainForXPilot;
    char	*denyHosts;

    bool	playersOnRadar;
    bool	missilesOnRadar;
    bool	minesOnRadar;
    bool	nukesOnRadar;
    bool	treasuresOnRadar;
    bool	asteroidsOnRadar;
    bool 	identifyMines;
    bool	distinguishMissiles;
    int		maxMissilesPerPack;
    int		maxMinesPerPack;
    bool	targetTeamCollision;
    bool	treasureKillTeam;
    bool	captureTheFlag;
    bool	treasureCollisionDestroys;
    bool	treasureCollisionMayKill;
    bool	wreckageCollisionMayKill;
    bool	asteroidCollisionMayKill;

    double	ballConnectorSpringConstant;
    double	ballConnectorDamping;
    double	maxBallConnectorRatio;
    double	ballConnectorLength;
    bool	connectorIsString;
    double	ballRadius;

    double 	dropItemOnKillProb;
    double	detonateItemOnKillProb;
    double 	movingItemProb;
    double	randomItemProb;
    double	rogueHeatProb;
    double	rogueMineProb;
    double	itemProbMult;
    double	cannonItemProbMult;
    double	asteroidItemProb;
    int		asteroidMaxItems;
    double	maxItemDensity;
    double	maxAsteroidDensity;
    double	itemConcentratorRadius;
    double	itemConcentratorProb;
    double	asteroidConcentratorRadius;
    double	asteroidConcentratorProb;
    double	gameDuration;
    bool	baselessPausing;
    double	pauseTax;
    int		pausedFPS;
    int		waitingFPS;

    char	*motdFileName;
    char	*scoreTableFileName;
    char	*adminMessageFileName;
    int		adminMessageFileSizeLimit;
    char	*rankFileName;
    char	*rankWebpageFileName;
    double	frictionSetting;
    double	blockFriction;
    bool	blockFrictionVisible;
    double	coriolis;
    double	checkpointRadius;
    int		raceLaps;
    bool	lockOtherTeam;
    bool	loseItemDestroys;
    int		maxOffensiveItems;
    int		maxDefensiveItems;

    int		maxVisibleObject;
    bool	pLockServer;
    bool	sound;
    int		timerResolution;

    int		maxRoundTime;
    int		roundsToPlay;

    bool	useWreckage;
    bool	ignore20MaxFPS;
    char	*password;

    char	*robotUserName;
    char	*robotHostName;

    char	*tankUserName;
    char	*tankHostName;
    double	tankScoreDecrement;

    bool	selfImmunity;

    char	*defaultShipShape;
    char	*tankShipShape;

    int		clientPortStart;
    int		clientPortEnd;

    int		maxPauseTime;
    int		maxIdleTime;
    int		maxClientsPerIP;

    int		playerLimit;
    int		playerLimit_orig;
    int		recordMode;
    int		recordFlushInterval;
    int		constantScoring;
    int		eliminationRace;
    char	*dataURL;
    char	*recordFileName;
    double	gameSpeed;
    bool	maraTurnqueue;
    bool	ngControls;
    double	constantSpeed;
    bool	ballStyles;
    bool	ignoreMaxFPS;
    bool	polygonMode;
    bool	teamcup;
    char	*teamcupStatServer;
    int		teamcupStatPort;
    int		teamcupMatchNumber;
} options;


extern shape_t		ball_wire;
extern shape_t		wormhole_wire;

/* determine if a block is one of SPACE_BLOCKS */
#define EMPTY_SPACE(s)	BIT(1U << (s), SPACE_BLOCKS)

static inline vector_t World_gravity(world_t *world, clpos_t pos)
{
    return world->gravity[CLICK_TO_BLOCK(pos.cx)][CLICK_TO_BLOCK(pos.cy)];
}

enum TeamPickType {
    PickForHuman	= 1,
    PickForRobot	= 2
};

#ifndef	_WINDOWS
#define	APPNAME	"xpilot-ng-server"
#else
#define	APPNAME	"XPilotNGServer"
#endif


/*
 * Prototypes for cell.c
 */
void Free_cells(world_t *world);
void Alloc_cells(world_t *world);
void Cell_init_object(world_t *world, object_t *obj);
void Cell_add_object(world_t *world, object_t *obj);
void Cell_remove_object(world_t *world, object_t *obj);
void Cell_get_objects(world_t *world, clpos_t pos, int r, int max, object_t ***list, int *count);

/*
 * Prototypes for collision.c
 */
void Check_collision(world_t *world);
int IsOffensiveItem(enum Item i);
int IsDefensiveItem(enum Item i);
int CountOffensiveItems(player_t *pl);
int CountDefensiveItems(player_t *pl);

/*
 * Prototypes for id.c
 */
int peek_ID(void);
int request_ID(void);
void release_ID(int id);

/*
 * Prototypes for walls.c
 */
void Groups_init(void);
void Walls_init(world_t *world);
void Treasure_init(world_t *world);
void Move_init(world_t *world);
void Move_object(object_t *obj);
void Move_player(player_t *pl);
void Turn_player(player_t *pl);
int is_inside(int x, int y, hitmask_t hitmask, const object_t *obj);
int shape_is_inside(int cx, int cy, hitmask_t hitmask, const object_t *obj,
		    const shape_t *s, int dir);
int Polys_to_client(unsigned char **);
void Ball_line_init(world_t *world);
void Player_crash(player_t *pl, int crashtype, int mapobj_ind, int pt);
void Object_crash(object_t *obj, int crashtype, int mapobj_ind);

/*
 * Prototypes for event.c
 */
int Handle_keyboard(player_t *pl);
void Pause_player(player_t *pl, bool on);
int Player_lock_closest(player_t *pl, bool next);
bool team_dead(int team);
void filter_mods(world_t *world, modifiers_t *mods);

/*
 * Prototypes for map.c
 */
void World_init(world_t *world);
void World_free(world_t *world);
bool Grok_map(world_t *world);
bool Grok_map_options(world_t *world);

int World_place_base(world_t *world, clpos_t pos, int dir, int team);
int World_place_cannon(world_t *world, clpos_t pos, int dir, int team);
int World_place_check(world_t *world, clpos_t pos, int ind);
int World_place_fuel(world_t *world, clpos_t pos, int team);
int World_place_grav(world_t *world, clpos_t pos, double force, int type);
int World_place_target(world_t *world, clpos_t pos, int team);
int World_place_treasure(world_t *world, clpos_t pos, int team, bool empty,
			 int ball_style);
int World_place_wormhole(world_t *world, clpos_t pos, wormType type);
int World_place_item_concentrator(world_t *world, clpos_t pos);
int World_place_asteroid_concentrator(world_t *world, clpos_t pos);
int World_place_friction_area(world_t *world, clpos_t pos, double fric);

void World_add_temporary_wormholes(world_t *world, clpos_t pos1, clpos_t pos2);
void Wormhole_line_init(world_t *world);

void Compute_gravity(world_t *world);
double Wrap_findDir(double dx, double dy);
double Wrap_cfindDir(int dx, int dy);
double Wrap_length(int dx, int dy);
int Find_closest_team(world_t *world, clpos_t pos);


/*
 * Prototypes for xpmap.c
 */
setup_t *Xpmap_init_setup(world_t *world);
void Xpmap_print(void);
void Xpmap_grok_map_data(world_t *world);
void Xpmap_allocate_checks(world_t *world);
void Xpmap_tags_to_internal_data(world_t *world, bool create_objects);
void Xpmap_find_map_object_teams(world_t *world);
void Xpmap_find_base_direction(world_t *world);
void Xpmap_blocks_to_polygons(world_t *world);


/*
 * Prototypes for xp2map.c
 */
bool isXp2MapFile(int fd);
bool parseXp2MapFile(int fd, optOrigin opt_origin, world_t *world);


/*
 * Prototypes for cmdline.c
 */
void tuner_none(world_t *world);
void tuner_dummy(world_t *world);
void Check_playerlimit(world_t *world);
void Timing_setup(world_t *world);
bool Init_options(void);
void Free_options(void);

/*
 * Prototypes for play.c
 */
void Thrust(player_t *pl);
void Record_shove(player_t *pl, player_t *pusher, long shove_time);
void Delta_mv(object_t *ship, object_t *obj);
void Delta_mv_elastic(object_t *obj1, object_t *obj2);
void Obj_repel(object_t *obj1, object_t *obj2, int repel_dist);
void Item_damage(player_t *pl, double prob);
void Tank_handle_detach(player_t *pl);
void Add_fuel(pl_fuel_t *, double);
void Update_tanks(pl_fuel_t *);
void Place_item(world_t *world, player_t *pl, int type);
int Choose_random_item(world_t *world);
void Tractor_beam(player_t *pl);
void General_tractor_beam(world_t *world, player_t *pl, clpos_t pos,
			  int items, player_t *victim, bool pressor);
void Place_mine(player_t *pl);
void Place_moving_mine(player_t *pl);
void Place_general_mine(world_t *world, player_t *pl, int team, long status,
			clpos_t pos, vector_t vel, modifiers_t mods);
void Detonate_mines(player_t *pl);
char *Describe_shot(int type, long status, modifiers_t mods, int hit);
void Fire_ecm(player_t *pl);
void Fire_general_ecm(world_t *world, player_t *pl, int team, clpos_t pos);
void Update_connector_force(world_t *world, ballobject_t *ball);
void Fire_shot(player_t *pl, int type, int dir);
void Fire_general_shot(world_t *world, player_t *pl, int team, bool cannon,
		       clpos_t pos, int type, int dir,
		       modifiers_t mods, int target_id);
void Fire_normal_shots(player_t *pl);
void Fire_main_shot(player_t *pl, int type, int dir);
void Fire_left_shot(player_t *pl, int type, int dir, int gun);
void Fire_right_shot(player_t *pl, int type, int dir, int gun);
void Fire_left_rshot(player_t *pl, int type, int dir, int gun);
void Fire_right_rshot(player_t *pl, int type, int dir, int gun);

bool Friction_area_hitfunc(group_t *groupptr, move_t *move);

void Team_immunity_init(world_t *world);
void Hitmasks_init(world_t *world);
void Transfer_tag(player_t *oldtag_pl, player_t *newtag_pl);
/*double Handle_tag(double score, player_t *victim_pl, player_t* killer_pl);*/
void Check_tag(void);
void Delete_shot(world_t *world, int ind);
void Fire_laser(player_t *pl);
void Fire_general_laser(world_t *world, player_t *pl, int team, clpos_t pos,
			int dir, modifiers_t mods);
void Do_deflector(player_t *pl);
void Do_transporter(player_t *pl);
void Do_general_transporter(world_t *world, player_t *pl, clpos_t pos,
			    player_t *victim, int *item, double *amount);
bool Initiate_hyperjump(player_t *pl);
void do_lose_item(player_t *pl);
void Update_torpedo(world_t *world, torpobject_t *torp);
void Update_missile(world_t *world, missileobject_t *shot);
void Update_mine(world_t *world, mineobject_t *mine);
void Make_debris(world_t  *world,
		 clpos_t  pos,
		 vector_t vel,
		 int      owner_id,
		 int      owner_team,
		 int      type,
		 double   mass,
		 long     status,
		 int      color,
		 int      radius,
		 int      num_debris,
		 int      min_dir,    int    max_dir,
		 double   min_speed,  double max_speed,
		 double   min_life,   double max_life);
void Make_wreckage(world_t  *world,
		   clpos_t  pos,
		   vector_t vel,
		   int      owner_id,
		   int      owner_team,
		   double   min_mass,   double max_mass,
		   double   total_mass,
		   long     status,
		   int      color,
		   int      max_wreckage,
		   int      min_dir,    int    max_dir,
		   double   min_speed,  double max_speed,
		   double   min_life,   double max_life);
void Make_item(world_t *world, clpos_t pos,
	       vector_t vel,
	       int item, int num_per_pack,
	       long status);
void Explode_fighter(player_t *pl);
void Throw_items(player_t *pl);
void Detonate_items(player_t *pl);
void add_temp_wormholes(world_t *world, int xin, int yin, int xout, int yout);
void remove_temp_wormhole(world_t *world, int ind);


/*
 * Prototypes for asteroid.c
 */
void Break_asteroid(world_t *world, wireobject_t *asteroid);
void Asteroid_update(world_t *world);
list_t Asteroid_get_list(void);
void Asteroid_line_init(world_t *world);

/*
 * Prototypes for cannon.c
 */
void Cannon_init(cannon_t *cannon);
void Cannon_add_item(cannon_t *cannon, int item, double amount);
void Cannon_throw_items(cannon_t *cannon);
void Cannon_check_defense(cannon_t *cannon);
void Cannon_check_fire(cannon_t *cannon);
void Cannon_dies(cannon_t *cannon, player_t *pl);
hitmask_t Cannon_hitmask(cannon_t *cannon);
void Cannon_set_hitmask(int group, cannon_t *cannon);
bool Cannon_hitfunc(group_t *groupptr, move_t *move);
void World_restore_cannon(world_t *world, cannon_t *cannon);
void World_remove_cannon(world_t *world, cannon_t *cannon);


/*
 * Prototypes for command.c
 */
void Handle_player_command(player_t *pl, char *cmd);
player_t *Get_player_by_name(const char *str,
			     int *errcode, const char **errorstr_p);


/*
 * Prototypes for player.c
 */
player_t *Players(int ind);
int GetInd(int id);

static inline player_t *Player_by_id(int id)
{
    int ind = GetInd(id);

#if 0
    if (ind < 0 || ind >= NumPlayers) {
	warn("ind = %d, (ind < 0 || ind >= NumPlayers)", ind);
	return NULL;
    }
#endif

    return Players(ind);
}

static inline bool Player_is_playing(player_t *pl)
{
    if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER|KILLED) == PLAYING)
	return true;
    return false;
}

static inline bool Player_is_active(player_t *pl)
{
    if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER) == PLAYING)
	return true;
    return false;
}

static inline bool Player_is_waiting(player_t *pl)
{
    if (BIT(pl->status, GAME_OVER) && pl->mychar == 'W')
	return true;
    return false;
}

static inline bool Player_is_self_destructing(player_t *pl)
{
    return (pl->self_destruct_count > 0.0) ? true : false;
}

static inline void Player_self_destruct(player_t *pl, bool on)
{
    if (on) {
	if (Player_is_self_destructing(pl))
	    return;
	pl->self_destruct_count = SELF_DESTRUCT_DELAY;
    }
    else
	pl->self_destruct_count = 0.0;
}

static inline bool Player_is_tank(player_t *pl)
{
    return (BIT(pl->type_ext, OBJ_EXT_TANK) == OBJ_EXT_TANK) ? true : false;
}

static inline bool Player_is_robot(player_t *pl)
{
    return (BIT(pl->type_ext, OBJ_EXT_ROBOT) == OBJ_EXT_ROBOT) ? true : false;
}

static inline bool Player_is_human(player_t *pl)
{
    return (!BIT(pl->type_ext, OBJ_EXT_TANK|OBJ_EXT_ROBOT)) ? true : false;
}

static inline bool Player_owns_tank(player_t *pl, player_t *tank)
{
    if (Player_is_tank(tank)
	&& tank->lock.pl_id != NO_ID  /* kps - probably redundant */
	&& tank->lock.pl_id == pl->id)
	return true;
    return false;
}

/*
 * Used where we wish to know if a player is simply on the same team.
 * Replacement for TEAM
 */
static inline bool Players_are_teammates(player_t *pl1, player_t *pl2)
{
    world_t *world = &World;

    if (BIT(world->rules->mode, TEAM_PLAY)
	&& pl1->team != TEAM_NOT_SET
	&& pl1->team == pl2->team)
	return true;
    return false;
}

/*
 * Used where we wish to know if two players are members of the same alliance.
 * Replacement for ALLIANCE
 */
static inline bool Players_are_allies(player_t *pl1, player_t *pl2)
{
    if (pl1->alliance != ALLIANCE_NOT_SET
	&& pl1->alliance == pl2->alliance)
	return true;
    return false;
}

void Pick_startpos(player_t *pl);
void Go_home(player_t *pl);
void Compute_sensor_range(player_t *pl);
void Player_add_tank(player_t *pl, double tank_fuel);
void Player_remove_tank(player_t *pl, int which_tank);

static inline void Player_add_fuel(player_t *pl, double amount)
{
    Add_fuel(&(pl->fuel), amount);
}

static inline bool Player_used_emergency_shield(player_t *pl)
{
    if (BIT(pl->used, (HAS_SHIELD|HAS_EMERGENCY_SHIELD)) ==
	(HAS_SHIELD|HAS_EMERGENCY_SHIELD))
	return true;
    return false;
}

void Player_hit_armor(player_t *pl);
void Player_used_kill(player_t *pl);
void Player_set_mass(player_t *pl);
int Init_player(world_t *world, int ind, shipshape_t *ship);
void Alloc_players(int number);
void Free_players(void);
void Update_score_table(world_t *world);
void Reset_all_players(world_t *world);
void Check_team_members(world_t *world, int);
void Compute_game_status(world_t *world);
void Delete_player(player_t *pl);
void Add_spectator(player_t *pl);
void Delete_spectator(player_t *pl);
void Detach_ball(player_t *pl, ballobject_t *ball);
void Kill_player(player_t *pl, bool add_rank_death);
void Player_death_reset(player_t *pl, bool add_rank_death);
void Team_game_over(world_t *world, int winning_team, const char *reason);
void Individual_game_over(world_t *world, int winner);
void Race_game_over(world_t *world);
bool Team_immune(int id1, int id2);

static inline void Player_set_float_dir(player_t *pl, double new_float_dir)
{
    if (options.ngControls && new_float_dir != pl->float_dir) {
	pl->float_dir = new_float_dir;
	pl->float_dir_cos = cos(pl->float_dir * 2.0 * PI / RES);
	pl->float_dir_sin = sin(pl->float_dir * 2.0 * PI / RES);
    } else
	pl->float_dir = new_float_dir;
}

/*
 * Prototypes for robot.c
 */
void Parse_robot_file(void);
void Robot_init(world_t *world);
void Robot_delete(player_t *robot, bool kicked);
void Robot_destroy(player_t *robot);
void Robot_update(world_t *world);
void Robot_invite(player_t *robot, player_t *inviter);
void Robot_war(player_t *robot, player_t *killer);
void Robot_reset_war(player_t *robot);
int Robot_war_on_player(player_t *robot);
void Robot_go_home(player_t *robot);
void Robot_program(player_t *robot, int victim_id);
void Robot_message(player_t *robot, const char *message);

/*
 * Prototypes for rules.c
 */
void Tune_item_probs(world_t *world);
void Tune_item_packs(world_t *world);
void Set_initial_resources(world_t *world);
void Set_world_items(world_t *world);
void Set_world_rules(world_t *world);
void Set_world_asteroids(world_t *world);
void Set_misc_item_limits(world_t *world);
void Tune_asteroid_prob(world_t *world);

/*
 * Prototypes for server.c
 */
int End_game(void);
int Pick_team(int pick_for_type);
void Server_info(char *str, size_t max_size);
void Log_game(const char *heading);
const char *Describe_game_status(void);
void Game_Over(void);
void Server_log_admin_message(player_t *pl, const char *str);
int plock_server(bool on);
void Main_loop(void);


/*
 * Prototypes for contact.c
 */
void Contact_cleanup(void);
int Contact_init(void);
void Contact(int fd, void *arg);
void Queue_kick(const char *nick);
void Queue_loop(world_t *world);
int Queue_advance_player(char *name, char *msg, size_t size);
int Queue_show_list(char *msg, size_t size);
void Set_deny_hosts(world_t *world);

/*
 * Prototypes for metaserver.c
 */
void Meta_send(char *mesg, size_t len);
int Meta_from(char *addr, int port);
void Meta_gone(void);
void Meta_init(void);
void Meta_update(int change);

/*
 * Prototypes for frame.c
 */
void Frame_update(void);
void Set_message(const char *message);
void Set_player_message(player_t *pl, const char *message);
void Set_message_f(const char *format, ...);
void Set_player_message_f(player_t *pl, const char *format, ...);

/*
 * Prototypes for update.c
 */
void Update_objects(world_t *world);
void Autopilot(player_t *pl, bool on);
void Cloak(player_t *pl, bool on);
void Deflector(player_t *pl, bool on);
void Emergency_thrust(player_t *pl, bool on);
void Emergency_shield(player_t *pl, bool on);
void Phasing(player_t *pl, bool on);

/*
 * Prototypes for option.c
 */
void Options_parse(void);
void Options_free(void);
bool Convert_string_to_int(const char *value_str, int *int_ptr);
bool Convert_string_to_float(const char *value_str, double *float_ptr);
bool Convert_string_to_bool(const char *value_str, bool *bool_ptr);
void Convert_list_to_string(list_t list, char **string);
void Convert_string_to_list(const char *value, list_t *list_ptr);

/*
 * Prototypes for parser.c
 */
int Parser_list_option(int *ind, char *buf);
bool Parser(int argc, char **argv, world_t *world);
int Tune_option(char *name, char *val);
int Get_option_value(const char *name, char *value, size_t size);

/*
 * Prototypes for fileparser.c
 */
bool parseDefaultsFile(const char *filename, world_t *world);
bool parsePasswordFile(const char *filename, world_t *world);
bool parseMapFile(const char *filename, world_t *world);
void expandKeyword(const char *keyword);

/*
 * Prototypes for laser.c
 */
void Laser_pulse_hits_player(player_t *pl, pulseobject_t *pulse);

/*
 * Prototypes for alliance.c
 */
int Invite_player(player_t *pl, player_t *ally);
int Cancel_invitation(player_t *pl);
int Refuse_alliance(player_t *pl, player_t *ally);
int Refuse_all_alliances(player_t *pl);
int Accept_alliance(player_t *pl, player_t *ally);
int Accept_all_alliances(player_t *pl);
int Get_alliance_member_count(int id);
void Player_join_alliance(player_t *pl, player_t *ally);
void Dissolve_all_alliances(void);
int Leave_alliance(player_t *pl);
void Alliance_player_list(player_t *pl);

/*
 * Prototypes for object.c
 */
object_t *Object_allocate(void);
void Object_free_ind(int ind);
void Object_free_ptr(object_t *obj);
void Alloc_shots(world_t *world, int number);
void Free_shots(world_t *world);
const char *Object_typename(object_t *obj);

/*
 * Prototypes for polygon.c
 */
void P_edgestyle(const char *id, int width, int color, int style);
void P_polystyle(const char *id, int color, int texture_id, int defedge_id,
		 int flags);
void P_bmpstyle(const char *id, const char *filename, int flags);
void P_start_polygon(clpos_t pos, int style);
void P_offset(clpos_t offset, int edgestyle);
void P_vertex(clpos_t pos, int edgestyle);
void P_end_polygon(void);
int P_start_ballarea(void);
void P_end_ballarea(void);
int P_start_balltarget(int team, int treasure_ind);
void P_end_balltarget(void);
int P_start_target(int target_ind);
void P_end_target(void);
int P_start_cannon(int cannon_ind);
void P_end_cannon(void);
int P_start_wormhole(int wormhole_ind);
void P_end_wormhole(void);
void P_start_decor(void);
void P_end_decor(void);
int P_start_friction_area(int fa_ind);
void P_end_friction_area(void);
int P_get_bmp_id(const char *s);
int P_get_edge_id(const char *s);
int P_get_poly_id(const char *s);
/*void P_grouphack(int type, void (*f)(int group, void *mapobj));*/
void P_set_hitmask(int group, hitmask_t hitmask);

/*
 * Prototypes for showtime.c
 */
char *showtime(void);

/*
 * Prototypes for srecord.c
 */
void Init_recording(world_t *world);
void Handle_recording_buffers(void);
void Get_recording_data(void);

/*
 * Prototypes for target.c
 */
void Target_update(world_t *world);
void Object_hits_target(object_t *obj, target_t *targ, double player_cost);
hitmask_t Target_hitmask(target_t *targ);
void Target_set_hitmask(int group, target_t *targ);
void Target_init(world_t *world);
void World_restore_target(world_t *world, target_t *targ);
void World_remove_target(world_t *world, target_t *targ);

/*
 * Prototypes for treasure.c
 */
void Make_treasure_ball(world_t *world, treasure_t *t);
void Ball_hits_goal(ballobject_t *ball, group_t *groupptr);
void Ball_is_replaced(ballobject_t *ball);
void Ball_is_destroyed(ballobject_t *ball);
bool Balltarget_hitfunc(group_t *groupptr, move_t *move);

/*
 * Prototypes for wormhole.c
 */
void Traverse_wormhole(player_t *pl);
void Hyperjump(player_t *pl);
void Object_hits_wormhole(object_t *obj, int ind);
hitmask_t Wormhole_hitmask(wormhole_t *wormhole);
bool Wormhole_hitfunc(group_t *groupptr, move_t *move);
void World_remove_wormhole(world_t *world, wormhole_t *wormhole);


#endif
