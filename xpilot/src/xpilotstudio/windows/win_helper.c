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

/***************************************************************************/
/* SetSimpleCheck                                                          */
/* Arguments :                                                             */
/*    hwndDlg                                                              */
/*    CtrlID                                                               */
/*    CtrlIDMaster: the control id of the master, so that one control can  */
/*    set the state of another.                                            */
/*    bCheck                                                               */
/* Purpose :                                                               */
/***************************************************************************/
void SetSimpleCheck (HWND hwndDlg, int CtrlID, int CtrlIDMaster, int bCheck)
{
	HWND hwndCtrl = GetDlgItem (hwndDlg, CtrlID) ;


	Button_SetCheck(hwndCtrl, bCheck);
}
/***************************************************************************/
/* SetEditText                                                             */
/* Arguments :                                                             */
/*    hwndDlg                                                              */
/*    CtrlID                                                               */
/*    lpsz                                                                 */
/*                                                                         */
/* Purpose :                                                               */
/***************************************************************************/
void SetEditText (HWND hwndDlg, int CtrlID, LPCSTR lpsz)
{
	HWND hwndCtrl = GetDlgItem (hwndDlg, CtrlID) ;
	
	Edit_SetText(hwndCtrl, lpsz) ;
	
}
/***************************************************************************/
/* UpdateMapPrefText                                                       */
/* Arguments :                                                             */
/*    hwndDlg                                                              */
/*    CtrlID                                                               */
/*    lpsz                                                                 */
/*                                                                         */
/* Purpose :                                                               */
/***************************************************************************/
void UpdateMapPrefText (HWND hwndDlg, int CtrlID, LPTSTR lpsz)
{
	HWND hwndCtrl = GetDlgItem (hwndDlg, CtrlID) ;
	
	Edit_GetText(hwndCtrl, lpsz, 255) ;
	
}
/***************************************************************************/
/* UpdateMapPrefCheck                                                      */
/* Arguments :                                                             */
/*    hwndDlg                                                              */
/*    CtrlIDYes                                                            */
/*                                                                         */
/* Purpose :                                                               */
/***************************************************************************/
int UpdateMapPrefCheck (HWND hwndDlg, int CtrlID)
{
	HWND hwndCtrl = GetDlgItem (hwndDlg, CtrlID) ;
	
	int  nCheck;
	int bCheck;
	
	if (nCheck = Button_GetCheck (hwndCtrl))
	{
		bCheck = 1;
	}
	else
	{
		bCheck = 0;
	}
    return bCheck;
}
/***************************************************************************/
/* InitListBox                                                             */
/* Arguments :                                                             */
/*    hwndDlg                                                              */
/*    CtrlID                                                               */
/*    lpsz                                                                 */
/*                                                                         */
/* Purpose :                                                               */
/***************************************************************************/
void InitListBox (HWND hwndDlg, int CtrlID, LPCSTR lpsz)
{
	HWND hwndListBox = GetDlgItem (hwndDlg, CtrlID) ;
	
	ListBox_AddString(hwndListBox, "0-Straight");
	ListBox_AddString(hwndListBox, "1-Random");
	ListBox_AddString(hwndListBox, "2-Good");
	ListBox_AddString(hwndListBox, "3-Accurate");
	if(!strcmp(lpsz, "0"))
	{
		ListBox_SetCurSel(hwndListBox, 0);
	}
	if(!strcmp(lpsz, "1"))
	{
		ListBox_SetCurSel(hwndListBox, 1);
	}
	if(!strcmp(lpsz, "2"))
	{
		ListBox_SetCurSel(hwndListBox, 2);
	}
	if(!strcmp(lpsz, "3"))
	{
		ListBox_SetCurSel(hwndListBox, 3);
	}
	
	
}
/***************************************************************************/
/* UpdateMapPrefList                                                       */
/* Arguments :                                                             */
/*    hwndDlg                                                              */
/*    CtrlID                                                               */
/*    lpsz                                                                 */
/*                                                                         */
/* Purpose :                                                               */
/***************************************************************************/
void UpdateMapPrefList (HWND hwndDlg, int CtrlID, LPTSTR lpsz)
{
	HWND hwndCtrl = GetDlgItem (hwndDlg, CtrlID) ;
	int selected = 1;
	
	selected = ListBox_GetCurSel(hwndCtrl) ;
	
	sprintf(lpsz, "%d", selected);
}
/***************************************************************************/
/* ToggleMenuItem                                                          */
/* Arguments :                                                             */
/*    hMenu                                                                */
/*    item                                                                 */
/*    state                                                                */
/*                                                                         */
/* Purpose :                                                               */
/***************************************************************************/
void ToggleMenuItem(HMENU hMenu, int item, BOOL state)
{
	if (state)
		CheckMenuItem (hMenu, item, MF_CHECKED) ;
	else
		CheckMenuItem (hMenu, item, MF_UNCHECKED) ;

}
