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

#include "win_xpstudio.h"

extern HINSTANCE hInst;
DWORD dwChildStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CAPTION ;


/***************************************************************************/
/* PropSheetProc                                                           */
/* Arguments :                                                             */
/*    hwhndDlg                                                             */
/*   uMsg                                                                  */
/*   lParam                                                                */
/*                                                                         */
/* Purpose :   Main Prop Sheet procedure                                   */
/***************************************************************************/

int CALLBACK PropSheetProc (HWND hwhndDlg, UINT uMsg, LPARAM lParam)
{
	switch(uMsg)
	{
	case PSCB_INITIALIZED :
		// Process PSCB_INITIALIZED	
		break ;
		
	case PSCB_PRECREATE :
		// Process PSCB_PRECREATE
		break ;
		
	default :
		// Unknown message
		break ;
	}
	return FALSE;
}
/***************************************************************************/
/* CreatePropertySheet                                                     */
/* Arguments :                                                             */
/*    hwndParent                                                           */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/* Purpose :   Creates the main property sheet                             */
/***************************************************************************/
BOOL CreatePropertySheet (HWND hwndParent)
{
	int i, numsheets = 9;
	int a[9] = {IDD_MAPDATA,
		IDD_MAPDATA2,
		IDD_MAPDATA3,
		IDD_MAPDATA4,
		IDD_MAPDATA5,
		IDD_INITITEMS,
		IDD_MAXITEMS,
		IDD_PROBS,
		IDD_SCRMULT
	};
	PROPSHEETHEADER pshead ;
	PROPSHEETPAGE   pspage[10] ;


	
	// Initialize property sheet HEADER data
	ZeroMemory (&pshead, sizeof (PROPSHEETHEADER)) ;
	pshead.dwSize  = sizeof (PROPSHEETHEADER) ;
	pshead.dwFlags = PSH_PROPSHEETPAGE |
		PSH_USECALLBACK | PSH_NOAPPLYNOW ;
	pshead.hwndParent  = hwndParent ;
	pshead.hInstance   = hInst ;
	pshead.hIcon       = LoadIcon (hInst, MAKEINTRESOURCE(IDI_MAINICON)) ;
	pshead.pszCaption  = "Map Preferences" ;
	pshead.nPages      = numsheets ;
	pshead.nStartPage  = 0 ;
	pshead.ppsp        = pspage ;
	pshead.pfnCallback = PropSheetProc ;
	
	// Zero out property PAGE data
	ZeroMemory (&pspage, (numsheets+1) * sizeof (PROPSHEETPAGE)) ;

	for (i = 0; i < numsheets; i++)
	{
		pspage[i].dwSize      = sizeof (PROPSHEETPAGE) ;
		pspage[i].dwFlags     = PSP_USECALLBACK | PSP_USEICONID ;
		pspage[i].hInstance   = hInst ;
		pspage[i].pszTemplate = MAKEINTRESOURCE (a[i]);
		pspage[i].pfnDlgProc  = PrefsDefaultDlgProc ;
		pspage[i].lParam      = (LPARAM) &dwChildStyle ;
		pspage[i].pfnCallback = PrefsPageProc ;
	}
	//Comments page
	pspage[numsheets].dwSize      = sizeof (PROPSHEETPAGE) ;
	pspage[numsheets].dwFlags     = PSP_USECALLBACK | PSP_USEICONID ;
	pspage[numsheets].hInstance   = hInst ;
	pspage[numsheets].pszTemplate = MAKEINTRESOURCE (IDD_COMMENTS) ;
	pspage[numsheets].pfnDlgProc  = PrefsCommentsDlgProc ;
	pspage[numsheets].lParam      = (LPARAM) &dwChildStyle ;
	pspage[numsheets].pfnCallback = PrefsPageProc ;	
	
	return PropertySheet (&pshead) ;
}
/***************************************************************************/
/* PrefsPageProc                                                          */
/* Arguments :                                                             */
/*    hwnd                                                                 */
/*    uMsg                                                                 */
/*    ppsp                                                                 */
/*                                                                         */
/* Purpose :   The first prefs page proc                                   */
/***************************************************************************/
UINT CALLBACK 
PrefsPageProc (HWND  hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
	switch (uMsg)
	{
	case PSPCB_CREATE :
		return TRUE ;
		
	case PSPCB_RELEASE :
		return 0;
	}
	
	return 0 ;
}
/***************************************************************************/
/* PrefsDefaultDlgProc                                                     */
/* Arguments :                                                             */
/*    hwndDlg                                                              */
/*    msg                                                                  */
/*    wParam                                                               */
/*    lParam                                                               */
/* Purpose :   The first prefs page dialog procedure                       */
/***************************************************************************/
BOOL CALLBACK 
PrefsDefaultDlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int count;
	max_str_t tempc;
	int tempi;
	LPXPSTUDIODOCUMENT lpXpStudioDocument;
	LPMAPDOCUMENT lpMapDocument ;
	
	switch (msg)
	{
	case WM_INITDIALOG :
		{
			lpXpStudioDocument = (LPXPSTUDIODOCUMENT) GetWindowLong (hwndActive, GWL_USERDATA);
			lpMapDocument = lpXpStudioDocument->lpMapDocument;

			for (count = 0; count < NUMPREFS; count++)
			{
				switch (lpMapDocument->PrefsArray[count].type)
				{
				case YESNO : 
						SetSimpleCheck (hwndDlg, lpMapDocument->PrefsArray[count].id1, 0, *lpMapDocument->PrefsArray[count].intvar);
					break;
				case MAPWIDTH:
				case MAPHEIGHT:	
				case STRING:
				case COORD:
				case INT:
				case POSINT:
				case FLOAT:
				case POSFLOAT: 
					SetEditText (hwndDlg, lpMapDocument->PrefsArray[count].id1, lpMapDocument->PrefsArray[count].charvar);
					break;
				case LISTINT:
					InitListBox (hwndDlg, lpMapDocument->PrefsArray[count].id1, lpMapDocument->PrefsArray[count].charvar);
					break;
				}
			}
			return TRUE ;
		}
		
	case WM_COMMAND :
		{
			return TRUE ;
		}
	case WM_NOTIFY :
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			lpXpStudioDocument = (LPXPSTUDIODOCUMENT) GetWindowLong (hwndActive, GWL_USERDATA);
			lpMapDocument = lpXpStudioDocument->lpMapDocument;

			
			if (pnmh->code == PSN_APPLY)
			{
				for (count = 0; count < NUMPREFS; count++)
				{
					switch (lpMapDocument->PrefsArray[count].type)
					{
					case YESNO :/*Verify that we can find the control, so we dont mistakenly
								clear good values we don't mean to.*/
						if (GetDlgItem(hwndDlg, lpMapDocument->PrefsArray[count].id1))
						{
							tempi = UpdateMapPrefCheck(hwndDlg, lpMapDocument->PrefsArray[count].id1);
							if (tempi != *lpMapDocument->PrefsArray[count].intvar)
							{
								*lpMapDocument->PrefsArray[count].intvar = tempi;
								lpMapDocument->PrefsArray[count].output = TRUE;

						}
						}
						break;
						

				case MAPWIDTH:
						if (GetDlgItemText(hwndDlg, lpMapDocument->PrefsArray[count].id1, tempc, 255))

						if ( strcmp(tempc, lpMapDocument->PrefsArray[count].charvar) )
							{
								strcpy(lpMapDocument->PrefsArray[count].charvar, tempc);
								if (atoi(lpMapDocument->PrefsArray[count].charvar) > MAX_MAP_SIZE)
									itoa(MAX_MAP_SIZE, lpMapDocument->PrefsArray[count].charvar, 10);
								if (atoi(lpMapDocument->PrefsArray[count].charvar) < 1)
									itoa(1, lpMapDocument->PrefsArray[count].charvar, 10);
								lpMapDocument->width = atoi(lpMapDocument->PrefsArray[count].charvar);
							}
						break;
						
				case MAPHEIGHT:	
					if (GetDlgItemText(hwndDlg, lpMapDocument->PrefsArray[count].id1, tempc, 255))
						if ( strcmp(tempc, lpMapDocument->PrefsArray[count].charvar) )
						{
							strcpy(lpMapDocument->PrefsArray[count].charvar, tempc);
							if (atoi(lpMapDocument->PrefsArray[count].charvar) > MAX_MAP_SIZE)
								itoa(MAX_MAP_SIZE, lpMapDocument->PrefsArray[count].charvar, 10);
							if (atoi(lpMapDocument->PrefsArray[count].charvar) < 1)
								itoa(1, lpMapDocument->PrefsArray[count].charvar, 10);
							lpMapDocument->height = atoi(lpMapDocument->PrefsArray[count].charvar);
						}
						break;
						
				case STRING:
				case COORD:
				case INT:
				case POSINT:
				case FLOAT:
				case POSFLOAT:
					if (GetDlgItemText(hwndDlg, lpMapDocument->PrefsArray[count].id1, tempc, 255))
						if ( strcmp(tempc, lpMapDocument->PrefsArray[count].charvar) )
						{
							strcpy(lpMapDocument->PrefsArray[count].charvar, tempc);
							lpMapDocument->PrefsArray[count].output = TRUE;
						}
						break;
				case LISTINT:
					if (GetDlgItem(hwndDlg, lpMapDocument->PrefsArray[count].id1))
					{
						UpdateMapPrefList(hwndDlg, lpMapDocument->PrefsArray[count].id1, tempc);
						if (strcmp(tempc, lpMapDocument->PrefsArray[count].charvar))
						{
							strcpy(lpMapDocument->PrefsArray[count].charvar, tempc);
							lpMapDocument->PrefsArray[count].output = TRUE;
						}
					}
					break;
					
					}
				}
				lpMapDocument->changed = 1;
			}
			return TRUE ;
		}
	
	default :
		return FALSE ;
    }
}
/***************************************************************************/
/* PrefsCommentsDlgProc                                                    */
/* Arguments :                                                             */
/*    hwndDlg                                                              */
/*    msg                                                                  */
/*    wParam                                                               */
/*    lParam                                                               */
/* Purpose :   The Comments page dialog procedure                          */
/***************************************************************************/
BOOL CALLBACK 
PrefsCommentsDlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char  *comments_from=NULL, *tmp=NULL;
	unsigned i, j, count =0;
	LPXPSTUDIODOCUMENT lpXpStudioDocument;
	LPMAPDOCUMENT lpMapDocument ;
	
	switch (msg)
	{
	case WM_INITDIALOG :
		{
			lpXpStudioDocument = (LPXPSTUDIODOCUMENT) GetWindowLong (hwndActive, GWL_USERDATA);
			lpMapDocument = lpXpStudioDocument->lpMapDocument;
			comments_from = NULL;
			tmp = NULL;
			
			/*allocate the memory for the comments*/
			if (lpMapDocument->MapStruct.comments != NULL)
			{
				comments_from = (char *) malloc(sizeof(char)*2);
				sprintf(comments_from,"\000");
				
				for (i = 0; i < strlen(lpMapDocument->MapStruct.comments); i++) 
				{
					if (lpMapDocument->MapStruct.comments[i] != '\n')
					{
						tmp = (char *) malloc(strlen(comments_from)+2);
						sprintf(tmp,"%s%c",comments_from,lpMapDocument->MapStruct.comments[i]);
					}
					else
					{
						tmp = (char *) malloc(strlen(comments_from)+4);
						sprintf(tmp,"%s\r%c",comments_from,lpMapDocument->MapStruct.comments[i]);
					}
						free(comments_from);
						comments_from = tmp;
				}
			}
			if (comments_from != NULL)
				SetEditText (hwndDlg, IDC_COMMENTS, comments_from);
			
			return TRUE ;
		}
		
	case WM_COMMAND :
		{
			
			return TRUE ;
		}
	case WM_NOTIFY :
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			lpXpStudioDocument = (LPXPSTUDIODOCUMENT) GetWindowLong (hwndActive, GWL_USERDATA);
			lpMapDocument = lpXpStudioDocument->lpMapDocument;
			
			if (pnmh->code == PSN_APPLY)
			{
				comments_from = (char *) malloc((count = GetWindowTextLength(GetDlgItem (hwndDlg, IDC_COMMENTS))+1)*sizeof(char) );
				Edit_GetText(GetDlgItem (hwndDlg, IDC_COMMENTS), comments_from, count);

				tmp = (char *) malloc(sizeof(char));
				for (i = 0, j=0; i < count; i++)
				{
					if (comments_from[i] == '\r')
						i++;

					tmp[j] = comments_from[i];
					j++;

					tmp = (char *) realloc(tmp, j * sizeof(char)+sizeof(char));
				}			
				
				free(lpMapDocument->MapStruct.comments);
				free(comments_from);
				lpMapDocument->MapStruct.comments = tmp;
			}
			return TRUE ;
		}
		
	default :
		return FALSE ;
	}
}
