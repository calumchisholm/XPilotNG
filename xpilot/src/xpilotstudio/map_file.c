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
int startx, starty;
int type, team;
LPMAPDOCUMENT lpTempMapDocument; //Temporary pointer to map document
								//used by callback functions below.

int get_bmp_id(LPMAPDOCUMENT lpMapDocument, const char *s)
{
    int i;

    for (i = 0; i < lpMapDocument->MapGeometry.num_bstyles; i++)
	if (!strcmp(lpMapDocument->MapGeometry.bstyles[i].id, s))
	    return i;
    ErrorHandler("Undeclared bmpstyle %s", s);
    return 0;
}


int get_edge_id(LPMAPDOCUMENT lpMapDocument, const char *s)
{
    int i;

    for (i = 0; i < lpMapDocument->MapGeometry.num_estyles; i++)
	if (!strcmp(lpMapDocument->MapGeometry.estyles[i].id, s))
	    return i;
    ErrorHandler("Undeclared edgestyle %s", s);
    return -1;
}


int get_poly_id(LPMAPDOCUMENT lpMapDocument, const char *s)
{
    int i;

    for (i = 0; i < lpMapDocument->MapGeometry.num_pstyles; i++)
	if (!strcmp(lpMapDocument->MapGeometry.pstyles[i].id, s))
	    return i;
    ErrorHandler("Undeclared polystyle %s", s);
    return 0;
}

