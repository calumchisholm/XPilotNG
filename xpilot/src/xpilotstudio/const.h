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
/*True & False*/
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define VOIDBACK	999

/*Document Types!!!*/
#define MAPFILE		0
#define SHIPFILE	1
#define ROBOTFILE	2

/*MapEditor*/
#define MAX_MAP_SIZE			31500
#define MAX_LINE_LEN			100
#define MINIMAP_WIDTH			1000
#define DEFAULT_WIDTH			3500
#define DEFAULT_HEIGHT			3500
#define DEFAULT_MAP_ZOOM		10
#define TOOLSWIDTH				70


#define UNTITLED "(untitled)"


/*Option Types*/
#define MAPWIDTH         0
#define MAPHEIGHT        1
#define MAPDATA          2
#define STRING           3
#define YESNO            4
#define FLOAT            5
#define POSFLOAT         6
#define INT              7
#define POSINT           8
#define COORD            9
#define LISTINT			 10

/*All the block types*/
#define IDM_MAP_WALL				9000
#define IDM_MAP_CANNON				9001
#define IDM_MAP_BASE				9002
#define IDM_MAP_FUEL				9003
#define IDM_MAP_TARGET				9004
#define IDM_MAP_BALLAREA			9005
#define IDM_MAP_BALLTARGET			9006
#define IDM_MAP_BALL				9007
#define IDM_MAP_ITEM_CONC			9008
#define IDM_MAP_GRAVITY				9009
#define IDM_MAP_CIRCULAR_GRAVITY	9010
#define IDM_MAP_WORMHOLE			9011
#define IDM_MAP_CURRENT				9012
#define IDM_MAP_DECOR				9013
#define IDM_MAP_CHECKPOINT			9014

//Variant types for wormholes.
#define IDM_MAP_WORM_NORMAL		700
#define IDM_MAP_WORM_OUT		701
#define IDM_MAP_WORM_IN			702

//Variant types for pos/neg gravity, circular currents.
#define IDM_MAP_POSITIVE		710
#define IDM_MAP_NEGATIVE		711

//Variant types for walls.
//Walls cannot be set to normal, this constant is provided
//to enable checking that none of the others such as hidden are set.
#define IDM_MAP_NORMAL			720
#define IDM_MAP_HIDDEN			721

/*Map Modification Options*/
#define IDM_PICKITEM			850
#define IDM_ADDVERTEX			851
#define IDM_DELVERTEX			852
#define IDM_MOVEITEM			853
#define IDM_DELETEITEM			854
#define IDM_UPDATE_ITEM_PARAMS	855
#define IDM_MOVEVERTEX			856
#define IDM_REORDERCHECKPOINT	857

/*All the ship symbol types*/
#define IDM_VERTEX				900
#define IDM_ENGINE				901
#define IDM_MAINGUN				902
#define IDM_LEFTFRONTGUN		903
#define IDM_RIGHTFRONTGUN		904
#define IDM_LEFTREARGUN			905
#define IDM_RIGHTREARGUN		906
#define IDM_MISSILERACK			907
#define IDM_LEFTLIGHT			908
#define IDM_RIGHTLIGHT			909

/*The total number of map options supported...must be updated to add new prefs*/
#define NUMPREFS 207

#define IDC_MAPWIDTH                    1001
#define IDC_MAPHEIGHT                   1002
#define IDC_MAPNAME                     1003
#define IDC_MAPAUTHOR                   1004
#define IDC_DEFAULTSFILENAME            1005
#define IDC_LIMITEDLIVES	            1006
#define IDC_WORLDLIVES                  1007
#define IDC_RESET	                    1008
#define IDC_SHIPMASS                    1009
#define IDC_GRAVITY                     1010
#define IDC_GRAVITYANGLE                1011
#define IDC_GRAVITYPOINT                1012
#define IDC_GRAVITYPOINTSOURCE			1013
#define IDC_GRAVITYCLOCKWISE			1014
#define IDC_GRAVITYANTICLOCKWISE		1015
#define IDC_SHOTSGRAVITY		        1016
#define IDC_SHOTMASS                    1017
#define IDC_SHOTSPEED                   1018
#define IDC_SHOTLIFE                    1019
#define IDC_MAXPLAYERSHOTS              1020
#define IDC_FIREREPEATRATE              1021
#define IDC_FRICTION                    1022
#define IDC_EDGEWRAP	                1023
#define IDC_PLAYERLIMIT					1024

