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
		lpMapDocument->MapGeometry.num_balls++;
		break;
	case IDM_MAP_BASE:
		fDrawing = FALSE;
		num = 4;
		lpMapDocument->MapGeometry.num_bases++;
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
			return (AddItemToPolygonlist(lpMapDocument, addpolylist[num].pglp,
			x, y, dx, dy, teamSet, variantSet, 0, closing));
	}
	else
	{
			return (AddItemToItemlist(additemlist[num].itmlp,
			x, y, teamSet, dirSet,
			variantSet, additemlist[num].type));
	}
}
/***************************************************************************/
/* *AddItemToItemlist                                                      */
/* Arguments :                                                             */
/*   *varitmlp: an itemlist                                                */
/*   x                                                                    */
/*   y                                                                    */
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
//	case IDM_MAP_CHECKPOINT:
		item->variant = variant;
		break;
	default:
		item->variant = 0;
		break;
	}
	return FALSE;
}
/***************************************************************************/
/* *AddItemToPolygonlist                                                   */
/* Arguments :                                                             */
/*   *varpglp: a polygon list                                              */
/*   x                                                                     */
/*   y                                                                     */
/*   team                                                                  */
/*   variant                                                               */
/*		0: Normal Walls                                                    */
/*		1: Hidden Walls                                                    */
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
								  int dx, int dy, int team, int variant,
								  int coordtype, int closing)
{
	polygonlist *polygon = NULL;
	polygonlist *plgnlp = NULL;
	XP_POINT *vert = NULL;
	static XP_POINT prevpoint;
	static XP_POINT startpoint;
	
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
	}

	//If the new point is too far away, or we didn't move at all,
	//Then we have nothing valuable to contribute so return to CreateItem.
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
	vert->variant = variant;

	AddVertexToList(polygon, vert);
	polygon->team = team;

	return FALSE;
}
/***************************************************************************/
/* *AddVertexToList                                                        */
/* Arguments :                                                             */
/*   **polygon                                                             */
/*   *vert                                                                 */
/* Purpose : Add the vertext to the list.                                  */
/***************************************************************************/
int AddVertexToList(polygonlist *polygon, XP_POINT *vert)
{

	if (polygon->vertex == NULL)
	{
		polygon->vertex = vert;
		fCreatingPolygon = TRUE;
		polygon->num_verts = 1;
		polygon->selected = 0;
	}
	else
	{
		polygon->num_verts++;
		polygon->vertex = realloc(polygon->vertex, polygon->num_verts*sizeof(XP_POINT));
		polygon->vertex[polygon->num_verts-1].x = vert->x;
		polygon->vertex[polygon->num_verts-1].y = vert->y;
		polygon->vertex[polygon->num_verts-1].delta_x = vert->delta_x;
		polygon->vertex[polygon->num_verts-1].delta_y = vert->delta_y;
		polygon->vertex[polygon->num_verts-1].variant = vert->variant;
		free(vert);
	}
	return FALSE;
}