static void tagstart(void *data, const char *el, const char **attr)
{
	//Create a list of all the types of polygons
	//that can be loaded.
	struct polygontypes {
		struct polygonlist **pglp;
	};
	struct polygontypes polylist[4] =
	{
		&lpTempMapDocument->MapGeometry.walls,
			&lpTempMapDocument->MapGeometry.ballareas,
			&lpTempMapDocument->MapGeometry.balltargets,
			&lpTempMapDocument->MapGeometry.decors,
	};
    int x, y, dir, variant, style = -1, edgestyle = -1;
	static int current_estyle;
	char temp[6];
	
    if (!strcasecmp(el, "Polystyle")) {
		lpTempMapDocument->MapGeometry.pstyles[lpTempMapDocument->MapGeometry.num_pstyles].id[sizeof(lpTempMapDocument->MapGeometry.pstyles[0].id) - 1] = 0;
		lpTempMapDocument->MapGeometry.pstyles[lpTempMapDocument->MapGeometry.num_pstyles].color = 0;
		lpTempMapDocument->MapGeometry.pstyles[lpTempMapDocument->MapGeometry.num_pstyles].texture_id = 0;
		lpTempMapDocument->MapGeometry.pstyles[lpTempMapDocument->MapGeometry.num_pstyles].defedge_id = 0;
		lpTempMapDocument->MapGeometry.pstyles[lpTempMapDocument->MapGeometry.num_pstyles].flags = 0;
		
		while (*attr) {
			if (!strcasecmp(*attr, "id"))
				strncpy(lpTempMapDocument->MapGeometry.pstyles[lpTempMapDocument->MapGeometry.num_pstyles].id, *(attr + 1),
				sizeof(lpTempMapDocument->MapGeometry.pstyles[0].id) - 1);
			if (!strcasecmp(*attr, "color"))
			{
				lpTempMapDocument->MapGeometry.pstyles[lpTempMapDocument->MapGeometry.num_pstyles].color = strtol(*(attr + 1), NULL, 16);
			}
			if (!strcasecmp(*attr, "texture"))
				lpTempMapDocument->MapGeometry.pstyles[lpTempMapDocument->MapGeometry.num_pstyles].texture_id = get_bmp_id(lpTempMapDocument, *(attr + 1));
			if (!strcasecmp(*attr, "defedge"))
				lpTempMapDocument->MapGeometry.pstyles[lpTempMapDocument->MapGeometry.num_pstyles].defedge_id = get_edge_id(lpTempMapDocument, *(attr + 1));
			if (!strcasecmp(*attr, "flags"))
				lpTempMapDocument->MapGeometry.pstyles[lpTempMapDocument->MapGeometry.num_pstyles].flags = atoi(*(attr + 1)); /* names @!# */
			attr += 2;
		}
		if (lpTempMapDocument->MapGeometry.pstyles[lpTempMapDocument->MapGeometry.num_pstyles].defedge_id == 0) {
			ErrorHandler("Polygon default edgestyle cannot be omitted or set "
				"to 'internal'!");
			exit(1);
		}
		lpTempMapDocument->MapGeometry.num_pstyles++;
    }
	
    if (!strcasecmp(el, "Edgestyle")) {
		lpTempMapDocument->MapGeometry.estyles[lpTempMapDocument->MapGeometry.num_estyles].id[sizeof(lpTempMapDocument->MapGeometry.estyles[0].id) - 1] = 0;
		lpTempMapDocument->MapGeometry.estyles[lpTempMapDocument->MapGeometry.num_estyles].width = 0;
		lpTempMapDocument->MapGeometry.estyles[lpTempMapDocument->MapGeometry.num_estyles].color = 0;
		lpTempMapDocument->MapGeometry.estyles[lpTempMapDocument->MapGeometry.num_estyles].style = 0;
		while (*attr) {
			if (!strcasecmp(*attr, "id"))
				strncpy(lpTempMapDocument->MapGeometry.estyles[lpTempMapDocument->MapGeometry.num_estyles].id, *(attr + 1),
				sizeof(lpTempMapDocument->MapGeometry.estyles[0].id) - 1);
			if (!strcasecmp(*attr, "width"))
				lpTempMapDocument->MapGeometry.estyles[lpTempMapDocument->MapGeometry.num_estyles].width = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "color"))
			{
				lpTempMapDocument->MapGeometry.estyles[lpTempMapDocument->MapGeometry.num_estyles].color = strtol(*(attr + 1), NULL, 16);
			}
			if (!strcasecmp(*attr, "style")) /* !@# names later */
				lpTempMapDocument->MapGeometry.estyles[lpTempMapDocument->MapGeometry.num_estyles].style = atoi(*(attr + 1));
			attr += 2;
		}
		lpTempMapDocument->MapGeometry.num_estyles++;
    }
	
    if (!strcasecmp(el, "Bmpstyle")) {
		lpTempMapDocument->MapGeometry.bstyles[lpTempMapDocument->MapGeometry.num_bstyles].flags = 0;
		lpTempMapDocument->MapGeometry.bstyles[lpTempMapDocument->MapGeometry.num_bstyles].filename[sizeof(lpTempMapDocument->MapGeometry.bstyles[0].filename) - 1] = 0;
		lpTempMapDocument->MapGeometry.bstyles[lpTempMapDocument->MapGeometry.num_bstyles].id[sizeof(lpTempMapDocument->MapGeometry.bstyles[0].id) - 1] = 0;
		/* add checks that these are filled !@# */
		
		while (*attr) {
			if (!strcasecmp(*attr, "id"))
				strncpy(lpTempMapDocument->MapGeometry.bstyles[lpTempMapDocument->MapGeometry.num_bstyles].id, *(attr + 1),
				sizeof(lpTempMapDocument->MapGeometry.bstyles[0].id) - 1);
			if (!strcasecmp(*attr, "filename"))
				strncpy(lpTempMapDocument->MapGeometry.bstyles[lpTempMapDocument->MapGeometry.num_bstyles].filename, *(attr + 1),
				sizeof(lpTempMapDocument->MapGeometry.bstyles[0].filename) - 1);
			if (!strcasecmp(*attr, "scalable"))
				if (!strcasecmp(*(attr + 1), "yes"))
					lpTempMapDocument->MapGeometry.bstyles[lpTempMapDocument->MapGeometry.num_bstyles].flags |= 1;
				attr += 2;
		}
		lpTempMapDocument->MapGeometry.num_bstyles++;
    }

	if (!strcasecmp(el, "Walls")) {
		type = 0;
    }
    if (!strcasecmp(el, "BallArea")) {
		while (*attr) {
			if (!strcasecmp(*attr, "team"))
				team = atoi(*(attr + 1));
			attr += 2;
		}
		type = 1;
    }
    if (!strcasecmp(el, "BallTarget")) {
		while (*attr) {
			if (!strcasecmp(*attr, "team"))
				team = atoi(*(attr + 1));
			attr += 2;
		}
		type = 2;
    }
 	if (!strcasecmp(el, "Decor")) {
		type = 3;
    }
   
	if (!strcasecmp(el, "Polygon")) {
		while (*attr) {
			if (!strcasecmp(*attr, "x"))
				startx = atoi(*(attr + 1)) / 64;
			if (!strcasecmp(*attr, "y"))
				starty = atoi(*(attr + 1)) / 64;
		    if (!strcasecmp(*attr, "style"))
				style = get_poly_id(lpTempMapDocument, *(attr + 1));
			attr += 2;
		}
		if (style == -1) {
			ErrorHandler("Currently you must give polygon style, no default");
			exit(1);
		}
		AddItemToPolygonlist(lpTempMapDocument, polylist[type].pglp,
				startx, starty, 0, 0, style, team, 0, FALSE);
		current_estyle = lpTempMapDocument->MapGeometry.pstyles[style].defedge_id;
		return;
    }


    if (!strcasecmp(el, "Option")) {
	AddOption(lpTempMapDocument, (char *)*(attr + 1), (char *)*(attr + 3), TRUE, TRUE);
	return;
    }
  
	if (!strcasecmp(el, "Base")) {
		while (*attr) {
			if (!strcasecmp(*attr, "team"))
				team = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1)) / 64;
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1)) / 64;
			if (!strcasecmp(*attr, "dir"))
				dir = atoi(*(attr + 1));
			attr += 2;
		}
			AddItemToItemlist(&lpTempMapDocument->MapGeometry.bases,
			x, y,
			team, dir, 0, IDM_MAP_BASE);
		return;
    }

    if (!strcasecmp(el, "Fuel")) {
		team = 99;
		while (*attr) {
			if (!strcasecmp(*attr, "team"))
				team = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1)) / 64;
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1)) / 64;
			attr += 2;
		}
			AddItemToItemlist(&lpTempMapDocument->MapGeometry.fuels,
			x, y,
			team, 0, 0, IDM_MAP_FUEL);
		return;
    }

	if (!strcasecmp(el, "Ball")) {
		while (*attr) {
			if (!strcasecmp(*attr, "team"))
				team = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1)) / 64;
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1)) / 64;
			attr += 2;
		}
			AddItemToItemlist(&lpTempMapDocument->MapGeometry.balls,
			x, y,
			team, 0, 0, IDM_MAP_BALL);
		return;
    }

	if (!strcasecmp(el, "Check")) {
		lpTempMapDocument->MapGeometry.num_checkpoints++;
		while (*attr) {
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1));
			attr += 2;
		}
			AddItemToItemlist(&lpTempMapDocument->MapGeometry.checkpoints,
			x, y,
			0, 0, lpTempMapDocument->MapGeometry.num_checkpoints, IDM_MAP_CHECKPOINT);
		return;
    }
