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
#include "win_xpstudio.h"
//To Add Buttons to Toolbars:
//1. Add a constant for your button to const.h or win_resdefines.h
//   as needed, depending on if that constant is referenced by portable
//   code or only windows code. Use const.h if possible.
//2. Add your tooltip text to the szTbStrings array similarly to the
//   format shown, remember where you inserted it.
//3. Add the constant from step 1 to the CommandToString array, in the
//   same order as you added it to the first array above. The order
//   is important and must be maintained.
//4. Add your button to the desired TBBUTTON array, depending upon which
//   toolbar you wish to add to, the first number in the array init string
//	 is the number of the bitmap you should add to the matching
//   bitmap file. You could also create a new toolbar, but this requires
//   more skill, as you also have to add new bitmap files to the project.
//5. Go to: InitToolBars and see the 'toolbars' array
//   In the line corresponding to the toolbar youve changed, update the
//   item numBitmaps, to correspond to the correct total number of
//   buttons for that toolbar.
//   If you added a toolbar, you must add a new item to the array, and
//   specify all the correct initialization information. Dont forget
//   to update the index for the number of toolbars.

#define ToolBar_ButtonStructSize(hwnd) \
(void)SendMessage((hwnd), TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0L)

#define ToolBar_AddBitmap(hwnd, nButtons, lptbab) \
(int)SendMessage((hwnd), TB_ADDBITMAP, (WPARAM)nButtons, (LPARAM)(LPTBADDBITMAP) lptbab)

#define ToolBar_AddButtons(hwnd, uNumButtons, lpButtons) \
(BOOL)SendMessage((hwnd), TB_ADDBUTTONS, (WPARAM)(UINT)uNumButtons, (LPARAM)(LPTBBUTTON)lpButtons)


DWORD dwToolBarStyles = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
TBSTYLE_WRAPABLE | TBSTYLE_TOOLTIPS | CCS_NODIVIDER;

int cyToolBar ;

char szTbStrings[] ="New\0Open\0Save\0Cut\0Copy\0Paste\0Undo\0Pen\0Modify Item\0Erase\0"
"Shape\0Clear\0"
"Crop\0Select\0Zoom In\0Zoom Out\0Rotate\0"
"Mirror Vertically\0Mirror Horizontally\0Adjust Horizontally Left\0Adjust Horizontally Right\0"
"Adjust Vertically Up\0Adjust Vertically Down\0Map Properties\0"
"Full Solid\0Cannon\0Base\0"
"Fuel Block\0Target\0Ball Area\0Ball Target\0Ball\0Item Concentrator\0"
"Push/Pull Gravities\0Circular Gravity\0"
"Wormhole\0I\\O Wormhole\0Out Wormhole\0In Wormhole\0Current\0Decoration Wall\0"
"Positive\0Negative\0Hidden\0Check Map\0View Source\0Launch Map\0Next Check Point\0"
"Pick Item\0Add Vertex\0Delete Vertex\0Move Vertex\0Move Item\0Reorder Checkpoint\0Delete Item\0"
"Vertex\0Engine\0Main Gun\0Left Front Gun\0Right Front Gun\0Left Rear Gun\0"
"Right Rear Gun\0Missile Rack\0Left Light\0Right Light\0";

