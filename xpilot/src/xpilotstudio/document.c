/*
* XpilotStudio, the XPilot Map Editor for Windows 95/98/NT.  Copyright (C) 2000 by
*
*      Jarrod L. Miller           <jlmiller@ctitech.com>
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
*
*/

#include "main.h"

/***************************************************************************/
/* CreateNewXpDocument                                                     */
/* Arguments :                                                             */
/*                                                                         */
/* Purpose :   Creates a new map editor window                             */
/***************************************************************************/
LPXPSTUDIODOCUMENT CreateNewXpDocument(int type){
	LPXPSTUDIODOCUMENT lpXpStudioDocument;

	lpXpStudioDocument = (LPXPSTUDIODOCUMENT) malloc(sizeof(XPSTUDIODOCUMENT));
	lpXpStudioDocument->type = type;
	lpXpStudioDocument->lpMapDocument = NULL;
	lpXpStudioDocument->lpShipDocument = NULL;

	return lpXpStudioDocument;
}
/***************************************************************************/
/*  DestroyXpDocument                                                      */
/* Arguments :                                                             */
/* LPMAPDOCUMENT lpMapDocument                                             */
/* Purpose :   Function to free the specified top level document.          */
/***************************************************************************/
void DestroyXpDocument(LPXPSTUDIODOCUMENT lpXpStudioDocument)
{
	if (lpXpStudioDocument != NULL)
		free(lpXpStudioDocument);
}
/***************************************************************************/
/*  CreateNewMapDoc                                                        */
/* Arguments :                                                             */
/* Purpose :   Create a new map document              .                    */
/***************************************************************************/
LPMAPDOCUMENT CreateNewMapDoc() {
	LPMAPDOCUMENT lpMapDocument;
//	XP_POINT *vert; //For Debuging
	int i;
	
	if ((lpMapDocument = (LPMAPDOCUMENT) malloc(sizeof(MAPDOCUMENT))) == NULL)
	{
		ErrorHandler("Couldn't create new map document!");
		return NULL;
	}
	else
	{
	memset(lpMapDocument, 0, sizeof(MAPDOCUMENT));
	lpMapDocument->MapStruct.comments = NULL;
	lpMapDocument->view_zoom=1;
	lpMapDocument->selectedbool=FALSE;
//	lpMapDocument->MapGeometry.ballareas = NULL;
//	lpMapDocument->MapGeometry.balltargets = NULL;
//	lpMapDocument->MapGeometry.walls = NULL;
//	lpMapDocument->MapGeometry.targets = NULL;
	}
	
	/*Create a prefs template, setting the default values, then point the documents
	prefs array at the newly created prefs array.*/
	{
	/*format for prefs structure {
	"map option name-lowercase",
	"map option name alternate-lowercase",
	"number of digits",
	"type of option",
	"name of map structure variable-for numbers",
	"&name of map structure variable-for yesno",
	"Is option output by default? (set to true when value is changed, most are false)",
	"ID for this preference.",
		*/  
		prefs_t Prefs_Template[NUMPREFS]=
		{
			{ "mapwidth"     		,"mapwidth",3,MAPWIDTH, lpMapDocument->MapStruct.mapWidth,0, TRUE, IDC_MAPWIDTH},
			{ "mapheight"    		,"mapheight",3,MAPHEIGHT,lpMapDocument->MapStruct.mapHeight,0, TRUE, IDC_MAPHEIGHT},
			{ "mapname"             ,"mapname",255,STRING,lpMapDocument->MapStruct.mapName,0, TRUE, IDC_MAPNAME},
			{ "mapauthor"    		,"mapauthor",255,STRING,lpMapDocument->MapStruct.mapAuthor,0, TRUE, IDC_MAPAUTHOR},
			{ "defaultsfilename"		,"defaults",255,STRING,lpMapDocument->MapStruct.defaultsFileName,0, FALSE, IDC_DEFAULTSFILENAME},
			{ "limitedlives"		,"limitedlives",0,YESNO,0,&lpMapDocument->MapStruct.limitedLives, FALSE, IDC_LIMITEDLIVES},
			{ "worldlives"   		,"lives",3,POSINT,lpMapDocument->MapStruct.worldLives,0, FALSE, IDC_WORLDLIVES},
			{ "reset"			,"reset",0,YESNO,0,&lpMapDocument->MapStruct.reset, FALSE, IDC_RESET},
			{ "shipmass"     		,"shipmass",6,POSFLOAT,lpMapDocument->MapStruct.shipMass,0, FALSE, IDC_SHIPMASS},
			{ "gravity"      		,"gravity",6,FLOAT,lpMapDocument->MapStruct.gravity,0, FALSE, IDC_GRAVITY},
			{ "gravityangle" 		,"gravityangle",3,POSINT,lpMapDocument->MapStruct.gravityAngle,0, FALSE, IDC_GRAVITYANGLE},
			{ "gravitypoint" 		,"gravitypoint",7,COORD,lpMapDocument->MapStruct.gravityPoint,0, FALSE, IDC_GRAVITYPOINT},
			{ "gravitypointsource"		,"gravitypointsource",0,YESNO,0,&lpMapDocument->MapStruct.gravityPointSource, FALSE, IDC_GRAVITYPOINTSOURCE},
			{ "gravityclockwise"		,"gravityclockwise",0,YESNO,0,&lpMapDocument->MapStruct.gravityClockwise, FALSE, IDC_GRAVITYCLOCKWISE},
			{ "gravityanticlockwise"	,"gravityanticlockwise",0,YESNO,0,&lpMapDocument->MapStruct.gravityAnticlockwise, FALSE, IDC_GRAVITYANTICLOCKWISE},
			{ "shotsgravity" 		,"shotsgravity",0,YESNO,0,&lpMapDocument->MapStruct.shotsGravity, FALSE, IDC_SHOTSGRAVITY},
			{ "shotmass"     		,"shotmass",6,POSFLOAT,lpMapDocument->MapStruct.shotMass,0, FALSE, IDC_SHOTMASS},
			{ "shotspeed"    		,"shotspeed",6,FLOAT,lpMapDocument->MapStruct.shotSpeed,0, FALSE, IDC_SHOTSPEED},
			{ "shotlife"     		,"shotlife",3,POSINT,lpMapDocument->MapStruct.shotLife,0, FALSE, IDC_SHOTLIFE},
			{ "maxplayershots"		,"shots",3,POSINT,lpMapDocument->MapStruct.maxPlayerShots,0, FALSE, IDC_MAXPLAYERSHOTS},
			{ "firerepeatrate"		,"firerepeat",19,POSINT,lpMapDocument->MapStruct.fireRepeatRate,0, FALSE, IDC_FIREREPEATRATE},
			{ "friction"			,"friction",19,POSFLOAT,lpMapDocument->MapStruct.friction,0, FALSE, IDC_FRICTION},
			{ "edgewrap"			,"edgewrap",0,YESNO,0,&lpMapDocument->MapStruct.edgeWrap, FALSE, IDC_EDGEWRAP},
			
			{ "playerstartsshielded"	,"playerstartshielded",0,YESNO,0,&lpMapDocument->MapStruct.playerStartsShielded, FALSE, IDC_PLAYERSTARTSSHIELDED},
			{ "shotswallbounce"		,"shotswallbounce",0,YESNO,0,&lpMapDocument->MapStruct.shotsWallBounce, FALSE, IDC_SHOTSWALLBOUNCE},
			{ "ballswallbounce"		,"ballswallbounce",0,YESNO,0,&lpMapDocument->MapStruct.ballsWallBounce, FALSE, IDC_BALLSWALLBOUNCE},
			{ "mineswallbounce"		,"mineswallbounce",0,YESNO,0,&lpMapDocument->MapStruct.minesWallBounce, FALSE, IDC_MINESWALLBOUNCE},
			{ "itemswallbounce"		,"itemswallbounce",0,YESNO,0,&lpMapDocument->MapStruct.itemsWallBounce, FALSE, IDC_ITEMSWALLBOUNCE},
			{ "missileswallbounce"		,"missileswallbounce",0,YESNO,0,&lpMapDocument->MapStruct.missilesWallBounce, FALSE, IDC_MISSILESWALLBOUNCE},
			{ "sparkswallbounce"		,"sparkswallbounce",0,YESNO,0,&lpMapDocument->MapStruct.sparksWallBounce, FALSE, IDC_SPARKSWALLBOUNCE},
			{ "debriswallbounce"		,"debriswallbounce",0,YESNO,0,&lpMapDocument->MapStruct.debrisWallBounce, FALSE, IDC_DEBRISWALLBOUNCE},
			{ "maxobjectwallbouncespeed"	,"maxobjectbouncespeed",19,POSFLOAT,lpMapDocument->MapStruct.maxObjectWallBounceSpeed,0, FALSE, IDC_MAXOBJECTWALLBOUNCESPEED},
			{ "maxshieldedwallbouncespeed"	,"maxshieldedbouncespeed",19,POSFLOAT,lpMapDocument->MapStruct.maxShieldedWallBounceSpeed,0, FALSE, IDC_MAXSHIELDEDWALLBOUNCESPEED},
			{ "maxunshieldedwallbouncespeed","maxunshieldedbouncespeed",19,POSFLOAT,lpMapDocument->MapStruct.maxUnshieldedWallBounceSpeed,0, FALSE, IDC_MAXUNSHIELDEDWALLBOUNCESPEED},
			{ "playerwallbouncebrakefactor"	,"playerwallbrake",19,POSFLOAT,lpMapDocument->MapStruct.playerWallBounceBrakeFactor,0, FALSE, IDC_PLAYERWALLBOUNCEBRAKEFACTOR},
			{ "objectwallbouncebrakefactor"	,"objectwallbrake",19,POSFLOAT,lpMapDocument->MapStruct.objectWallBounceBrakeFactor,0, FALSE, IDC_OBJECTWALLBOUNCEBRAKEFACTOR},
			{ "objectwallbouncelifefactor"	,"objectwallbouncelifefactor",19,POSFLOAT,lpMapDocument->MapStruct.objectWallBounceLifeFactor,0, FALSE, IDC_OBJECTWALLBOUNCELIFEFACTOR},
			{ "wallbouncefueldrainmult"	,"wallbouncedrain",19,POSFLOAT,lpMapDocument->MapStruct.wallBounceFuelDrainMult,0, FALSE, IDC_WALLBOUNCEFUELDRAINMULT},
			{ "wallbouncedestroyitemprob" ,"wallbouncedestroyitemprob",19,POSFLOAT,lpMapDocument->MapStruct.wallBounceDestroyItemProb,0, FALSE, IDC_WALLBOUNCEDESTROYITEMPROB},
			{ "loseitemdestroys" ,"loseitemdestroys",0,YESNO,0,&lpMapDocument->MapStruct.loseItemDestroys, FALSE, IDC_LOSEITEMDESTROYS},
			
			{ "limitedvisibility"		,"limitedvisibility",0,YESNO,0,&lpMapDocument->MapStruct.limitedVisibility, FALSE, IDC_LIMITEDVISIBILITY},
			{ "minvisibilitydistance"	,"minvisibility",19,POSFLOAT,lpMapDocument->MapStruct.minVisibilityDistance,0, FALSE, IDC_MINVISIBILITYDISTANCE},
			{ "maxvisibilitydistance"	,"maxvisibility",19,POSFLOAT,lpMapDocument->MapStruct.maxVisibilityDistance,0, FALSE, IDC_MAXVISIBILITYDISTANCE},
			{ "oneplayeronly"		,"oneplayeronly",0,YESNO,0,&lpMapDocument->MapStruct.onePlayerOnly, FALSE, IDC_ONEPLAYERONLY},
			{ "teamplay"			,"teams",0,YESNO,0,&lpMapDocument->MapStruct.teamPlay, FALSE, IDC_TEAMPLAY},
			{ "teamimmunity"		,"teamimmunity",0,YESNO,0,&lpMapDocument->MapStruct.teamImmunity, FALSE, IDC_TEAMIMMUNITY},
			{ "teamcannons"			,"teamcannons",0,YESNO,0,&lpMapDocument->MapStruct.teamCannons, FALSE, IDC_TEAMCANNONS},
			{ "teamfuel"			,"teamfuel",0,YESNO,0,&lpMapDocument->MapStruct.teamFuel, FALSE, IDC_TEAMFUEL},
			{ "targetkillteam"		,"targetkillteam",0,YESNO,0,&lpMapDocument->MapStruct.targetKillTeam, FALSE, IDC_TARGETKILLTEAM},
			{ "targetteamcollision"		,"targetcollision",0,YESNO,0,&lpMapDocument->MapStruct.targetTeamCollision, FALSE, IDC_TARGETTEAMCOLLISION},
			{ "targetsync"			,"targetsync",0,YESNO,0,&lpMapDocument->MapStruct.targetSync, FALSE, IDC_TARGETSYNC},
			{ "treasurekillteam"		,"treasurekillteam",0,YESNO,0,&lpMapDocument->MapStruct.treasureKillTeam, FALSE, IDC_TREASUREKILLTEAM},
			{ "treasurecollisiondestroys"	,"treasurecollisiondestroys",0,YESNO,0,&lpMapDocument->MapStruct.treasureCollisionDestroys, FALSE, IDC_TREASURECOLLISIONDESTROYS},
			{ "treasurecollisionmaykill"	,"treasureunshieldedcollisionkills",0,YESNO,0,&lpMapDocument->MapStruct.treasureCollisionMayKill, FALSE, IDC_TREASURECOLLISIONMAYKILL},
			{ "wreckagecollisionmaykill"	,"wreckageunshieldedcollisionkills",0,YESNO,0,&lpMapDocument->MapStruct.wreckageCollisionMayKill, FALSE, IDC_WRECKAGECOLLISIONMAYKILL},  
			{ "distinguishmissiles"		,"distinguishmissiles",0,YESNO,0,&lpMapDocument->MapStruct.distinguishMissiles, FALSE, IDC_DISTINGUISHMISSILES},
			{ "keepshots"			,"keepshots",0,YESNO,0,&lpMapDocument->MapStruct.keepShots, FALSE, IDC_KEEPSHOTS},
			{ "identifymines"		,"identifymines",0,YESNO,0,&lpMapDocument->MapStruct.identifyMines, FALSE, IDC_IDENTIFYMINES},
			{ "playersonradar"		,"playersradar",0,YESNO,0,&lpMapDocument->MapStruct.playersOnRadar, FALSE, IDC_PLAYERSONRADAR},
			{ "missilesonradar"		,"missilesradar",0,YESNO,0,&lpMapDocument->MapStruct.missilesOnRadar, FALSE, IDC_MISSILESONRADAR},
			{ "minesonradar"		,"minesradar",0,YESNO,0,&lpMapDocument->MapStruct.minesOnRadar, FALSE, IDC_MINESONRADAR},
			{ "nukesonradar"		,"nukesradar",0,YESNO,0,&lpMapDocument->MapStruct.nukesOnRadar, FALSE, IDC_NUKESONRADAR},
			{ "treasuresonradar"		,"treasuresradar",0,YESNO,0,&lpMapDocument->MapStruct.treasuresOnRadar, FALSE, IDC_TREASURESONRADAR},
			{ "shieldeditempickup"		,"shielditem",0,YESNO,0,&lpMapDocument->MapStruct.shieldedItemPickup, FALSE, IDC_SHIELDEDITEMPICKUP},
			{ "shieldedmining"		,"shieldmine",0,YESNO,0,&lpMapDocument->MapStruct.shieldedMining, FALSE, IDC_SHIELDEDMINING},
			{ "cloakedexhaust"		,"cloakedexhaust",0,YESNO,0,&lpMapDocument->MapStruct.cloakedExhaust, FALSE, IDC_CLOAKEDEXHAUST},  
			{ "cloakedshield"		,"cloakedshield",0,YESNO,0,&lpMapDocument->MapStruct.cloakedShield, FALSE, IDC_CLOAKEDSHIELD},
			
			{ "cannonsuseitems"		,"cannonspickupitems",0,YESNO,0,&lpMapDocument->MapStruct.cannonsUseItems, FALSE, IDC_CANNONSUSEITEMS},   
			{ "cannonsmartness"		,"cannonsmartness",1,LISTINT,lpMapDocument->MapStruct.cannonSmartness,0, FALSE, IDC_CANNONSMARTNESSLIST},
			{ "ecmsreprogrammines"		,"ecmsreprogrammines",0,YESNO,0,&lpMapDocument->MapStruct.ecmsReprogramMines, FALSE, IDC_ECMSREPROGRAMMINES},
			{ "gravityvisible"		,"gravityvisible",0,YESNO,0,&lpMapDocument->MapStruct.gravityVisible, FALSE, IDC_GRAVITYVISIBLE},
			{ "wormholevisible"		,"wormholevisible",0,YESNO,0,&lpMapDocument->MapStruct.wormholeVisible, FALSE, IDC_WORMHOLEVISIBLE},   
			{ "wormtime"			,"wormtime",19,POSINT,lpMapDocument->MapStruct.wormTime,0, FALSE, IDC_WORMTIME},
			{ "itemconcentratorvisible"	,"itemconcentratorvisible",0,YESNO,0,&lpMapDocument->MapStruct.itemConcentratorVisible, FALSE, IDC_ITEMCONCENTRATORVISIBLE},
			{ "allowsmartmissiles"		,"allowsmarts",0,YESNO,0,&lpMapDocument->MapStruct.allowSmartMissiles, FALSE, IDC_ALLOWSMARTMISSILES},   
			{ "allowheatseekers"		,"allowheats",0,YESNO,0,&lpMapDocument->MapStruct.allowHeatSeekers, FALSE, IDC_ALLOWHEATSEEKERS},
			{ "allowtorpedoes"		,"allowtorps",0,YESNO,0,&lpMapDocument->MapStruct.allowTorpedoes, FALSE, IDC_ALLOWTORPEDOES},
			{ "allowplayercrashes"		,"allowplayercrashes",0,YESNO,0,&lpMapDocument->MapStruct.allowPlayerCrashes, FALSE, IDC_ALLOWPLAYERCRASHES},
			{ "allowplayerbounces"		,"allowplayerbounces",0,YESNO,0,&lpMapDocument->MapStruct.allowPlayerBounces, FALSE, IDC_ALLOWPLAYERBOUNCES},
			{ "allowplayerkilling"		,"killings",0,YESNO,0,&lpMapDocument->MapStruct.allowPlayerKilling, FALSE, IDC_ALLOWPLAYERKILLING},
			{ "allowshields"		,"shields",0,YESNO,0,&lpMapDocument->MapStruct.allowShields, FALSE, IDC_ALLOWSHIELDS},
			{ "allownukes"			,"nukes",0,YESNO,0,&lpMapDocument->MapStruct.allowNukes, FALSE, IDC_ALLOWNUKES},
			{ "allowclusters"		,"clusters",0,YESNO,0,&lpMapDocument->MapStruct.allowClusters, FALSE, IDC_ALLOWCLUSTERS},
			{ "allowmodifiers"		,"modifiers",0,YESNO,0,&lpMapDocument->MapStruct.allowModifiers, FALSE, IDC_ALLOWMODIFIERS},
			{ "allowlasermodifiers"		,"lasermodifiers",0,YESNO,0,&lpMapDocument->MapStruct.allowLaserModifiers, FALSE, IDC_ALLOWLASERMODIFIERS},
			{ "allowshipshapes"		,"shipshapes",0,YESNO,0,&lpMapDocument->MapStruct.allowShipShapes, FALSE, IDC_ALLOWSHIPSHAPES},
			{ "maxmissilesperpack"		,"maxmissilesperpack",6,INT,lpMapDocument->MapStruct.maxMissilesPerPack,0, FALSE, IDC_MAXMISSILESPERPACK},
			{ "missilelife"			,"missilelife",19,POSINT,lpMapDocument->MapStruct.missileLife,0, FALSE, IDC_MISSILELIFE},
			{ "rogueheatprob"		,"rogueheatprob",19,POSFLOAT,lpMapDocument->MapStruct.rogueHeatProb,0, FALSE, IDC_ROGUEHEATPROB},
			{ "maxminesperpack"		,"maxminesperpack",6,INT,lpMapDocument->MapStruct.maxMinesPerPack,0, FALSE, IDC_MAXMINESPERPACK},
			{ "minelife"			,"minelife",19,POSINT,lpMapDocument->MapStruct.mineLife,0, FALSE, IDC_MINELIFE},
			{ "minefusetime"		,"minefusetime",19,POSFLOAT,lpMapDocument->MapStruct.mineFuseTime,0, FALSE, IDC_MINEFUSETIME},
			{ "baseminerange"		,"baseminerange",19,POSINT,lpMapDocument->MapStruct.baseMineRange,0, FALSE, IDC_BASEMINERANGE},
			{ "roguemineprob"		,"roguemineprob",19,POSFLOAT,lpMapDocument->MapStruct.rogueMineProb,0, FALSE, IDC_ROGUEMINEPROB},
			
			{ "timing"			,"race",0,YESNO,0,&lpMapDocument->MapStruct.timing, FALSE, IDC_TIMING},
			{ "maxroundtime"		,"maxroundtime",6,POSINT,lpMapDocument->MapStruct.maxRoundTime,0, FALSE, IDC_MAXROUNDTIME},
			{ "numberofrounds"		,"roundstoplay",6,POSINT,lpMapDocument->MapStruct.numberOfRounds,0, FALSE, IDC_NUMBEROFROUNDS},
			{ "gameduration"		,"time",6,POSFLOAT,lpMapDocument->MapStruct.gameDuration,0, FALSE, IDC_GAMEDURATION},
			{ "rounddelay"			,"rounddelay",6,POSINT,lpMapDocument->MapStruct.roundDelay,0, FALSE, IDC_ROUNDDELAY},
			{ "laserisstungun"		,"stungun",0,YESNO,0,&lpMapDocument->MapStruct.laserIsStunGun, FALSE, IDC_LASERISSTUNGUN},
			{ "nukeminsmarts"		,"nukeminsmarts",6,POSINT,lpMapDocument->MapStruct.nukeMinSmarts,0, FALSE, IDC_NUKEMINSMARTS},
			{ "nukeminmines"		,"nukeminmines",6,POSINT,lpMapDocument->MapStruct.nukeMinMines,0, FALSE, IDC_NUKEMINMINES},
			{ "nukeclusterdamage"		,"nukeclusterdamage",19,POSFLOAT,lpMapDocument->MapStruct.nukeClusterDamage,0 , FALSE, IDC_NUKECLUSTERDAMAGE},
			{ "itemconcentratorradius"	,"itemconcentratorrange",19,POSFLOAT,lpMapDocument->MapStruct.itemConcentratorRadius,0 , FALSE, IDC_ITEMCONCENTRATORRADIUS},
			{ "maxitemdensity"		,"maxitemdensity",19,POSFLOAT,lpMapDocument->MapStruct.maxItemDensity,0, FALSE, IDC_MAXITEMDENSITY},
			{ "checkpointradius"		,"checkpointradius",19,POSFLOAT,lpMapDocument->MapStruct.checkpointRadius,0, FALSE, IDC_CHECKPOINTRADIUS},
			{ "racelaps"			,"racelaps",6,POSINT,lpMapDocument->MapStruct.raceLaps,0, FALSE, IDC_RACELAPS},
			{ "resetonhuman" 		,"humanreset",0,YESNO,0,&lpMapDocument->MapStruct.resetOnHuman, FALSE, IDC_RESETONHUMAN},
			{ "reporttometaserver"		,"reportmeta",0,YESNO,0,&lpMapDocument->MapStruct.reportToMetaServer, FALSE, IDC_REPORTTOMETASERVER},
			{ "searchdomainforxpilot"	,"searchdomainforxpilot",0,YESNO,0,&lpMapDocument->MapStruct.searchDomainForXPilot, FALSE, IDC_SEARCHDOMAINFORXPILOT},
			{ "contactport"			,"port",19,INT,lpMapDocument->MapStruct.contactPort,0, FALSE, IDC_CONTACTPORT},
			{ "noquit"			,"noquit",0,YESNO,0,&lpMapDocument->MapStruct.noQuit, FALSE, IDC_NOQUIT},
			{ "idlerun"			,"rawmode",0,YESNO,0,&lpMapDocument->MapStruct.idleRun, FALSE, IDC_IDLERUN},
			{ "denyhosts"			,"denyhosts",255,STRING,lpMapDocument->MapStruct.denyHosts,0, FALSE, IDC_DENYHOSTS},
			{ "plockserver"			,"plockserver",0,YESNO,0,&lpMapDocument->MapStruct.pLockServer, FALSE, IDC_PLOCKSERVER},
			{ "lockotherteam"		,"lockotherteam",0,YESNO,0,&lpMapDocument->MapStruct.lockOtherTeam, FALSE, IDC_LOCKOTHERTEAM},
			{ "allowviewing"		,"allowviewing",0,YESNO,0,&lpMapDocument->MapStruct.allowViewing, FALSE, IDC_ALLOWVIEWING},
			{ "framespersecond"		,"fps",6,POSINT,lpMapDocument->MapStruct.framesPerSecond,0, FALSE, IDC_FRAMESPERSECOND},
			{ "ignore20maxfps"	,"ignore20maxfps",0,YESNO,0,&lpMapDocument->MapStruct.ignore20MaxFPS, FALSE, IDC_IGNORE20MAXFPS},
			{ "analyticalcollisiondetection"	,"analyticalcollisiondetection",0,YESNO,0,&lpMapDocument->MapStruct.analyticalCollisionDetection, FALSE, IDC_ANALYTICALCOLLISIONDETECTION},
			{ "password"			,"password",255,STRING,lpMapDocument->MapStruct.password,0, FALSE, IDC_PASSWORD},
			{ "timerresolution"		,"timerresolution",6,POSINT,lpMapDocument->MapStruct.timerResolution,0, FALSE, IDC_TIMERRESOLUTION},
			{ "usewreckage"			,"usewreckage",0,YESNO,0,&lpMapDocument->MapStruct.useWreckage, FALSE, IDC_USEWRECKAGE},
			
			
			{ "initialfuel"			,"initialfuel",19,POSFLOAT,lpMapDocument->MapStruct.initialFuel,0, FALSE, IDC_INITIALFUEL},
			{ "initialtanks"		,"initialtanks",19,POSFLOAT,lpMapDocument->MapStruct.initialTanks,0, FALSE, IDC_INITIALTANKS},
			{ "initialecms"			,"initialecms",19,POSFLOAT, lpMapDocument->MapStruct.initialECMs,0, FALSE, IDC_INITIALECMS},
			{ "initialmines"		,"initialmines",19,POSFLOAT,lpMapDocument->MapStruct.initialMines,0, FALSE, IDC_INITIALMINES},
			{ "initialmissiles"		,"initialmissiles",19,POSFLOAT,lpMapDocument->MapStruct.initialMissiles,0, FALSE, IDC_INITIALMISSILES},
			{ "initialcloaks"		,"initialcloaks",19,POSFLOAT,lpMapDocument->MapStruct.initialCloaks,0, FALSE, IDC_INITIALCLOAKS},
			{ "initialsensors"		,"initialsensors",19,POSFLOAT,lpMapDocument->MapStruct.initialSensors,0, FALSE, IDC_INITIALSENSORS},
			{ "initialwideangles"		,"initialwideangles",19,POSFLOAT,lpMapDocument->MapStruct.initialWideangles,0, FALSE, IDC_INITIALWIDEANGLES},
			{ "initialrearshots"		,"initialrearshots",19,POSFLOAT,lpMapDocument->MapStruct.initialRearshots,0, FALSE, IDC_INITIALREARSHOTS},
			{ "initialafterburners"		,"initialafterburners",19,POSFLOAT,lpMapDocument->MapStruct.initialAfterburners,0, FALSE, IDC_INITIALAFTERBURNERS},
			{ "initialtransporters"		,"initialtransporters",19,POSFLOAT,lpMapDocument->MapStruct.initialTransporters,0, FALSE, IDC_INITIALTRANSPORTERS},
			{ "initialdeflectors"		,"initialdeflectors",19,POSFLOAT,lpMapDocument->MapStruct.initialDeflectors,0, FALSE, IDC_INITIALDEFLECTORS},
			{ "initialphasings"		,"initialphasings",19,POSFLOAT,lpMapDocument->MapStruct.initialPhasings,0, FALSE, IDC_INITIALPHASINGS},
			{ "initialhyperjumps"		,"initialhyperjumps",19,POSFLOAT,lpMapDocument->MapStruct.initialHyperJumps,0, FALSE, IDC_INITIALHYPERJUMPS},
			{ "initialemergencythrusts"	,"initialemergencythrusts",19,POSFLOAT,lpMapDocument->MapStruct.initialEmergencyThrusts,0, FALSE, IDC_INITIALEMERGENCYTHRUSTS},
			{ "initiallasers"		,"initiallasers",19,POSFLOAT,lpMapDocument->MapStruct.initialLasers,0, FALSE, IDC_INITIALLASERS},
			{ "initialtractorbeams"		,"initialtractorbeams",19,POSFLOAT,lpMapDocument->MapStruct.initialTractorBeams,0, FALSE, IDC_INITIALTRACTORBEAMS},
			{ "initialautopilots"		,"initialautopilots",19,POSFLOAT,lpMapDocument->MapStruct.initialAutopilots,0, FALSE, IDC_INITIALAUTOPILOTS},
			{ "initialemergencyshields"	,"initialemergencyshields",19,POSFLOAT,lpMapDocument->MapStruct.initialEmergencyShields,0, FALSE, IDC_INITIALEMERGENCYSHIELDS},
			{ "initialmirrors"		,"initialmirrors",19,POSFLOAT,lpMapDocument->MapStruct.initialMirrors,0, FALSE, IDC_INITIALMIRRORS},
			{ "initialarmor"		,"initialarmors",19,POSFLOAT,lpMapDocument->MapStruct.initialArmor,0, FALSE, IDC_INITIALARMOR},
			
			{ "maxoffensiveitems"		,"maxoffensiveitems",19,POSINT,lpMapDocument->MapStruct.maxOffensiveItems,0, FALSE, IDC_MAXOFFENSIVEITEMS},
			{ "maxdefensiveitems"		,"maxdefensiveitems",19,POSINT,lpMapDocument->MapStruct.maxDefensiveItems,0, FALSE, IDC_MAXDEFENSIVEITEMS},
			{ "maxfuel"			,"maxfuel",19,POSINT,lpMapDocument->MapStruct.maxFuel,0, FALSE, IDC_MAXFUEL},
			{ "maxtanks"			,"maxtanks",19,POSINT,lpMapDocument->MapStruct.maxTanks,0, FALSE, IDC_MAXTANKS},
			{ "maxecms"			,"maxecms",19,POSINT,lpMapDocument->MapStruct.maxECMs,0, FALSE, IDC_MAXECMS},
			{ "maxmines"			,"maxmines",19,POSINT,lpMapDocument->MapStruct.maxMines,0, FALSE, IDC_MAXMINES},
			{ "maxmissiles"			,"maxmissiles",19,POSINT,lpMapDocument->MapStruct.maxMissiles,0, FALSE, IDC_MAXMISSILES},
			{ "maxcloaks"			,"maxcloaks",19,POSINT,lpMapDocument->MapStruct.maxCloaks,0, FALSE, IDC_MAXCLOAKS},
			{ "maxsensors"			,"maxsensors",19,POSINT,lpMapDocument->MapStruct.maxSensors,0, FALSE, IDC_MAXSENSORS},
			{ "maxwideangles"		,"maxwideangles",19,POSINT,lpMapDocument->MapStruct.maxWideangles,0, FALSE, IDC_MAXWIDEANGLES},
			{ "maxrearshots"		,"maxrearshots",19,POSINT,lpMapDocument->MapStruct.maxRearshots,0, FALSE, IDC_MAXREARSHOTS},
			{ "maxafterburners"		,"maxafterburners",19,POSINT,lpMapDocument->MapStruct.maxAfterburners,0, FALSE, IDC_MAXAFTERBURNERS},
			{ "maxtransporters"		,"maxtransporters",19,POSINT,lpMapDocument->MapStruct.maxTransporters,0, FALSE, IDC_MAXTRANSPORTERS},
			{ "maxdeflectors"		,"maxdeflectors",19,POSINT,lpMapDocument->MapStruct.maxDeflectors,0, FALSE, IDC_MAXDEFLECTORS},
			{ "maxphasings"			,"maxphasings",19,POSINT,lpMapDocument->MapStruct.maxPhasings,0, FALSE, IDC_MAXPHASINGS},
			{ "maxhyperjumps"		,"maxhyperjumps",19,POSINT,lpMapDocument->MapStruct.maxHyperJumps,0, FALSE, IDC_MAXHYPERJUMPS},
			{ "maxemergencythrusts"		,"maxemergencythrusts",19,POSINT,lpMapDocument->MapStruct.maxEmergencyThrusts,0, FALSE, IDC_MAXEMERGENCYTHRUSTS},
			{ "maxlasers"			,"maxlasers",19,POSINT,lpMapDocument->MapStruct.maxLasers,0, FALSE, IDC_MAXLASERS},
			{ "maxtractorbeams"		,"maxtractorbeams",19,POSINT,lpMapDocument->MapStruct.maxTractorBeams,0, FALSE, IDC_MAXTRACTORBEAMS},
			{ "maxautopilots"		,"maxautopilots",19,POSINT,lpMapDocument->MapStruct.maxAutopilots,0, FALSE, IDC_MAXAUTOPILOTS},
			{ "maxemergencyshields"		,"maxemergencyshields",19,POSINT,lpMapDocument->MapStruct.maxEmergencyShields,0, FALSE, IDC_MAXEMERGENCYSHIELDS},
			{ "maxmirrors"			,"maxmirrors",19,POSINT,lpMapDocument->MapStruct.maxMirrors,0, FALSE, IDC_MAXMIRRORS},
			{ "maxarmor"			,"maxarmors",19,POSINT,lpMapDocument->MapStruct.maxArmor,0, FALSE, IDC_MAXARMOR},
			
			{ "itemenergypackprob"		,"itemenergypackprob",19,POSFLOAT,lpMapDocument->MapStruct.itemEnergyPackProb,0, FALSE, IDC_ITEMENERGYPACKPROB},
			{ "itemtankprob"		,"itemtankprob",19,POSFLOAT,lpMapDocument->MapStruct.itemTankProb,0, FALSE, IDC_ITEMTANKPROB},
			{ "itemecmprob"			,"itemecmprob",19,POSFLOAT,lpMapDocument->MapStruct.itemECMProb,0, FALSE, IDC_ITEMECMPROB},
			{ "itemmineprob"		,"itemmineprob",19,POSFLOAT,lpMapDocument->MapStruct.itemMineProb,0, FALSE, IDC_ITEMMINEPROB},
			{ "itemmissileprob"		,"itemmissileprob",19,POSFLOAT,lpMapDocument->MapStruct.itemMissileProb,0, FALSE, IDC_ITEMMISSILEPROB},
			{ "itemcloakprob"		,"itemcloakprob",19,POSFLOAT,lpMapDocument->MapStruct.itemCloakProb,0, FALSE, IDC_ITEMCLOAKPROB},
			{ "itemsensorprob"		,"itemsensorprob",19,POSFLOAT,lpMapDocument->MapStruct.itemSensorProb,0, FALSE, IDC_ITEMSENSORPROB},
			{ "itemwideangleprob"		,"itemwideangleprob",19,POSFLOAT,lpMapDocument->MapStruct.itemWideangleProb,0, FALSE, IDC_ITEMWIDEANGLEPROB},
			{ "itemrearshotprob"		,"itemrearshotprob",19,POSFLOAT,lpMapDocument->MapStruct.itemRearshotProb,0, FALSE, IDC_ITEMREARSHOTPROB},
			{ "itemafterburnerprob"		,"itemafterburnerprob",19,POSFLOAT,lpMapDocument->MapStruct.itemAfterburnerProb,0, FALSE, IDC_ITEMAFTERBURNERPROB},
			{ "itemtransporterprob"		,"itemtransporterprob",19,POSFLOAT,lpMapDocument->MapStruct.itemTransporterProb,0, FALSE, IDC_ITEMTRANSPORTERPROB},
			{ "itemlaserprob"		,"itemlaserprob",19,POSFLOAT,lpMapDocument->MapStruct.itemLaserProb,0, FALSE, IDC_ITEMLASERPROB},
			{ "itememergencythrustprob"	,"itememergencythrustprob",19,POSFLOAT,lpMapDocument->MapStruct.itemEmergencyThrustProb,0, FALSE, IDC_ITEMEMERGENCYTHRUSTPROB},
			{ "itemtractorbeamprob"		,"itemtractorbeamprob",19,POSFLOAT,lpMapDocument->MapStruct.itemTractorBeamProb,0, FALSE, IDC_ITEMTRACTORBEAMPROB},
			{ "itemautopilotprob"		,"itemautopilotprob",19,POSFLOAT,lpMapDocument->MapStruct.itemAutopilotProb,0, FALSE, IDC_ITEMAUTOPILOTPROB},
			{ "itememergencyshieldprob"	,"itememergencyshieldprob",19,POSFLOAT,lpMapDocument->MapStruct.itemEmergencyShieldProb,0, FALSE, IDC_ITEMEMERGENCYSHIELDPROB},
			{ "itemdeflectorprob"		,"itemdeflectorprob",19,POSFLOAT,lpMapDocument->MapStruct.itemDeflectorProb,0, FALSE, IDC_ITEMDEFLECTORPROB},
			{ "itemhyperjumpprob"		,"itemhyperjumpprob",19,POSFLOAT,lpMapDocument->MapStruct.itemHyperJumpProb,0, FALSE, IDC_ITEMHYPERJUMPPROB},
			{ "itemphasingprob"		,"itemphasingprob",19,POSFLOAT,lpMapDocument->MapStruct.itemPhasingProb,0, FALSE, IDC_ITEMPHASINGPROB},
			{ "itemmirrorprob"		,"itemmirrorprob",19,POSFLOAT,lpMapDocument->MapStruct.itemMirrorProb,0, FALSE, IDC_ITEMMIRRORPROB},
			{ "itemarmorprob"		,"itemarmorprob",19,POSFLOAT,lpMapDocument->MapStruct.itemArmorProb,0, FALSE, IDC_ITEMARMORPROB},
			{ "movingitemprob"		,"movingitemprob",19,POSFLOAT,lpMapDocument->MapStruct.movingItemProb,0, FALSE, IDC_MOVINGITEMPROB},
			{ "dropitemonkillprob"		,"dropitemonkillprob",19,POSFLOAT,lpMapDocument->MapStruct.dropItemOnKillProb,0, FALSE, IDC_DROPITEMONKILLPROB},
			{ "detonateitemonkillprob"	,"detonateitemonkillprob",19,POSFLOAT,lpMapDocument->MapStruct.detonateItemOnKillProb,0, FALSE, IDC_DETONATEITEMONKILLPROB},
			{ "destroyitemincollisionprob"	,"destroyitemincollisionprob",19,POSFLOAT,lpMapDocument->MapStruct.destroyItemInCollisionProb,0, FALSE, IDC_DESTROYITEMINCOLLISIONPROB},
			{ "itemconcentratorprob"	,"itemconcentratorprob",19,POSFLOAT,lpMapDocument->MapStruct.itemConcentratorProb,0, FALSE, IDC_ITEMCONCENTRATORPROB},
			{ "itemprobmult"		,"itemprobfact",19,POSFLOAT,lpMapDocument->MapStruct.itemProbMult,0, FALSE, IDC_ITEMPROBMULT},
			{ "cannonitemprobmult"		,"cannonitemprobmult",19,POSFLOAT,lpMapDocument->MapStruct.cannonItemProbMult,0, FALSE, IDC_CANNONITEMPROBMULT},
			
			{ "shotkillscoremult"		,"shotkillscoremult",19,POSFLOAT,lpMapDocument->MapStruct.shotKillScoreMult,0, FALSE, IDC_SHOTKILLSCOREMULT},
			{ "torpedokillscoremult"	,"torpedokillscoremult",19,POSFLOAT,lpMapDocument->MapStruct.torpedoKillScoreMult,0, FALSE, IDC_TORPEDOKILLSCOREMULT},
			{ "smartkillscoremult"		,"smartkillscoremult",19,POSFLOAT,lpMapDocument->MapStruct.smartKillScoreMult,0, FALSE, IDC_SMARTKILLSCOREMULT},
			{ "heatkillscoremult"		,"heatkillscoremult",19,POSFLOAT,lpMapDocument->MapStruct.heatKillScoreMult,0, FALSE, IDC_HEATKILLSCOREMULT},
			{ "clusterkillscoremult"	,"clusterkillscoremult",19,POSFLOAT,lpMapDocument->MapStruct.clusterKillScoreMult,0, FALSE, IDC_CLUSTERKILLSCOREMULT},
			{ "laserkillscoremult"		,"laserkillscoremult",19,POSFLOAT,lpMapDocument->MapStruct.laserKillScoreMult,0, FALSE, IDC_LASERKILLSCOREMULT},
			{ "tankkillscoremult"		,"tankkillscoremult",19,POSFLOAT,lpMapDocument->MapStruct.tankKillScoreMult,0, FALSE, IDC_TANKKILLSCOREMULT},
			{ "runoverkillscoremult"	,"runoverkillscoremult",19,POSFLOAT,lpMapDocument->MapStruct.runoverKillScoreMult,0, FALSE, IDC_RUNOVERKILLSCOREMULT},
			{ "ballkillscoremult"		,"ballkillscoremult",19,POSFLOAT,lpMapDocument->MapStruct.ballKillScoreMult,0, FALSE, IDC_BALLKILLSCOREMULT},
			{ "explosionkillscoremult"	,"explosionkillscoremult",19,POSFLOAT,lpMapDocument->MapStruct.explosionKillScoreMult,0, FALSE, IDC_EXPLOSIONKILLSCOREMULT},
			{ "shovekillscoremult"		,"shovekillscoremult",19,POSFLOAT,lpMapDocument->MapStruct.shoveKillScoreMult,0, FALSE, IDC_SHOVEKILLSCOREMULT},
			{ "crashscoremult"		,"crashscoremult",19,POSFLOAT,lpMapDocument->MapStruct.crashScoreMult,0, FALSE, IDC_CRASHKILLSCOREMULT},
			{ "minescoremult"		,"minescoremult",19,POSFLOAT,lpMapDocument->MapStruct.mineScoreMult,0, FALSE, IDC_MINEKILLSCOREMULT},
			};			
			
			for (i = 0; i < NUMPREFS; i++)
				lpMapDocument->PrefsArray[i] = Prefs_Template[i];
			
			NewMapInit(lpMapDocument);
	}
	return lpMapDocument;
}
/***************************************************************************/
/*  DestroyMapDoc                                                          */
/* Arguments :                                                             */
/* LPMAPDOCUMENT lpMapDocument                                             */
/* Purpose :   Function to free the specified map document.                */
/***************************************************************************/
void DestroyMapDoc(LPMAPDOCUMENT lpMapDocument)
{
		if (lpMapDocument != NULL)
			free(lpMapDocument);
}
/***************************************************************************/
/*  CreateNewShipDoc                                                       */
/* Arguments :                                                             */
/* Purpose :   Create a new ship document              .                   */
/***************************************************************************/
LPSHIPDOCUMENT CreateNewShipDoc() {
	LPSHIPDOCUMENT lpShipDocument;

	if ((lpShipDocument = (LPSHIPDOCUMENT) malloc(sizeof(SHIPDOCUMENT))) == NULL)
	{
		ErrorHandler("Couldn't create new ship document!");
		return NULL;
	}
/*	else
	{
	memset(lpShipDocument, 0, sizeof(SHIPDOCUMENT));
	lpShipDocument->undolist = NULL;
	lpShipDocument->shiplist = Alloc_Shiplist();
	lpShipDocument->shiplist->s = Convert_shape_str(NULL);
	lpShipDocument->shiplist->n = NULL;
	lpShipDocument->curship = 0;
	lpShipDocument->shp = lpShipDocument->shiplist->s;
	}*/

	return lpShipDocument;
}
/***************************************************************************/
/*  DestroyShipDoc                                                         */
/* Arguments :                                                             */
/* LPSHIPDOCUMENT lpShipDocument                                           */
/* Purpose :   Function to free the specified map document.                */
/***************************************************************************/
void DestroyShipDoc(LPSHIPDOCUMENT lpShipDocument)
{
		if (lpShipDocument != NULL)
			free(lpShipDocument);
}