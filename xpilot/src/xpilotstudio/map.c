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
/*Functions for working with maps, such as creating deleting. We should
only use ANSI 'c' code here, so that its compatible on any OS.*/
#include "main.h"
#include "default_settings.h"

int iSelectionMapSyms = 0;
int iSelectionMapTools = 0;
int iSelectionShape = 0;
int iSelectionWormhole = 0;
int iSelectionPolarity = 0;
int iSelectionWallType = 0;
int iSelectionMapModify = 0;

int fDrawing = 0;
int fCreatingPolygon;
int fDragging = 0;

int teamSet = 0;
int dirSet = 32;
int polaritySet = 0;
int variantSet = 0;

int SaveAllPrefs;

/***************************************************************************/
/* NewMap                                                                  */
/* Arguments :                                                             */
/*   lpMapDocument: pointer to document data                                */
/* Purpose :                                                               */
/***************************************************************************/
int NewMapInit(LPMAPDOCUMENT lpMapDocument)
{
	int                   i;//,j;
	
	if (lpMapDocument->MapStruct.comments)
		free(lpMapDocument->MapStruct.comments);
	
	lpMapDocument->MapStruct.comments = (char *) NULL;
    lpMapDocument->MapStruct.mapName[0] = lpMapDocument->MapStruct.mapFileName[0] = (char ) NULL;
	lpMapDocument->width = DEFAULT_WIDTH;
	sprintf(lpMapDocument->MapStruct.mapWidth,"%d",lpMapDocument->width);
	lpMapDocument->height = DEFAULT_HEIGHT;
	sprintf(lpMapDocument->MapStruct.mapHeight,"%d",lpMapDocument->height);
	lpMapDocument->view_zoom = DEFAULT_MAP_ZOOM;
    lpMapDocument->changed = 0;

	for (i = 4; i < NUMPREFS; i++)
		lpMapDocument->PrefsArray[i].output = FALSE;

	Setup_default_server_options(lpMapDocument);
	return 0;
}
/***************************************************************************/
/* Setup_default_server_options                                            */
/* Arguments :                                                             */
/*    lpMapDocument - pointer to map document.                             */
/* Purpose :   Begin filling default options                               */
/***************************************************************************/
void Setup_default_server_options(LPMAPDOCUMENT lpMapDocument)
{
	int i;
	
	for(i=0;i<num_default_settings;i++)
		AddOption(lpMapDocument, default_settings[i].name,
		default_settings[i].value, FALSE, TRUE);
	
	return;
}
/***************************************************************************/
/* DoModifyCommand                                                         */
/* Arguments :                                                             */
/*    lpMapDocument - pointer to map document.                             */
/*    x - xcoord                                                          */
/*    y - ycoord                                                          */
/*    iSelectionMapModify - the selected modify command                    */
/* Purpose :   Do the selected modify command                              */
/***************************************************************************/
int DoModifyCommand(LPMAPDOCUMENT lpMapDocument, int x, int y, int iSelectionMapModify)
{
	XP_POINT *vert = NULL;
	static int startx, starty, deltax, deltay;
	int i;

	switch (iSelectionMapModify)
	{
	case IDM_PICKITEM:
		SelectItem(lpMapDocument, x, y);
		break;
	case IDM_ADDVERTEX:
		if (lpMapDocument->selectedpoly)
		{
			vert = (XP_POINT *) malloc(sizeof(XP_POINT));
			vert->x = x;
			vert->y = y;
			vert->delta_x = min(x - lpMapDocument->selectedpoly->vertex[lpMapDocument->selectedpoly->num_verts-1].x,
				lpMapDocument->width - lpMapDocument->selectedpoly->vertex[lpMapDocument->selectedpoly->num_verts-1].x + x);
			vert->delta_y = y - lpMapDocument->selectedpoly->vertex[lpMapDocument->selectedpoly->num_verts-1].y;
			vert->variant = variantSet;
			AddVertexToList(lpMapDocument->selectedpoly, vert);
		}
		else
			SelectItem(lpMapDocument, x, y);		
		break;
	case IDM_DELVERTEX:
		if (lpMapDocument->selectedpoly && lpMapDocument->selectedpoly->num_verts >= 2)
		{
			DeleteVertex(&lpMapDocument->selectedpoly, lpMapDocument->numselvert);
		}
		else if (lpMapDocument->selectedpoly)
		{
			DeleteMapItem(lpMapDocument);
		}
		else
			SelectItem(lpMapDocument, x, y);		
		break;
	case IDM_MOVEITEM:
		if (lpMapDocument->selectedpoly || lpMapDocument->selecteditem)
		{
			if (!fDragging)
			{
				startx = x;
				starty = y;
				fDragging = TRUE;
			}
			else
			{
				deltax = x - startx;
				deltay = y - starty;
				if (lpMapDocument->selectedpoly)
				{
					lpMapDocument->selectedpoly->vertex[0].x = WRAP_XPIXEL(lpMapDocument->selectedpoly->vertex[0].x+deltax);
					lpMapDocument->selectedpoly->vertex[0].y = WRAP_YPIXEL(lpMapDocument->selectedpoly->vertex[0].y+deltay);
					for (i = 1; i<lpMapDocument->selectedpoly->num_verts; i++)
					{
						lpMapDocument->selectedpoly->vertex[i].x = lpMapDocument->selectedpoly->vertex[i-1].x + lpMapDocument->selectedpoly->vertex[i].delta_x;
						lpMapDocument->selectedpoly->vertex[i].y = lpMapDocument->selectedpoly->vertex[i-1].y + lpMapDocument->selectedpoly->vertex[i].delta_y;
					}
				}
				else
				{
					lpMapDocument->selecteditem->pos.x = WRAP_XPIXEL(lpMapDocument->selecteditem->pos.x + deltax);
					lpMapDocument->selecteditem->pos.y = WRAP_YPIXEL(lpMapDocument->selecteditem->pos.y + deltay);
				}
				fDragging = FALSE;
			}
		}
		else
			SelectItem(lpMapDocument, x, y);		
		break;
	}
	return 0;
}
/***************************************************************************/
/* UpdateSelected                                                          */
/* Arguments :                                                             */
/*    lpMapDocument - pointer to map document.                             */
/* Purpose :   Change the selected items settings                          */
/***************************************************************************/
void UpdateSelected(LPMAPDOCUMENT lpMapDocument)
{
	if (lpMapDocument->selectedbool)
	{
		if (lpMapDocument->selecteditem)
		{
			lpMapDocument->selecteditem->direction = dirSet;
			lpMapDocument->selecteditem->team = teamSet;
			lpMapDocument->selecteditem->variant = variantSet;
		}
		else if (lpMapDocument->selectedpoly)
		{
			lpMapDocument->selectedpoly->team = teamSet;
		}
	}
}
