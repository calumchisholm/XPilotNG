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

static POINT ptCurse;
/***************************************************************************/
/* CreateShipEditor                                                         */
/* Arguments :                                                             */
/*                                                                         */
/* Purpose :   Creates a new map editor window                             */
/***************************************************************************/
HWND CreateShipEditor(HWND hwndLocClient){
    MDICREATESTRUCT mdicreate;
	HWND hwndNewShip;

	mdicreate.szClass = "ShipEditor";
	mdicreate.szTitle = "New Ship";
	mdicreate.hOwner	= hInst;
	mdicreate.x = CW_USEDEFAULT;
	mdicreate.cx = CW_USEDEFAULT;
	mdicreate.y = CW_USEDEFAULT;
	mdicreate.cy = CW_USEDEFAULT;
	mdicreate.style = WS_VSCROLL | WS_HSCROLL;
	mdicreate.lParam = 0;
	
	hwndNewShip = (HWND)SendMessage (hwndLocClient,
		WM_MDICREATE,
		0,
		(LONG)(LPMDICREATESTRUCT)&mdicreate);

	return hwndNewShip;
}
/***************************************************************************/
/* ShipEditorWndProc                                                       */
/* Arguments :                                                             */
/*    iMsg                                                                 */
/*    wParam                                                               */
/*    lParam                                                               */
/*                                                                         */
/* Purpose :   The procedure for the ship editor, contains ship list & ship*/
/***************************************************************************/
LRESULT CALLBACK ShipEditorWndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	LPXPSTUDIODOCUMENT lpXpStudioDocument;
//	LPMAPDOCUMENT lpShipDocument ;
	static HWND hwndLocClient, hwndLocFrame;

	switch (iMsg)
	{
	case WM_CREATE :
		lpXpStudioDocument = CreateNewXpDocument(SHIPFILE);
		lpXpStudioDocument->lpShipDocument = CreateNewShipDoc();
		SetWindowLong(hwnd, GWL_USERDATA, (long) lpXpStudioDocument);
		hwndActive = hwnd;
		hwndLocClient = GetParent(hwnd);
		hwndLocFrame = GetParent (hwndLocClient);
		return 0;
	case WM_MDIACTIVATE:
		if(lParam == (LPARAM) hwnd)
		{
			SendMessage(hwndLocFrame, WM_COMMAND, (WPARAM) UPDATE_COMMANDS, 0);
//			DoCaption(lpMapDocument, hwnd, lpMapDocument->MapStruct.mapName);
		} 
		/*Redisplay the menubar*/
		DrawMenuBar (hwndLocFrame);
	}
	//Pass unprocessed message to DefMDIChildProc
    return DefMDIChildProc (hwnd, iMsg, wParam, lParam);
}