/*    if (!strcasecmp(el, "Current")) {
		while (*attr) {
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "dir"))
				dir = atoi(*(attr + 1));
			attr += 2;
		}
			AddItemToItemlist(&lpTempMapDocument->MapGeometry.currents,
			x, y,
			0, dir, 0, IDM_MAP_CURRENT);
		return;
    }

	if (!strcasecmp(el, "Target")) {
		while (*attr) {
			if (!strcasecmp(*attr, "team"))
				team = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1));
			attr += 2;
		}
			AddItemToItemlist(&lpTempMapDocument->MapGeometry.targets,
			x, y,
			team, 0, 0, IDM_MAP_TARGET);
		return;
    }


	if (!strcasecmp(el, "Cannon")) {
		while (*attr) {
			if (!strcasecmp(*attr, "team"))
				team = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "dir"))
				dir = atoi(*(attr + 1));
			attr += 2;
		}
			AddItemToItemlist(&lpTempMapDocument->MapGeometry.cannons,
			x, y,
			team, dir, 0, IDM_MAP_CANNON);
		return;
    }
	if (!strcasecmp(el, "Concentrator")) {
		while (*attr) {
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1));
			attr += 2;
		}
			AddItemToItemlist(&lpTempMapDocument->MapGeometry.itemconcentrators,
			x, y,
			0, 0, 0, IDM_MAP_ITEM_CONC);
		return;
    }
	if (!strcasecmp(el, "Gravity")) {
		while (*attr) {
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "variant"))
				variant = atoi(*(attr + 1));
			attr += 2;
		}
			AddItemToItemlist(&lpTempMapDocument->MapGeometry.gravities,
			x, y,
			0, 0, variant, IDM_MAP_GRAVITY);
		return;
    }
	if (!strcasecmp(el, "CircularGravity")) {
		while (*attr) {
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "variant"))
				variant = atoi(*(attr + 1));
			attr += 2;
		}
			AddItemToItemlist(&lpTempMapDocument->MapGeometry.circulargravities,
			x, y,
			0, 0, variant, IDM_MAP_CIRCULAR_GRAVITY);
		return;
    }

	if (!strcasecmp(el, "Wormhole")) {
		while (*attr) {
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "variant"))
				variant = atoi(*(attr + 1));
			attr += 2;
		}
			AddItemToItemlist(&lpTempMapDocument->MapGeometry.wormholes,
			x, y,
			0, 0, variant, IDM_MAP_WORMHOLE);
		return;
    }*/



	if (!strcasecmp(el, "Offset")) {
		while (*attr) {
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1)) / 64;
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1)) / 64;
			if (!strcasecmp(*attr, "style"))
				edgestyle = get_edge_id(lpTempMapDocument, *(attr + 1));
			attr += 2;
		}
		if (edgestyle != -1 && edgestyle != current_estyle) {
			current_estyle = edgestyle;
		}
		AddItemToPolygonlist(lpTempMapDocument, polylist[type].pglp,
			x, y, 0, 0, current_estyle, team, 1, FALSE);
		return;
	}
    return;
}

