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
#else
#include "main.h"
#endif
char *Errors;
/***************************************************************************/
/* ErrorHandler                                                            */
/* Arguments :                                                             */
/*   szMessage - error messge to be output                                 */
/*   ... - variable number of arguments, subsituted for %s, %d, etc.       */
/* Purpose : Display an Error notification box with the specified message  */
/***************************************************************************/
void ErrorHandler(char *szMessage, ...)
{
	va_list marker; 
	char szBuffer[512];
	
	// Figure through the extra arguments.
	va_start(marker, szMessage);
	vsprintf(szBuffer, szMessage, marker);
	va_end(marker);              
	
	// Display the error message.
#ifdef _WINDOWS
	MessageBox(NULL, szBuffer, "XPilotStudio Error", MB_ICONEXCLAMATION | MB_OK);
#else
	;/*Unix Messaging routine here...something similar to MessageBox!*/
#endif
} 
