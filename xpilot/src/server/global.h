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

#ifndef	GLOBAL_H
#define	GLOBAL_H

#ifndef OBJECT_H
/* need player */
#include "object.h"
#endif

#ifndef MAP_H
/* need World_map */
#include "map.h"
#endif

#ifndef LIST_H
/* need list_t */
#include "list.h"
#endif

#ifndef MAX
#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

typedef struct {
    char	owner[80];
    char	host[80];
} server;

/*
 * Global data.
 */

#define FPS		framesPerSecond
#define NumObjs		(ObjCount + 0)

extern player		**PlayersArray;
extern object		*Obj[];
extern ecm_t		*Ecms[];
extern trans_t		*Transporters[];
extern long		frame_loops;
extern long		frame_loops_slow;
extern double		frame_time;
extern int		observerStart;
extern int		NumPlayers;
extern int		NumObservers;
extern int		NumOperators;
extern int		NumPseudoPlayers;
extern int		NumQueuedPlayers;
extern int		ObjCount;
extern int		NumEcms;
extern int		NumTransporters;
extern int		NumAlliances;
extern int		NumRobots, maxRobots, minRobots;
extern int		login_in_progress;
extern char		ShutdownReason[];
extern sock_t		contactSocket;
extern time_t		serverTime;
extern char		*robotFile;
extern int		robotsTalk, robotsLeave, robotLeaveLife;
extern int		robotLeaveScore, robotLeaveRatio;
extern int		robotTeam;
extern bool		restrictRobots, reserveRobotTeam;
extern int 		robotTicksPerSecond;
extern World_map	World;
extern server		Server;
extern list_t		expandList;
extern double		ShotsMass, ShipMass, ShotsSpeed, Gravity;
extern double		ballMass;
extern int		ShotsMax;
extern double		ShotsLife;
extern double		pulseSpeed, pulseLength;
extern double		pulseLife;
extern bool		ShotsGravity;
extern bool		shotHitFuelDrainUsesKineticEnergy;
extern double		fireRepeatRate;
extern double		laserRepeatRate;
extern long		DEF_BITS, KILL_BITS, DEF_HAVE, DEF_USED, USED_KILL;
extern int		GetIndArray[];
extern int		ShutdownServer, ShutdownDelay;
extern bool		Log;
extern bool		silent;
extern bool		RawMode;
extern bool		NoQuit;
extern bool		logRobots;
extern int		framesPerSecond;
extern long		main_loops;
extern char		*mapFileName;
extern int		mapRule;
extern char		*mapData;
extern int		mapWidth;
extern int		mapHeight;
extern char		*mapName;
extern char		*mapAuthor;
extern int 		contactPort;
extern char		*serverHost;
extern char		*serverAddr;
extern char		*greeting;
extern bool		crashWithPlayer;
extern bool		bounceWithPlayer;
extern bool		playerKillings;
extern bool		playerShielding;
extern bool		playerStartsShielded;
extern bool		shotsWallBounce;
extern bool		ballsWallBounce;
extern bool		ballCollisions;
extern bool		ballSparkCollisions;
extern bool		minesWallBounce;
extern bool		itemsWallBounce;
extern bool		missilesWallBounce;
extern bool		sparksWallBounce;
extern bool		debrisWallBounce;
extern bool		asteroidsWallBounce;
extern bool		pulsesWallBounce;
extern bool		cloakedExhaust;
extern bool		cloakedShield;
extern bool		ecmsReprogramMines;
extern bool		ecmsReprogramRobots;
extern double		maxObjectWallBounceSpeed;
extern double		maxShieldedWallBounceSpeed;
extern double		maxUnshieldedWallBounceSpeed;
extern double		maxShieldedWallBounceAngle;
extern double		maxUnshieldedWallBounceAngle;
extern double		playerWallBrakeFactor;
extern double		objectWallBrakeFactor;
extern double		objectWallBounceLifeFactor;
extern double		wallBounceFuelDrainMult;
extern double		wallBounceDestroyItemProb;