static void tagend(void *data, const char *el)
{
    if (strcmp(el, "Polygon"))
	return;
	fCreatingPolygon = FALSE;
    return;
}

int Load_lines(int fd)
{
    char buff[8192];
    int len;
    XML_Parser p = XML_ParserCreate(NULL);

    if (!p) {
	ErrorHandler("Creating Expat instance for map parsing failed.\n");
	exit(1);
    }
    XML_SetElementHandler(p, tagstart, tagend);
    do {
	len = read(fd, buff, 8192);
	if (len < 0) {
	    ErrorHandler("Error reading map!");
	    return FALSE;
	}
	if (!XML_Parse(p, buff, len, !len)) {
	    ErrorHandler("Parse error at line %d:\n%s\n",
		  XML_GetCurrentLineNumber(p),
		  XML_ErrorString(XML_GetErrorCode(p)));
	    exit(1);
	}
    } while (len);
    return 1;
}
/***************************************************************************/
/* LoadMap                                                                 */
/* Arguments :                                                             */
/*   lpMapDocument: pointer to document data                                */
/*   pstrFileName                                                          */
/* Purpose :                                                               */
/***************************************************************************/
int LoadMap(LPMAPDOCUMENT lpMapDocument, char *pstrFileName) 
{
	FILE	*ifile = NULL;
	int		corrupted=0;
	int		fd;

	if (NULL == (ifile = fopen (pstrFileName, "rb")))
	{
		return FALSE ;
	}
	lpTempMapDocument = lpMapDocument;
    fd = fileno(ifile);
	
    Load_lines(fd);

	if (ifile) fclose(ifile); 
	if (corrupted)
	{
		ErrorHandler("%s : File is invalid or corrupted!", pstrFileName);
		return FALSE;
	}
	
	return TRUE;		
}

