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
    int x, y, dir, variant, hidden;

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
 	if (!strcasecmp(el, "Decorations")) {
		type = 3;
    }
   
	if (!strcasecmp(el, "Polygon")) {
		while (*attr) {
			if (!strcasecmp(*attr, "x"))
				startx = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "y"))
				starty = atoi(*(attr + 1));
			attr += 2;
		}
		AddItemToPolygonlist(lpTempMapDocument, polylist[type].pglp,
				startx, starty, 0, 0, team, 0, 0, FALSE);
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
				x = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "dir"))
				dir = atoi(*(attr + 1));
			attr += 2;
		}
			AddItemToItemlist(&lpTempMapDocument->MapGeometry.bases,
			x, y,
			team, dir, 0, IDM_MAP_BASE);
		lpTempMapDocument->MapGeometry.num_bases++;
		return;
    }

    if (!strcasecmp(el, "Current")) {
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
    if (!strcasecmp(el, "Fuel")) {
		while (*attr) {
			if (!strcasecmp(*attr, "team"))
				team = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1));
			attr += 2;
		}
			AddItemToItemlist(&lpTempMapDocument->MapGeometry.fuels,
			x, y,
			team, 0, 0, IDM_MAP_FUEL);
		lpTempMapDocument->MapGeometry.num_fuels++;
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

	if (!strcasecmp(el, "Ball")) {
		while (*attr) {
			if (!strcasecmp(*attr, "team"))
				team = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1));
			attr += 2;
		}
			AddItemToItemlist(&lpTempMapDocument->MapGeometry.balls,
			x, y,
			team, 0, 0, IDM_MAP_BALL);
		lpTempMapDocument->MapGeometry.num_balls++;
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

	if (strcasecmp(el, "Offset"))
	return;
		while (*attr) {
			if (!strcasecmp(*attr, "x"))
				x = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "y"))
				y = atoi(*(attr + 1));
			if (!strcasecmp(*attr, "hidden"))
			{
				if (!strcasecmp(*(attr + 1), "yes"))
					hidden = 1;
				else
					hidden = 0;
			}
			attr += 2;
		}
		AddItemToPolygonlist(lpTempMapDocument, polylist[type].pglp,
		x, y, 0, 0, team, hidden, 1, FALSE);
    return;
}