TBBUTTON tbmapfile[] = 
{
	STD_FILENEW, IDM_NEW, TBSTATE_ENABLED, TBSTYLE_BUTTON,0,0,0,0,
	STD_FILEOPEN, IDM_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON,0,0,0,0,
	STD_FILESAVE, IDM_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON,0,0,0,0,
	0,0, TBSTATE_ENABLED, TBSTYLE_SEP,0,0,0,0,
//	STD_CUT, IDM_EDIT_CUT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,0,0,0,
//	STD_COPY, IDM_EDIT_COPY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,0,0,0,
//	STD_PASTE, IDM_EDIT_PASTE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,0,0,0,
	0,0, TBSTATE_ENABLED, TBSTYLE_SEP,0,0,0,0,
	STD_UNDO, IDM_EDIT_UNDO, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,0,0,0,
} ;
TBBUTTON tbmaptools[] = 
{
	0, IDM_PEN, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	1, IDM_MODIFYITEM, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
//	18, IDM_SHAPE, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
//	2, IDM_SELECT, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
//	0, 0, TBSTATE_ENABLED, TBSTYLE_SEP,0,0,0,0,
//	3, IDM_CLEAR, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
//	14, IDM_CROP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
//	0, 0, TBSTATE_ENABLED, TBSTYLE_SEP,0,0,0,0,
//	4, IDM_ZOOMIN, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
//	5, IDM_ZOOMOUT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
//	0, 0, TBSTATE_ENABLED, TBSTYLE_SEP,0,0,0,0,
//	6, IDM_ROTATE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
//	7, IDM_MIRRORV, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
//	8, IDM_MIRRORH, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
//	10, IDM_CYCLEH, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
//	12, IDM_CYCLEHNEG, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
//	11, IDM_CYCLEV, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
//	13, IDM_CYCLEVNEG, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
//	0, 0, TBSTATE_ENABLED, TBSTYLE_SEP,0,0,0,0,
	9, IDM_PROPERTIES, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
//	15, IDM_CHECKMAP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
//	16, IDM_MAPSOURCE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
//	17, IDM_LAUNCHMAP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
} ;

TBBUTTON tbmapmodify[] = 
{
	0, IDM_PICKITEM, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	1, IDM_ADDVERTEX, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	2, IDM_DELVERTEX, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	3, IDM_MOVEVERTEX, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	4, IDM_MOVEITEM, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	6, IDM_REORDERCHECKPOINT, TBSTATE_INDETERMINATE, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	5, IDM_DELETEITEM, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
} ;

TBBUTTON tbwormhole[] = 
{
	0, IDM_MAP_WORM_NORMAL, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	1, IDM_MAP_WORM_OUT, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	2, IDM_MAP_WORM_IN, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
} ;
TBBUTTON tbpolarity[] = 
{
	0, IDM_MAP_POSITIVE, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	1, IDM_MAP_NEGATIVE, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
} ;
TBBUTTON tbwalltype[] = 
{
	0, IDM_MAP_HIDDEN, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0, 0, 0,
} ;

//TO DO: Get all mapsyms acceptable or removed so none are INDETERMINATE.
TBBUTTON tbmapsyms[] = 
{
	0, IDM_MAP_WALL, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	1, IDM_MAP_CANNON, TBSTATE_INDETERMINATE, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	2, IDM_MAP_BASE, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	3, IDM_MAP_FUEL, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	4, IDM_MAP_TARGET, TBSTATE_INDETERMINATE, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	5, IDM_MAP_BALLAREA, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	6, IDM_MAP_BALLTARGET, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	7, IDM_MAP_BALL, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	8, IDM_MAP_ITEM_CONC, TBSTATE_INDETERMINATE, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	9, IDM_MAP_GRAVITY, TBSTATE_INDETERMINATE, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	10, IDM_MAP_CIRCULAR_GRAVITY, TBSTATE_INDETERMINATE, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	11, IDM_MAP_WORMHOLE, TBSTATE_INDETERMINATE, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	12, IDM_MAP_CURRENT, TBSTATE_INDETERMINATE, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	13, IDM_MAP_DECOR, TBSTATE_INDETERMINATE, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	14, IDM_MAP_CHECKPOINT, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
} ;

TBBUTTON tbshiptools[] = 
{
	0, IDM_PEN, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	1, IDM_ERASE, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	2, IDM_CLEAR, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
	3, IDM_ROTATE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
	0, 0, TBSTATE_ENABLED, TBSTYLE_SEP,0,0,0,0,
	4, IDM_MIRRORV, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
	5, IDM_MIRRORH, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
	0, 0, TBSTATE_ENABLED, TBSTYLE_SEP,0,0,0,0,
	6, IDM_CYCLEH, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
	7, IDM_CYCLEHNEG, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
	8, IDM_CYCLEV, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
	9, IDM_CYCLEVNEG, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0,
} ;

TBBUTTON tbshipsyms[] = 
{
	0, IDM_VERTEX, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	1, IDM_ENGINE, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	2, IDM_MAINGUN, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	3, IDM_LEFTFRONTGUN, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	4, IDM_RIGHTFRONTGUN, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	5, IDM_LEFTREARGUN, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	6, IDM_RIGHTREARGUN, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	7, IDM_MISSILERACK, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	8, IDM_LEFTLIGHT, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
	9, IDM_RIGHTLIGHT, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0,
} ;

