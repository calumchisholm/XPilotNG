/*
* XpilotStudio, the XPilot Map Editor for Windows 95/98/NT.  Copyright (C) 2000 by
*
*      Jarrod L. Miller           <jlmiller@ctitech.com>
*	The Xpilot Authors	<xpilot@xpilot.org>
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
*
* See the file COPYRIGHT.TXT for current copyright information.
*/

typedef char    max_str_t[255];

typedef struct {
//   blockdata	data[MAX_MAP_SIZE][MAX_MAP_SIZE];
   char		gravity[7];
   char		shipMass[7];
   char		shotMass[7];
   char		shotSpeed[7];
   char		shotLife[4];
   char		fireRepeatRate[7];
   char		maxPlayerShots[4];
   int		shotsGravity;
   int		idleRun;
   int		noQuit;
   char		mapWidth[8];
   char		mapHeight[8];
   max_str_t	mapFileName;
   max_str_t	mapName;
   max_str_t	mapAuthor;
   char		contactPort[7];
   int		allowPlayerCrashes;
   int		allowPlayerBounces;
   int		allowPlayerKilling;
   int		allowShields;
   int		playerStartsShielded;
   int		shotsWallBounce;
   int		ballsWallBounce;
   int		minesWallBounce;
   int		itemsWallBounce;
   int		missilesWallBounce;
   int		sparksWallBounce;
   int		debrisWallBounce;
   int		cloakedExhaust;
   int		cloakedShield;
   char		maxObjectWallBounceSpeed[20];
   char		maxShieldedWallBounceSpeed[20];
   char		maxUnshieldedWallBounceSpeed[20];
   char		playerWallBounceBrakeFactor[20];
   char		objectWallBounceBrakeFactor[20];
   char		objectWallBounceLifeFactor[20];
   char		wallBounceFuelDrainMult[20];
   char		wallBounceDestroyItemProb[20];
   int		reportToMetaServer;
   int		searchDomainForXPilot;
   max_str_t	denyHosts;
   int		limitedVisibility;
   char		minVisibilityDistance[20];
   char		maxVisibilityDistance[20];
   int		limitedLives;
   char		worldLives[4];
   int		reset;
   int		resetOnHuman;
   int		teamPlay;
   int		teamFuel;
   int		teamCannons;
   char		cannonSmartness[1];
   int		cannonsUseItems;
   int		keepShots;
   int		teamImmunity;
   int		ecmsReprogramMines;
   int		targetKillTeam;
   int		targetTeamCollision;
   int		targetSync;
   int		treasureKillTeam;
   int		treasureCollisionDestroys;
   int		treasureCollisionMayKill;
   int		wreckageCollisionMayKill;
   int		ignore20MaxFPS;
   int		timing;
   int		edgeWrap;
   char		gravityPoint[8];
   char		gravityAngle[4];
   int		gravityPointSource;
   int		gravityClockwise;
   int		gravityAnticlockwise;
   int		gravityVisible;
   int		wormholeVisible;
   int		itemConcentratorVisible;
   char		wormTime[20];
   max_str_t	defaultsFileName;
   char		framesPerSecond[7];
   int		allowSmartMissiles;
   int		allowHeatSeekers;
   int		allowTorpedoes;
   int		allowNukes;
   int		allowClusters;
   int		allowModifiers;
   int		allowLaserModifiers;
   int		allowShipShapes;
   int		playersOnRadar;
   int		missilesOnRadar;
   int		minesOnRadar;
   int		nukesOnRadar;
   int		treasuresOnRadar;
   int		distinguishMissiles;
   char		maxMissilesPerPack[7];
   char		maxMinesPerPack[7];
   int		identifyMines;
   int		shieldedItemPickup;
   int		shieldedMining;
   int		laserIsStunGun;
   char		nukeMinSmarts[7];
   char		nukeMinMines[7];
   char		nukeClusterDamage[20];
   char		mineFuseTime[20];
   char		mineLife[20];
   char		missileLife[20];
   char		baseMineRange[20];
   char		shotKillScoreMult[20];
   char		torpedoKillScoreMult[20];
   char		smartKillScoreMult[20];
   char		heatKillScoreMult[20];
   char		clusterKillScoreMult[20];
   char		laserKillScoreMult[20];
   char		tankKillScoreMult[20];
   char		runoverKillScoreMult[20];
   char		ballKillScoreMult[20];
   char		explosionKillScoreMult[20];
   char		shoveKillScoreMult[20];
   char		crashScoreMult[20];
   char		mineScoreMult[20];
   char		movingItemProb[20];
   char		dropItemOnKillProb[20];
   char		detonateItemOnKillProb[20];
   char		destroyItemInCollisionProb[20];
   char		itemProbMult[20];
   char		cannonItemProbMult[20];
   char		maxItemDensity[20];
   char		itemConcentratorRadius[20];
   char		itemConcentratorProb[20];
   char		rogueHeatProb[20];
   char		rogueMineProb[20];
   char		itemEnergyPackProb[20];
   char		itemTankProb[20];
   char		itemECMProb[20];
   char		itemArmorProb[20];
   char		itemMineProb[20];
   char		itemMissileProb[20];
   char		itemCloakProb[20];
   char		itemSensorProb[20];
   char		itemWideangleProb[20];
   char		itemRearshotProb[20];
   char		itemAfterburnerProb[20];
   char		itemTransporterProb[20];
   char		itemMirrorProb[20];
   char		itemDeflectorProb[20];
   char		itemHyperJumpProb[20];
   char		itemPhasingProb[20];
   char		itemLaserProb[20];
   char		itemEmergencyThrustProb[20];
   char		itemTractorBeamProb[20];
   char		itemAutopilotProb[20];
   char		itemEmergencyShieldProb[20];
   char		initialFuel[20];
   char		initialTanks[20];
   char		initialECMs[20];
   char		initialArmor[20];
   char		initialMines[20];
   char		initialMissiles[20];
   char		initialCloaks[20];
   char		initialSensors[20];
   char		initialWideangles[20];
   char		initialRearshots[20];
   char		initialAfterburners[20];
   char		initialTransporters[20];
   char		initialMirrors[20];
   char		maxArmor[20];
   char		initialDeflectors[20];
   char		initialHyperJumps[20];
   char		initialPhasings[20];
   char		initialLasers[20];
   char		initialEmergencyThrusts[20];
   char		initialTractorBeams[20];
   char		initialAutopilots[20];
   char		initialEmergencyShields[20];
   char		maxFuel[20];
   char		maxTanks[20];
   char		maxECMs[20];
   char		maxMines[20];
   char		maxMissiles[20];
   char		maxCloaks[20];
   char		maxSensors[20];
   char		maxWideangles[20];
   char		maxRearshots[20];
   char		maxAfterburners[20];
   char		maxTransporters[20];
   char		maxMirrors[20];
   char		maxDeflectors[20];
   char		maxPhasings[20];
   char		maxHyperJumps[20];
   char		maxEmergencyThrusts[20];
   char		maxLasers[20];
   char		maxTractorBeams[20];
   char		maxAutopilots[20];
   char		maxEmergencyShields[20];
   char		gameDuration[7];
   int		allowViewing;
   char		friction[20];
   char		checkpointRadius[20];
   char		raceLaps[7];
   int		lockOtherTeam;
   int		loseItemDestroys;
   char		roundDelay[7];
   char		maxRoundTime[20];
   int		pLockServer;
   char		timerResolution[20];
   max_str_t	password;
   char		numberOfRounds[7];
/* I don't know if these are final*/
   char		playerLimit[7];
   char		recordMode[7];
   int		constantScoring;
   int		elimination;
/**********************************/
   /*This isn't actually a map or server option, but I
   think it's relevant to the option structure, so here it is.*/
   char		*comments; /*Any comments attatched to the options struct.*/
} xpoption_t, *lpxpoption_t;

