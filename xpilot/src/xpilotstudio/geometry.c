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

#include "main.h"

/***************************************************************************/
/* CreateItem                                                              */
/* Arguments :                                                             */
/*   lpMapDocument: pointer to document data                               */
/*   dx                                                                    */
/*   dy                                                                    */
/*   iSelectionMapSyms                                                     */
/*   closing                                                               */
/* Return :                                                                */
/*   TRUE if errors                                                        */
/*   FALSE if successful                                                   */
/* Purpose :                                                               */
/***************************************************************************/
int CreateItem(LPMAPDOCUMENT lpMapDocument, int x, int y, int dx, int dy,
			   int iSelectionMapSyms, int closing)
{	
	int num;
	int ispoly = 0;
	struct addablepolygons {
		struct polygonlist **pglp;
	};
	struct addablepolygons addpolylist[4] =
	{
		&lpMapDocument->MapGeometry.walls,
		&lpMapDocument->MapGeometry.decors,
		&lpMapDocument->MapGeometry.balltargets,
		&lpMapDocument->MapGeometry.ballareas,
	};
	struct addableitems {
		struct itemlist **itmlp;
		int type;
	};
	struct addableitems additemlist[11] =
	{
		{&lpMapDocument->MapGeometry.targets, IDM_MAP_TARGET},
		{&lpMapDocument->MapGeometry.fuels, IDM_MAP_FUEL},
		{&lpMapDocument->MapGeometry.cannons, IDM_MAP_CANNON},
		{&lpMapDocument->MapGeometry.balls, IDM_MAP_BALL},
		{&lpMapDocument->MapGeometry.bases, IDM_MAP_BASE},
		{&lpMapDocument->MapGeometry.currents, IDM_MAP_CURRENT},
		{&lpMapDocument->MapGeometry.itemconcentrators, IDM_MAP_ITEM_CONC},
		{&lpMapDocument->MapGeometry.gravities, IDM_MAP_GRAVITY},
		{&lpMapDocument->MapGeometry.circulargravities, IDM_MAP_CIRCULAR_GRAVITY},
		{&lpMapDocument->MapGeometry.wormholes, IDM_MAP_WORMHOLE},
		{&lpMapDocument->MapGeometry.checkpoints, IDM_MAP_CHECKPOINT}
	};

	GetSetEnterableInfo();
	switch(iSelectionMapSyms)
	{
	case IDM_MAP_WALL:
		num = 0;
		ispoly = TRUE;
		break;
	case IDM_MAP_DECOR:
		num = 1;
		ispoly = TRUE;
		break;
	case IDM_MAP_BALLTARGET:
		num = 2;
		ispoly = TRUE;
		break;
	case IDM_MAP_BALLAREA:
		num = 3;
		ispoly = TRUE;
		break;
	case IDM_MAP_TARGET:
		fDrawing = FALSE;
		num = 0;
		break;
	case IDM_MAP_FUEL:
		fDrawing = FALSE;
		num = 1;
		break;
	case IDM_MAP_CANNON:
		fDrawing = FALSE;
		num = 2;	
		break;
	case IDM_MAP_BALL:
		fDrawing = FALSE;
		num = 3;
		break;
	case IDM_MAP_BASE:
		fDrawing = FALSE;
		num = 4;
		break;
	case IDM_MAP_CURRENT:
		fDrawing = FALSE;
		num = 5;
		break;
	case IDM_MAP_ITEM_CONC:
		fDrawing = FALSE;
		num = 6;
		break;
	case IDM_MAP_GRAVITY:
		fDrawing = FALSE;
		num = 7;
		break;
	case IDM_MAP_CIRCULAR_GRAVITY:
		fDrawing = FALSE;
		num = 8;
		break;
	case IDM_MAP_WORMHOLE:
		fDrawing = FALSE;
		num = 9;
		break;
	case IDM_MAP_CHECKPOINT:
		fDrawing = FALSE;
		lpMapDocument->MapGeometry.num_checkpoints++;
		variantSet = lpMapDocument->MapGeometry.num_checkpoints;
		num = 10;
		break;	//todo: ADD CASES FOR OTHER DRAWABLE ENTITIES...
	}

	if (ispoly)
	{
		/*ToDo Add information for processing of polygon styles*/
			return (AddItemToPolygonlist(lpMapDocument, addpolylist[num].pglp,
			x, y, dx, dy, 0, teamSet, 0, closing));
	}
	else
	{
			return (AddItemToItemlist(additemlist[num].itmlp,
			x, y, teamSet, dirSet,
			variantSet, additemlist[num].type));
	}
}
/***************************************************************************/
/* AddItemToItemlist                                                       */
/* Arguments :                                                             */
/*   *varitmlp: an itemlist                                                */
/*   x                                                                     */
/*   y                                                                     */
/*   team                                                                  */
/*   direction                                                             */
/*   variant                                                               */
/*   type                                                                  */
/* Return :                                                                */
/*   TRUE if errors                                                        */
/*   FALSE if successful                                                   */
/* Purpose : Create and add an item to the itemlist                        */
/***************************************************************************/
int AddItemToItemlist(itemlist **varitmlp, int x, int y,
							int team, int direction,
							int variant, int type)
{
	itemlist *item = NULL;
	itemlist *itmlp = NULL;

	if (*varitmlp == NULL)
	{
		item = (itemlist *) malloc(sizeof(itemlist));
		item->next = NULL;
		*varitmlp = item;
	}
	else
	{
		item = (itemlist *) malloc(sizeof(itemlist));
		item->next = NULL;
		itmlp = *varitmlp;
		while(itmlp->next != NULL)
			itmlp = itmlp->next;
		itmlp->next = item;
	}
	item->pos.x = x;
	item->pos.y = y;
	item->team = team;
	item->selected = FALSE;

	/*Enter the bounding box information for items. Using generic block sizes for now,
	but at some point this should be better sized to the object.*/
	item->bounding_box = (XP_POINT *) malloc(sizeof(XP_POINT)*4);
	item->bounding_box[0].x= item->pos.x-17;
	item->bounding_box[0].y= item->pos.y+17;
	item->bounding_box[1].delta_x = 35;
	item->bounding_box[1].delta_y= 0;
	item->bounding_box[2].delta_x = 0;
	item->bounding_box[2].delta_y= -35;
	item->bounding_box[3].delta_x = -35;
	item->bounding_box[3].delta_y= 0;

	
	/*Set the angle for rotatable blocks*/
	switch (type)
	{
	case IDM_MAP_CANNON:
	case IDM_MAP_BASE:
	case IDM_MAP_CURRENT:
		item->direction = direction;
		break;
	default:
		item->direction = 0;
		break;
	}
	/*Set the variant type*/
	switch (type)
	{
	case IDM_MAP_WORMHOLE:
	case IDM_MAP_CIRCULAR_GRAVITY:
	case IDM_MAP_GRAVITY:
		item->variant = variant;
		break;
	default:
		item->variant = 0;
		break;
	}
	return FALSE;
}
/***************************************************************************/
/* AddItemToPolygonlist                                                    */
/* Arguments :                                                             */
/*   *varpglp: a polygon list                                              */
/*   x                                                                     */
/*   y                                                                     */
/*   team                                                                  */
/*   coordtype                                                             */
/*		TRUE if coordinates are specified as offsets, as loaded from a file*/
/*		FALSE if coordinates are x,y, as drawn by user                     */
/*   closing                                                               */
/* Return :                                                                */
/*   TRUE if errors                                                        */
/*   FALSE if successful                                                   */
/* Purpose : Create and add a vertex to the polygon list                   */
/***************************************************************************/
int AddItemToPolygonlist(LPMAPDOCUMENT lpMapDocument, polygonlist **varpglp, int x, int y,
								  int dx, int dy, int style, int team,
								  int coordtype, int closing)
{
	polygonlist *polygon = NULL;
	polygonlist *plgnlp = NULL;
	XP_POINT *vert = NULL;
	static XP_POINT prevpoint;
	static XP_POINT startpoint;
	int checkDir = FALSE;
	
	vert = (XP_POINT *) malloc(sizeof(XP_POINT));
	
	if (!coordtype && !fCreatingPolygon && !closing)
	{
		prevpoint.x = 0;
		prevpoint.y = 0;
		if (*varpglp == NULL)
		{
			polygon = (polygonlist *) malloc(sizeof(polygonlist));
			polygon->vertex = NULL;
			polygon->next = NULL;
			*varpglp = polygon;
		}
		else
		{
			polygon = (polygonlist *) malloc(sizeof(polygonlist));
			polygon->vertex = NULL;
			polygon->next = NULL;
			plgnlp = *varpglp;
			while (plgnlp->next != NULL)
				plgnlp = plgnlp->next;
			plgnlp->next = polygon;
		}
		vert->x = x;
		vert->y = y;
		vert->delta_x = 1; //Delta must be above 0 to create vertex...even if this is
		vert->delta_y = 1; //The first point and a delta value has no meaning.

		startpoint.delta_x = 0;
		startpoint.delta_y = 0;

		startpoint.x = prevpoint.x = x;
		startpoint.y = prevpoint.y = y;
		
		polygon->polygon_style = style;
		vert->edge_style = lpMapDocument->MapGeometry.pstyles[style].defedge_id;
	}
	else if (!coordtype)
	{
		if (closing && lpMapDocument->selectedpoly)
			polygon = lpMapDocument->selectedpoly;
		else
		{			
			polygon = *varpglp;
			while (polygon->next != NULL)
				polygon = polygon->next;
		}

		if (closing)
		{
			vert->delta_x = polygon->vertex[0].x - polygon->vertex[polygon->num_verts-1].x;
			vert->delta_y = polygon->vertex[0].y - polygon->vertex[polygon->num_verts-1].y;
			vert->x = polygon->vertex[0].x;
			vert->y = polygon->vertex[0].y;
			fCreatingPolygon = FALSE;
			checkDir = TRUE;
		}
		else
		{		
			vert->delta_x = dx;
			vert->delta_y = dy;
			vert->x = x;
			vert->y = y;
			
			prevpoint.x = x;
			prevpoint.y = y;
		}
		vert->edge_style = style;
	}
	else
	{
		polygon = *varpglp;
		while (polygon->next != NULL)
			polygon = polygon->next;
		vert->delta_x = x;
		vert->delta_y = y;
		prevpoint.x = vert->x = prevpoint.x + x;
		prevpoint.y = vert->y = prevpoint.y + y;
		vert->edge_style = style;
	}

	//If the new point is too far away, or we didn't move at all,
	//Then we have nothing valuable to contribute so RETURN.
	if ((vert->delta_x == 0 && vert->delta_y == 0) || (hypot(vert->delta_x, vert->delta_y) > MAX_LINE_LEN))
	{
		if (closing)
		{
			ErrorHandler("Polygon Must Be Closable!");
			fCreatingPolygon = TRUE;
		}
		free(vert);
		return TRUE;
	}

	AddVertexToList(polygon, vert);
	polygon->team = team;

	if (checkDir)
	{
		if (!IsCounterClockwise(polygon))
			ReversePolygonOrientation(polygon);
	}

	return FALSE;
}
/***************************************************************************/
/* SelectItem                                                              */
/* Arguments :                                                             */
/*    lpMapDocument - pointer to map document.                             */
/*    x - xcoord                                                           */
/*    y - ycoord                                                           */
/*    iSelectionMapModify - the selected modify command                    */
/* Return :                                                                */
/*   FALSE always successful                                               */
/* Purpose :   Select whatever is at the current coords                    */
/* Revised 3/23/01: This function has to do some farely complex selection, */
/* as we wish to be able to select vertexes, edges, entire polygons, items.*/
/* I'm sure my code here is going to suck. Just a warning. ;-)             */
/***************************************************************************/
int SelectItem(LPMAPDOCUMENT lpMapDocument, int x, int y)
{
	polygonlist *pglp = NULL;
	itemlist *itmlp = NULL;
	XP_POINT *tempvlist = NULL;
	int i, vtx;
	int numchecks = 15;
	struct checkable {
		struct polygonlist *pglp;
		struct itemlist *itmlp;
		int type;
	};

	struct checkable checklist[15] =
	{
		{NULL, lpMapDocument->MapGeometry.balls, IDM_MAP_BALL},
		{NULL, lpMapDocument->MapGeometry.fuels, IDM_MAP_FUEL},
		{lpMapDocument->MapGeometry.walls, NULL, IDM_MAP_WALL},
		{lpMapDocument->MapGeometry.balltargets, NULL, IDM_MAP_BALLTARGET},
		{lpMapDocument->MapGeometry.ballareas, NULL, IDM_MAP_BALLAREA},
		{lpMapDocument->MapGeometry.decors, NULL, IDM_MAP_DECOR},
		{NULL, lpMapDocument->MapGeometry.bases, IDM_MAP_BASE},
		{NULL, lpMapDocument->MapGeometry.checkpoints, IDM_MAP_CHECKPOINT},
		{NULL, lpMapDocument->MapGeometry.targets, IDM_MAP_TARGET},
		{NULL, lpMapDocument->MapGeometry.cannons, IDM_MAP_CANNON},
		{NULL, lpMapDocument->MapGeometry.currents, IDM_MAP_CURRENT},
		{NULL, lpMapDocument->MapGeometry.gravities, IDM_MAP_GRAVITY},
		{NULL, lpMapDocument->MapGeometry.circulargravities, IDM_MAP_CIRCULAR_GRAVITY},
		{NULL, lpMapDocument->MapGeometry.itemconcentrators, IDM_MAP_ITEM_CONC},
		{NULL, lpMapDocument->MapGeometry.wormholes, IDM_MAP_WORMHOLE},
	};

	UpdateSelections(lpMapDocument, TRUE);
	//lpMapDocument->selectedbool = FALSE;
	//lpMapDocument->selectedpoly = NULL;
	//lpMapDocument->selecteditem = NULL;
	//lpMapDocument->selectedvert = FALSE;
	for (i = 0; i < numchecks; i++)
	{
		//Check the polygon list specified to see if we selected one of the
		//polygons in that list.
		pglp = checklist[i].pglp;		
		while (pglp != NULL)
		{
			if (!lpMapDocument->selectedbool)
			{
				//First check to see if this is a polygon vertex.
				vtx = IsVertexOfPolygon(pglp->vertex, pglp->num_verts, x, y);
				if (vtx != -1)
				{
					lpMapDocument->selectedbool = TRUE;
					lpMapDocument->numselvert = vtx;
					pglp->vertex[vtx].selected = TRUE;
					pglp->selected = TRUE;
					lpMapDocument->selectedpoly = pglp;
					lpMapDocument->selectedtype = IDM_VERTEX;
					lpMapDocument->selectedvert = &pglp->vertex[vtx];
					//ErrorHandler("x: %d, y: %d", lpMapDocument->selectedvert->x, lpMapDocument->selectedvert->y);
					return FALSE;
				}

				//If this isn't a vertex then we should see if it's
				//inside a polygon.
				pglp->selected = InsidePolygon(pglp->vertex,pglp->num_verts,x, y);
				//If the starting point of the polygon is wrapped around the other
				//side, then the above check won't work. So check to see if it was wrapped.

				if (!lpMapDocument->selectedbool && !pglp->selected)
					pglp->selected = InsidePolygon(pglp->vertex,pglp->num_verts, x-lpMapDocument->width, y+lpMapDocument->height);
				if (!lpMapDocument->selectedbool && !pglp->selected)
					pglp->selected = InsidePolygon(pglp->vertex,pglp->num_verts, x, y+lpMapDocument->height);
				if (!lpMapDocument->selectedbool && !pglp->selected)
					pglp->selected = InsidePolygon(pglp->vertex,pglp->num_verts, x+lpMapDocument->width, y+lpMapDocument->height);

				if (!lpMapDocument->selectedbool && !pglp->selected)
					pglp->selected = InsidePolygon(pglp->vertex,pglp->num_verts, x-lpMapDocument->width, y);
				if (!lpMapDocument->selectedbool && !pglp->selected)
					pglp->selected = InsidePolygon(pglp->vertex,pglp->num_verts, x+lpMapDocument->width, y);

				if (!lpMapDocument->selectedbool && !pglp->selected)
					pglp->selected = InsidePolygon(pglp->vertex,pglp->num_verts, x-lpMapDocument->width, y-lpMapDocument->height);
				if (!lpMapDocument->selectedbool && !pglp->selected)
					pglp->selected = InsidePolygon(pglp->vertex,pglp->num_verts, x, y-lpMapDocument->height);
				if (!lpMapDocument->selectedbool && !pglp->selected)
					pglp->selected = InsidePolygon(pglp->vertex,pglp->num_verts, x+lpMapDocument->width, y-lpMapDocument->height);

			}
			else
				pglp->selected = FALSE;


			if (pglp->selected)
			{
				lpMapDocument->selectedbool = TRUE;
				lpMapDocument->selectedpoly = pglp;
				lpMapDocument->selectedtype = checklist[i].type;
				teamSet = pglp->team;
			}

			pglp = pglp->next;
		}


		//Now check the itemlist specified.
		itmlp =  checklist[i].itmlp; 
		while (itmlp != NULL)
		{
			if (!lpMapDocument->selectedbool)
			{
				//Check within the specified bounding box.//
				//TO DO. Make bounding box variable num of points instead of req'd 4.
				//This would require each item having a polygon instead of a vertex list attribute.

				itmlp->selected = InsidePolygon(itmlp->bounding_box,4,x, y);
				//If the starting point of the polygon is wrapped around the other
				//side, then the above check won't work. So check to see if it was wrapped.

				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(itmlp->bounding_box,4, x-lpMapDocument->width, y+lpMapDocument->height);
				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(itmlp->bounding_box,4, x, y+lpMapDocument->height);
				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(itmlp->bounding_box,4, x+lpMapDocument->width, y+lpMapDocument->height);

				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(itmlp->bounding_box,4, x-lpMapDocument->width, y);
				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(itmlp->bounding_box,4, x+lpMapDocument->width, y);

				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(itmlp->bounding_box,4, x-lpMapDocument->width, y-lpMapDocument->height);
				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(itmlp->bounding_box,4, x, y-lpMapDocument->height);
				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(itmlp->bounding_box,4, x+lpMapDocument->width, y-lpMapDocument->height);

			}
			else
				itmlp->selected = FALSE;


			if (itmlp->selected)
			{
				lpMapDocument->selectedbool = TRUE;
				lpMapDocument->selecteditem = itmlp;
				lpMapDocument->selectedtype = checklist[i].type;
				teamSet = itmlp->team;
				dirSet = itmlp->direction;
				variantSet = itmlp->variant;
			}
			itmlp = itmlp->next;
		}

	}
	return FALSE;
}
/***************************************************************************/
/* DeleteMapItem		                                                   */
/* Arguments :                                                             */
/*    lpMapDocument - pointer to map document.                             */
/* Purpose : Delete the selected polygon.                                  */
/***************************************************************************/
void DeleteMapItem(LPMAPDOCUMENT lpMapDocument)
{
	int num;
	polygonlist *pglp, *tpglp;
	itemlist *itmlp;

	struct deleteable {
		struct polygonlist **pglp;
		struct itemlist **itmlp;
	};
	struct deleteable deletelist[15] =
	{
		{&lpMapDocument->MapGeometry.walls, NULL},
		{&lpMapDocument->MapGeometry.decors, NULL},
		{&lpMapDocument->MapGeometry.balltargets, NULL},
		{&lpMapDocument->MapGeometry.ballareas, NULL},
		{NULL, &lpMapDocument->MapGeometry.balls},
		{NULL, &lpMapDocument->MapGeometry.bases},
		{NULL, &lpMapDocument->MapGeometry.cannons},
		{NULL, &lpMapDocument->MapGeometry.checkpoints},
		{NULL, &lpMapDocument->MapGeometry.circulargravities},
		{NULL, &lpMapDocument->MapGeometry.currents},
		{NULL, &lpMapDocument->MapGeometry.fuels},
		{NULL, &lpMapDocument->MapGeometry.gravities},
		{NULL, &lpMapDocument->MapGeometry.itemconcentrators},
		{NULL, &lpMapDocument->MapGeometry.targets},
		{NULL, &lpMapDocument->MapGeometry.wormholes}
	};

	switch (lpMapDocument->selectedtype)
	{
	case IDM_MAP_WALL:
		num = 0;
		break;
	case IDM_MAP_DECOR:
		num = 1;
		break;
	case IDM_MAP_BALLTARGET:
		num = 2;
		break;
	case IDM_MAP_BALLAREA:
		num = 3;
		break;
	case IDM_MAP_BALL:
		num = 4;
		break;
	case IDM_MAP_BASE:
		num = 5;
		break;
	case IDM_MAP_CANNON:
		num = 6;
		break;
	case IDM_MAP_CHECKPOINT:
		num = 7;
		break;
	case IDM_MAP_CIRCULAR_GRAVITY:
		num = 8;
		break;
	case IDM_MAP_CURRENT:
		num = 9;
		break;
	case IDM_MAP_FUEL:
		num = 10;
		break;
	case IDM_MAP_GRAVITY:
		num = 11;
		break;
	case IDM_MAP_ITEM_CONC:
		num = 12;
		break;
	case IDM_MAP_TARGET:
		num = 13;
		break;
	case IDM_MAP_WORMHOLE:
		num = 14;
		break;
	case IDM_VERTEX:
		//If we have a vertex selected, we can delete that right
		//here and then RETURN.
		DeleteVertex(&lpMapDocument->selectedpoly, lpMapDocument->numselvert);
		UpdateSelections(lpMapDocument, TRUE);
		return;
	}
	lpMapDocument->selectedbool = FALSE;

	if (lpMapDocument->selectedpoly)
	{
		pglp = *deletelist[num].pglp;
		if (pglp == lpMapDocument->selectedpoly)
		{
			if(pglp->next)
			{
				*deletelist[num].pglp = pglp->next;
			}
			else
			{
				free (*deletelist[num].pglp);
				*deletelist[num].pglp = NULL;
			}
			lpMapDocument->selectedpoly = NULL;
			return;
		}
		else
		{
			while (pglp->next != NULL && pglp->next != lpMapDocument->selectedpoly)
			{
				pglp = pglp->next;
			}
			if (pglp->next->next)
			{
				tpglp = pglp->next->next;
				free(pglp->next->vertex);
				free(pglp->next);
				pglp->next = tpglp;
			}
			else
			{
				free(pglp->next->vertex);
				free(pglp->next);
				pglp->next = NULL;
			}
		}
		lpMapDocument->selectedpoly = NULL;
	}
	else if (lpMapDocument->selecteditem)
	{
		switch (lpMapDocument->selectedtype)
		{
		case IDM_MAP_CHECKPOINT:
			lpMapDocument->MapGeometry.num_checkpoints--;
			break;
		}

		itmlp = *deletelist[num].itmlp;
		if (itmlp == lpMapDocument->selecteditem)
		{
			if(itmlp->next)
			{
				*deletelist[num].itmlp = itmlp->next;
			}
			else
			{
				free (*deletelist[num].itmlp);
				*deletelist[num].itmlp = NULL;
			}
			lpMapDocument->selecteditem = NULL;
			return;
		}
		else
		{
			while (itmlp->next != NULL && itmlp->next != lpMapDocument->selecteditem)
			{
				itmlp = itmlp->next;
			}
			if (itmlp->next->next)
				itmlp->next = itmlp->next->next;
			else
				itmlp->next = NULL;
		}
		free (lpMapDocument->selecteditem);
		lpMapDocument->selecteditem = NULL;
	}
}
/***************************************************************************/
/* Wrap_length	                                                           */
/* Arguments :                                                             */
/*    lpMapDocument - pointer to map document.                             */
/*		dx: x coord to start at.                                           */
/*		dy: y coord to start at.                                           */
/* Return :                                                                */
/*   the wrapped length                                                    */
/* Purpose : find the shortest distance to a point.                        */
/***************************************************************************/
double Wrap_length(LPMAPDOCUMENT lpMapDocument, double dx, double dy)
{
	dx = WRAP_DX(dx);
	dy = WRAP_DY(dy);
	return LENGTH(dx, dy);
}
/***************************************************************************/
/* ReorderItemTo	     	                                               */
/* Arguments :                                                             */
/*	iteml: a list of items                                                 */
/*	item: an item                                                          */
/*  num: the new number for the checkpoint.                                */
/* Purpose : Reorder items in tan item list to be in a different order     */
/***************************************************************************/
/*void ReorderItemTo(itemlist **iteml, itemlist **item, int num)
{
	itemlist *itmlp = NULL;
	itemlist *item2 = *item;
	itemlist *tmp = NULL;
	int i, count;

	itmlp = *iteml;

 	count=0;
	while (itmlp->next != NULL)
	{
		count++;
		itmlp = itmlp->next;
	}

	itmlp = *iteml;
	i = 0;
	//We're not reordering or replacing the first or last points right?
	if ((num != 0) && (num != count) && (*iteml != *item))
	{
		tmp = item2->next;
		while (itmlp->next != NULL)
		{
			if (itmlp->next == item2)
				itmlp->next = tmp;
			else
			{
			if (i == num-1)
				{
					item2->next = itmlp->next;
					itmlp->next = item2;
				}
			}
			i++;
			itmlp = itmlp->next;
		}
	}
	//Okay, we are reordering or replacing, which requires some special shuffling
	else
	{
		//We're replacing the first item
		if (num == 0)
		{
			tmp = item2->next;
			//We're swapping the first and second items
			if (itmlp->next == item2)
			{
				item2->next = *iteml;
				*iteml = item2;
				while (itmlp->next != NULL)
				{
					if (itmlp->next == item2)
						itmlp->next = tmp;
					i++;
					itmlp = itmlp->next;
				}

			}
			//We're moving a later item into the first's spot and shifting
			//everything else down.
			else
			{				
				while (itmlp->next != NULL)
				{
					if (itmlp->next == item2)
						itmlp->next = tmp;
					else
					{
						if (i == 0)
						{
							item2->next = *iteml;
							*iteml = item2;
						}
					}
					i++;
					itmlp = itmlp->next;
				}
			}
		}
		//We're moving the first item to a later position
		else
		{
			if (*item == itmlp->next)
				return;
			*iteml = itmlp->next;
			while (itmlp->next != NULL)
			{
				if (i == num-1)
				{
					item2->next = itmlp->next;
					itmlp->next = item2;
				}
				i++;
				itmlp = itmlp->next;
			}
		}
	}

}*/
/***************************************************************************/
/* FindClosestItemInList                                                   */
/* Arguments :                                                             */
/*    lpMapDocument - pointer to map document.                             */
/*	x: the xcoord to check                                                 */
/*	y: the ycoord to check                                                 */
/*	item: a list of items                                                  */
/* Return :                                                                */
/*   number of closest vertex if one is close enough                       */
/*   0 if no vertex is close enough                                        */
/* Purpose : Find the closest item in an item list                         */
/***************************************************************************/
/*int FindClosestItemInList(LPMAPDOCUMENT lpMapDocument, int x, int y, itemlist **item)
{
	itemlist *itmlp;
	int n = 0;
	int i, l, delta, closest;

	delta = lpMapDocument->width;
	itmlp = *item;

	i = 0;
	while (itmlp != NULL)
	{
		l = Wrap_length(lpMapDocument, (x - itmlp->pos.x),
			(y - itmlp->pos.y));
		if (l <= delta)
		{
			delta = l;
			closest = i;
		}
		i++;
		itmlp = itmlp->next;
	}
//	if (delta < 50)
		return closest; //Return this point, cause we're within range
//	else
//		return 0; //Just let the first point be selected.
}
*/
/***************************************************************************/
/* AddStyleToMap                                                           */
/* Arguments :                                                             */
/* Return :                                                                */
/*   TRUE if errors                                                        */
/*   FALSE if successful                                                   */
/* Purpose : Adds a style to the map                                       */
/***************************************************************************/
int AddStyleToMap(LPMAPDOCUMENT lpMapDocument, int type, char *idstr, int width, int color,
				  int style, char *edgestr, int flags, char *texturestr, char *filenamestr)
{
	int i;

	switch (type)
	{
		//Edge Style
	case 0: 
		i = lpMapDocument->MapGeometry.num_estyles; /*remember that estyles[0] is the first one!*/
		sprintf(lpMapDocument->MapGeometry.estyles[i].id, "%s", idstr);
		lpMapDocument->MapGeometry.estyles[i].width = width;
		lpMapDocument->MapGeometry.estyles[i].color = color;
		lpMapDocument->MapGeometry.estyles[i].style = style;
		lpMapDocument->MapGeometry.num_estyles++;
		break;
		//Polygon Style
	case 1: 
		i = lpMapDocument->MapGeometry.num_pstyles;/*remember that pstyles[0] is the first one!*/
		sprintf(lpMapDocument->MapGeometry.pstyles[i].id, "%s", idstr);
		lpMapDocument->MapGeometry.pstyles[i].color = color;
		lpMapDocument->MapGeometry.pstyles[i].defedge_id = get_edge_id(lpMapDocument, edgestr);
		if (texturestr)
			lpMapDocument->MapGeometry.pstyles[i].texture_id = get_bmp_id(lpMapDocument, texturestr);
		lpMapDocument->MapGeometry.pstyles[i].flags = flags;
		lpMapDocument->MapGeometry.num_pstyles++;
		break;
		//Bitmap Style
	case 2: 
		i = lpMapDocument->MapGeometry.num_bstyles;/*remember that bstyles[0] is the first one!*/
		sprintf(lpMapDocument->MapGeometry.bstyles[i].id, "%s", idstr);
		sprintf(lpMapDocument->MapGeometry.bstyles[i].filename, "%s", filenamestr);
		lpMapDocument->MapGeometry.bstyles[i].flags = flags;
		lpMapDocument->MapGeometry.num_bstyles++;
		break;
	}

	return FALSE;
}
/***************************************************************************/
/* InsideSelected                                                          */
/* Arguments :                                                             */
/*    lpMapDocument - pointer to map document.                             */
/*	x: the xcoord to check                                                 */
/*	y: the ycoord to check                                                 */
/* Return :                                                                */
/*   TRUE if outside                                                       */
/*   FALSE if inside                                                       */
/* Purpose : Findout if the point specified is within the selected item    */
/***************************************************************************/
int InsideSelected(LPMAPDOCUMENT lpMapDocument, int x, int y)
{
	int rval = 0, num = 0;
	XP_POINT *tempptlist;

	if (lpMapDocument->selectedbool)
		if (lpMapDocument->selectedpoly)
		{
			tempptlist = lpMapDocument->selectedpoly->vertex;
			num = lpMapDocument->selectedpoly->num_verts;
		}
		else if (lpMapDocument->selecteditem)
		{
			tempptlist = lpMapDocument->selecteditem->bounding_box;
			num = 4;
		}

		rval = InsidePolygon(tempptlist, num, x, y);
		//If the starting point of the polygon is wrapped around the other
		//side, then the above check won't work. So check to see if it was wrapped.
		
		if (!rval)
			rval = InsidePolygon(tempptlist, num, x-lpMapDocument->width, y+lpMapDocument->height);
		if (!rval)
			rval = InsidePolygon(tempptlist, num, x, y+lpMapDocument->height);
		if (!rval)
			rval = InsidePolygon(tempptlist, num, x+lpMapDocument->width, y+lpMapDocument->height);
		
		if (!rval)
			rval = InsidePolygon(tempptlist, num, x-lpMapDocument->width, y);
		if (!rval)
			rval = InsidePolygon(tempptlist, num, x+lpMapDocument->width, y);
		
		if (!rval)
			rval = InsidePolygon(tempptlist, num, x-lpMapDocument->width, y-lpMapDocument->height);
		if (!rval)
			rval = InsidePolygon(tempptlist, num, x, y-lpMapDocument->height);
		if (!rval)
			rval = InsidePolygon(tempptlist, num, x+lpMapDocument->width, y-lpMapDocument->height);

		return rval;		
}

