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

/***************************************************************************/
/* GetSetEnterableInfo                                                     */
/* Arguments :                                                             */
/* Purpose : Retrieve the setable information for teams, like number,angle */
/***************************************************************************/
void GetSetEnterableInfo(){
	int i;
//Todo: Implement for linux
#ifdef _WINDOWS
	i = SendMessage(hwndUpDownTeam, UDM_GETPOS, 0, 0);
#endif
	teamSet = i;

#ifdef _WINDOWS
	i = SendMessage(hwndUpDownDirection, UDM_GETPOS, 0, 0);
#endif
	dirSet = i;



	switch(iSelectionMapSyms)
	{
	case IDM_MAP_WORMHOLE:
		variantSet = iSelectionWormhole - IDM_MAP_WORM_NORMAL;
		break;
	case IDM_MAP_GRAVITY:
	case IDM_MAP_CIRCULAR_GRAVITY:
		variantSet = iSelectionPolarity - IDM_MAP_POSITIVE;
		break;
	case IDM_MAP_WALL:
	case IDM_MAP_BALLTARGET:
	case IDM_MAP_BALLAREA:
	case IDM_MAP_DECOR:
		variantSet = iSelectionWallType;
		break;
	default:
		variantSet = 0;
		break;
	}
}
