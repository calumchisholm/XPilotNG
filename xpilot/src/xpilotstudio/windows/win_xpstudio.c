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

/*
This is the main entry point for the windows program.
*/

#include "win_xpstudio.h"

unsigned char *xpilotmap_dir=NULL;

HWND hwndMain = NULL;
HWND hwndClient = NULL;

/*Common Bars*/
HWND hwndStatusBar = NULL;
HWND hwndFileToolBar = NULL;

/*Map Only Bars*/
HWND hwndMapSymsToolBar = NULL;
HWND hwndMapToolsToolBar = NULL;
HWND hwndShapeToolBar = NULL;
HWND hwndWormholeToolBar = NULL;
HWND hwndPolarityToolBar = NULL;
HWND hwndWallTypeToolBar = NULL;
HWND hwndMapModifyToolBar = NULL;

/*Ship Only Bars*/
HWND hwndShipToolsToolBar = NULL;
HWND hwndShipSymsToolBar = NULL;

/*The enterable teamnumber*/
HWND hwndLabelTeam = NULL;
HWND hwndEditTeam = NULL;
HWND hwndUpDownTeam = NULL;

/*The enterable angle.*/
HWND hwndLabelDirection = NULL;
HWND hwndEditDirection = NULL;
HWND hwndUpDownDirection = NULL;

HWND hwndActive = NULL;
HWND hwndTemp = NULL;

HMENU hMenuMain;
HMENU hMenuMainWindow;
HMENU hMenuMap;
HMENU hMenuMapWindow;
HMENU hMenuShip;
HMENU hMenuShipWindow;
HINSTANCE hInst;

unsigned long  *zfilterIndex = NULL;
static PSTR szCmdLineMap;
static int wincxClient, wincyClient; 

/*Handles to pens and brushes*/
HBRUSH hBrushFuel;
HPEN hPenSelected;
HPEN hPenHidden;
/***************************************************************************/
/* WinMain                                                                 */
/* Arguments :                                                             */
/*    hInstance                                                            */
/*   hPrevInstance                                                         */
/*   szCmdLine                                                             */
/*   iCmdShow                                                              */
/* Purpose :   The main windows procedure                                  */
/***************************************************************************/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
	HACCEL  hAccel;
	MSG		 msg ;
	WNDCLASSEX  wndclass ;
	hInst = hInstance;
	szCmdLineMap = szCmdLine;

	/******************************************************************/
	/*The main window*/
	wndclass.cbSize        = sizeof (wndclass) ;
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = MainWndProc ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = 0 ;
	wndclass.hInstance     = hInstance ;
	wndclass.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_MAINICON)) ;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	wndclass.hbrBackground = (HBRUSH) GetSysColorBrush (COLOR_ACTIVEBORDER) ;
	wndclass.lpszMenuName  = NULL ;
	wndclass.lpszClassName = szAppName ;
	wndclass.hIconSm       = NULL;

    if (!RegisterClassEx (&wndclass) )
		return FALSE;
	
	/*The Map Editor Top Level Window Class*/
	wndclass.lpfnWndProc   = ShipEditorWndProc ;
	wndclass.lpszClassName = "ShipEditor" ;

    if (!RegisterClassEx (&wndclass) )
		return FALSE;

	/*The Map Editor Top Level Window Class*/
	wndclass.lpfnWndProc   = MapEditorWndProc ;
	wndclass.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
	wndclass.lpszClassName = "MapEditor" ;

    if (!RegisterClassEx (&wndclass) )
		return FALSE;


	hMenuMain = LoadMenu (hInst, MAKEINTRESOURCE(IDR_MAINMENU));
	hMenuMap = LoadMenu (hInst, MAKEINTRESOURCE(IDR_MAPEDITORMENU));
	hMenuShip = LoadMenu (hInst, MAKEINTRESOURCE(IDR_SHIPEDITORMENU));

	hMenuMainWindow = GetSubMenu (hMenuMain, 0);
	hMenuMapWindow = GetSubMenu (hMenuMap, 5);	
	hMenuShipWindow = GetSubMenu (hMenuShip, 3);

	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

	hwndMain = CreateWindow (szAppName,	// window class name
		szAppName,							// window caption
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,								// parent window handle
		hMenuMain,							// window menu handle
		hInstance,							// program instance handle
		NULL);								// creation parameters

	hwndClient = GetWindow(hwndMain, GW_CHILD);

    ShowWindow (hwndMain, iCmdShow);
    UpdateWindow (hwndMain);
 	

    /* Enter main message loop */
    while (GetMessage (&msg, NULL, 0, 0)){
		/* If a keyboard message is for the MDI , let the MDI client
		* take care of it.  Otherwise, check to see if it's a normal
		* accelerator key (like F3 = Zoom In).  Otherwise, just handle
		* the message as usual.
		*/
		if ( !TranslateMDISysAccel (hwndClient, &msg) &&
			!TranslateAccelerator (hwndMain, hAccel, &msg)){
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
    }

    return msg.wParam ;
}
/***************************************************************************/
/* MainWndProc                                                             */
/* Arguments :                                                             */
/*    iMsg                                                                 */
/*    wParam                                                               */
/*    lParam                                                               */
/*                                                                         */
/* Purpose :   The procedure for the main window                           */
/***************************************************************************/
LRESULT CALLBACK MainWndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	LPXPSTUDIODOCUMENT lpXpStudioDocument = NULL;
	LPMAPDOCUMENT lpMapDocument = NULL;