extern bool		limitedVisibility;
extern double		minVisibilityDistance;
extern double		maxVisibilityDistance;
extern bool		limitedLives;
extern int		worldLives;
extern bool		endOfRoundReset;
extern int		resetOnHuman;
extern bool		allowAlliances;
extern bool		announceAlliances;
extern bool		teamPlay;
extern bool		teamFuel;
extern bool		teamCannons;
extern int		cannonSmartness;
extern bool		cannonsUseItems;
extern bool		cannonsDefend;
extern bool		cannonFlak;
extern int		cannonDeadTime;
extern bool		keepShots;
extern bool		teamAssign;
extern bool		teamImmunity;
extern bool		teamShareScore;
extern bool		timing;
extern bool		ballrace;
extern bool		ballrace_connect;
extern bool		edgeWrap;
extern bool		edgeBounce;
extern bool		extraBorder;
extern ipos		gravityPoint;
extern double		gravityAngle;
extern bool		gravityPointSource;
extern bool		gravityClockwise;
extern bool		gravityAnticlockwise;
extern bool		gravityVisible;
extern bool		wormholeVisible;
extern bool		itemConcentratorVisible;
extern bool		asteroidConcentratorVisible;
extern int		wormTime;
extern int		nukeMinSmarts;
extern int		nukeMinMines;
extern double		nukeClusterDamage;
extern int		mineFuseTime;
extern int		mineLife;
extern double		minMineSpeed;
extern double		missileLife;
extern int		baseMineRange;
extern int		mineShotDetonateDistance;

extern double		shotKillScoreMult;
extern double		torpedoKillScoreMult;
extern double		smartKillScoreMult;
extern double		heatKillScoreMult;
extern double		clusterKillScoreMult;
extern double		laserKillScoreMult;
extern double		tankKillScoreMult;
extern double		runoverKillScoreMult;
extern double		ballKillScoreMult;
extern double		explosionKillScoreMult;
extern double		shoveKillScoreMult;
extern double		crashScoreMult;
extern double		mineScoreMult;
extern double		selfKillScoreMult;
extern double		selfDestructScoreMult;
extern double		unownedKillScoreMult;
extern double		asteroidPoints;
extern double		cannonPoints;
extern double		asteroidMaxScore;
extern double		cannonMaxScore;

extern double		destroyItemInCollisionProb;
extern bool		updateScores;
extern bool 		allowSmartMissiles;
extern bool 		allowHeatSeekers;
extern bool 		allowTorpedoes;
extern bool 		allowNukes;
extern bool		allowClusters;
extern bool		allowModifiers;
extern bool		allowLaserModifiers;
extern bool		allowShipShapes;
extern bool		allowPlayerPasswords;

extern bool		shieldedItemPickup;
extern bool		shieldedMining;
extern bool		laserIsStunGun;
extern bool		targetKillTeam;
extern bool		targetSync;
extern int		targetDeadTime;
extern bool		reportToMetaServer;
extern bool		searchDomainForXPilot;
extern char		*denyHosts;

extern bool		playersOnRadar;
extern bool		missilesOnRadar;
extern bool		minesOnRadar;
extern bool		nukesOnRadar;
extern bool		treasuresOnRadar;
extern bool		asteroidsOnRadar;
extern bool 		identifyMines;
extern bool		distinguishMissiles;
extern int		maxMissilesPerPack;
extern int		maxMinesPerPack;
extern bool		targetTeamCollision;
extern bool		treasureKillTeam;
extern bool		captureTheFlag;
extern bool		treasureCollisionDestroys;
extern bool		treasureCollisionMayKill;
extern bool		wreckageCollisionMayKill;
extern bool		asteroidCollisionMayKill;

extern double		ballConnectorSpringConstant;
extern double		ballConnectorDamping;
extern double		maxBallConnectorRatio;
extern double		ballConnectorLength;
extern bool		connectorIsString;
extern double		ballRadius;

