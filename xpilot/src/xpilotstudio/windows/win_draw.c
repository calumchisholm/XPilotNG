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
/* DrawHighlightline                                                       */
/* Arguments :                                                             */
/*    hwnd - the parent window.                                            */
/*    ptBeg - the point to begin at.                                       */
/*    ptEnd - the point to end at.                                         */
/*                                                                         */
/* Purpose :   Draws a ROP2 line between two points.                       */
/***************************************************************************/
void DrawHighlightLine (HWND hwnd, POINT ptBeg, POINT ptEnd)
{
	static POINT pt[1];
	HPEN hPen;
	HDC hdc ;
	
	pt[0].x = ptBeg.x;
	pt[0].y = ptBeg.y;
	pt[1].x = ptEnd.x;
	pt[1].y = ptEnd.y;
	
	
	hPen = CreatePen(PS_SOLID, 0, RGB(255,0,0));
	
	hdc = GetDC (hwnd) ;
	SelectObject (hdc, hPen) ;
	
	SetROP2 (hdc, R2_NOT) ;
	
	Polyline (hdc, pt, 2) ;
	DeleteObject(SelectObject(hdc, (HPEN) BLACK_PEN));
	ReleaseDC (hwnd, hdc) ;
}
/***************************************************************************/
/* GetZoomFont                                                             */
/* Arguments :                                                             */
/*   hdc: handle to device context.                                        */
/*   lpMapDocument: pointer to map document.                               */
/* Purpose : Creates a font of the selected size, return the handle to the */
/* previous font.                                                          */
/***************************************************************************/
HFONT GetZoomFont(HDC hdc, LPMAPDOCUMENT lpMapDocument)
{
	HFONT blockFont;
	
	
	blockFont = CreateFont((int)lpMapDocument->view_zoom, (int)(lpMapDocument->view_zoom*.3), 0, 0, 600, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, NULL);
	
	
    SetTextColor(hdc, RGB(255,255,255));
	SetBkMode(hdc, TRANSPARENT);
	return((HFONT) SelectObject(hdc, blockFont));	
}
/***************************************************************************/
/* DeleteZoomFont                                                          */
/* Arguments :                                                             */
/*   hdc: handle to device context.                                        */
/*   gotoFont: handle to font.                                             */
/* Purpose : Deletes a font of returning to the selected font. This fuction*/
/* is here to simplify code.                                               */
/***************************************************************************/
void DeleteZoomFont(HDC hdc, HFONT gotoFont)
{
	SetTextColor(hdc, RGB(0,0,0));
	SetBkMode(hdc, OPAQUE);
	DeleteObject(SelectObject(hdc, gotoFont));
}
