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
#ifdef _WINDOWS
#include "windows\win_xpstudio.h"
#endif

#ifdef _WINDOWS
HDC mapDC;
HDC shiplistDC;
HDC shipviewDC;

HPEN hEdgePens[256];
HBRUSH hPolygonBrushes[256];
#endif

segment_t mapicon_seg[16] = {
	{ 2, {  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00}, {-17.50, 17.50,  0.00,   0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00} }, //0: Base
	{ 5, {  0.00, 35.00, 20.00, 35.00, 20.00,  0.00,  0.00,  0.00,  0.00,  0.00}, {  0.00,  0.00,-12.00,   0.00, 12.00,  0.00,  0.00,  0.00,  0.00,  0.00} }, //1: Current
	{ 5, {-17.50,-17.50, 17.50, 17.50,-17.50,  0.00,  0.00,  0.00,  0.00,  0.00}, {-17.50, 17.50, 17.50, -17.50,-17.50,  0.00,  0.00,  0.00,  0.00,  0.00} }, //2: Target
	{ 0, {  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00}, {  0.00,  0.00,  0.00,   0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00} }, //3: Ball
	{ 4, {  0.00, 12.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00}, { 17.50,  0.00,-17.50,  17.50,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00} }, //4: Cannon
	{ 5, {-17.50,-17.50, 17.50, 17.50,-17.50,  0.00,  0.00,  0.00,  0.00,  0.00}, {-17.50, 17.50, 17.50, -17.50,-17.50,  0.00,  0.00,  0.00,  0.00,  0.00} }, //5: Fuel
	{ 5, {-12.00, 12.00,  0.00,-12.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00}, { 12.00, 12.00,-12.00,  12.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00} }, //6: ItemConcentrator
	{ 5, {-12.00, 12.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00}, {  0.00,  0.00,  0.00,  12.00,-12.00,  0.00,  0.00,  0.00,  0.00,  0.00} }, //7: GravityPos
	{ 2, {-12.00, 12.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00}, {  0.00,  0.00,  0.00,   0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00} }, //8: GravityNeg
	{ 3, {-10.00, -5.00, -6.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00}, {-16.00,-15.00, -8.00,   0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00} }, //9: GravityClockwise
	{ 3, { 10.00,  5.00,  6.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00}, {-16.00,-15.00, -8.00,   0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00} }, //10: GravityCounterClockwise
	{ 0, {  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00}, {  0.00,  0.00,  0.00,   0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00} }, //11: Wormhole normal
	{ 5, {-17.50, 17.50, 17.50, 17.50,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00}, {-17.50, 17.50,  0.00,  17.50, 17.50,  0.00,  0.00,  0.00,  0.00,  0.00} }, //12: Wormhole out
	{ 5, {  0.00,-17.50, 17.50,-17.50,-17.50,  0.00,  0.00,  0.00,  0.00,  0.00}, {-17.50,-17.50, 17.50, -17.50,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00} }, //13: Wormhole in
	{ 4, {  6.00,  6.00, 29.00,  6.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00}, { -9.00,  9.00,  0.00,  -9.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00} }, //14: Decorative Ship
	{ 5, {-10.00,  0.00, 10.00,  0.00,-10.00,  0.00,  0.00,  0.00,  0.00,  0.00}, {  0.00,-10.00,  0.00,  10.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00} }, //15: Checkpoint
};
/***************************************************************************/
/* DrawMapEntire                                                           */
/* Arguments :                                                             */
/*   lpMapDocument: pointer to map document.                               */
/* Purpose : Draw the entire map.                                          */
/* Currently, this function draws each item nine times, to be sure it is   */
/* Displayed correctly. This is of course extremely inefficient, but I     */
/* haven't found any other way to do it and still have lines start off     */
/* screen or end off screen.                                               */
/***************************************************************************/
void DrawMapEntire(LPMAPDOCUMENT lpMapDocument)
{
	int i;
	polygonlist *pglp;
	itemlist *itmlp;
	int numpgondraw = 4;
	float zoom = lpMapDocument->view_zoom;
	struct pgondraw {
		struct polygonlist *pglp;
		int type;
		int color;
	};
	struct pgondraw pgondrawlist[4] =
	{
		{lpMapDocument->MapGeometry.walls, IDM_MAP_WALL, COLOR_WALL},
		{lpMapDocument->MapGeometry.decors, IDM_MAP_DECOR, COLOR_DECOR},
		{lpMapDocument->MapGeometry.ballareas, IDM_MAP_BALLAREA, COLOR_BALLAREA},
		{lpMapDocument->MapGeometry.balltargets, IDM_MAP_BALLTARGET, COLOR_BALLTARGET},
	};
	int numitemdraw = 11;
	struct itemdraw {
		struct itemlist *itmlp;
		int type;
		int color;
	};
	struct itemdraw itemdrawlist[11] =
	{
		{lpMapDocument->MapGeometry.targets, IDM_MAP_TARGET, COLOR_TARGET},
		{lpMapDocument->MapGeometry.fuels, IDM_MAP_FUEL, COLOR_WALL},
		{lpMapDocument->MapGeometry.cannons, IDM_MAP_CANNON, COLOR_CANNON},
		{lpMapDocument->MapGeometry.balls, IDM_MAP_BALL, COLOR_BALL},
		{lpMapDocument->MapGeometry.bases, IDM_MAP_BASE, COLOR_BASE},
		{lpMapDocument->MapGeometry.currents, IDM_MAP_CURRENT, COLOR_CURRENT},
		{lpMapDocument->MapGeometry.itemconcentrators, IDM_MAP_ITEM_CONC, COLOR_ITEM_CONC},
		{lpMapDocument->MapGeometry.gravities, IDM_MAP_GRAVITY, COLOR_GRAVITY},
		{lpMapDocument->MapGeometry.circulargravities, IDM_MAP_CIRCULAR_GRAVITY, COLOR_GRAVITY},
		{lpMapDocument->MapGeometry.wormholes, IDM_MAP_WORMHOLE, COLOR_WORMHOLE},
		{lpMapDocument->MapGeometry.checkpoints, IDM_MAP_CHECKPOINT, COLOR_CHECKPOINT},
	};


#ifdef _WINDOWS
	HFONT storeFont;
	HPEN hPenCurrent;
	mapDC = GetDC(hwndTemp);
	storeFont = GetZoomFont(mapDC, lpMapDocument);
	hPenSelected = CreatePen (PS_SOLID,0,COLOR_SELECT);

	for (i = 1; i <= lpMapDocument->MapGeometry.num_estyles; i++)
	{
			/*For some reason, the color orders from the file go BGR instead of RGB.
			We want to leave the number itself alone though, since thats the way
			xpilot does it. So just swap the values.*/
		hEdgePens[i] = CreatePen(lpMapDocument->MapGeometry.estyles[i].style,
			lpMapDocument->MapGeometry.estyles[i].width, 
			RGB(GetBValue(lpMapDocument->MapGeometry.estyles[i].color),
			GetGValue(lpMapDocument->MapGeometry.estyles[i].color),
			GetRValue(lpMapDocument->MapGeometry.estyles[i].color))
			);
	}
	for (i = 0; i < lpMapDocument->MapGeometry.num_pstyles; i++)
	{
		if (lpMapDocument->MapGeometry.pstyles[i].flags == 1)
		{
		hPolygonBrushes[i] = CreateSolidBrush(
			RGB(GetBValue(lpMapDocument->MapGeometry.pstyles[i].color),
			GetGValue(lpMapDocument->MapGeometry.pstyles[i].color),
			GetRValue(lpMapDocument->MapGeometry.pstyles[i].color))
			);
		}
		/*To Do: Else for pattern brushes*/
	}

#endif

	for (i = 0; i < numpgondraw; i++)
	{
		if (!pgondrawlist[i].pglp)
			continue;
		pglp = pgondrawlist[i].pglp;
		//Row Above
		DrawPolygonList(lpMapDocument, pglp, (int) ((lpMapDocument->view_x-lpMapDocument->width)/zoom), (int) ((lpMapDocument->view_y-lpMapDocument->height)/zoom));
		DrawPolygonList(lpMapDocument, pglp, (int) (lpMapDocument->view_x/zoom), (int) ((lpMapDocument->view_y-lpMapDocument->height)/zoom));
		DrawPolygonList(lpMapDocument, pglp, (int) ((lpMapDocument->view_x+lpMapDocument->width)/zoom), (int) ((lpMapDocument->view_y-lpMapDocument->height)/zoom));
		
		//Left and Right
		DrawPolygonList(lpMapDocument, pglp, (int) ((lpMapDocument->view_x-lpMapDocument->width)/zoom), (int) (lpMapDocument->view_y/zoom));
		DrawPolygonList(lpMapDocument, pglp, (int) (lpMapDocument->view_x/zoom), (int) (lpMapDocument->view_y/zoom)); //DefaultView
		DrawPolygonList(lpMapDocument, pglp, (int) ((lpMapDocument->view_x+lpMapDocument->width)/zoom), (int) (lpMapDocument->view_y/zoom));
		
		//Row Below
		DrawPolygonList(lpMapDocument, pglp, (int) ((lpMapDocument->view_x-lpMapDocument->width)/zoom), (int) ((lpMapDocument->view_y+lpMapDocument->height)/zoom));
		DrawPolygonList(lpMapDocument, pglp, (int) (lpMapDocument->view_x/zoom), (int) ((lpMapDocument->view_y+lpMapDocument->height)/zoom));
		DrawPolygonList(lpMapDocument, pglp, (int) ((lpMapDocument->view_x+lpMapDocument->width)/zoom), (int) ((lpMapDocument->view_y+lpMapDocument->height)/zoom));
	}

	for (i = 0; i < numitemdraw; i++)
	{
		if (!itemdrawlist[i].itmlp)
			continue;
		itmlp = itemdrawlist[i].itmlp;
#ifdef _WINDOWS
		hPenCurrent = CreatePen(PS_SOLID, 0, itemdrawlist[i].color);
		SelectObject(mapDC, hPenCurrent);
#endif
		//Row Above
		DrawItemList(lpMapDocument, itmlp, itemdrawlist[i].type, (int) ((lpMapDocument->view_x-lpMapDocument->width)/zoom), (int) ((lpMapDocument->view_y-lpMapDocument->height)/zoom));
		DrawItemList(lpMapDocument, itmlp, itemdrawlist[i].type, (int) (lpMapDocument->view_x/zoom), (int) ((lpMapDocument->view_y-lpMapDocument->height)/zoom));
		DrawItemList(lpMapDocument, itmlp, itemdrawlist[i].type, (int) ((lpMapDocument->view_x+lpMapDocument->width)/zoom), (int) ((lpMapDocument->view_y-lpMapDocument->height)/zoom));
		
		//Left and Right
		DrawItemList(lpMapDocument, itmlp, itemdrawlist[i].type, (int) ((lpMapDocument->view_x-lpMapDocument->width)/zoom), (int) (lpMapDocument->view_y/zoom));
		DrawItemList(lpMapDocument, itmlp, itemdrawlist[i].type, (int) (lpMapDocument->view_x/zoom), (int) (lpMapDocument->view_y/zoom)); //DefaultView
		DrawItemList(lpMapDocument, itmlp, itemdrawlist[i].type, (int) ((lpMapDocument->view_x+lpMapDocument->width)), (int) (lpMapDocument->view_y/zoom));
		
		//Row Below
		DrawItemList(lpMapDocument, itmlp, itemdrawlist[i].type, (int) ((lpMapDocument->view_x-lpMapDocument->width)/zoom), (int) ((lpMapDocument->view_y+lpMapDocument->height)/zoom));
		DrawItemList(lpMapDocument, itmlp, itemdrawlist[i].type, (int) (lpMapDocument->view_x/zoom), (int) ((lpMapDocument->view_y+lpMapDocument->height)/zoom));
		DrawItemList(lpMapDocument, itmlp, itemdrawlist[i].type, (int) ((lpMapDocument->view_x+lpMapDocument->width)/zoom), (int) ((lpMapDocument->view_y+lpMapDocument->height)/zoom));
#ifdef _WINDOWS
		DeleteObject(SelectObject(mapDC, GetStockObject(BLACK_PEN)));
#endif
	}

	if (lpMapDocument->selectedbool && lpMapDocument->selectedtype == IDM_VERTEX)
	{
#ifdef _WINDOWS
		SelectObject(mapDC, hPenSelected);
#endif
		//Row Above
		DrawSymbol(lpMapDocument, IDM_VERTEX, lpMapDocument->selectedvert->x, lpMapDocument->selectedvert->y, (int) ((lpMapDocument->view_x-lpMapDocument->width)/zoom), (int) ((lpMapDocument->view_y-lpMapDocument->height)/zoom));
		DrawSymbol(lpMapDocument, IDM_VERTEX, lpMapDocument->selectedvert->x, lpMapDocument->selectedvert->y, (int) (lpMapDocument->view_x/zoom), (int) ((lpMapDocument->view_y-lpMapDocument->height)/zoom));
		DrawSymbol(lpMapDocument, IDM_VERTEX, lpMapDocument->selectedvert->x, lpMapDocument->selectedvert->y, (int) ((lpMapDocument->view_x+lpMapDocument->width)/zoom), (int) ((lpMapDocument->view_y-lpMapDocument->height)/zoom));
		
		//Left and Right
		DrawSymbol(lpMapDocument, IDM_VERTEX, lpMapDocument->selectedvert->x, lpMapDocument->selectedvert->y, (int) ((lpMapDocument->view_x-lpMapDocument->width)/zoom), (int) (lpMapDocument->view_y/zoom));
		DrawSymbol(lpMapDocument, IDM_VERTEX, lpMapDocument->selectedvert->x, lpMapDocument->selectedvert->y, (int) (lpMapDocument->view_x/zoom), (int) (lpMapDocument->view_y/zoom)); //DefaultView
		DrawSymbol(lpMapDocument, IDM_VERTEX, lpMapDocument->selectedvert->x, lpMapDocument->selectedvert->y, (int) ((lpMapDocument->view_x+lpMapDocument->width)), (int) (lpMapDocument->view_y/zoom));
		
		//Row Below
		DrawSymbol(lpMapDocument, IDM_VERTEX, lpMapDocument->selectedvert->x, lpMapDocument->selectedvert->y, (int) ((lpMapDocument->view_x-lpMapDocument->width)/zoom), (int) ((lpMapDocument->view_y+lpMapDocument->height)/zoom));
		DrawSymbol(lpMapDocument, IDM_VERTEX, lpMapDocument->selectedvert->x, lpMapDocument->selectedvert->y, (int) (lpMapDocument->view_x/zoom), (int) ((lpMapDocument->view_y+lpMapDocument->height)/zoom));
		DrawSymbol(lpMapDocument, IDM_VERTEX, lpMapDocument->selectedvert->x, lpMapDocument->selectedvert->y, (int) ((lpMapDocument->view_x+lpMapDocument->width)/zoom), (int) ((lpMapDocument->view_y+lpMapDocument->height)/zoom));
	}
	

#ifdef _WINDOWS
	SelectObject(mapDC, GetStockObject(BLACK_PEN));
	for (i = 1; hEdgePens[i] != NULL; i++)
	{
		DeleteObject(hEdgePens[i]);
	}
	for (i = 0; hPolygonBrushes[i] != NULL; i++)
	{
		DeleteObject(hPolygonBrushes[i]);
	}

	DeleteObject(hPenSelected);
	DeleteZoomFont(mapDC, storeFont);
	ReleaseDC(hwndTemp,mapDC);
#endif

}
/***************************************************************************/
/* DrawSymbol                                                              */
/* Arguments :                                                             */
/*   type: the type of symbol to draw                                      */
/*   ax: where to draw x coord                                             */
/*   ay: where to draw y coord                                             */
/*   offsx : offset in x dir                                               */
/*   offsy : offset in y dir                                               */
/* Purpose : Draw the specified list of polygons.                          */
/***************************************************************************/
void DrawSymbol(LPMAPDOCUMENT lpMapDocument, int type, int ax, int ay, int offsx, int offsy)
{
	int x, y;
	float zoom = lpMapDocument->view_zoom;

	x = ax;
	y = lpMapDocument->height - ay;
	switch (type)
	{
	case IDM_VERTEX:
		Arc(mapDC, (int)(x/zoom-offsx-4),(int)(y/zoom-offsy-4),
			(int)(x/zoom-offsx+5),(int)(y/zoom-offsy+5),
			(int)(x/zoom-offsx),(int)(y/zoom-offsy),
			(int)(x/zoom-offsx),(int)(y/zoom-offsy));
		break;
	}
}
/***************************************************************************/
/* DrawPolygonList                                                         */
/* Arguments :                                                             */
/*   lpMapDocument: pointer to map document.                               */
/*   pglp: pointer to the polygon to draw.                                 */
/*   offsx : offset in x dir                                               */
/*   offsy : offset in y dir                                               */
/* Purpose : Draw the specified list of polygons.                          */
/***************************************************************************/
void DrawPolygonList(LPMAPDOCUMENT lpMapDocument, polygonlist *pglp,
					 int offsx, int offsy)
{
	polygonlist *pglptemp;
//	char strng[2];
	int x = 0, y = 0, i;
	int startx = 0, starty = 0;
	float zoom = lpMapDocument->view_zoom;
#ifdef _WINDOWS
	POINT polypoints[256];

	pglptemp = pglp;

	while (pglptemp != NULL)
	{
			if (lpMapDocument->MapGeometry.pstyles[pglptemp->polygon_style].flags == 1)
			{
				for (i = 0; i < pglptemp->num_verts; i++)
				{
					polypoints[i].x = (int)(pglptemp->vertex[i].x/zoom)-offsx;
					polypoints[i].y = (int)((lpMapDocument->height-pglptemp->vertex[i].y)/zoom)-offsy;
				}
				if (!lpMapDocument->selectedvert && pglptemp->selected && lpMapDocument->selectedbool && !fDrawing)
					SelectObject(mapDC, hPenSelected);
				else
					SelectObject(mapDC, hEdgePens[lpMapDocument->MapGeometry.pstyles[pglptemp->polygon_style].defedge_id]);

				SelectObject(mapDC, hPolygonBrushes[pglptemp->polygon_style]);
				Polygon(mapDC, polypoints, pglptemp->num_verts);
			}
			if (lpMapDocument->MapGeometry.pstyles[pglptemp->polygon_style].flags == 0)
			{
				x = (int)(pglptemp->vertex[0].x/zoom);
				y = (int)((lpMapDocument->height-pglptemp->vertex[0].y)/zoom);
				MoveToEx(mapDC, x-offsx,
					y-offsy, NULL);
				SelectObject(mapDC, hEdgePens[lpMapDocument->MapGeometry.pstyles[pglptemp->polygon_style].defedge_id]);
				
				if (!lpMapDocument->selectedvert && pglptemp->selected && lpMapDocument->selectedbool && !fDrawing)
					SelectObject(mapDC, hPenSelected);
				
				for(i = 1; i < pglptemp->num_verts; i++)
				{
					x += (int)(pglptemp->vertex[i].delta_x/zoom);
					y -= (int)(pglptemp->vertex[i].delta_y/zoom);
					
					if (pglptemp->vertex[i].edge_style != pglptemp->vertex[i-1].edge_style)
						if (!pglptemp->selected)
						{
							SelectObject(mapDC, hEdgePens[pglptemp->vertex[i].edge_style]);
						}
						
						//Draw the line
						LineTo(mapDC, x-offsx,
							y-offsy);
				}
			}
			pglptemp = pglptemp->next;
	}

	SelectObject(mapDC, GetStockObject(BLACK_PEN));
	SelectObject(mapDC, GetStockObject(BLACK_BRUSH));
#endif

}
/***************************************************************************/
/* DrawItemList                                                            */
/* Arguments :                                                             */
/*   lpMapDocument: pointer to map document.                               */
/*   itmlp: pointer to the itemlist to draw.                               */
/*   type: the type of item to draw                                        */
/*   offsx : offset in x dir                                               */
/*   offsy : offset in y dir                                               */
/* Purpose : Draw the specified list of items.                             */
/***************************************************************************/
void DrawItemList(LPMAPDOCUMENT lpMapDocument, itemlist *itmlp, int type,
				  int offsx, int offsy)
{
	itemlist item;
	int x = 0, y = 0;
	int x2 = 0, y2 = 0;
//	char strng[2];
	int picnum, i, numcheck=1;
	double angle; //The angle to draw at.
	double zoom = lpMapDocument->view_zoom; //The current map zoom.
	
	int draw; //Do we need to draw?
//	int tofsx, tofsy; //Tweaks for text offsets, block dependant.
#ifdef _WINDOWS
	RECT	rect;
	POINT	points[10]; //Win point structure, accepts only (long) dims
	HPEN hstorePen;

	hBrushFuel = CreateSolidBrush (COLOR_FUEL);
#endif

	while (itmlp != NULL)
	{
		item = *itmlp;
		x = item.pos.x;
		y = lpMapDocument->height-item.pos.y;
		switch (type)
		{
		case IDM_MAP_BASE:
			picnum = 0;
			break;
		case IDM_MAP_CURRENT:
			picnum = 1;
			break;
		case IDM_MAP_TARGET:
			picnum = 2;
			break;
		case IDM_MAP_BALL:
			picnum = 3;
			break;
		case IDM_MAP_CANNON:
			picnum = 4;
			break;
		case IDM_MAP_FUEL:
			picnum = 5;
			break;
		case IDM_MAP_ITEM_CONC:
			picnum = 6;
			break;
		case IDM_MAP_GRAVITY:
			if (item.variant)
				picnum = 8;
			else
				picnum = 7;
			break;
		case IDM_MAP_CIRCULAR_GRAVITY:
			if (item.variant)
				picnum = 10;
			else
				picnum = 9;
			break;
		case IDM_MAP_WORMHOLE:
			switch (item.variant)
			{
			case 0: picnum = 11; //Normal wormholes
				break;
			case 1:	picnum = 12; //Out Only Wormholes
				break;
			case 2: picnum = 13; //In Only Wormholes
				break;
			}
			break;
		case IDM_MAP_CHECKPOINT:
			picnum = 15;
			break;
		}
		//Convert direction to a usable angle, converting to radians at the same
		//time.
		angle = (item.direction*360/128)*3.14/180;
		//FILL THE POINTS ARRAY WITH THE SIZES FROM THE ICON SEGMENT
		for (i=0;i<mapicon_seg[picnum].num_points; i++)
		{
			points[i].x = (int)((x+(long) ( (mapicon_seg[picnum].x[i] * cos(angle))
					+ (mapicon_seg[picnum].y[i] * sin(angle)) ))/zoom - offsx);
			points[i].y = (int)((y+(long) ( (mapicon_seg[picnum].y[i] * cos(angle))
					- (mapicon_seg[picnum].x[i] * sin(angle)) ))/zoom - offsy);
		}
		
		if (item.selected && lpMapDocument->selectedbool)
#ifdef _WINDOWS
			hstorePen = SelectObject(mapDC, hPenSelected);
#endif

////////Draw the main picture segment
		switch (type)
		{
		case IDM_MAP_FUEL: //This should actually be drawn with a fillRect GDI command.
#ifdef _WINDOWS
			rect.left = points[0].x+1;
			rect.top = points[0].y+1;
			rect.right = points[2].x-1;
			rect.bottom = points[2].y-1;
			FillRect(mapDC, &rect, hBrushFuel);
#endif
		case IDM_MAP_WORMHOLE: if (picnum==20) //No vectors to draw if normal type.
							   break;
		case IDM_MAP_CANNON:
		case IDM_MAP_BALL:
		case IDM_MAP_TARGET:
		case IDM_MAP_BASE:
		case IDM_MAP_CIRCULAR_GRAVITY:
		case IDM_MAP_GRAVITY:
		case IDM_MAP_ITEM_CONC:
		case IDM_MAP_CURRENT:
		case IDM_MAP_CHECKPOINT:
#ifdef _WINDOWS
			Polyline(mapDC, points, mapicon_seg[picnum].num_points);
#endif
			break;
		}
	
////////Now draw any secondary vectors, if there are any.
		draw = 0;
		switch (type)
		{
		case IDM_MAP_BASE:
			picnum = 14;
			draw = 1;
			break;
		}
		if (draw)
		{
			for (i=0;i<mapicon_seg[picnum].num_points; i++)
			{
				points[i].x = (int)((x+(long) ( (mapicon_seg[picnum].x[i] * cos(angle))
					+ (mapicon_seg[picnum].y[i] * sin(angle)) ))/zoom - offsx);
				points[i].y = (int)((y+(long) ( (mapicon_seg[picnum].y[i] * cos(angle))
					- (mapicon_seg[picnum].x[i] * sin(angle)) ))/zoom - offsy);
			}
#ifdef _WINDOWS
			Polyline(mapDC, points, mapicon_seg[picnum].num_points);
#endif
		}
////////
////////Now draw any curved picture stuff
		switch (type)
		{
		case IDM_MAP_BALL:
#ifdef _WINDOWS
			Arc(mapDC, (int)((x-8)/zoom-offsx),(int)((y-8)/zoom-offsy),
				(int)((x+9)/zoom-offsx),(int)((y+9)/zoom-offsy),
				(int)(x/zoom-offsx),(int)(y/zoom-offsy),
				(int)(x/zoom-offsx),(int)(y/zoom-offsy));
#endif
			break;
/*		case IDM_MAP_GRAVITY:
#ifdef _WINDOWS
			Arc(mapDC, (int)(x-15)-offsx,(int)(y-15)-offsy,
				(int)(x+16)-offsx,(int)(y+16)-offsy,
				x-offsx,y-offsy,
				x-offsx,y-offsy);
#endif
			break;
		case IDM_MAP_CIRCULAR_GRAVITY:
#ifdef _WINDOWS
			Arc(mapDC,(int)(x-15)-offsx,(int)(y-15)-offsy,
				(int)(x+16)-offsx,(int)(y+16)-offsy,
				(int)(x-5)-offsx,(int)(y-15)-offsy,
				(int)(x+5)-offsx,(int)(y-15)-offsy);
#endif
			break;
		case IDM_MAP_WORMHOLE:
#ifdef _WINDOWS
			Arc(mapDC, (int)(x-17.5)-offsx,(int)(y-17.5)-offsy,
				(int)(x+17.5)-offsx,(int)(y+17.5)-offsy,
				x-offsx,y-offsy,
				x-offsx,y-offsy);
			Arc(mapDC, (int)(x-17.5)-offsx,(int)(y-17.5)-offsy,
				(int)(x+5)-offsx,(int)(y+5)-offsy,
				x-offsx,y-offsy,
				x-offsx,y-offsy);
			Arc(mapDC, (int)(x-17.5)-offsx,(int)(y-17.5)-offsy,
				(int)(x-5)-offsx,(int)(y-5)-offsy,
				x-offsx,y-offsy,
				x-offsx,y-offsy);
#endif
			break;*/
		}
/*////////
////////Draw any text if there is any.
		draw = 0;
		tofsx = tofsy = 0;
		switch (type)
		{
		case IDM_MAP_TARGET: //Center the text for some
		case IDM_MAP_BALL:
		case IDM_MAP_FUEL:
			tofsx = -3;
			tofsy = -6;
		case IDM_MAP_CANNON: //Others leave it where it is.
		case IDM_MAP_BASE:
			sprintf(strng, "%d\0", item.team);
			draw = TRUE;
			break;
		case IDM_MAP_CHECKPOINT:
			sprintf(strng, "%d\0", numcheck);
			numcheck++;
			tofsx = -3;
			tofsy = -6;
			draw = TRUE;
			break;
		}
		if (draw)
#ifdef _WINDOWS
			rect.left = x+tofsx-offsx;
		    rect.top = y+tofsy-offsy;
			rect.right = x-tofsx-offsx;
			rect.bottom = y-tofsy-offsy;
			DrawText(mapDC, strng, strlen(strng), &rect, DT_NOCLIP);
#endif
////////*/

	if (item.selected && lpMapDocument->selectedbool)
#ifdef _WINDOWS
		SelectObject(mapDC, hstorePen);
#endif
	itmlp = itmlp->next;
	}

#ifdef _WINDOWS
	DeleteObject(hBrushFuel);
#endif
}
