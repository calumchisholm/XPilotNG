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

/*This file should contain all the windows specific prototypes.
All OS independent prototypes should be placed in common\common_proto.h,
*/

/*win_xpstudio.c*/
LRESULT CALLBACK MainWndProc (HWND, UINT, WPARAM, LPARAM) ;
void UpdateStatusBarSettings(LPMAPDOCUMENT);
void UpdateCommands(HWND);
void ShowMapPallete(HWND, BOOL);
void ShowShipPallete(HWND, BOOL);
void ShowNoWindowPallete(HWND, BOOL);
BOOL CALLBACK NewDocDlgProc (HWND, UINT, WPARAM, LPARAM);
/*win_mapeditor*/
HWND CreateMapEditor(HWND);
LRESULT CALLBACK MapEditorWndProc (HWND, UINT, WPARAM, LPARAM) ;

/*win_shipeditor*/
HWND CreateShipEditor(HWND);
LRESULT CALLBACK ShipEditorWndProc (HWND, UINT, WPARAM, LPARAM) ;

/*win_toolbar.c*/
int InitToolBars (HWND);
HWND InitFileToolBar (HWND);
HWND InitMapSymsToolBar (HWND);
HWND InitMapModifyToolBar (HWND);
HWND InitMapToolsToolBar (HWND);
HWND InitWormholeToolBar (HWND);
HWND InitPolarityToolBar (HWND);
HWND InitWallTypeToolBar (HWND);
HWND InitShipToolsToolBar (HWND);
HWND InitShipSymsToolBar (HWND);
void CopyToolTipText (LPTOOLTIPTEXT);

/*win_draw.c*/
void DrawHighlightLine (HWND, POINT, POINT);
HFONT GetZoomFont(HDC, LPMAPDOCUMENT);
void DeleteZoomFont(HDC, HFONT);


/*win_file.c*/
void	XpFileInitialize (HWND) ;
BOOL	XpFileOpenDlg    (HWND, PSTR, PSTR) ;
BOOL	XpFileSaveDlg    (HWND, PSTR, PSTR) ;
BOOL	XpTextSaveDlg    (HWND, PSTR, PSTR) ;
BOOL	XpOnlySaveDlg    (HWND, PSTR, PSTR) ;

/*win_helper.c*/
void SetSimpleCheck (HWND, int, int, int);
void SetEditText (HWND, int, LPCSTR);
void UpdateMapPrefText (HWND, int, LPTSTR);
int UpdateMapPrefCheck (HWND, int);
void InitListBox (HWND, int, LPCSTR );
void UpdateMapPrefList (HWND, int, LPTSTR);
void ToggleMenuItem(HMENU, int, BOOL);

/*win_prefs.c*/
BOOL CreatePropertySheet (HWND);
int CALLBACK PropSheetProc (HWND, UINT, LPARAM);
BOOL CALLBACK PrefsDefaultDlgProc (HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK PrefsPageProc (HWND, UINT, LPPROPSHEETPAGE);
BOOL CALLBACK PrefsCommentsDlgProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PrefsTypesDlgProc (HWND, UINT, WPARAM, LPARAM);
BOOL WINAPI InitListViewColumns(HWND, int);
BOOL WINAPI InitListViewItems(HWND, LPMAPDOCUMENT, int);
LPSTR DupString(LPSTR);

