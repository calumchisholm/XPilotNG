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


#define IDR_ACCELERATOR			1001
#define IDB_LOGO				1002
#define IDI_MAINICON			1003

#define UPDATE_COMMANDS			54321

#define IDM_TILE				501
#define IDM_CASCADE				502
#define IDM_ARRANGE				503
#define IDM_CLOSEALL			504
#define IDM_HELP				2001
#define IDM_ABOUT				2001

#define IDR_MAINMENU			3001
/*Control Defines for Main Window Menu*/
#define IDM_NEWMAPEDITOR		30001
#define IDM_NEWSHIPEDITOR		30002
#define IDM_HELP_CONTENTS		30003

/*******************Mapeditor*************/
#define IDB_MAPTOOLSTOOLBAR		201
#define IDB_BLOCKSTOOLBAR		202
#define IDB_WORMHOLESUBBAR		204
#define IDB_POLARITYSUBBAR		206

#define IDR_MAPEDITORMENU		4001
#define IDM_NEW					40001
#define IDM_OPEN				40002
#define IDM_CLOSE				40003
#define IDM_SAVE				40004
#define IDM_SAVEAS				40005
#define IDM_SAVESELECTEDAS		40006
#define IDM_LOADPREFS			40007
#define IDM_SAVEPREFS			40008
#define IDM_WILDMAP				40009
#define IDM_EXIT				40010


#define IDM_EDIT_UNDO			40011
#define IDM_EDIT_CUT			40012
#define IDM_EDIT_COPY			40013
#define IDM_EDIT_PASTE			40014
#define IDM_CROP				40015
#define IDM_MAPDIR				40018
#define IDM_SAVEALLPREFS		40019

#define IDM_PROPERTIES			40020
#define IDM_TYPESEDITOR			40021

#define IDM_REFRESH				40023

/*Buttons...Also correspond to some menu picks*/
#define IDM_PEN				40100
//#define IDM_MODIFYITEM		40101
#define IDM_ERASE			40102
//#define IDM_SELECT			40103
#define IDM_ZOOMIN			40104
#define IDM_ZOOMOUT			40105
#define IDM_ROTATE			40106
#define IDM_MIRRORV			40107
#define IDM_MIRRORH			40108
#define IDM_CLEAR			40109
#define IDM_CYCLEH			40110
#define IDM_CYCLEV			40111
#define IDM_CYCLEHNEG		40112
#define IDM_CYCLEVNEG		40113
#define IDM_CHECKMAP		40114
#define IDM_MAPSOURCE		40115
#define IDM_LAUNCHMAP		40116

#define IDD_MAPDATA			201			
#define IDD_MAPDATA2		202
#define IDD_MAPDATA3		203
#define IDD_MAPDATA4		204
#define IDD_MAPDATA5		205
#define IDD_INITITEMS		206
#define IDD_MAXITEMS		207
#define IDD_PROBS			208
#define IDD_SCRMULT			209
#define IDD_COMMENTS		210
#define IDD_ABOUT			211
#define IDD_NEWDOCUMENT		214
#define IDD_TYPESEDITOR		215

/*Controls for the types editor*/
#define IDC_TYPESELECTOR		50000
#define IDC_TYPESLISTCONTAINER	50001
#define IDC_ADDTYPE				50002
#define IDC_REMOVETYPES			50003

#define IDS_POLYGONTYPE_C1              60001
#define IDS_POLYGONTYPE_C2              60002
#define IDS_POLYGONTYPE_C3              60003
#define IDS_POLYGONTYPE_C4              60004
#define IDS_POLYGONTYPE_C5              60005

/*Controls for selection in the New Document Dialog*/
#define IDC_RADIONEWMAPEDITOR		301
#define IDC_RADIONEWSHIPEDITOR		302

#define IDC_COMMENTS                    10001

#define IDC_ABOUTTEXT					11001
#define IDC_ABOUTVERSION				11002
#define IDC_ABOUTXPILOTVERSION			11003

#define IDC_MAPERRORSTEXT				12001
/**************End Mapeditor*********************/

/*******************Shipeditor*************/
#define IDR_SHIPEDITORMENU		5001
#define IDB_SHIPTOOLSTOOLBAR			5002
#define IDB_SHIPSYMSTOOLBAR		5003

#define IDD_SHIPEDITOR	13001
#define IDC_SHIPNAME	13002
#define IDC_SHIPAUTHOR	13003
/**************End Shipeditor*********************/
