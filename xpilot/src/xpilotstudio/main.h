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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <memory.h>
#include <math.h>
#include <float.h>
#include <process.h>
#include <limits.h>
#include <stdarg.h>
#include <io.h>

/*Include the expat XML Library
This must be installed on your system
and is not part of most standard libraries*/
#include <expat.h>

#include "const.h"
#include "objects.h"
#include "default_colors.h"
#include "proto.h"
#include "map.h"



static char szAppName[] = "XpilotStudio";


//The number of the last team base entered
extern int teamSet;
extern int dirSet;
extern int polaritySet;
extern int variantSet;

//The currently selected buttons
extern int iSelectionMapSyms;
//extern int iSelectionMapTools;
extern int iSelectionShape;
extern int iSelectionWormhole;
extern int iSelectionPolarity;
extern int iSelectionMapModify;
extern int bSegHidden;

extern int iSelectionShipTools;
extern int iSelectionShipSyms;

extern int fDrawing;
extern int fCreatingPolygon;
extern int fDragging;
extern int fReordering;

extern unsigned char *xpilotmap_dir;

extern char *Errors;

#ifdef _WINDOWS
#define	strcasecmp(__a, __b)	stricmp(__a, __b)
#define	strncasecmp(__a, __b, __c)	strnicmp(__a, __b, __c)
#endif