extern double 		dropItemOnKillProb;
extern double		detonateItemOnKillProb;
extern double 		movingItemProb;
extern double		randomItemProb;
extern double            rogueHeatProb;
extern double            rogueMineProb;
extern double		itemProbMult;
extern double		cannonItemProbMult;
extern double		asteroidItemProb;
extern int		asteroidMaxItems;
extern double		maxItemDensity;
extern double		maxAsteroidDensity;
extern int		itemConcentratorRadius;
extern double		itemConcentratorProb;
extern int		asteroidConcentratorRadius;
extern double		asteroidConcentratorProb;
extern double		gameDuration;
extern bool		baselessPausing;
extern int		pausedFPS;
extern int		waitingFPS;
extern int		game_lock;
extern int		mute_baseless;

extern char		*motdFileName;
extern char	       	*scoreTableFileName;
extern char		*adminMessageFileName;
extern int		adminMessageFileSizeLimit;
extern time_t		gameOverTime;

extern double		friction;
extern double		blockFriction;
extern bool		blockFrictionVisible;
extern double		coriolis, coriolisCosine, coriolisSine;
extern double		checkpointRadius;
extern int		raceLaps;
extern bool		lockOtherTeam;
extern bool 		loseItemDestroys;
extern int		maxOffensiveItems;
extern int		maxDefensiveItems;

extern int		maxVisibleObject;
extern bool		pLockServer;
extern int		timerResolution;

extern int		roundDelaySeconds;
extern int		round_delay;
extern int		round_delay_send;
extern int		maxRoundTime;
extern int		roundtime;
extern int		roundsToPlay;
extern int		roundsPlayed;

extern bool		useWreckage;
extern bool		ignore20MaxFPS;
extern char		*password;

extern char		*robotUserName;
extern char		*robotHostName;

extern char		*tankUserName;
extern char		*tankHostName;
extern int		tankScoreDecrement;

extern bool		selfImmunity;

extern char		*defaultShipShape;
extern char		*tankShipShape;

extern int		clientPortStart;
extern int		clientPortEnd;

extern int		maxPauseTime;

extern long		KILLING_SHOTS;
extern unsigned		SPACE_BLOCKS;

extern int		numberOfRounds;
extern int		playerLimit;
extern int		playerLimit_orig;
extern int		recordMode;
extern int		recordFlushInterval;
extern int		constantScoring;
extern int		eliminationRace;
extern char		*dataURL;
extern char		*recordFileName;
extern double		gameSpeed;
extern double		timeStep;
extern double		ecmSizeFactor;
extern char		*playerPasswordsFileName;
extern int		playerPasswordsFileSizeLimit;
extern bool		maraTurnqueue;
extern bool		ignoreMaxFPS;
extern bool		polygonMode;

#define Bases(ind)		(&World.bases[(ind)])
#define Fuels(ind)		(&World.fuels[(ind)])
#define Cannons(ind)		(&World.cannons[(ind)])
#define Checks(ind)		(&World.checks[(ind)])
#define Gravs(ind)		(&World.gravs[(ind)])
#define Targets(ind)		(&World.targets[(ind)])
#define Treasures(ind)		(&World.treasures[(ind)])
#define Wormholes(ind)		(&World.wormholes[(ind)])
#define AsteroidConcs(ind)	(&World.asteroidConcs[(ind)])
#define ItemConcs(ind)		(&World.itemConcs[(ind)])
#define Teams(team)		(&World.teams[(team)])

/* determine if a block is one of SPACE_BLOCKS */
#define EMPTY_SPACE(s)	BIT(1U << (s), SPACE_BLOCKS)

#define Player_by_id(id)	Players(GetInd(id))

static inline vector World_gravity(clpos pos)
{
    return World.gravity[CLICK_TO_BLOCK(pos.cx)][CLICK_TO_BLOCK(pos.cy)];
}

#endif /* GLOBAL_H */