static void tagend(void *data, const char *el)
{
    if (strcmp(el, "Polygon"))
	return;
	fCreatingPolygon = FALSE;
//	type = team = 0;
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
	fprintf(ofile,"<Featurecount bases=\"%d\" balls=\"%d\" fuels=\"%d\" checks=\"%d\"/>\n",
		lpMapDocument->MapGeometry.num_bases,
		lpMapDocument->MapGeometry.num_balls,
		lpMapDocument->MapGeometry.num_fuels,
		lpMapDocument->MapGeometry.num_checkpoints);
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

	//Walls
	if (lpMapDocument->MapGeometry.walls != NULL)
	{
		pglp = lpMapDocument->MapGeometry.walls;
		WritePolygonList(ofile, pglp, IDM_MAP_WALL);
	}
	
	//Decorations
	if (lpMapDocument->MapGeometry.decors != NULL)
	{
		pglp = lpMapDocument->MapGeometry.decors;
		WritePolygonList(ofile, pglp, IDM_MAP_DECOR);
	}
	
	//Ball
	if (lpMapDocument->MapGeometry.balls != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.balls;
		WriteItemList(ofile, itmlp, IDM_MAP_BALL);
	}

	//BallAreas
	if (lpMapDocument->MapGeometry.ballareas != NULL)
	{
		pglp = lpMapDocument->MapGeometry.ballareas;
		WritePolygonList(ofile, pglp, IDM_MAP_BALLAREA);
	}
	
	//BallTargets
	if (lpMapDocument->MapGeometry.balltargets != NULL)
	{
		pglp = lpMapDocument->MapGeometry.balltargets;
		WritePolygonList(ofile, pglp, IDM_MAP_BALLTARGET);
	}
	
	//Targets
	if (lpMapDocument->MapGeometry.targets != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.targets;
		WriteItemList(ofile, itmlp, IDM_MAP_TARGET);
	}
	
	//Fuels
	if (lpMapDocument->MapGeometry.fuels != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.fuels;
		WriteItemList(ofile, itmlp, IDM_MAP_FUEL);
	}
				
	//Cannons
	if (lpMapDocument->MapGeometry.cannons != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.cannons;
		WriteItemList(ofile, itmlp, IDM_MAP_CANNON);
	}
	
	//Bases
	if (lpMapDocument->MapGeometry.bases != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.bases;
		WriteItemList(ofile, itmlp, IDM_MAP_BASE);
	}
	
	//Currents
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
	}

	//Checkpoints
	if (lpMapDocument->MapGeometry.checkpoints != NULL)
	{
		itmlp = lpMapDocument->MapGeometry.checkpoints;
		WriteItemList(ofile, itmlp, IDM_MAP_CHECKPOINT);
	}
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
			if (loadData)
			return (LoadMapData(lpMapDocument, value));
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
/* LoadMapData                                                             */
/* Arguments :                                                             */
/*   lpMapDocument: pointer to document data                                */
/*   value                                                                 */
/* Purpose :                                                               */
/***************************************************************************/
int LoadMapData(LPMAPDOCUMENT lpMapDocument, char *value)
{
/*	int                   x=0, y=0; 
	
	while ( *value != '\0' )
	{
		if ( *value == '\n' )
		{
			x = 0;
			y ++;
		}
		else
		{
			lpMapDocument->MapStruct.data[x++][y].cdata = *value;
		}
		value ++;
	}
	CountBases(lpMapDocument);
	CountCheckPoints(lpMapDocument);*/
	return 0;
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
/*   pglp: pointer to the polygon to draw.                                 */
/* Purpose : Write out the specified polygon.                              */
/***************************************************************************/
void WritePolygonList(FILE *ofile, polygonlist *pglp, int type)
{
	int i, count;

	/*Special information tags that affect all polygons
	in this list are entered here.*/
	switch (type)
	{
		case IDM_MAP_WALL:
		fprintf(ofile, "<Walls>\n");
		break;
		case IDM_MAP_DECOR:
		fprintf(ofile, "<Decorations>\n");
		break;
	}
	while (pglp != NULL)
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

		fprintf(ofile, "<Polygon x=\"%d\" y=\"%d\"",
			pglp->vertex[0].x, pglp->vertex[0].y);
		//Are there any hidden edges?
		count = CountEdgesOfType(&pglp, IDM_MAP_HIDDEN);
		if (count > 0)
			fprintf(ofile, " hidedges=\"%d\"", count);
		//ToDo: Add counts and attributes for other types of edges here.

		//Close the tag.
		fprintf(ofile, ">\n");

		for(i = 1; i< pglp->num_verts; i++)
		{
			fprintf(ofile, "<Offset x=\"%d\" y=\"%d\"", pglp->vertex[i].delta_x, pglp->vertex[i].delta_y);
			if (pglp->vertex[i].hidden)
				fprintf(ofile, " hidden=\"yes\"");

			fprintf(ofile, "/>\n");
		}
		/*Close out the polygon Tag*/
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
		pglp = pglp->next;
	}
	/*Close out the list if necessary*/
	switch (type)
	{
		case IDM_MAP_WALL:
		fprintf(ofile, "</Walls>\n");
		break;
		case IDM_MAP_DECOR:
		fprintf(ofile, "</Decorations>\n");
		break;
	}
}
/***************************************************************************/
/* WriteItemList                                                           */
/* Arguments :                                                             */
/*   pglp: pointer to the polygon to draw.                                 */
/* Purpose : Write out the specified polygon.                              */
/***************************************************************************/
void WriteItemList(FILE *ofile, itemlist *itmlp, int type)
{
	while (itmlp != NULL)
	{
		switch (type){
		case IDM_MAP_BASE:
			fprintf(ofile, "<Base x=\"%d\" y=\"%d\" team=\"%d\" dir=\"%d\"/>\n",
				itmlp->pos.x, itmlp->pos.y,
				itmlp->team, itmlp->direction);
			break;
		case IDM_MAP_CURRENT:
			fprintf(ofile, "<Current x=\"%d\" y=\"%d\" dir=\"%d\"/>\n", 
				itmlp->pos.x, itmlp->pos.y,
				itmlp->direction);
			break;
		case IDM_MAP_FUEL:
			fprintf(ofile, "<Fuel x=\"%d\" y=\"%d\" team=\"%d\"/>\n",
				itmlp->pos.x, itmlp->pos.y,
				itmlp->team);
			break;
		case IDM_MAP_TARGET:
			fprintf(ofile, "<Target x=\"%d\" y=\"%d\" team=\"%d\"/>\n",
				itmlp->pos.x, itmlp->pos.y,
				itmlp->team);
			break;
		case IDM_MAP_BALL:
			fprintf(ofile, "<Ball x=\"%d\" y=\"%d\" team=\"%d\"/>\n",
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
			break;
		case IDM_MAP_CHECKPOINT:
			fprintf(ofile, "<Check x=\"%d\" y=\"%d\"/>\n",
				itmlp->pos.x, itmlp->pos.y);
			break;
		}
		itmlp = itmlp->next;
	}
}