/***************************************************************************/
/* SaveMap                                                                 */
/* Arguments :                                                             */
/*   lpMapDocument: pointer to document data                                */
/*   pstrFileName                                                          */
/*   saveData                                                              */
/*   selected                                                              */
/* Purpose :                                                               */
/***************************************************************************/
int SaveMap(LPMAPDOCUMENT lpMapDocument, char * pstrFileName, int saveData, int selected)
{
	FILE *ofile = NULL;
	int n,i,j;
	time_t tim;
	char timestrn[26];
	polygonlist *pglp;
	itemlist *itmlp;
	
	if (NULL == (ofile = fopen (pstrFileName, "wb")))
	{
		ErrorHandler("Couldn't open file *%s* for writing!", pstrFileName);
		return FALSE ;
	}
	//Get the time and convert it to a cleaner syntax, overwriting the
	//including newline char.
	time(&tim);
	strcpy(timestrn, ctime(&tim));
	strcpy(timestrn+24, "\0");

	//Now output the document header.
	fprintf(ofile, "<XPilotMap>\n");
	fprintf(ofile,"<!-- Created by %s on %s -->\n",szAppName,timestrn);
	if (lpMapDocument->MapStruct.comments != NULL)
	{
	fprintf(ofile,"<!--");
		for (i = 0; (unsigned) i < strlen(lpMapDocument->MapStruct.comments); i++)
			if (lpMapDocument->MapStruct.comments[i] != '\n')
			{
			fprintf(ofile,"%c", lpMapDocument->MapStruct.comments[i]);
			}
			else 
			  fprintf(ofile,"\n");
	fprintf(ofile,"-->\n\n");
	}
	fprintf(ofile, "<GeneralOptions>\n");
	for ( n=0; n< NUMPREFS; n++ )
	{
		switch (lpMapDocument->PrefsArray[n].type)
		{
		case MAPWIDTH:					   
		case MAPHEIGHT:
		case STRING:
		case INT:
		case POSINT:
		case FLOAT:
		case POSFLOAT:
		case COORD:
		case LISTINT:
			if ( strlen(lpMapDocument->PrefsArray[n].charvar) != (int) NULL )
			  if ((lpMapDocument->PrefsArray[n].output == TRUE) ||
				  (SaveAllPrefs == TRUE))
			  {
				fprintf(ofile,"<Option name=\"%s\" value=\"",lpMapDocument->PrefsArray[n].name);
				for (j = 0; (unsigned) j < strlen(lpMapDocument->PrefsArray[n].charvar); j++)
				{
					switch (lpMapDocument->PrefsArray[n].charvar[j])
					{
					case '&':
						fprintf(ofile, "&amp;");
						break;
					case '"':
						fprintf(ofile, "&quot;");
						break;
					case '\'':
						fprintf(ofile, "&apos;");
						break;
					case '<':
						fprintf(ofile, "&lt;");
						break;
					case '>':
						fprintf(ofile, "&gt;");
						break;
					default:
						fprintf(ofile, "%c",
							lpMapDocument->PrefsArray[n].charvar[j]);
						break;
					}
				}
				fprintf(ofile, "\"/>\n");
			  }
			break;
		case YESNO:
			if ((lpMapDocument->PrefsArray[n].output == TRUE) ||
				(SaveAllPrefs == TRUE))
				if ( (*lpMapDocument->PrefsArray[n].intvar) == 0 ) 
					fprintf(ofile,"<Option name=\"%s\" value=\"no\"/>\n",
					lpMapDocument->PrefsArray[n].name);
				else
					fprintf(ofile,"<Option name=\"%s\" value=\"yes\"/>\n",
					lpMapDocument->PrefsArray[n].name);
			break;
		}
	}
	fprintf(ofile, "</GeneralOptions>\n");

	WriteStyles(ofile, lpMapDocument);

	//Walls
	if (lpMapDocument->MapGeometry.walls != NULL)
	{
		pglp = lpMapDocument->MapGeometry.walls;
		WritePolygonList(ofile, lpMapDocument, pglp, IDM_MAP_WALL);
	}
	
	//Decorations
	if (lpMapDocument->MapGeometry.decors != NULL)
	{
		pglp = lpMapDocument->MapGeometry.decors;
		WritePolygonList(ofile, lpMapDocument, pglp, IDM_MAP_DECOR);
	}
	

	//BallAreas
	if (lpMapDocument->MapGeometry.ballareas != NULL)
	{
		pglp = lpMapDocument->MapGeometry.ballareas;
		WritePolygonList(ofile, lpMapDocument, pglp, IDM_MAP_BALLAREA);
	}
	
	//BallTargets
	if (lpMapDocument->MapGeometry.balltargets != NULL)
	{
		pglp = lpMapDocument->MapGeometry.balltargets;
		WritePolygonList(ofile, lpMapDocument, pglp, IDM_MAP_BALLTARGET);
	}

	//Ball
	if (lpMapDocument->MapGeometry.balls != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.balls;
		WriteItemList(ofile, itmlp, IDM_MAP_BALL);
	}

	//Fuels
	if (lpMapDocument->MapGeometry.fuels != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.fuels;
		WriteItemList(ofile, itmlp, IDM_MAP_FUEL);
	}

	//Bases
	if (lpMapDocument->MapGeometry.bases != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.bases;
		WriteItemList(ofile, itmlp, IDM_MAP_BASE);
	}

	//Checkpoints
	if (lpMapDocument->MapGeometry.checkpoints != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.checkpoints;
		WriteItemList(ofile, itmlp, IDM_MAP_CHECKPOINT);
	}

/*	//Targets
	if (lpMapDocument->MapGeometry.targets != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.targets;
		WriteItemList(ofile, itmlp, IDM_MAP_TARGET);
	}
	
				
	//Cannons
	if (lpMapDocument->MapGeometry.cannons != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.cannons;
		WriteItemList(ofile, itmlp, IDM_MAP_CANNON);
	}*/
	
	
/*	//Currents
	if (lpMapDocument->MapGeometry.currents != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.currents;
		WriteItemList(ofile, itmlp, IDM_MAP_CURRENT);
	}
	
	//Item Concentrators
	if (lpMapDocument->MapGeometry.itemconcentrators != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.itemconcentrators;
		WriteItemList(ofile, itmlp, IDM_MAP_ITEM_CONC);
	}
	
	//Gravity Push/Pulls
	if (lpMapDocument->MapGeometry.gravities != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.gravities;
		WriteItemList(ofile, itmlp, IDM_MAP_GRAVITY);
	}
	
	//Gravity Circulars
	if (lpMapDocument->MapGeometry.circulargravities != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.circulargravities;
		WriteItemList(ofile, itmlp, IDM_MAP_CIRCULAR_GRAVITY);
	}
	
	//Wormholes
	if (lpMapDocument->MapGeometry.wormholes != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.wormholes;
		WriteItemList(ofile, itmlp, IDM_MAP_WORMHOLE);
	}*/

	//Output the footer and close the file.
	fprintf(ofile, "</XPilotMap>\n");
	fclose(ofile);

	strcpy(lpMapDocument->MapStruct.mapFileName, pstrFileName);
	return FALSE;
}
/***************************************************************************/
/* AddOption                                                               */
/* Arguments :                                                             */
/*   lpMapDocument: pointer to document data                                */
/*    *name                                                                */
/*    *value                                                               */
/*    output                                                               */
/*    loadData                                                             */
/* Purpose :                                                               */
/***************************************************************************/
int AddOption(LPMAPDOCUMENT lpMapDocument, char *name, char *value, int output, int loadData)
{
	int		option, i;
	char	*tmp = NULL;

	for (i=0; i < (int) strlen(name); i++)
	{
		if (isupper(name[i]))
			name[i] = tolower(name[i]);
	}

	option = FindOption(lpMapDocument, name);

	if ( option >= NUMPREFS )
	{
		if (lpMapDocument->MapStruct.comments == NULL)
		{
			lpMapDocument->MapStruct.comments = (char *) malloc(strlen(name)+strlen(value)+sizeof(int)*2);
			sprintf(lpMapDocument->MapStruct.comments,"%s:%s\n",name,value);
		}
		else
		{
			tmp = (char *) malloc(strlen(lpMapDocument->MapStruct.comments)+strlen(name)+strlen(value)+sizeof(int)*2);
			sprintf(tmp,"%s%s:%s\n",lpMapDocument->MapStruct.comments,name,value);
			free(lpMapDocument->MapStruct.comments);
			lpMapDocument->MapStruct.comments = tmp;
		}
		return 0;
	}
	if (output)
		lpMapDocument->PrefsArray[option].output = TRUE;

		switch(lpMapDocument->PrefsArray[option].type)
		{

		case MAPDATA:
//			if (loadData)
//			return (LoadMapData(lpMapDocument, value));
			return 0;
		case MAPWIDTH:
			lpMapDocument->width = atoi(value);
			strncpy(lpMapDocument->MapStruct.mapWidth,value,6);
			return 0;
			
		case MAPHEIGHT:
			lpMapDocument->height = atoi(value);
			strncpy(lpMapDocument->MapStruct.mapHeight,value,6);
			return 0;
			
		case STRING:
		case COORD:
			strncpy(lpMapDocument->PrefsArray[option].charvar,
				value,lpMapDocument->PrefsArray[option].length);
			return 0;
			
		case YESNO:
			(*(lpMapDocument->PrefsArray[option].intvar)) = YesNo(value);
			return 0;
			
		case INT:
		case POSINT:
		case FLOAT:
		case POSFLOAT:
		case LISTINT:
			strcpy(lpMapDocument->PrefsArray[option].charvar,
				StrToNum(value,lpMapDocument->PrefsArray[option].length,
				lpMapDocument->PrefsArray[option].type));
			return 0;
		}
		return 1;
}
/***************************************************************************/
/* YesNo                                                                   */
/* Arguments :                                                             */
/*   val                                                                   */
/* Purpose :                                                               */
/***************************************************************************/
int YesNo(char *val)
{
	if ( (tolower(val[0]) == 'y') || (tolower(val[0]) == 't') )
	return 1;

	return 0;
}