HWND hwndToolTip;
static HWND hwndTT;
//extern char szTbStrings[];


/* Map toolbar button command to string index. */
int CommandToString[] = 
{IDM_NEW,IDM_OPEN,IDM_SAVE,IDM_EDIT_CUT,IDM_EDIT_COPY,IDM_EDIT_PASTE,
IDM_EDIT_UNDO,IDM_PEN,IDM_MODIFYITEM,IDM_ERASE,IDM_SHAPE,
IDM_CLEAR,IDM_CROP,IDM_SELECT,IDM_ZOOMIN,IDM_ZOOMOUT, IDM_ROTATE,
IDM_MIRRORV,IDM_MIRRORH,IDM_CYCLEH,IDM_CYCLEHNEG,IDM_CYCLEV,
IDM_CYCLEVNEG,IDM_PROPERTIES,IDM_MAP_WALL,IDM_MAP_CANNON,
IDM_MAP_BASE,IDM_MAP_FUEL,IDM_MAP_TARGET,IDM_MAP_BALLAREA,IDM_MAP_BALLTARGET,
IDM_MAP_BALL,IDM_MAP_ITEM_CONC,IDM_MAP_GRAVITY,IDM_MAP_CIRCULAR_GRAVITY,
IDM_MAP_WORMHOLE,IDM_MAP_WORM_NORMAL,IDM_MAP_WORM_OUT,IDM_MAP_WORM_IN,
IDM_MAP_CURRENT,IDM_MAP_DECOR,IDM_MAP_POSITIVE,IDM_MAP_NEGATIVE,IDM_MAP_HIDDEN,
IDM_CHECKMAP,IDM_MAPSOURCE,
IDM_LAUNCHMAP,IDM_MAP_CHECKPOINT,
IDM_PICKITEM,IDM_ADDVERTEX,IDM_DELVERTEX,IDM_MOVEVERTEX,IDM_MOVEITEM,IDM_REORDERCHECKPOINT,IDM_DELETEITEM,
IDM_VERTEX,IDM_ENGINE,IDM_MAINGUN,IDM_LEFTFRONTGUN,IDM_RIGHTFRONTGUN,
IDM_LEFTREARGUN,IDM_RIGHTREARGUN,IDM_MISSILERACK,IDM_LEFTLIGHT,IDM_RIGHTLIGHT,
-1   
} ;
/***************************************************************************/
/* InitMapSymsToolBar                                                      */
/* Arguments :                                                             */
/*   hwndParent                                                            */
/* Purpose : Initializes the Map symbols Toolbar                           */
/***************************************************************************/
int InitToolBars (HWND hwndParent)
{
	int i;

	struct toolbar {
		LPTBBUTTON ptbbutton;
		HWND	*hwndToolbar;
		DWORD	dStyles;
		int		numBitmaps;
		HINSTANCE	hInst;
		int		bitmapID;
		int		posx;
		int		posy;
		int		width;
		int		height;
		int		*iSelection;
		int		select;
	};
#define NUMTOOLBARS 9
	struct toolbar toolbars[NUMTOOLBARS] =
	{
		{&tbmapfile[0], &hwndFileToolBar, dwToolBarStyles | CCS_TOP, 6, HINST_COMMCTRL, IDB_STD_SMALL_COLOR, 0, 0, 200, 20, NULL, 0},
		{&tbmapsyms[0], &hwndMapSymsToolBar, dwToolBarStyles | CCS_NORESIZE, 15, hInst, IDB_BLOCKSTOOLBAR, 0, 54, TOOLSWIDTH, 135, &iSelectionMapSyms, IDM_MAP_WALL},
		{&tbmaptools[0], &hwndMapToolsToolBar, dwToolBarStyles | CCS_NOMOVEY | TBSTYLE_FLAT, 3, hInst, IDB_MAPTOOLSTOOLBAR, 0, 25, 0, 0, &iSelectionMapTools, IDM_PEN},
		{&tbmapmodify[0], &hwndMapModifyToolBar, dwToolBarStyles | CCS_NORESIZE, 7, hInst, IDB_MAPMODIFYTOOLBAR, 0, 54, 24, 160, &iSelectionMapModify, IDM_PICKITEM},
		{&tbwormhole[0], &hwndWormholeToolBar, dwToolBarStyles | CCS_NOMOVEY | CCS_NORESIZE, 3, hInst, IDB_WORMHOLESUBBAR, 0, 220, TOOLSWIDTH, 40, &iSelectionWormhole, IDM_MAP_WORM_NORMAL},
		{&tbpolarity[0], &hwndPolarityToolBar, dwToolBarStyles | CCS_NOMOVEY | CCS_NORESIZE, 2, hInst, IDB_POLARITYSUBBAR, 0, 220, TOOLSWIDTH, 40, &iSelectionPolarity, IDM_MAP_POSITIVE},
		{&tbwalltype[0], &hwndWallTypeToolBar, dwToolBarStyles | CCS_NOMOVEY | CCS_NORESIZE, 1, hInst, IDB_WALLTYPESUBBAR, 0, 220, TOOLSWIDTH, 40, NULL, IDM_MAP_NORMAL},
		{&tbshiptools[0], &hwndShipToolsToolBar, dwToolBarStyles | CCS_NOMOVEY | TBSTYLE_FLAT, 10, hInst, IDB_SHIPTOOLSTOOLBAR, 0, 28, 120, 120, &iSelectionShipTools, IDM_PEN},
		{&tbshipsyms[0], &hwndShipSymsToolBar, dwToolBarStyles | CCS_NORESIZE, 10, hInst, IDB_SHIPSYMSTOOLBAR, 0, 54, 24, 300, &iSelectionShipSyms, IDM_VERTEX}
	};

	
	for (i = 0; i < NUMTOOLBARS; i++)
	{
		*toolbars[i].hwndToolbar = CreateToolbarEx (hwndParent,
			toolbars[i].dStyles,
			1, toolbars[i].numBitmaps,
			toolbars[i].hInst,
			toolbars[i].bitmapID,
			&toolbars[i].ptbbutton[0],
			toolbars[i].numBitmaps,
			0, 0, 16, 16,
			sizeof (TBBUTTON)) ;		
		
		if (i > 0 && *toolbars[i].hwndToolbar != hwndWallTypeToolBar)
		{
			SetWindowPos (*toolbars[i].hwndToolbar, HWND_TOP, toolbars[i].posx, toolbars[i].posy,
			toolbars[i].width, toolbars[i].height, 0);

			*toolbars[i].iSelection = toolbars[i].select;
			SendMessage(*toolbars[i].hwndToolbar, TB_CHECKBUTTON, toolbars[i].select, TRUE);
		}
		else if (*toolbars[i].hwndToolbar == hwndWallTypeToolBar)
		{
			SetWindowPos (*toolbars[i].hwndToolbar, HWND_TOP, toolbars[i].posx, toolbars[i].posy,
			toolbars[i].width, toolbars[i].height, 0);
//			SendMessage(*toolbars[i].hwndToolbar, TB_CHECKBUTTON, toolbars[i].select, TRUE);
		}
	}
	return FALSE ;
}
/***************************************************************************/
/* CopyToolTipText                                                         */
/* Arguments :                                                             */
/*   lpttt                                                                 */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/* Purpose : Copys tool tip text to a toolbar                              */
/***************************************************************************/
void CopyToolTipText (LPTOOLTIPTEXT lpttt)
{
	int i ;
	int iButton = lpttt->hdr.idFrom ;
	int cb ;
	int cMax ;
	LPSTR pString ;
	LPSTR pDest = lpttt->lpszText ;
	
	/* Map command ID to string index */
	for (i = 0 ; CommandToString[i] != -1 ; i++)
	{
		if (CommandToString[i] == iButton)
		{
			iButton = i ;
			break ;
		}
	}
	
	/* To be safe, count number of strings in text */
	pString = szTbStrings ;
	cMax = 0 ;
	while (*pString != '\0')
	{
		cMax++ ;
		cb = lstrlen (pString) ;
		pString += (cb + 1) ;
	}
	
	/* Check for valid parameter */
	if (iButton > cMax)
	{
		pString = "Invalid Button Index" ;
	}
	else
	{
		/* Cycle through to requested string */
		pString = szTbStrings ;
		for (i = 0 ; i < iButton ; i++)
		{
			cb = lstrlen (pString) ;
			pString += (cb + 1) ;
		}
	}
	
	lstrcpy (pDest, pString) ;
}