//	LPSHIPDOCUMENT lpShipDocument;
	HMENU hMenu = NULL;
	static HWND hwndLocClient;
	static HINSTANCE hWndInstance;
	CLIENTCREATESTRUCT clientcreate;
	char szTempFileName[_MAX_PATH] ;
	char szTempTitleName[_MAX_FNAME + _MAX_EXT] ;
	int *lpParts;
	
	switch (iMsg)
	{
	case WM_CREATE :
		clientcreate.hWindowMenu  = hMenuMain;
		clientcreate.idFirstChild = 90000 ;

		hwndLocClient = CreateWindow ("MDICLIENT", NULL,
			WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
			0, 0, 0, 0, hwnd, (HMENU) 1, hInst,
			(LPSTR) &clientcreate) ;

		hWndInstance = ((LPCREATESTRUCT) lParam)->hInstance;
		XpFileInitialize (hwnd);
//		zfilterIndex = (unsigned long *) malloc(sizeof(unsigned long));
		InitToolBars(hwnd);

		//Create the user friendly spin boxes for integer based team number & angle values.
		//These are defined as an updown box with an edit control as its buddy.
		hwndLabelTeam = CreateWindow("STATIC",
			"Team:",
			WS_CHILD,
			0, 300,
			TOOLSWIDTH-2, 20,
			hwnd,
			NULL,
			hInst,
			NULL);

		hwndEditTeam = CreateWindowEx(WS_EX_CLIENTEDGE,
			"EDIT",
			NULL,
			WS_CHILD | ES_RIGHT | ES_READONLY | ES_NUMBER,
			0, 320,
			TOOLSWIDTH-2, 20,
			hwnd,
			NULL,
			hInst,
			NULL);
		
		hwndUpDownTeam = CreateUpDownControl(WS_CHILD | WS_BORDER | UDS_ALIGNRIGHT | UDS_SETBUDDYINT | UDS_WRAP,
				0, 0,
				0, 0,
				hwnd,
				99990,
				hInst,
				hwndEditTeam,
				9, 0, 0);

		hwndLabelDirection = CreateWindow("STATIC",
			"Direction:",
			WS_CHILD,
			0, 350,
			TOOLSWIDTH-2, 20,
			hwnd,
			NULL,
			hInst,
			NULL);
		
		hwndEditDirection = CreateWindowEx(WS_EX_CLIENTEDGE,
			"EDIT",
			NULL,
			WS_CHILD | ES_RIGHT |ES_READONLY | ES_NUMBER,
			0, 370,
			TOOLSWIDTH-2, 20,
			hwnd,
			NULL,
			hInst,
			NULL);

		hwndUpDownDirection = CreateUpDownControl(WS_CHILD | WS_BORDER |
			UDS_ALIGNRIGHT | UDS_SETBUDDYINT | UDS_WRAP,
				0, 0,
				0, 0,
				hwnd,
				99991,
				hInst,
				hwndEditDirection,
				127, 0, 0);

		hwndStatusBar = CreateStatusWindow (WS_CHILD |
			WS_VISIBLE | WS_CLIPSIBLINGS | SBARS_SIZEGRIP |
			CCS_BOTTOM,
			"Ready",
			hwnd,
			2);		
		
		lpParts = (int *) malloc(sizeof(int) * 4);
		lpParts[0] = 200;
		lpParts[1] = 300;
		lpParts[2] = 400;
		lpParts[3] = 900;
		SendMessage(hwndStatusBar, SB_SETPARTS, (WPARAM) 4,(LPARAM) lpParts);
		SendMessage(hwnd, WM_COMMAND, (WPARAM) UPDATE_COMMANDS, 0);
		return 0;

	case WM_SIZE :
		wincxClient = LOWORD (lParam) ;
		wincyClient = HIWORD (lParam) ;
		{
			int x,y,cx,cy;
			RECT rWindow ;
			
			GetWindowRect(hwndStatusBar, &rWindow);
			cy = rWindow.bottom - rWindow.top;
			x = 0;
			y = wincyClient - cy;
			cx = wincxClient;
			MoveWindow (hwndStatusBar, x, y, cx, cy, TRUE);
			MoveWindow (hwndLocClient, TOOLSWIDTH, 54, wincxClient-TOOLSWIDTH, wincyClient - 75, TRUE);
		}
		return 0;
		
	case WM_COMMAND :
		hMenu = GetMenu(hwnd);
		switch (LOWORD (wParam))
		{
		case IDM_MAP_WALL :
		case IDM_MAP_CANNON :
		case IDM_MAP_BASE :
		case IDM_MAP_FUEL :
		case IDM_MAP_TARGET :
		case IDM_MAP_BALLAREA :
		case IDM_MAP_BALLTARGET :
		case IDM_MAP_BALL :
		case IDM_MAP_ITEM_CONC :
		case IDM_MAP_GRAVITY :
		case IDM_MAP_CIRCULAR_GRAVITY :
		case IDM_MAP_WORMHOLE :
		case IDM_MAP_CURRENT :
		case IDM_MAP_DECOR :
		case IDM_MAP_CHECKPOINT :
			iSelectionMapSyms = LOWORD (wParam);
			SendMessage(hwnd, WM_COMMAND, (WPARAM) UPDATE_COMMANDS, 0);
			return 0;

		case IDM_MAP_WORM_NORMAL :
		case IDM_MAP_WORM_OUT :
		case IDM_MAP_WORM_IN :
			iSelectionWormhole = LOWORD (wParam);
			SendMessage(hwnd, WM_COMMAND, (WPARAM) UPDATE_COMMANDS, 0);
			if (IsWindow (hwndActive))
			{
				PostMessage (hwndActive, WM_COMMAND, IDM_UPDATE_ITEM_PARAMS, lParam);
				InvalidateRect(hwndActive, NULL, TRUE);
			}
			return 0;

		case IDM_MAP_POSITIVE :
		case IDM_MAP_NEGATIVE :
			iSelectionPolarity = LOWORD (wParam);
			SendMessage(hwnd, WM_COMMAND, (WPARAM) UPDATE_COMMANDS, 0);
			if (IsWindow (hwndActive))
			{
				PostMessage (hwndActive, WM_COMMAND, IDM_UPDATE_ITEM_PARAMS, lParam);
				InvalidateRect(hwndActive, NULL, TRUE);
			}
			return 0;
		case IDM_MAP_NORMAL :
		case IDM_MAP_HIDDEN :
			iSelectionWallType = LOWORD (wParam);
			SendMessage(hwnd, WM_COMMAND, (WPARAM) UPDATE_COMMANDS, 0);
			if (IsWindow (hwndActive))
			{
				PostMessage (hwndActive, WM_COMMAND, IDM_UPDATE_ITEM_PARAMS, lParam);
				InvalidateRect(hwndActive, NULL, TRUE);
			}
			return 0;
		case IDM_PEN :
		case IDM_MODIFYITEM:
			if (hwndActive)
			{
				lpXpStudioDocument = (LPXPSTUDIODOCUMENT) GetWindowLong (hwndActive, GWL_USERDATA);
				lpMapDocument = lpXpStudioDocument->lpMapDocument;
				lpMapDocument->selectedbool = FALSE;
			}
			fDragging = FALSE;
			iSelectionMapTools = LOWORD (wParam);
			SendMessage(hwnd, WM_COMMAND, (WPARAM) UPDATE_COMMANDS, 0);
			return 0;
		case IDM_ADDVERTEX:
		case IDM_PICKITEM:
		case IDM_DELVERTEX:
		case IDM_MOVEVERTEX:
		case IDM_MOVEITEM:
		case IDM_REORDERCHECKPOINT:
			fDragging = FALSE;
			iSelectionMapModify = LOWORD (wParam);
			return 0;
		case IDM_NEW: 
			if (!DialogBox (hWndInstance, MAKEINTRESOURCE(IDD_NEWDOCUMENT), hwnd, NewDocDlgProc))
			{
				return 0;
			}
			break;
		case IDM_NEWMAPEDITOR:
			hwndActive = CreateMapEditor(hwndLocClient);
			ShowWindow (hwndActive, SW_SHOW) ;
			UpdateWindow (hwndActive);
			break;
		case IDM_NEWSHIPEDITOR:
			hwndActive = CreateShipEditor(hwndLocClient);
			ShowWindow (hwndActive, SW_SHOW);
			UpdateWindow(hwndActive);
			break;

		case IDM_OPEN :
			szTempFileName[0]  = '\0' ;
			szTempTitleName[0]  = '\0' ;
//			zfilterIndex = (unsigned long *) malloc(sizeof(unsigned long));
			if (XpFileOpenDlg (hwnd, szTempFileName, szTempTitleName))
			{
				//ToDo: Change this switch! We should be
				//Testing for the document type, not being dependant
				//Upon the setting of the open box's type field.
				switch (*zfilterIndex)
				{
				case 1: //Xpilot Map
					hwndActive = CreateMapEditor(hwndLocClient);
					if (!hwndActive)
					{
						ErrorHandler("Could not create new map window!");
						return 1;
					}
					lpXpStudioDocument = (LPXPSTUDIODOCUMENT) GetWindowLong (hwndActive, GWL_USERDATA);
					lpMapDocument = lpXpStudioDocument->lpMapDocument;
					if (!LoadMap(lpMapDocument, szTempFileName))
					{
						ErrorHandler("Could not read file %s!", szTempTitleName);
						return 1;
					}
					strcpy(lpMapDocument->MapStruct.mapFileName, szTempFileName);
					break;
				}
			}
			else
				return 1;
			zfilterIndex = NULL;
//			free(zfilterIndex);
			break;
		case IDM_SAVE :
			hwndActive = (HWND) SendMessage (hwndLocClient, WM_MDIGETACTIVE, 0, 0);
			lpXpStudioDocument = (LPXPSTUDIODOCUMENT) GetWindowLong (hwndActive, GWL_USERDATA);
			
			switch (lpXpStudioDocument->type)
			{
			case MAPFILE:
				lpMapDocument = lpXpStudioDocument->lpMapDocument;
				strcpy(szTempFileName, lpMapDocument->MapStruct.mapFileName);
				if (szTempFileName[0])
				{
					if (!SaveMap(lpMapDocument, szTempFileName, TRUE, FALSE))
					{
						lpMapDocument->changed = FALSE ;
//						DoCaption (lpMapDocData, hwndChild, lpMapDocData->MapStruct.mapName) ;
						return 0 ;
					}
					else
						ErrorHandler("Could not write file %s!", szTempTitleName);
					break;
				}
				break;
			default: //todo: get rid of this and add saving for other types
				return 0;
			}
			// fall through
		case IDM_SAVEAS :
			hwndActive = (HWND) SendMessage (hwndLocClient, WM_MDIGETACTIVE, 0, 0);
			lpXpStudioDocument = (LPXPSTUDIODOCUMENT) GetWindowLong (hwndActive, GWL_USERDATA);

//			zfilterIndex = (unsigned long *) malloc(sizeof(unsigned long));
			szTempFileName[0]  = '\0' ;
			szTempTitleName[0]  = '\0' ;
			switch (lpXpStudioDocument->type)
			{
			case MAPFILE:
				lpMapDocument = lpXpStudioDocument->lpMapDocument;
				if (XpFileSaveDlg (hwnd, szTempFileName, szTempTitleName))
				{
					BOOL rValue = FALSE; //Only true if error is returned.
					
					switch (*zfilterIndex)
					{
					case 1: //Xpilot Map
						if (rValue = !SaveMap(lpMapDocument, szTempFileName, TRUE, FALSE))
						{
							lpMapDocument->changed = FALSE ;
							//DoCaption (lpMapDocData, hwndChild, lpMapDocData->MapStruct.mapName) ;
							return 0 ;
						}
						break;
					/*
					case 2: //X11 Pixmap
						rValue = SaveXpmFile(lpMapDocData, szTempFileName);
						break;
					case 3: //X11 Bitmap
						rValue = SaveXbmFile(lpMapDocData, szTempFileName);
						break;			
					case 4: //PBMPLUS Bitmap
						rValue = SavePbmPlusFile(lpMapDocData, szTempFileName, 0);
						break;
					case 5: //PBMPLUS Pixmap
						rValue = SavePbmPlusFile(lpMapDocData, szTempFileName, 1);
						break;
					case 6: //PBMPLUS Pixmap
						rValue = SavePbmPlusFile(lpMapDocData, szTempFileName, 2);
						break;
					case 7: //Windows Bitmap
						rValue = SaveBmpFile(hwndChild, szTempFileName);
						break;*/
					}
					if (rValue)
						ErrorHandler("Could not write file %s!", szTempTitleName);
				}
				break;
			default: //todo: add processing for other map types
				break;
			}
			zfilterIndex = NULL;
//			free(zfilterIndex);
			return 0;

		case UPDATE_COMMANDS:
			UpdateCommands(hwndLocClient);
			break;
		
		//messages for arranging windows
		case IDM_TILE :
			SendMessage (hwndLocClient, WM_MDITILE, 0, 0);
			return 0;
			
		case IDM_CASCADE :
			SendMessage (hwndLocClient, WM_MDICASCADE, 0, 0);
			return 0;
			
		case IDM_ARRANGE :
			SendMessage (hwndLocClient, WM_MDIICONARRANGE, 0, 0);
			return 0;

		default: //Pass to active child...
			if (IsWindow (hwndActive))
			{
				SendMessage (hwndActive, WM_COMMAND, LOWORD (wParam), lParam);
			}
			break ;        // ...and then to DefFrameProc
		}
		break;

		case WM_NOTIFY :
			{
				LPNMHDR pnmh = (LPNMHDR) lParam ;
				
				if (pnmh->code == TTN_NEEDTEXT)
				{
					LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT) lParam;
					CopyToolTipText (lpttt);
				}
				if (pnmh->code == UDN_DELTAPOS)
				{
					if (IsWindow (hwndActive))
					{
						PostMessage (hwndActive, WM_COMMAND, IDM_UPDATE_ITEM_PARAMS, lParam);
					}
				}
				return 0;
			}
		case WM_DESTROY : 
			PostQuitMessage (0) ;
			return 0;
    }
	return DefFrameProc (hwnd,hwndLocClient,iMsg,wParam,lParam);
}
/***************************************************************************/
/* UpdateStatusBarSettings                                                 */
/* Purpose : Update the information displayed in the status bar            */
/***************************************************************************/
void UpdateStatusBarSettings(LPMAPDOCUMENT lpMapDocument)
{
	char sbtext[100];

	if (lpMapDocument->MapStruct.edgeWrap)
		sprintf(sbtext, "edgeWrap:On");
	else
		sprintf(sbtext, "edgeWrap:Off");
	SendMessage(hwndStatusBar, SB_SETTEXT, (WPARAM) 1 | 0, (LPARAM) (LPSTR) sbtext); 

	sprintf(sbtext, "Author: %s", lpMapDocument->MapStruct.mapAuthor);
	SendMessage(hwndStatusBar, SB_SETTEXT, (WPARAM) 3 | 0, (LPARAM) (LPSTR) sbtext); 

}
/***************************************************************************/
/* UpdateCommands                                                          */
/* Arguments :                                                             */
/* Purpose :   Update the editor menus, commands etc, for the current type */
/***************************************************************************/
void UpdateCommands(HWND hwndLocClient)
{
	LPXPSTUDIODOCUMENT lpXpStudioDocument;
	BOOL show = FALSE;
	HWND hwndTmp;

	hwndTmp = (HWND) SendMessage (hwndLocClient, WM_MDIGETACTIVE, 0, 0);

	lpXpStudioDocument = (LPXPSTUDIODOCUMENT) GetWindowLong (hwndTmp, GWL_USERDATA);
	if (lpXpStudioDocument != NULL) {
		show = TRUE;
		ShowNoWindowPallete(hwndLocClient, show);
		//Switch function should turn on the correct pallete stuff, and turn
		//off the incorrect.
		switch (lpXpStudioDocument->type)
		{
		case MAPFILE:
			ShowShipPallete(hwndLocClient, FALSE);
			ShowMapPallete(hwndLocClient, TRUE);
			break;		
		case SHIPFILE:
			ShowMapPallete(hwndLocClient, FALSE);
			ShowShipPallete(hwndLocClient, TRUE);
			break;
			//Add functions for other Pallete configurations
		}
	}
	else
	{
		ShowNoWindowPallete(hwndLocClient, show);
	}
	DrawMenuBar (GetParent(hwndLocClient));
}
/***************************************************************************/
/* ShowNoWindowPallete                                                     */
/* Arguments :                                                             */
/*  show: should this stuff be shown?                                      */
/* Purpose :   Update the interface because no windows are shown.          */
/***************************************************************************/
void ShowNoWindowPallete(HWND hwndLocClient, BOOL show)
{
		ShowMapPallete(hwndLocClient, show);
		ShowShipPallete(hwndLocClient, show);
		SendMessage (hwndLocClient, WM_MDISETMENU, (WPARAM) hMenuMain, (LPARAM) hMenuMain);
		SendMessage(hwndFileToolBar, TB_HIDEBUTTON, IDM_SAVE, (LPARAM) MAKELONG(!show, 0));
		SendMessage(hwndFileToolBar, TB_HIDEBUTTON, IDM_EDIT_UNDO, (LPARAM) MAKELONG(!show, 0));
		SendMessage(hwndFileToolBar, TB_HIDEBUTTON, IDM_EDIT_CUT, (LPARAM) MAKELONG(!show, 0));
		SendMessage(hwndFileToolBar, TB_HIDEBUTTON, IDM_EDIT_COPY, (LPARAM) MAKELONG(!show, 0));
		SendMessage(hwndFileToolBar, TB_HIDEBUTTON, IDM_EDIT_PASTE, (LPARAM) MAKELONG(!show, 0));
}
/***************************************************************************/
/* ShowMapPallete                                                          */
/* Arguments :                                                             */
/*  show: should this stuff be shown?                                      */
/* Purpose :   Update the map toolbars, pallete, etc...                    */
/***************************************************************************/
void ShowMapPallete(HWND hwndLocClient, BOOL show)
{
	LPXPSTUDIODOCUMENT lpXpStudioDocument = NULL;
	LPMAPDOCUMENT lpMapDocument = NULL;
	HWND hwndTmp;
	int showms = FALSE;
	int type;
	char txt[10];

	hwndTmp = (HWND) SendMessage (hwndLocClient, WM_MDIGETACTIVE, 0, 0);
	if (hwndTmp)
	{
		lpXpStudioDocument = (LPXPSTUDIODOCUMENT) GetWindowLong (hwndTmp, GWL_USERDATA);
		lpMapDocument = lpXpStudioDocument->lpMapDocument;
	}
	if (show)
	{
		if (lpMapDocument)
		{
			if (iSelectionMapTools == IDM_PEN)
			{
				showms = TRUE;
			}
			else
			{
				showms = FALSE;
			}
		}
		else
			showms = FALSE;
		ShowWindow(hwndMapModifyToolBar, !showms);
	}
	else
		ShowWindow(hwndMapModifyToolBar, FALSE);

	ShowWindow(hwndMapSymsToolBar, showms);

	ShowWindow(hwndMapToolsToolBar, show);
	SendMessage (hwndLocClient, WM_MDISETMENU, (WPARAM) hMenuMap, (LPARAM) hMenuMapWindow);

	if ( showms || (lpMapDocument && lpMapDocument->selectedbool) )
	{
		SendMessage(hwndFileToolBar, TB_HIDEBUTTON, IDM_EDIT_CUT,
			(LPARAM) MAKELONG(FALSE, 0));
		SendMessage(hwndFileToolBar, TB_HIDEBUTTON, IDM_EDIT_COPY,
			(LPARAM) MAKELONG(FALSE, 0));
		SendMessage(hwndFileToolBar, TB_HIDEBUTTON, IDM_EDIT_PASTE,
			(LPARAM) MAKELONG(FALSE, 0));

		type = iSelectionMapSyms;

		if (lpMapDocument->selectedbool)
		{
			type = lpMapDocument->selectedtype;
		}
		sprintf(txt, "%d", teamSet);
		Edit_SetText(hwndEditTeam, txt);
		sprintf(txt, "%d", dirSet);
		Edit_SetText(hwndEditDirection, txt);
		
		switch (type)
		{
		case IDM_MAP_CANNON:
		case IDM_MAP_BASE:
		case IDM_MAP_FUEL:
		case IDM_MAP_TARGET:
		case IDM_MAP_BALLAREA:
		case IDM_MAP_BALLTARGET:
		case IDM_MAP_BALL:
			ShowWindow(hwndLabelTeam, SW_SHOW);
			ShowWindow(hwndEditTeam, SW_SHOW);
			ShowWindow(hwndUpDownTeam, SW_SHOW);
			break;
		default:
			ShowWindow(hwndLabelTeam, SW_HIDE);
			ShowWindow(hwndEditTeam, SW_HIDE);
			ShowWindow(hwndUpDownTeam, SW_HIDE);
			break;
		}
		switch (type)
		{
		case IDM_MAP_CANNON:
		case IDM_MAP_BASE:
		case IDM_MAP_CURRENT:
			ShowWindow(hwndLabelDirection, SW_SHOW);
			ShowWindow(hwndEditDirection, SW_SHOW);
			ShowWindow(hwndUpDownDirection, SW_SHOW);
			break;
		default:
			ShowWindow(hwndLabelDirection, SW_HIDE);
			ShowWindow(hwndEditDirection, SW_HIDE);
			ShowWindow(hwndUpDownDirection, SW_HIDE);
			break;
		}
		switch (type)
		{
		case IDM_MAP_CIRCULAR_GRAVITY:
		case IDM_MAP_GRAVITY:
			ShowWindow(hwndPolarityToolBar, SW_SHOW);
			break;
		default:
			ShowWindow(hwndPolarityToolBar, SW_HIDE);
			break;
		}
		switch (type)
		{
		case IDM_MAP_WORMHOLE:
			ShowWindow(hwndWormholeToolBar, SW_SHOW);
			break;
		default:
			ShowWindow(hwndWormholeToolBar, SW_HIDE);
			break;
		}		
		switch (type)
		{
		case IDM_MAP_WALL:
		case IDM_MAP_BALLTARGET:
		case IDM_MAP_BALLAREA:
		case IDM_MAP_DECOR:
			ShowWindow(hwndWallTypeToolBar, SW_SHOW);
			break;
		default:
			ShowWindow(hwndWallTypeToolBar, SW_HIDE);
			break;
		}
	}
	else
	{
			ShowWindow(hwndWormholeToolBar, SW_HIDE);
			ShowWindow(hwndWallTypeToolBar, SW_HIDE);
			ShowWindow(hwndLabelTeam, SW_HIDE);
			ShowWindow(hwndEditTeam, SW_HIDE);
			ShowWindow(hwndUpDownTeam, SW_HIDE);
			ShowWindow(hwndLabelDirection, SW_HIDE);
			ShowWindow(hwndEditDirection, SW_HIDE);
			ShowWindow(hwndUpDownDirection, SW_HIDE);
			ShowWindow(hwndPolarityToolBar, SW_HIDE);
	}
}
/***************************************************************************/
/* ShowShipPallete                                                         */
/* Arguments :                                                             */
/*  show: should this stuff be shown?                                      */
/* Purpose :   Update the tool pallete                                     */
/***************************************************************************/
void ShowShipPallete(HWND hwndLocClient, BOOL show)
{
	if (show)
	{
		SendMessage (hwndLocClient, WM_MDISETMENU, (WPARAM) hMenuShip, (LPARAM) hMenuShipWindow);
		SendMessage(hwndFileToolBar, TB_HIDEBUTTON, IDM_EDIT_CUT, (LPARAM) MAKELONG(TRUE, 0));
		SendMessage(hwndFileToolBar, TB_HIDEBUTTON, IDM_EDIT_COPY, (LPARAM) MAKELONG(TRUE, 0));
		SendMessage(hwndFileToolBar, TB_HIDEBUTTON, IDM_EDIT_PASTE, (LPARAM) MAKELONG(TRUE, 0));
	}
	ShowWindow(hwndShipToolsToolBar, show);
	ShowWindow(hwndShipSymsToolBar, show);
}
/***************************************************************************/
/* NewDocDlgProc                                                           */
/* Arguments :                                                             */
/*    hDlg                                                                 */
/*    iMsg                                                                 */
/*    wParam                                                               */
/*    lParam                                                               */
/* Purpose :   The procedure for the Wild Map prefs                        */
/***************************************************************************/
BOOL CALLBACK NewDocDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static int itemselect = 0;
	int itemcommand;
	int state;
	switch(iMsg)
	{
	case WM_INITDIALOG :
		if (itemselect)
			Button_SetCheck(GetDlgItem(hDlg, itemselect), BST_CHECKED);
		else
			Button_SetCheck(GetDlgItem(hDlg, IDC_RADIONEWMAPEDITOR), BST_CHECKED);

		return TRUE;
		
	case WM_COMMAND :
		switch (LOWORD (wParam))
		{
		case IDOK: 
			if (BST_CHECKED == (state = Button_GetCheck(GetDlgItem(hDlg, IDC_RADIONEWMAPEDITOR))))
			{
				itemselect = IDC_RADIONEWMAPEDITOR;
				itemcommand = IDM_NEWMAPEDITOR;
			}
			else if  (BST_CHECKED == (state = Button_GetCheck(GetDlgItem(hDlg, IDC_RADIONEWSHIPEDITOR))))
			{
				itemselect = IDC_RADIONEWSHIPEDITOR;
				itemcommand = IDM_NEWSHIPEDITOR;
			}
			SendMessage(hwndMain, WM_COMMAND, itemcommand, 0);
			EndDialog (hDlg, 1);
			return TRUE;
		case IDCANCEL :
			EndDialog (hDlg, 0);
			return FALSE;
		}
		break;
	}
	return FALSE;
}