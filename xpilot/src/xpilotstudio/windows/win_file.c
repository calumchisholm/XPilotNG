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

static OPENFILENAME ofn ;

/***************************************************************************/
/* XpFileInitialize                                                        */
/* Arguments :                                                             */
/*    hwnd                                                                 */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/* Purpose : Initializes the windows common dialog boxes                   */
/***************************************************************************/
void XpFileInitialize (HWND hwnd)
{
			
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hwndOwner         = hwnd ;
	ofn.hInstance         = NULL ;
	ofn.lpstrCustomFilter = NULL ;
	ofn.nMaxCustFilter    = 0 ;
	ofn.nFilterIndex      = 0;
	ofn.lpstrFile         = NULL ;          // Set in Open and Close functions
	ofn.nMaxFile          = _MAX_PATH ;
	ofn.lpstrFileTitle    = NULL ;          // Set in Open and Close functions
	ofn.nMaxFileTitle     = _MAX_FNAME + _MAX_EXT ;
	ofn.lpstrInitialDir   = (LPCTSTR) xpilotmap_dir ;
	ofn.lpstrTitle        = NULL ;
	ofn.Flags             = 0 ;             // Set in Open and Close functions
	ofn.nFileOffset       = 0 ;
	ofn.nFileExtension    = 0 ;
	ofn.lpstrDefExt       = "xp2" ;
	ofn.lCustData         = 0L ;
	ofn.lpfnHook          = NULL ;
	ofn.lpTemplateName    = NULL ;
}
/***************************************************************************/
/* XpFileOpenDlg                                                           */
/* Arguments :                                                             */
/*    hwnd                                                                 */
/*    pstrFileName                                                         */
/*    pstrTitleName                                                        */
/*                                                                         */
/* Purpose : runs the file open common dialogv                             */
/***************************************************************************/
BOOL XpFileOpenDlg (HWND hwnd, PSTR pstrFileName, PSTR pstrTitleName)
{
	int success;
	static char szOFilter[] = "Xpilot Map Files (*.xp2)\0*.xp2\0"  \
							"Xpilot Ship Files (*.sh2)\0*.sh2\0"  \
							"All Files (*.*)\0*.*\0\0" ;
	ofn.lpstrFilter       = szOFilter ;
	ofn.hwndOwner         = hwnd ;
	ofn.lpstrFile         = pstrFileName ;
	ofn.lpstrFileTitle    = pstrTitleName ;
	ofn.Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT ;
	success = GetOpenFileName (&ofn) ;
	
	zfilterIndex = &ofn.nFilterIndex;
	return success;

}

/***************************************************************************/
/* XpFileSaveDlg                                                           */
/* Arguments :                                                             */
/*    hwnd                                                                 */
/*    pstrFileName                                                         */
/*    pstrTitleName                                                        */
/*                                                                         */
/* Purpose : runs the file save common dialog                              */
/***************************************************************************/
BOOL XpFileSaveDlg (HWND hwnd, PSTR pstrFileName, PSTR pstrTitleName)
{
	int success;

	static char szSFilter[] = "Xpilot Map (*.XP2)\0*.xp2;\0"  \
							/*  "Windows Bitmap (*.bmp)\0*.bmp\0" \
							  "Jpeg Files (*.jpg)\0*.jpg\0" \
							  "Gif Image Files (*.gif)\0*.gif\0" \
							  "Tiff Image Files (*.tif)\0*.tif\0";
   							  "All Files (*.*)\0*.*\0\0"*/
							  ;

	ofn.lpstrFilter       = szSFilter ;
	ofn.hwndOwner         = hwnd ;
	ofn.lpstrFile         = pstrFileName ;
	ofn.lpstrFileTitle    = pstrTitleName ;
	ofn.Flags             = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT ;
	ofn.nFilterIndex	  = 1 ;
	success = GetSaveFileName (&ofn) ;
	zfilterIndex = &ofn.nFilterIndex;
	return success;
}
/***************************************************************************/
/* XpTextSaveDlg                                                           */
/* Arguments :                                                             */
/*    hwnd                                                                 */
/*    pstrFileName                                                         */
/*    pstrTitleName                                                        */
/*                                                                         */
/* Purpose : runs the file save common dialog                              */
/***************************************************************************/
BOOL XpTextSaveDlg (HWND hwnd, PSTR pstrFileName, PSTR pstrTitleName)
{
	int success;

	static char szSFilter[] = "Text File (*.txt)\0*.txt\0"  \
							  ;

	ofn.lpstrFilter       = szSFilter ;
	ofn.hwndOwner         = hwnd ;
	ofn.lpstrFile         = pstrFileName ;
	ofn.lpstrFileTitle    = pstrTitleName ;
	ofn.Flags             = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT ;
	ofn.nFilterIndex	  = 1 ;
	success = GetSaveFileName (&ofn) ;
	return success;
}
/***************************************************************************/
/* XpOnlySaveDlg                                                           */
/* Arguments :                                                             */
/*    hwnd                                                                 */
/*    pstrFileName                                                         */
/*    pstrTitleName                                                        */
/*                                                                         */
/* Purpose : runs the file save common dialog                              */
/***************************************************************************/
BOOL XpOnlySaveDlg (HWND hwnd, PSTR pstrFileName, PSTR pstrTitleName)
{
	int success;

	static char szSFilter[] = "Xpilot Map (*.xp2)\0*.xp2\0"  \
							  ;

	ofn.lpstrFilter       = szSFilter ;
	ofn.hwndOwner         = hwnd ;
	ofn.lpstrFile         = pstrFileName ;
	ofn.lpstrFileTitle    = pstrTitleName ;
	ofn.Flags             = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT ;
	ofn.nFilterIndex	  = 1 ;
	success = GetSaveFileName (&ofn) ;
	return success;
}
