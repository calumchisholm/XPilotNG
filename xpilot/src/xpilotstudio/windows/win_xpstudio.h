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
/*This file should contain all the windows specific include files.
All OS independent files should be placed in common\main.h,
which is in turn included here.
*/
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <commctrl.h>

#include "..\main.h"
#include "win_resdefines.h"
#include "win_proto.h"
#include "win_mapeditor.h"
//#include "win_shipeditor.h"

extern HINSTANCE hInst;

extern HWND hwndMain;
extern HWND hwndClient;
extern HWND hwndStatusBar;
extern HWND hwndActive;
extern HWND hwndTemp; /*Temporary window handle for painting,
					  in case the window being painted isn't
					  actually the active window.*/
/*The pallete toolbars. Only the appropriate toolbars
for the current type of item being edited should be displayed*/
extern HWND hwndFileToolBar;			
extern HWND hwndMapSymsToolBar;
extern HWND hwndMapToolsToolBar;
extern HWND hwndWormholeToolBar;
extern HWND hwndPolarityToolBar;
extern HWND hwndMapModifyToolBar;

extern HWND hwndShipToolsToolBar;
extern HWND hwndShipSymsToolBar;

extern HWND hwndUpDownTeam;
extern HWND hwndUpDownDirection;

extern HMENU hMenuMap;
extern HMENU hMenuMapWindow;
extern HMENU hMenuShip;


extern HDC mapDC;
extern HDC shiplistDC;
extern HDC shipviewDC;

/*File...Open Dialog box filter*/
extern unsigned long *zfilterIndex;
