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

/*map.c*/
int NewMapInit(LPMAPDOCUMENT);
void Setup_default_server_options(LPMAPDOCUMENT);
int DoModifyCommand(LPMAPDOCUMENT, int, int, int);
void UpdateSelected(LPMAPDOCUMENT);

/*document.c*/
LPXPSTUDIODOCUMENT CreateNewXpDocument(int);
void DestroyXpDocument(LPXPSTUDIODOCUMENT);
LPMAPDOCUMENT CreateNewMapDoc();
void DestroyMapDoc(LPMAPDOCUMENT);
LPSHIPDOCUMENT CreateNewShipDoc();
void DestroyShipDoc(LPSHIPDOCUMENT);

/*map_file.c*/
static void tagstart(void *, const char *, const char **);
static void tagend(void *, const char *);
int Load_lines(int);
int LoadMap(LPMAPDOCUMENT, char *);
int SaveMap(LPMAPDOCUMENT, char *, int, int);
int	AddOption(LPMAPDOCUMENT, char *, char *, int, int);
int	YesNo(char *);
char	*StrToNum(char *, int, int);
int	LoadMapData(LPMAPDOCUMENT, char *);
char	*getMultilineValue();
int FindOption(LPMAPDOCUMENT, char *);
void WritePolygonList(FILE *, polygonlist *, int);
void WriteItemList(FILE *, itemlist *, int);

/*errors.c*/
void ErrorHandler(char *, ...);

/*geometry.c*/
int CreateItem(LPMAPDOCUMENT, int, int, int, int, int, int);
int AddItemToPolygonlist(LPMAPDOCUMENT, polygonlist **, int, int, int, int, int, int, int, int);
int AddVertexToList(polygonlist *, XP_POINT *);
int AddItemToItemlist(itemlist **, int, int, int, int, int, int);
int InsidePolygon(XP_POINT *, int, int, int);
int SelectItem(LPMAPDOCUMENT, int, int);
void DeleteMapItem(LPMAPDOCUMENT);
int IsCounterClockwise (polygonlist *);
void ReversePolygonOrientation(polygonlist *);
int FindClosestVertex(LPMAPDOCUMENT, polygonlist *, int, int);
double Wrap_length(LPMAPDOCUMENT, double, double);
void DeleteVertex(polygonlist **, int);
int CountEdgesOfType(polygonlist **, int);

/*draw.c*/
void DrawMapEntire(LPMAPDOCUMENT);
void DrawPolygonList(LPMAPDOCUMENT, polygonlist *, int, int, int);
void DrawVertexList(LPMAPDOCUMENT, polygonlist *, int, int, int);
void DrawItemList(LPMAPDOCUMENT, itemlist *, int, int, int);

/*helper.c*/
void GetSetEnterableInfo(void);