/*Mapdata2 PropSheet*/
#define IDC_PLAYERSTARTSSHIELDED	        2001
#define IDC_SHOTSWALLBOUNCE					2002
#define IDC_BALLSWALLBOUNCE					2003
#define IDC_MINESWALLBOUNCE					2004
#define IDC_ITEMSWALLBOUNCE					2005
#define IDC_MISSILESWALLBOUNCE				2006
#define IDC_SPARKSWALLBOUNCE				2007
#define IDC_DEBRISWALLBOUNCE				2008
#define IDC_MAXOBJECTWALLBOUNCESPEED		2009
#define IDC_MAXSHIELDEDWALLBOUNCESPEED		2010
#define IDC_MAXUNSHIELDEDWALLBOUNCESPEED	2011
#define IDC_PLAYERWALLBOUNCEBRAKEFACTOR		2012
#define IDC_OBJECTWALLBOUNCEBRAKEFACTOR		2013
#define IDC_OBJECTWALLBOUNCELIFEFACTOR		2014
#define IDC_WALLBOUNCEFUELDRAINMULT			2015
#define IDC_WALLBOUNCEDESTROYITEMPROB		2016
#define IDC_LOSEITEMDESTROYS				2017

/*Mapdata3 PropSheet*/
#define IDC_LIMITEDVISIBILITY           3001
#define IDC_MINVISIBILITYDISTANCE       3002
#define IDC_MAXVISIBILITYDISTANCE       3003
#define IDC_TEAMPLAY                    3004
#define IDC_TEAMIMMUNITY                3005
#define IDC_TEAMCANNONS   				3006
#define IDC_TEAMFUEL   					3007
#define IDC_TARGETKILLTEAM              3008
#define IDC_TARGETTEAMCOLLISION         3009
#define IDC_TARGETSYNC                  3010
#define IDC_TREASUREKILLTEAM            3011
#define IDC_TREASURECOLLISIONDESTROYS	3012
#define IDC_TREASURECOLLISIONMAYKILL    3013
#define IDC_WRECKAGECOLLISIONMAYKILL    3014
#define IDC_DISTINGUISHMISSILES         3015
#define IDC_KEEPSHOTS   				3016
#define IDC_IDENTIFYMINES   		    3017
#define IDC_PLAYERSONRADAR              3018
#define IDC_MISSILESONRADAR             3019
#define IDC_MINESONRADAR                3020
#define IDC_NUKESONRADAR                3021
#define IDC_TREASURESONRADAR            3022
#define IDC_SHIELDEDITEMPICKUP          3023
#define IDC_SHIELDEDMINING              3024
#define IDC_CLOAKEDEXHAUST              3025
#define IDC_CLOAKEDSHIELD               3026

/*Mapdata4 PropSheet*/
#define IDC_CANNONSUSEITEMS     		4001
#define IDC_CANNONSMARTNESSLIST	        4002
#define IDC_ECMSREPROGRAMMINES          4003
#define IDC_GRAVITYVISIBLE              4004
#define IDC_WORMHOLEVISIBLE             4005
#define IDC_WORMTIME					4006
#define IDC_ITEMCONCENTRATORVISIBLE     4007
#define IDC_ALLOWSMARTMISSILES          4008
#define IDC_ALLOWHEATSEEKERS            4009
#define IDC_ALLOWTORPEDOES              4010
#define IDC_ALLOWPLAYERCRASHES          4011
#define IDC_ALLOWPLAYERBOUNCES          4012
#define IDC_ALLOWPLAYERKILLING          4013
#define IDC_ALLOWSHIELDS                4014
#define IDC_ALLOWNUKES                  4015
#define IDC_ALLOWCLUSTERS               4016
#define IDC_ALLOWMODIFIERS              4017
#define IDC_ALLOWLASERMODIFIERS         4018
#define IDC_ALLOWSHIPSHAPES             4019
#define IDC_MAXMISSILESPERPACK          4020
#define IDC_MISSILELIFE                 4021
#define IDC_ROGUEHEATPROB               4022
#define IDC_MAXMINESPERPACK             4023
#define IDC_MINELIFE                    4024
#define IDC_MINEFUSETIME                4025
#define IDC_BASEMINERANGE               4026
#define IDC_ROGUEMINEPROB               4027

