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
#define C_COLUMNS 5 
typedef struct myitem_tag {
    LPSTR aCols[C_COLUMNS];
 } MYITEM; 

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
	int i, numsheets = 11;
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
	PROPSHEETPAGE   pspage[11] ;


	
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
	ZeroMemory (&pspage, numsheets * sizeof (PROPSHEETPAGE)) ;

	for (i = 0; i < numsheets-2; i++)
	{
		pspage[i].dwSize      = sizeof (PROPSHEETPAGE) ;
		pspage[i].dwFlags     = PSP_USECALLBACK | PSP_USEICONID ;
		pspage[i].hInstance   = hInst ;
		pspage[i].pszTemplate = MAKEINTRESOURCE (a[i]);
		pspage[i].pfnDlgProc  = PrefsDefaultDlgProc ;
		pspage[i].lParam      = (LPARAM) &dwChildStyle ;
		pspage[i].pfnCallback = PrefsPageProc ;
	}
	//Types page
	pspage[i].dwSize      = sizeof (PROPSHEETPAGE) ;
	pspage[i].dwFlags     = PSP_USECALLBACK | PSP_USEICONID ;
	pspage[i].hInstance   = hInst ;
	pspage[i].pszTemplate = MAKEINTRESOURCE (IDD_TYPESEDITOR) ;
	pspage[i].pfnDlgProc  = PrefsTypesDlgProc ;
	pspage[i].lParam      = (LPARAM) &dwChildStyle ;
	pspage[i].pfnCallback = PrefsPageProc ;	
	i++;
	
	//Comments page
	pspage[i].dwSize      = sizeof (PROPSHEETPAGE) ;
	pspage[i].dwFlags     = PSP_USECALLBACK | PSP_USEICONID ;
	pspage[i].hInstance   = hInst ;
	pspage[i].pszTemplate = MAKEINTRESOURCE (IDD_COMMENTS) ;
	pspage[i].pfnDlgProc  = PrefsCommentsDlgProc ;
	pspage[i].lParam      = (LPARAM) &dwChildStyle ;
	pspage[i].pfnCallback = PrefsPageProc ;	
	
	
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
/***************************************************************************/
/* PrefsTypesDlgProc                                                       */
/* Arguments :                                                             */
/*    hDlg                                                                 */
/*    iMsg                                                                 */
/*    wParam                                                               */
/*    lParam                                                               */
/* Purpose :   The procedure for the types editing dialog box              */
/***************************************************************************/
BOOL CALLBACK PrefsTypesDlgProc(HWND hwndDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	LPXPSTUDIODOCUMENT lpXpStudioDocument;
	LPMAPDOCUMENT lpMapDocument ;
	int n;

	switch(iMsg)
	{
	case WM_INITDIALOG :
		lpXpStudioDocument = (LPXPSTUDIODOCUMENT) GetWindowLong (hwndActive, GWL_USERDATA);
		lpMapDocument = lpXpStudioDocument->lpMapDocument;
		
		ComboBox_AddString(GetDlgItem (hwndDlg, IDC_TYPESELECTOR), "Edges\0");
		ComboBox_AddString(GetDlgItem (hwndDlg, IDC_TYPESELECTOR), "Polygons\0");
		
		ComboBox_SetCurSel(GetDlgItem (hwndDlg, IDC_TYPESELECTOR), 0);
		
		InitListViewColumns(GetDlgItem (hwndDlg, IDC_TYPESLISTCONTAINER), 0);   
		InitListViewItems(GetDlgItem (hwndDlg, IDC_TYPESLISTCONTAINER), lpMapDocument, 0);
		//IDC_TYPESLISTCONTAINER
		return TRUE;
	case WM_COMMAND :
		lpXpStudioDocument = (LPXPSTUDIODOCUMENT) GetWindowLong (hwndActive, GWL_USERDATA);
		lpMapDocument = lpXpStudioDocument->lpMapDocument;
		switch (HIWORD(wParam))
		{
		case CBN_SELCHANGE:
			n = ComboBox_GetCurSel(GetDlgItem (hwndDlg, IDC_TYPESELECTOR));
			InitListViewColumns(GetDlgItem (hwndDlg, IDC_TYPESLISTCONTAINER), n);   
			InitListViewItems(GetDlgItem (hwndDlg, IDC_TYPESLISTCONTAINER), lpMapDocument, n);
			return TRUE ;
		}
		return TRUE;
	}
	return FALSE;
}