/*We define our own point structure for OS compatiblity.
some Windows functions will still have to use the windows POINT struct.*/
typedef struct {
	int x, y;
	int delta_x, delta_y;
	int hidden; //Is the segment drawn to this point hidden?
} XP_POINT;

/*This creates a list of polygons, and should be used for simple
un owned polygons such as walls and decorations.*/
typedef struct polygonlist {
	XP_POINT *vertex;
	int num_verts;
	unsigned short team;
	int selected;
	struct polygonlist *next;
} polygonlist;

typedef struct itemlist {
	XP_POINT pos;
	unsigned short team;
	unsigned short direction;
	unsigned short variant;
	int selected;
	struct itemlist *next;
} itemlist;

typedef struct {
	polygonlist *walls; //List of Wall Polygons
	polygonlist *decors; //List of Decor Polygons
	polygonlist *ballareas; //List of Ball Area Polygons
	polygonlist *balltargets; //List of Ball Target Polygons
	itemlist *targets; //List of Target Items
	itemlist *fuels; //List of Fuel Items
	int num_fuels; //The total number of Fuels
	itemlist *cannons; //List of Cannon Items
	itemlist *balls; //List of Ball Items
	int num_balls; //The total number of Balls
	itemlist *gravities; //List of Gravities Items
	itemlist *circulargravities; //List of Circular Gravities
	itemlist *itemconcentrators; //List of Item Concentrators
	itemlist *bases; //List of Base Items
	int num_bases; //The total number of Bases
	itemlist *currents; //List of Currents Items
	itemlist *wormholes; //List of Wormhole Items
	itemlist *checkpoints; //List of Checkpoint Items
	int num_checkpoints; //The Total number of Checkpoints
} World_map;

typedef struct {
  char                   *name;
  char                   *value;
} charlie;

typedef struct {
   char                  *name;
   char					 *altname;
   int                   length, type;
   char                  *charvar;
   int                   *intvar;
   int			output;
   int			id1;
} prefs_t, *LPPREFSSTRUCT;

typedef struct {
	World_map MapGeometry; //The Map Geometry Structure.
	xpoption_t	MapStruct; //The Map Options Structure.
	prefs_t PrefsArray[226]; //Array for setting default values.
	int	width;   //Decimally Stored Width.
	int height;  //Decimally Stored Height.
	float view_zoom; //The current zoom magnification.
	int view_x;  //The current X scroll location.
	int view_y; //The current Y scroll location.
	int changed; //Has the current map changed?
	polygonlist *selectedpoly; //The selected polygon.
	int numselvert; //The number of the selected vertex.
	itemlist *selecteditem; //The selected Item
	int selectedtype; //The type of thing thats selected.	
	int selectedbool; //Is anything selected.
} MAPDOCUMENT, *LPMAPDOCUMENT;

typedef struct {
	/*To Do: Decide on the ship format, if its changing, otherwise
	use the existing*/
	int	changed; /*Have I been changed, requiring a save?*/
} SHIPDOCUMENT, *LPSHIPDOCUMENT;
/********************MainObjects************************/
typedef struct {
	int type;
	LPMAPDOCUMENT lpMapDocument;
	LPSHIPDOCUMENT lpShipDocument;
/*to do...add pointers to track documents of other types.*/
} XPSTUDIODOCUMENT, *LPXPSTUDIODOCUMENT;

/*An picture segment. 10 possible points for nice pictures*/
typedef struct {
   int		num_points;
   float	x[10]; //distance in x direction
   float	y[10]; //distance in y direction
} segment_t;