/*Mapdata5 PropSheet*/
#define IDC_TIMING							5001
#define IDC_MAXROUNDTIME					5002
#define IDC_GAMEDURATION					5003
#define IDC_ROUNDDELAY						5004
#define IDC_LASERISSTUNGUN					5005
#define IDC_NUKEMINSMARTS					5006
#define IDC_NUKEMINMINES					5007
#define IDC_NUKECLUSTERDAMAGE				5008
#define IDC_ITEMCONCENTRATORRADIUS			5009
#define IDC_MAXITEMDENSITY					5010
#define IDC_CHECKPOINTRADIUS				5011
#define IDC_RACELAPS						5012
#define IDC_RESETONHUMAN					5013
#define IDC_REPORTTOMETASERVER				5014
#define IDC_CONTACTPORT						5015
#define IDC_NOQUIT							5016
#define IDC_IDLERUN							5017
#define IDC_DENYHOSTS						5018
#define IDC_PLOCKSERVER						5019
#define IDC_LOCKOTHERTEAM					5020
#define IDC_ALLOWVIEWING					5021
#define IDC_FRAMESPERSECOND					5022
#define IDC_IGNORE20MAXFPS					5023
#define IDC_NUMBEROFROUNDS					5024
#define IDC_TIMERRESOLUTION					5025
#define IDC_PASSWORD						5026
#define IDC_SEARCHDOMAINFORXPILOT			5027
#define IDC_ELIMINATION						5028
#define IDC_RECORDMODE						5029

/*Initial Items PropSheet*/
#define IDC_INITIALFUEL                 6001
#define IDC_INITIALTANKS                6002
#define IDC_INITIALECMS                 6003
#define IDC_INITIALMINES                6004
#define IDC_INITIALMISSILES             6005
#define IDC_INITIALCLOAKS               6006
#define IDC_INITIALSENSORS              6007
#define IDC_INITIALWIDEANGLES           6008
#define IDC_INITIALREARSHOTS            6009
#define IDC_INITIALAFTERBURNERS         6010
#define IDC_INITIALTRANSPORTERS         6011
#define IDC_INITIALDEFLECTORS           6012
#define IDC_INITIALPHASINGS             6013
#define IDC_INITIALHYPERJUMPS           6014
#define IDC_INITIALEMERGENCYTHRUSTS     6015
#define IDC_INITIALLASERS               6016
#define IDC_INITIALTRACTORBEAMS         6017
#define IDC_INITIALAUTOPILOTS           6018
#define IDC_INITIALEMERGENCYSHIELDS     6019
#define IDC_INITIALMIRRORS				6020
#define IDC_INITIALARMOR				6021



/*Max Items PropSheet*/
#define IDC_MAXFUEL                     7001
#define IDC_MAXTANKS                    7002
#define IDC_MAXECMS                     7003
#define IDC_MAXMINES                    7004
#define IDC_MAXMISSILES                 7005
#define IDC_MAXCLOAKS                   7006
#define IDC_MAXSENSORS                  7007
#define IDC_MAXWIDEANGLES               7008
#define IDC_MAXREARSHOTS                7009
#define IDC_MAXAFTERBURNERS             7010
#define IDC_MAXTRANSPORTERS             7011
#define IDC_MAXDEFLECTORS               7012
#define IDC_MAXPHASINGS                 7013
#define IDC_MAXHYPERJUMPS               7014
#define IDC_MAXEMERGENCYTHRUSTS         7015
#define IDC_MAXLASERS                   7016
#define IDC_MAXTRACTORBEAMS             7017
#define IDC_MAXAUTOPILOTS               7018
#define IDC_MAXEMERGENCYSHIELDS         7019
#define IDC_MAXMIRRORS					7020
#define IDC_MAXARMOR					7021