// InitListViewColumns - adds columns to a list view control. 
// Returns TRUE if successful, or FALSE otherwise. 
// hwndLV - handle to the list view control. 
BOOL WINAPI InitListViewColumns(HWND hwndLV, int vtype)
{ 

	char g_achTemp[256];     // temporary buffer
	LVCOLUMN lvc; 
    int iCol;

	for (iCol = C_COLUMNS-1; iCol >= 0; iCol--) {
		ListView_DeleteColumn(hwndLV, iCol);
	}
	// Initialize the LVCOLUMN structure. 
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
    lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 100;
	lvc.pszText = g_achTemp;  
    // Add the columns.
	for (iCol = 0; iCol < C_COLUMNS; iCol++) { 
        lvc.iSubItem = iCol; 
//		switch (vtype)
//		{
//			case //HERE
        LoadString(hInst, IDS_POLYGONTYPE_C1 + iCol, 
                g_achTemp, sizeof(g_achTemp)); 
//		}
        if (ListView_InsertColumn(hwndLV, iCol, &lvc) == -1) 
            return FALSE;
	}
	return TRUE; 
}  

BOOL WINAPI InitListViewItems(HWND hwndLV, LPMAPDOCUMENT lpMapDocument,
							  int vtype)
{ 
	LVITEM lvi;
	int i = 0, iSubItem;
	//char tmpColor[64];
	//char tmp
	
	// Initialize LVITEM members that are common to all items. 
    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE; 
    lvi.state = 0;
	lvi.stateMask = 0; 
    lvi.pszText = LPSTR_TEXTCALLBACK;   // app. maintains text 
    lvi.iImage = 0;                     

	for (i = 0; lpMapDocument->MapGeometry.pstyles[i].id[0] != '\0'; i++)
	{
		// Allocate an application-defined structure to store the 
        // item label and the text of each subitem. 
        MYITEM *pItem = LocalAlloc(LPTR, sizeof(MYITEM));
		pItem->aCols[0] = lpMapDocument->MapGeometry.pstyles[i].id;

		for (iSubItem = 1; iSubItem < C_COLUMNS; iSubItem++)
		{
		pItem->aCols[iSubItem] = DupString("jlm2\0");
        }  

//		pItem->aCols[0] = DupString(lpMapDocument->MapGeometry.pstyles[i].id);
//		pItem->aCols[1] = lpMapDocument->MapGeometry.pstyles[i].color;
//		pItem->aCols[1] = DupString(lpMapDocument->MapGeometry.estyles[lpMapDocument->MapGeometry.pstyles[i].defedge_id].id);
//		pItem->aCols[3] = lpMapDocument->MapGeometry.pstyles[i].flags;

        
		// Initialize item-specific LVITEM members.
		lvi.iItem = i; 
        lvi.iSubItem = 0;
//		lvi.pszText = lpMapDocument->MapGeometry.pstyles[i].id;
		lvi.lParam = (LPARAM) pItem;    // item data  
        // Add the item.
		ListView_InsertItem(hwndLV, &lvi); 
	}
	return FALSE;
}  
LPSTR DupString(LPSTR lpsz)
{
	int cb = lstrlen(lpsz) + 1; 
	
    LPSTR lpszNew = LocalAlloc(LMEM_FIXED, cb);
	if (lpszNew != NULL) 
        CopyMemory(lpszNew, lpsz, cb);
	return lpszNew;
}  