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
/* AddVertexToList                                                         */
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
		polygon->vertex[polygon->num_verts-1].edge_style = vert->edge_style;
		free(vert);
	}
	return FALSE;
}
/***************************************************************************/
/* IsVertexOfPolygon 	                                                   */
/* Arguments :                                                             */
/*	polygon: a list of points                                              */
/*	N: the number of vertexes                                              */
/*	x: the xcoord to check                                                 */
/*	y: the ycoord to check                                                 */
/* Return :                                                                */
/*   number of vertex if on                                                */
/*   -1 if not on a vertex of the polygon                                  */
/* Purpose : Determine whether the specified coordinates are a             */
/* vertex of the selected polygon                                          */
/***************************************************************************/
int IsVertexOfPolygon(XP_POINT *plygn, int N, int x, int y)
{
	int i;

	for (i=0; i< N; i++)
	{
		if ((abs(x-plygn[i].x) <= 3) && (abs(y-plygn[i].y) <= 3))
			return i;
		else
			plygn[i].selected = FALSE;
	}

	return -1;
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
		//Be sure to shift the special attributes to the correct item,
		//because the variant is specified at the ending point, which will
		//have changed to be a different index.
		vertex[j].edge_style = pglp->vertex[i+1].edge_style;
	}
	//Now reload the main struct, recalculating when necessary.
	prevx = prevy = 0;
	for(i=0; i <pglp->num_verts; i++)
	{
		pglp->vertex[i].x = vertex[i].x;
		pglp->vertex[i].y = vertex[i].y;
		pglp->vertex[i].delta_x = pglp->vertex[i].x - prevx;
		pglp->vertex[i].delta_y = pglp->vertex[i].y - prevy;
		pglp->vertex[i].edge_style = vertex[i].edge_style;
		prevx = pglp->vertex[i].x;
		prevy = pglp->vertex[i].y;
	}
	free(vertex);
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
/*	polygon: a list of points                                              */
/*	num: the vertex to move                                                */
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
/* MovePolygon                                                             */
/* Arguments :                                                             */
/*    lpMapDocument - pointer to map document.                             */
/*	points: a list of points                                               */
/*	num: the number of vertexes                                            */
/*	deltax: the change along the x axis                                    */
/*	deltay: the change along the y axis                                    */
/* Purpose : Move the pointlist deltax and deltay                          */
/***************************************************************************/
int MovePolygon(LPMAPDOCUMENT lpMapDocument, XP_POINT *points, int num, int deltax, int deltay)
{
	int i;
	
	points[0].x = WRAP_XPIXEL(points[0].x+deltax);
	points[0].y = WRAP_YPIXEL(points[0].y+deltay);
	for (i = 1; i<num; i++)
	{
		points[i].x = points[i-1].x + points[i].delta_x;
		points[i].y = points[i-1].y + points[i].delta_y;
	}
	
	return FALSE;
}