/*Item Probabilities PropSheet*/
#define IDC_ITEMENERGYPACKPROB          8001
#define IDC_ITEMTANKPROB                8002
#define IDC_ITEMECMPROB                 8003
#define IDC_ITEMMINEPROB                8004
#define IDC_ITEMMISSILEPROB             8005
#define IDC_ITEMCLOAKPROB               8006
#define IDC_ITEMSENSORPROB              8007
#define IDC_ITEMWIDEANGLEPROB           8008
#define IDC_ITEMREARSHOTPROB            8009
#define IDC_ITEMAFTERBURNERPROB         8010
#define IDC_ITEMTRANSPORTERPROB         8011
#define IDC_ITEMLASERPROB               8012
#define IDC_ITEMEMERGENCYTHRUSTPROB     8013
#define IDC_ITEMTRACTORBEAMPROB         8014
#define IDC_ITEMAUTOPILOTPROB           8015
#define IDC_ITEMEMERGENCYSHIELDPROB     8016
#define IDC_ITEMDEFLECTORPROB           8017
#define IDC_ITEMHYPERJUMPPROB           8018
#define IDC_ITEMPHASINGPROB             8019
#define IDC_ITEMMIRRORPROB				8020
#define IDC_MOVINGITEMPROB              8021
#define IDC_DROPITEMONKILLPROB          8022
#define IDC_DETONATEITEMONKILLPROB      8023
#define IDC_DESTROYITEMINCOLLISIONPROB  8024
#define IDC_ITEMCONCENTRATORPROB        8025
#define IDC_ITEMPROBMULT                8026
#define IDC_CANNONITEMPROBMULT          8027
#define IDC_ITEMARMORPROB				8028

/*ScoreMultipliers PropSheet*/
#define IDC_SHOTKILLSCOREMULT           9001
#define IDC_TORPEDOKILLSCOREMULT        9002
#define IDC_SMARTKILLSCOREMULT          9003
#define IDC_HEATKILLSCOREMULT           9004
#define IDC_CLUSTERKILLSCOREMULT        9005
#define IDC_LASERKILLSCOREMULT          9006
#define IDC_TANKKILLSCOREMULT           9007
#define IDC_RUNOVERKILLSCOREMULT        9008
#define IDC_BALLKILLSCOREMULT           9009
#define IDC_EXPLOSIONKILLSCOREMULT      9010
#define IDC_SHOVEKILLSCOREMULT          9011
#define IDC_CRASHKILLSCOREMULT          9012
#define IDC_MINEKILLSCOREMULT           9013
#define IDC_CONSTANTSCORING				9014

/********************ShipEditor************************/
#define MAXLINELEN 150	/* max line length of files */
#define MAXSHIPLEN 800

#define MAX_SHIP_PTS	24	/* Max/min number of points & specials */
#define MIN_SHIP_PTS	3
#define MAX_GUN_PTS		3
#define MAX_LIGHT_PTS	3
#define MAX_RACK_PTS	4
#define MAXNAME			40	/* Max name / author field length */
#define MAXAUTHOR		40

#define MG 1	/* Symbolic names for shipshape fields */
#define EN 2
#define LG 4
#define RG 8
#define LL 16
#define RL 32
#define MR 64
#define NM 128
#define AU 256

#define VIEWLEFT	200
#define VIEWTOP		10
#define SMSHPSIZE	60


#define RES		128
#define sqr(x)	( (x)*(x) )

#define LENGTH(x, y)	( hypot( (double) (x), (double) (y) ) )

/*
 * Two macros for edge wrap of x and y coordinates measured in pixels.
 * Note that the correction needed shouldn't ever be bigger than one mapsize.
 */
#define WRAP_XPIXEL(x_)	\
	((x_) < 0 \
		? (x_) + lpMapDocument->width \
		: ((x_) >= lpMapDocument->width \
		    ? (x_) - lpMapDocument->width \
		    : (x_)))
	    

#define WRAP_YPIXEL(y_)	\
	((y_) < 0 \
		? (y_) + lpMapDocument->height \
		: ((y_) >= lpMapDocument->height \
		    ? (y_) - lpMapDocument->height \
		    : (y_)))

#define WRAP_DX(dx)	\
	((dx) < - (lpMapDocument->width >> 1) \
		? (dx) + lpMapDocument->width \
		: ((dx) > (lpMapDocument->width >> 1) \
		    ? (dx) - lpMapDocument->width \
		    : (dx)))
#define WRAP_DY(dy)	\
	    ((dy) < - (lpMapDocument->height >> 1) \
		? (dy) + lpMapDocument->height \
		: ((dy) > (lpMapDocument->height >> 1) \
		    ? (dy) - lpMapDocument->height \
		    : (dy)))
