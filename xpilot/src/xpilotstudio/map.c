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
int iSelectionShape = 0;
int iSelectionWormhole = 0;
int iSelectionPolarity = 0;
int iSelectionMapModify = 0;

int fDrawing = 0;
int fCreatingPolygon;
int fDragging = 0;
int fReordering = 0;

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

	/*Default EdgeStyles*/
//	AddStyleToMap(lpMapDocument, 0, "xpbluehidden", -1, strtol("4E7CFF", NULL, 16),0, NULL, 0, NULL, NULL);
//	AddStyleToMap(lpMapDocument, 0, "xpredhidden", -1, strtol("FF3A27", NULL, 16), 0, NULL, 0, NULL, NULL);
//	AddStyleToMap(lpMapDocument, 0, "yellow", 2, strtol("FFFF00", NULL, 16), 0, NULL, 0, NULL, NULL);
	/*Default PolygonStyles*/
//	AddStyleToMap(lpMapDocument, 1, "xpblue", 0, strtol("4E7CFF", NULL, 16), 0, "xpbluehidden", 1, NULL, NULL);
//	AddStyleToMap(lpMapDocument, 1, "xpred", 0, strtol("FF3A27", NULL, 16), 0, "xpredhidden", 1, NULL, NULL);
//	AddStyleToMap(lpMapDocument, 1, "emptyyellow", 0, strtol("FF", NULL, 16), 0, "yellow", 0, NULL, NULL);
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
	static itemlist *itmlp = NULL;
	static int startx, starty, deltax, deltay;
	int i;

	switch (iSelectionMapModify)
	{
	case IDM_PICKITEM:
		SelectItem(lpMapDocument, x, y);
		break;
	case IDM_MOVEITEM:
		if (lpMapDocument->selectedpoly || lpMapDocument->selecteditem || lpMapDocument->selectedvert)
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
				if (lpMapDocument->selectedpoly && !lpMapDocument->selectedvert)
				{
					MovePolygon(lpMapDocument, lpMapDocument->selectedpoly->vertex,
						lpMapDocument->selectedpoly->num_verts, deltax, deltay);
				}
				else if (lpMapDocument->selecteditem)
				{
					lpMapDocument->selecteditem->pos.x = WRAP_XPIXEL(lpMapDocument->selecteditem->pos.x + deltax);
					lpMapDocument->selecteditem->pos.y = WRAP_YPIXEL(lpMapDocument->selecteditem->pos.y + deltay);
					MovePolygon(lpMapDocument, lpMapDocument->selecteditem->bounding_box,
						4, deltax, deltay);
				}
				else if (lpMapDocument->selectedvert)
				{
					if (MoveVertex(&lpMapDocument->selectedpoly, lpMapDocument->numselvert, deltax, deltay))
					{
						StatusUpdate("Couldn't move vertex!");
					}
				}
				fDragging = FALSE;
			}
		}
		else
			SelectItem(lpMapDocument, x, y);		
		break;
/*	case IDM_REORDERCHECKPOINT:
		if (lpMapDocument->selecteditem && lpMapDocument->selectedtype == IDM_MAP_CHECKPOINT)
		{
//			if (!fReordering)
//			{
//				itmlp = lpMapDocument->selecteditem;
//				fReordering = TRUE;
//			}
//			else
//			{
				i = FindClosestItemInList(lpMapDocument, x, y, &lpMapDocument->MapGeometry.checkpoints);
				ReorderItemTo(&lpMapDocument->MapGeometry.checkpoints, &lpMapDocument->selecteditem, i);
				lpMapDocument->selecteditem = NULL;
//				ErrorHandler("%d", i);
//				fReordering = FALSE;
//				itmlp = NULL;
//			}
		}
		else
			SelectItem(lpMapDocument, x, y);		
		break;*/
	}
	return 0;
}
/***************************************************************************/
/* UpdateSelections                                                        */
/* Arguments :                                                             */
/*    lpMapDocument - pointer to map document.                             */
/*    clear - should all selections be cleared?                            */
/* Purpose :   Change the selected items settings                          */
/***************************************************************************/
void UpdateSelections(LPMAPDOCUMENT lpMapDocument, int clear)
{
	if (lpMapDocument->selectedbool && !clear)
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
	if (clear)
	{
		lpMapDocument->selectedbool = FALSE;
		lpMapDocument->selecteditem = NULL;
		lpMapDocument->selectedpoly = NULL;
		lpMapDocument->selectedvert = NULL;
		lpMapDocument->numselvert = -1;
		lpMapDocument->selectedtype = 0;
	}
}