/***************************************************************************/
/* InsidePolygon		                                                   */
/* Arguments :                                                             */
/*	polygon: a list of points                                              */
/*	N: the number of vertexes                                              */
/*	x: the xcoord to check                                                 */
/*	y: the ycoord to check                                                 */
/* Return :                                                                */
/*   TRUE if outside                                                       */
/*   FALSE if inside                                                       */
/* Purpose : Determine whether the specified coordinates are in  a polygon */
/***************************************************************************/
int InsidePolygon(XP_POINT *plygn, int N, int x, int y)
{
	//This Function Creates an new point array and
	//calculates the x,y coordinates of each point from the
	//delta values specified in the original points.
	//The new x,y coords are NOT wrapped, as this would
	//Invalidate the polygon.
	//The new values are not stored, merely used for checking.
	int counter = 0;
	int i;
	double xinters;
	XP_POINT p, p1, p2;
	XP_POINT *polygon;

	p.x = x;
	p.y = y;

	polygon = (XP_POINT *) malloc(sizeof(XP_POINT) * N);

	polygon[0].x = plygn[0].x;
	polygon[0].y = plygn[0].y;
	for (i=1; i<N; i++)
	{
		polygon[i].x = polygon[i-1].x+plygn[i].delta_x;
		polygon[i].y = polygon[i-1].y+plygn[i].delta_y;
	}

	p1 = polygon[0];
	for (i=1;i<=N;i++) {
			p2 = polygon[i % N];
		if (p.y > min(p1.y, p2.y)) {
			if (p.y <= max(p1.y, p2.y)){
				if (p.x <= max(p1.x, p2.x)) {
					if (p1.y != p2.y) {
						xinters = (p.y-p1.y) * (p2.x-p1.x)/(p2.y-p1.y)+p1.x;
						if (p1.x == p2.x || p.x <= xinters)
							counter++;
					}
				}
			}
		}
		p1=p2;
	}

	free(polygon);

	if (counter % 2 == 0)
		return(FALSE); //Inside
	else
		return(TRUE); //Outside
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
/***************************************************************************/
int SelectItem(LPMAPDOCUMENT lpMapDocument, int x, int y)
{
	polygonlist *pglp = NULL;
	itemlist *itmlp = NULL;
	XP_POINT *tempvlist = NULL;
	int i;
	int numchecks = 15;
	struct checkable {
		struct polygonlist *pglp;
		struct itemlist *itmlp;
		int type;
	};

	struct checkable checklist[15] =
	{
		{NULL, lpMapDocument->MapGeometry.balls, IDM_MAP_BALL},
		{lpMapDocument->MapGeometry.walls, NULL, IDM_MAP_WALL},
		{lpMapDocument->MapGeometry.balltargets, NULL, IDM_MAP_BALLTARGET},
		{lpMapDocument->MapGeometry.ballareas, NULL, IDM_MAP_BALLAREA},
		{NULL, lpMapDocument->MapGeometry.bases, IDM_MAP_BASE},
		{NULL, lpMapDocument->MapGeometry.targets, IDM_MAP_TARGET},
		{NULL, lpMapDocument->MapGeometry.cannons, IDM_MAP_CANNON},
		{NULL, lpMapDocument->MapGeometry.checkpoints, IDM_MAP_CHECKPOINT},
		{NULL, lpMapDocument->MapGeometry.currents, IDM_MAP_CURRENT},
		{NULL, lpMapDocument->MapGeometry.gravities, IDM_MAP_GRAVITY},
		{NULL, lpMapDocument->MapGeometry.circulargravities, IDM_MAP_CIRCULAR_GRAVITY},
		{NULL, lpMapDocument->MapGeometry.fuels, IDM_MAP_FUEL},
		{NULL, lpMapDocument->MapGeometry.itemconcentrators, IDM_MAP_ITEM_CONC},
		{NULL, lpMapDocument->MapGeometry.wormholes, IDM_MAP_WORMHOLE},
		{lpMapDocument->MapGeometry.decors, NULL, IDM_MAP_DECOR},
	};

	lpMapDocument->selectedbool = FALSE;
	lpMapDocument->selectedpoly = NULL;
	lpMapDocument->selecteditem = NULL;
	for (i = 0; i < numchecks; i++)
	{
		pglp = checklist[i].pglp;		
		while (pglp != NULL)
		{
			if (!lpMapDocument->selectedbool)
			{
				pglp->selected = InsidePolygon(pglp->vertex,pglp->num_verts,x, y);
				/*If the starting point of the polygon is wrapped around the other
				side, then the above check won't work. So check to see if it was wrapped.*/

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
//			if (vlp->selected && fItemSelected)
//				selectedpoly = vlp;

			pglp = pglp->next;
		}
		itmlp =  checklist[i].itmlp; 
		while (itmlp != NULL)
		{
			if (!lpMapDocument->selectedbool)
			{
				tempvlist = (XP_POINT *) malloc(sizeof(XP_POINT)*4);
				tempvlist[0].x= itmlp->pos.x-17;
				tempvlist[0].y= itmlp->pos.y+17;
				tempvlist[1].delta_x = 35;
				tempvlist[1].delta_y= 0;
				tempvlist[2].delta_x = 0;
				tempvlist[2].delta_y= -35;
				tempvlist[3].delta_x = -35;
				tempvlist[3].delta_y= 0;

				itmlp->selected = InsidePolygon(tempvlist,4,x, y);
				//If the starting point of the polygon is wrapped around the other
				//side, then the above check won't work. So check to see if it was wrapped.*/

				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(tempvlist,4, x-lpMapDocument->width, y+lpMapDocument->height);
				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(tempvlist,4, x, y+lpMapDocument->height);
				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(tempvlist,4, x+lpMapDocument->width, y+lpMapDocument->height);

				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(tempvlist,4, x-lpMapDocument->width, y);
				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(tempvlist,4, x+lpMapDocument->width, y);

				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(tempvlist,4, x-lpMapDocument->width, y-lpMapDocument->height);
				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(tempvlist,4, x, y-lpMapDocument->height);
				if (!lpMapDocument->selectedbool && !itmlp->selected)
					itmlp->selected = InsidePolygon(tempvlist,4, x+lpMapDocument->width, y-lpMapDocument->height);

				free(tempvlist);
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
/* IsCounterClockwise                                                      */
/* Arguments :                                                             */
/*    pglp - a simple polygon                                              */
/* Purpose :   Test to see if the polygon is counter clockwise             */
/* Return :                                                                */
/*   TRUE if counterclockwise                                              */
/*   FALSE if clockwise                                                    */
/***************************************************************************/
int IsCounterClockwise (polygonlist *pglp)
{
	int area = 0;
	int ai, i, j;

	for (i = pglp->num_verts-1, j=0; j < pglp->num_verts; i=j, j++)
	{
		ai = pglp->vertex[i].x * pglp->vertex[j].y -
		pglp->vertex[j].x * pglp->vertex[i].y;
		area+=ai;
	}
	
	if (area > 0)
		return TRUE;
	else
		return FALSE;
}
/***************************************************************************/
/* ReversePolygonOrientation                                               */
/* Arguments :                                                             */
/*    pglp - a simple polygon                                              */
/* Purpose :   Reverse the order of vertexes in a polygon, except the first*/
/***************************************************************************/
void ReversePolygonOrientation(polygonlist *pglp)
{
	XP_POINT *vertex;
	int i,j, prevx, prevy;

	vertex = (XP_POINT *) malloc(sizeof(XP_POINT)*pglp->num_verts);

	//Now swap the order of the points into the temporary struct.
	for (i=pglp->num_verts-1, j=0; j<pglp->num_verts; i--, j++)
	{
		vertex[j].x = pglp->vertex[i].x;
		vertex[j].y = pglp->vertex[i].y;
		//Be sure to shift the variant to the correct item, because
		//the variant is specified at the ending point, which will
		//have changed to be a different index.
		vertex[j].variant = pglp->vertex[i+1].variant;
	}
	//Now reload the main struct, recalculating when necessary.
	prevx = prevy = 0;
	for(i=0; i <pglp->num_verts; i++)
	{
		pglp->vertex[i].x = vertex[i].x;
		pglp->vertex[i].y = vertex[i].y;
		pglp->vertex[i].delta_x = pglp->vertex[i].x - prevx;
		pglp->vertex[i].delta_y = pglp->vertex[i].y - prevy;
		pglp->vertex[i].variant = vertex[i].variant;
		prevx = pglp->vertex[i].x;
		prevy = pglp->vertex[i].y;
	}
	free(vertex);
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
/* FindClosestVertex                                                       */
/* Arguments :                                                             */
/*    lpMapDocument - pointer to map document.                             */
/*	polygon: a list of points                                              */
/*	x: the xcoord to check                                                 */
/*	y: the ycoord to check                                                 */
/* Return :                                                                */
/*   number of closest vertex if one is close enough                       */
/*   0 if no vertex is close enough                                        */
/* Purpose : Find the vertex closest to x,y in a given polygon.            */
/***************************************************************************/
int FindClosestVertex(LPMAPDOCUMENT lpMapDocument, polygonlist *polygon, int x, int y)
{
	int n = 0;
	int i, l, delta, closest;

	delta = lpMapDocument->width;

	for(i=0; i<polygon->num_verts; i++)
	{
		l = Wrap_length(lpMapDocument, (x - polygon->vertex[i].x),
			(y - polygon->vertex[i].y));
		if (l <= delta)
		{
			delta = l;
			closest = i;
		}
	}
	if (delta < 50)
		return closest; //Return this point, cause we're within range
	else
		return 0; //Just let the first point be selected.
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
/* DeleteVertex                                                            */
/* Arguments :                                                             */
/*    lpMapDocument - pointer to map document.                             */
/*	polygon: a list of points                                              */
/*	num: the vertex to delete                                              */
/* Purpose : Delete the specified vertex                                   */
/***************************************************************************/
void DeleteVertex(polygonlist **polygon, int num)
{
	int i, j, prevx, prevy;

	XP_POINT *vert;
	polygonlist *pglp;
	pglp = *polygon;

	vert = (XP_POINT *) malloc(pglp->num_verts*sizeof(XP_POINT));
	prevx = prevy = 0;
	for (i = 0, j = 0; i < pglp->num_verts; i++, j++)
	{
		if (i == num)
			i++;
		vert[j].x = pglp->vertex[i].x;
		vert[j].y = pglp->vertex[i].y;
		vert[j].delta_x = vert[j].x - prevx;
		vert[j].delta_y = vert[j].y - prevy;
		prevx = vert[j].x;
		prevy = vert[j].y;
	}
	pglp->num_verts--;
	realloc(pglp->vertex, pglp->num_verts*sizeof(XP_POINT));
	for (i = 0; i < pglp->num_verts; i++)
	{
		pglp->vertex[i].x = vert[i].x;
		pglp->vertex[i].y = vert[i].y;
		pglp->vertex[i].delta_x = vert[i].delta_x;
		pglp->vertex[i].delta_y = vert[i].delta_y;
	}

	free(vert);
}
/***************************************************************************/
/* MoveVertex                                                              */
/* Arguments :                                                             */
/*    lpMapDocument - pointer to map document.                             */
/*	polygon: a list of points                                              */
/*	num: the vertex to delete                                              */
/*	deltax: the change along the x axis                                    */
/*	deltay: the change along the y axis                                    */
/* Purpose : Move the specified vertex the specified distance              */
/***************************************************************************/
int MoveVertex(polygonlist **polygon, int num, int deltax, int deltay)
{
	int tempdx, tempdy;
	polygonlist *pglp;
	pglp = *polygon;
	
	if ((num != 0) && (num!=pglp->num_verts-1))
	{
		//Make sure the new point wont be too far away from the previous(chain)
		//point.
		tempdx = pglp->vertex[num].delta_x + deltax;
		tempdy = pglp->vertex[num].delta_y + deltay;
		if ((hypot(tempdx, tempdy) > MAX_LINE_LEN))
		{
			return TRUE;
		}
		
		//Make sure the new point wont be too far away from the next
		//point.
		tempdx = pglp->vertex[num+1].delta_x-deltax;
		tempdy = pglp->vertex[num+1].delta_y-deltay;
		if ((hypot(tempdx, tempdy) > MAX_LINE_LEN))
		{
			return TRUE;
		}
	}
	else
	{
		//If we're the first point make sure the new point will be
		//valid.
		tempdx = pglp->vertex[pglp->num_verts-1].delta_x+deltax;
		tempdy = pglp->vertex[pglp->num_verts-1].delta_y+deltay;
		if ((hypot(tempdx, tempdy) > MAX_LINE_LEN))
		{
			return TRUE;
		}
		//Check against the second point.
		tempdx = pglp->vertex[1].delta_x-deltax;
		tempdy = pglp->vertex[1].delta_y-deltay;
		if ((hypot(tempdx, tempdy) > MAX_LINE_LEN))
		{
			return TRUE;
		}

	}

	if((num != 0) && (num != pglp->num_verts-1))
	{
		pglp->vertex[num].x += deltax;
		pglp->vertex[num].y += deltay;
		pglp->vertex[num].delta_x += deltax;
		pglp->vertex[num].delta_y += deltay;
		
		pglp->vertex[num+1].delta_x = pglp->vertex[num+1].delta_x-deltax;
		pglp->vertex[num+1].delta_y = pglp->vertex[num+1].delta_y-deltay;
	}
	else
	{
		pglp->vertex[0].x += deltax;
		pglp->vertex[0].y += deltay;
		pglp->vertex[1].delta_x -= deltax;
		pglp->vertex[1].delta_y -= deltay;
		pglp->vertex[pglp->num_verts-1].x += deltax;
		pglp->vertex[pglp->num_verts-1].y += deltay;
		pglp->vertex[pglp->num_verts-1].delta_x += deltax;
		pglp->vertex[pglp->num_verts-1].delta_y += deltay;
	}

	return FALSE;
}
/***************************************************************************/
/* CountEdgesOfType                                                        */
/* Arguments :                                                             */
/*    lpMapDocument - pointer to map document.                             */
/*	polygon: a list of points                                              */
/*	type: the type of edges to count                                       */
/* Purpose : Count the number of edges of the specified type               */
/***************************************************************************/
int CountEdgesOfType(polygonlist **polygon, int type)
{
	int i, count = 0;
	polygonlist *pglp;
	pglp = *polygon;

	for (i = 0; i < pglp->num_verts; i++)
	{
		if (pglp->vertex[i].variant != type)
			continue;
		else
			count++;
	}
	return count;
}
/***************************************************************************/
/* ReorderItemTo	     	                                               */
/* Arguments :                                                             */
/*	iteml: a list of items                                                  */
/*  num: the new number for the checkpoint.                                */
/* Purpose : Delete the selected polygon.                                  */
/***************************************************************************/
void ReorderItemTo(itemlist **iteml, itemlist **item, int num)
{
	itemlist *itmlp = NULL;
	itemlist *item2 = *item;
	itemlist *tmp = NULL;
	int i, count;

	itmlp = *iteml;

/* 	count=0;
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
	}*/

}
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
int FindClosestItemInList(LPMAPDOCUMENT lpMapDocument, int x, int y, itemlist **item)
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