/***************************************************************************/
/* char *StrToNum                                                          */
/* Arguments :                                                             */
/*   string                                                                */
/*   len                                                                   */
/*   type                                                                  */
/* Purpose :                                                               */
/***************************************************************************/
char *StrToNum(char *string, int len, int type)
{
	char                  *returnval;
	
	returnval = (char *) malloc(len+1);
	returnval[0] = (char) NULL;
	
	if ( type == FLOAT || type == INT)
	{
		if ( (string[0] == '-') || ((string[0] >= '0') && (string[0] <= '9')) )
			sprintf(returnval,"%s%c",returnval,string[0]);
	}
	else if ( (string[0] == '-') || ((string[0] >= '0') &&
        (string[0] <= '9')) || (string[0] == '.') ) 
		sprintf(returnval,"%s%c",returnval,string[0]);
	
	string++;
	while ( (string[0] != (char) NULL) && ((int) strlen(returnval) <= (len-1) ) )
	{
		if ( type == FLOAT || type == POSFLOAT )
		{
            sprintf(returnval,"%s%c",returnval,string[0]);
		}
		else if ((string[0] >= '0') && (string[0] <= '9'))
			sprintf(returnval,"%s%c",returnval,string[0]);
		string++;
	}
	return (char *) returnval;
}
/***************************************************************************/
/* FindOption                                                              */
/* Arguments :                                                             */
/*   lpMapDocument: pointer to document data                                */
/*    *name                                                                */
/* Purpose :                                                               */
/***************************************************************************/
int FindOption(LPMAPDOCUMENT lpMapDocument, char *name)
{
	int                   option;

	for (option = 0; option < NUMPREFS; option++)
	{
		if(!strcmp(name, lpMapDocument->PrefsArray[option].name))
			break;
		/*If the first name isn't it, check the alternative*/
		if(!strcmp(name, lpMapDocument->PrefsArray[option].altname))
			break;
	}

	return option;
}
/***************************************************************************/
/* WritePolygonList                                                        */
/* Arguments :                                                             */
/*   ofile : file to write out to.                                         */
/*   pglp: pointer to the polygon to draw.                                 */
/*   type : the type of polygon list this is.                              */
/* Purpose : Write out the specified polygon.                              */
/***************************************************************************/
void WritePolygonList(FILE *ofile, LPMAPDOCUMENT lpMapDocument, polygonlist *pglp, int type)
{
	int i, count;
	int writescale=64;

	/*Special information tags that affect all polygons
	in this list are entered here.*/
	switch (type)
	{
		case IDM_MAP_WALL:
		fprintf(ofile, "<Walls>\n");
		break;
		case IDM_MAP_DECOR:
		fprintf(ofile, "<Decor>\n");
		break;
	}
	while (pglp != NULL)
	{
		// Dont Write out the polygon unless it has at least 3 points.
		if (pglp->num_verts >= 3)
		{
		/*Special information tags that affect only this particular
		polygon are entered here*/
		switch (type)
		{
		case IDM_MAP_BALLAREA:
			fprintf(ofile, "<BallArea team=\"%d\">\n", pglp->team);
			break;
		case IDM_MAP_BALLTARGET:
			fprintf(ofile, "<BallTarget team=\"%d\">\n", pglp->team);
			break;
		}

		if (!IsCounterClockwise(pglp))
			ReversePolygonOrientation(pglp);

			fprintf(ofile, "<Polygon x=\"%d\" y=\"%d\" style=\"%s\"\>\n",
				pglp->vertex[0].x*writescale, pglp->vertex[0].y*writescale,
				lpMapDocument->MapGeometry.pstyles[pglp->polygon_style].id);
			
			for(i = 1; i< pglp->num_verts; i++)
			{
				fprintf(ofile, "<Offset x=\"%d\" y=\"%d\"",
					pglp->vertex[i].delta_x*writescale, pglp->vertex[i].delta_y*writescale);
				if (pglp->vertex[i].edge_style != pglp->vertex[i-1].edge_style)
					fprintf(ofile, " style=\"%s\"",
					lpMapDocument->MapGeometry.estyles[pglp->vertex[i].edge_style].id);
				fprintf(ofile, "/>\n");
			}
			//Close out the polygon Tag
			fprintf(ofile, "</Polygon>\n");
			switch (type)
			{
			case IDM_MAP_BALLAREA:
				fprintf(ofile, "</BallArea>\n");
				break;
			case IDM_MAP_BALLTARGET:
				fprintf(ofile, "</BallTarget>\n");
				break;
			}
		}
		pglp = pglp->next;
	}
	/*Close out the list if necessary*/
	switch (type)
	{
		case IDM_MAP_WALL:
		fprintf(ofile, "</Walls>\n");
		break;
		case IDM_MAP_DECOR:
		fprintf(ofile, "</Decor>\n");
		break;
	}
}
/***************************************************************************/
/* WriteItemList                                                           */
/* Arguments :                                                             */
/*   ofile : file to write out to.                                         */
/*   pglp: pointer to the polygon to draw.                                 */
/*   type : the type of item list this is.                                 */
/* Purpose : Write out the specified polygon.                              */
/***************************************************************************/
void WriteItemList(FILE *ofile, itemlist *itmlp, int type)
{
	int writescale=64;

	while (itmlp != NULL)
	{
		switch (type){
		case IDM_MAP_BASE:
			fprintf(ofile, "<Base x=\"%d\" y=\"%d\" team=\"%d\" dir=\"%d\"/>\n",
				itmlp->pos.x*writescale, itmlp->pos.y*writescale,
				itmlp->team, itmlp->direction);
			break;
		case IDM_MAP_FUEL:
			fprintf(ofile, "<Fuel x=\"%d\" y=\"%d\" team=\"%d\"/>\n",
				itmlp->pos.x*writescale, itmlp->pos.y*writescale,
				itmlp->team);
			break;
		case IDM_MAP_BALL:
			fprintf(ofile, "<Ball x=\"%d\" y=\"%d\" team=\"%d\"/>\n",
				itmlp->pos.x*writescale, itmlp->pos.y*writescale,
				itmlp->team);
			break;
		case IDM_MAP_CHECKPOINT:
			fprintf(ofile, "<Check x=\"%d\" y=\"%d\"/>\n",
				itmlp->pos.x*writescale, itmlp->pos.y*writescale);
			break;
/*		case IDM_MAP_CURRENT:
			fprintf(ofile, "<Current x=\"%d\" y=\"%d\" dir=\"%d\"/>\n", 
				itmlp->pos.x, itmlp->pos.y,
				itmlp->direction);
			break;
		case IDM_MAP_TARGET:
			fprintf(ofile, "<Target x=\"%d\" y=\"%d\" team=\"%d\"/>\n",
				itmlp->pos.x, itmlp->pos.y,
				itmlp->team);
			break;
		case IDM_MAP_CANNON:
			fprintf(ofile, "<Cannon x=\"%d\" y=\"%d\" team=\"%d\" dir=\"%d\"/>\n",
				itmlp->pos.x, itmlp->pos.y,
				itmlp->team, itmlp->direction);
			break;
		case IDM_MAP_ITEM_CONC:
			fprintf(ofile, "<Concentrator x=\"%d\" y=\"%d\"/>\n",
				itmlp->pos.x, itmlp->pos.y);
			break;
		case IDM_MAP_GRAVITY:
			fprintf(ofile, "<Gravity x=\"%d\" y=\"%d\" variant=\"%d\"/>\n",
				itmlp->pos.x, itmlp->pos.y,
				itmlp->variant);
			break;
		case IDM_MAP_CIRCULAR_GRAVITY:
			fprintf(ofile, "<CircularGravity x=\"%d\" y=\"%d\" variant=\"%d\"/>\n",
				itmlp->pos.x, itmlp->pos.y,
				itmlp->variant);
			break;
		case IDM_MAP_WORMHOLE:
			fprintf(ofile, "<Wormhole x=\"%d\" y=\"%d\" variant=\"%d\"/>\n",
				itmlp->pos.x, itmlp->pos.y,
				itmlp->variant);
			break;*/
		}
		itmlp = itmlp->next;
	}
}
/***************************************************************************/
/* WriteStyles                                                             */
/* Arguments :                                                             */
/*   ofile : file to write out to.                                         */
/*   lpMapDocument: pointer to document data                               */
/* Purpose : Write out the edge, polygon, and bitmap styles.               */
/***************************************************************************/
void WriteStyles(FILE *ofile, LPMAPDOCUMENT lpMapDocument)
{
	int i;

	for (i = 1; i < lpMapDocument->MapGeometry.num_estyles; i++)
		fprintf(ofile, "<Edgestyle id=\"%s\" width=\"%d\" color=\"%x\" style=\"%d\"/>\n",
		lpMapDocument->MapGeometry.estyles[i].id,
		lpMapDocument->MapGeometry.estyles[i].width,
		lpMapDocument->MapGeometry.estyles[i].color,
		lpMapDocument->MapGeometry.estyles[i].style);

	for (i = 0; i < lpMapDocument->MapGeometry.num_bstyles; i++)
	{
		fprintf(ofile, "<Bmpstyle filename=\"%s\" scalable=\"",
		lpMapDocument->MapGeometry.bstyles[i].filename);
		if (lpMapDocument->MapGeometry.bstyles[i].flags)
			fprintf(ofile, "yes");
		else
			fprintf(ofile, "no");
		fprintf(ofile, "\" />\n");
	}

	for (i = 0; i < lpMapDocument->MapGeometry.num_pstyles; i++)
		fprintf(ofile, "<Polystyle id=\"%s\" color=\"%x\" defedge=\"%s\" flags=\"%d\" />\n",
		lpMapDocument->MapGeometry.pstyles[i].id,
		lpMapDocument->MapGeometry.pstyles[i].color,
		lpMapDocument->MapGeometry.estyles[lpMapDocument->MapGeometry.pstyles[i].defedge_id].id,
		lpMapDocument->MapGeometry.pstyles[i].flags);
}