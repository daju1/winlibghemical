// MOLDYN_ATOMLIST_DIALOG.CPP

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "../stdafx.h"

#include "../../libghemical/src/long_stream.h"
#include "moldyn_atomlist_dialog.h"
#include "../../consol/filedlg.h"
#include "../../consol/resource.h"

#include "commctrl.h"

#include <vector>
#include <iostream>
#include <strstream>
using namespace std;

#define GWL_WNDPROC         (-4)

BOOL CALLBACK DlgProcList(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
HWND CreateListView (HWND hwndParent);
HWND CreateListView (HWND hwndParent, double part_h, double part_v); 
VOID SetView(HWND hWndListView, DWORD dwView);
BOOL InitListViewImageLists(HWND hWndListView);
BOOL InitListViewColumns(HWND hWndListView, vector<string> * headers);

#define MODAL_ATOMLIST_DLG 1

extern HINSTANCE hInst;								// current instance

#define XBITMAP 16 
#define YBITMAP 16 

#define BUFFER MAX_PATH 

/*################################################################################################*/
extern HINSTANCE hInst;								// current instance
BOOL CALLBACK DlgProcAtomlistMolDyn(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
    LPMEASUREITEMSTRUCT lpmis; 
	static moldyn_atomlist_dialog * moldyn_atmlst_dlg;
    switch (message) 
    { 
	case WM_INITDIALOG:
		{
			moldyn_atmlst_dlg = reinterpret_cast<moldyn_atomlist_dialog *>(lParam);
			moldyn_atmlst_dlg->hDlg = hDlg;
			moldyn_atmlst_dlg->OnInitDialog(moldyn_atmlst_dlg);
		}
		break;
    case WM_COMMAND: 
        switch (LOWORD(wParam)) 
        { 
		case IDOK: 
			{
				moldyn_atmlst_dlg->handler_ButtonOK(NULL, moldyn_atmlst_dlg);
	            //EndDialog(hDlg, wParam); 
			}
			break;
        case IDCANCEL: 
			{
				moldyn_atmlst_dlg->handler_ButtonCancel(NULL, moldyn_atmlst_dlg);
				EndDialog(hDlg, wParam); 
			}
        } 
		break;
    case WM_MEASUREITEM: 
		{
			lpmis = (LPMEASUREITEMSTRUCT) lParam; 
			// Set the height of the list box items. 
			lpmis->itemHeight = YBITMAP; 				
			
			return TRUE; 
		}
		break;
    case WM_DRAWITEM: 
		{
			moldyn_atmlst_dlg->OnWM_DRAWITEM(wParam,lParam, moldyn_atmlst_dlg);
		}
		break;
	case WM_USER+1:
		{
			// конец редактирования
			moldyn_atmlst_dlg->OnWM_USER_1(wParam,lParam, moldyn_atmlst_dlg);
		}
		break; 

	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code)
		{
		case NM_CLICK :
			{
				moldyn_atmlst_dlg->OnNM_CLICK(wParam,lParam, moldyn_atmlst_dlg);
			}
			break;
		case LVN_GETDISPINFO:
			{
				moldyn_atmlst_dlg->OnLVN_GETDISPINFO(wParam,lParam, moldyn_atmlst_dlg);
			}
			break;
		case LVN_ENDLABELEDIT:
			{
				moldyn_atmlst_dlg->OnLVN_ENDLABELEDIT(wParam,lParam, moldyn_atmlst_dlg);
			}
			break;


		}
		break;
	case WM_DESTROY:
		{
			if (moldyn_atmlst_dlg) delete moldyn_atmlst_dlg;
		}
    } 
    return FALSE; 
} 


moldyn_atomlist_dialog::moldyn_atomlist_dialog(moldyn * p1) : glade_dialog("glade/moldyn_atomlist_dialog.glade")
{
	mld = p1;
/*	this->headers		= new vector<string>;
	this->items			= new vector<vector<string> >;

	this->checkings		= new vector<vector<int> >;
	this->editings		= new vector<vector<int> >;
*/
#if MODAL_ATOMLIST_DLG
	if (DialogBoxParam(hInst, 
         MAKEINTRESOURCE(IDD_DIALOG_ATOMLIST_MOLDYN), 
         (HWND)NULL, (DLGPROC)DlgProcAtomlistMolDyn, (LPARAM)this)==IDOK) 
    {
        // Complete the command; szItemName contains the 
        // name of the item to delete. 
    }

    else 
    {
        // Cancel the command. 
		 cout << "trajview_dialog : glade_xml_get_widget() failed!!!" << endl; 
		 return; 
    } 
#else	
//	dialog = glade_xml_get_widget(xml, "moldyn_atomlist_dialog");
//	if (dialog == NULL) { cout << "moldyn_atomlist_dialog : glade_xml_get_widget() failed!!!" << endl; return; }
	if (CreateDialogParam(hInst, 
         MAKEINTRESOURCE(IDD_DIALOG_ATOMLIST_MOLDYN), 
         (HWND)NULL, (DLGPROC)DlgProcAtomlistMolDyn, (LPARAM)this)) 
    {
        // Complete the command; szItemName contains the 
        // name of the item to delete. 
		printf("moldyn_atomlist_dialog::moldyn_atomlist_dialog hDlg = %x\n", hDlg);
		ShowWindow(hDlg, 1);
	}
    else 
    {
        // Cancel the command. 
		 cout << "trajview_dialog : glade_xml_get_widget() failed!!!" << endl; 
		 return; 
    } 
#endif

}

moldyn_atomlist_dialog::~moldyn_atomlist_dialog(void)
{
	delete this->headers;
	delete this->items;

	delete this->checkings;
	delete this->editings;

}
typedef struct
{
	HWND hwndComboBox;
	COLORREF color;
} COLOMNDATA, *LPCOLOMNDATA;

extern int ChooseSurfColor(HWND hwndOwner, COLORREF& color);


WNDPROC wpOrigListWndProc; 

HBITMAP hbmpUnchecked, hbmpChecked; 
HBITMAP hbmpPicture, hbmpOld; 

void ErrorExit(LPTSTR lpszFunction, bool toExitProcess = false) 
{ 
    TCHAR szBuf[200]; 
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    wsprintf(szBuf, 
        "%s failed with error %d: %s", 
        lpszFunction, dw, lpMsgBuf); 
 
    MessageBox(NULL, szBuf, "Error", MB_OK); 

    LocalFree(lpMsgBuf);
	if(toExitProcess)
		ExitProcess(dw); 
}

#define TEXT_PART 0.25
RECT GetComboBoxRect(RECT * rcHeader)
{
	RECT rcCombo;
	
	rcCombo.left = rcHeader->left+(rcHeader->right - rcHeader->left)*TEXT_PART;
	rcCombo.top = rcHeader->top 
		//+ 3*(rcHD.bottom - rcHD.top)/2
				; 
	rcCombo.right = (rcHeader->right - rcHeader->left)*(1.0-TEXT_PART); 
	rcCombo.bottom = 10*(rcHeader->bottom - rcHeader->top);

	return rcCombo;

}
RECT GetTextRect(RECT * rcHeader)
{
	RECT rcText;
	rcText.top = rcHeader->top + (rcHeader->bottom-rcHeader->top)*0;
	rcText.bottom = rcHeader->bottom;
	rcText.left = rcHeader->left;
	rcText.right = rcHeader->right - (rcHeader->right - rcHeader->left)*(1.0-TEXT_PART);

	return rcText;
}

 
WNDPROC wpOrigEditWndProc;

LRESULT APIENTRY EditWndSubclassProc(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam) 
{ 
    if (uMsg == WM_GETDLGCODE) 
	{
        return DLGC_WANTALLKEYS ;  // 0x0004      /* Control wants all keys - All keyboard input         */
	} 
	else
    if (uMsg == WM_KILLFOCUS ) 
	{
		SendMessage(GetParent(hwnd), WM_USER+1,0,0);
	} 
	else
    if (uMsg == WM_CHAR ) 
	{
		if (wParam == 13 )
		{
			SendMessage(GetParent(hwnd), WM_USER+1,0,0);
		}
	} 
		
    return CallWindowProc(wpOrigEditWndProc, hwnd, uMsg, 
        wParam, lParam); 
} 
 
WNDPROC wpOrigComboBoxWndProc;

LRESULT APIENTRY ComboBoxWndSubclassProc(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam) 
{ 
    if (uMsg == WM_GETDLGCODE) 
	{
        return DLGC_WANTALLKEYS ;  // 0x0004      /* Control wants all keys - All keyboard input         */
	} 
	else
    if (uMsg == WM_SIZING ) 
	{
		printf("WM_SIZING\n");
		int column = GetWindowLong(hwnd, GWL_USERDATA);
	}
	else
    if (uMsg == WM_CHAR ) 
	{
		if (wParam == 13 )
		{
			SendMessage(GetParent(hwnd), WM_USER+2,0,0);
		}
	} 
		
    return CallWindowProc(wpOrigComboBoxWndProc, hwnd, uMsg, 
        wParam, lParam); 
} 

WNDPROC wpOrigHeaderWndProc;

LRESULT APIENTRY HeaderWndSubclassProc(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam) 
{ 
    if (uMsg == WM_GETDLGCODE) 
	{
        return DLGC_WANTALLKEYS ;  // 0x0004      /* Control wants all keys - All keyboard input         */
	} 
	else
    if (uMsg == WM_SIZING ) 
	{

	}
	else
    if (uMsg == WM_CHAR ) 
	{
		if (wParam == 13 )
		{
			SendMessage(GetParent(hwnd), WM_USER+2,0,0);
		}
	} 
		
    return CallWindowProc(wpOrigHeaderWndProc, hwnd, uMsg, 
        wParam, lParam); 
} 



WNDPROC wpOrigListViewWndProc;

// Новый отработчик сообщений ListView1 
LRESULT APIENTRY ListViewWndSubclassProc(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam) 
{ 
    if (uMsg == WM_GETDLGCODE) 
	{
        return DLGC_WANTALLKEYS ;  // 0x0004      /* Control wants all keys - All keyboard input         */
	} 
	else
	if(uMsg==WM_DRAWITEM) 
	{ 
		//структура с параметрами для рисования 
		DRAWITEMSTRUCT *ds=(DRAWITEMSTRUCT *)lParam; 
		//Номер колонки 
		int column=ds->itemID; 
		//Сохраним контекст 
		int idsave=SaveDC(ds->hDC); 

		//Получим текст заголовка колонки 
		char text[42]={0}; 
		HDITEM hdm; 
		memset(&hdm,0,sizeof(hdm)); 
		hdm.mask=HDI_TEXT; 
		hdm.pszText=text; 
		hdm.cchTextMax=40; 
		Header_GetItem(ds->hwndItem,column,&hdm); 
		//Нарисуем текст 
		int i=strlen(text); 

		RECT rcText = GetTextRect(&ds->rcItem);

		if(i) 
		{
			::DrawText(ds->hDC,text,i,&rcText,DT_CENTER); 
		}
		//восстанавливаем контекст 
		RestoreDC(ds->hDC,idsave);
		return 1; 
	} 
	else
		if (uMsg == LVM_SCROLL)
		{
			printf("LVM_SCROLL\n");
		}

	//Вызываем старый отработчик 
    return CallWindowProc(wpOrigListViewWndProc, hwnd, uMsg, 
        wParam, lParam); 
} 


#define IDCOMBO 11111
#define IDLIST 11112
#define HEADER_IN_FIRST_ROW	0

void moldyn_atomlist_dialog::OnInitDialog(void * data)
{
	moldyn_atomlist_dialog * ref = (moldyn_atomlist_dialog *) data;

//printf("\nmoldyn_atomlist_dialog::OnInitDialog() = %x ref = %x\n", ref->hDlg, ref);

	ref->rows = ref->mld->eng->GetAtomCount();

	ref->cols = 7;

//printf("rows = %d cols = %d\n", ref->rows, ref->cols);
#if HEADER_IN_FIRST_ROW
	ref->rows++;
#endif
	ref->edit_r = -1;
	ref->edit_c = -1;

	ref->hList = CreateListView (ref->hDlg, 0.8, 0.9);
	SetFocus(ref->hList);

	ShowWindow(ref->hList, 1);
	InitListViewImageLists(ref->hList);

	//--------------------------------------------------------------
	// Subclass the control. 
	wpOrigListViewWndProc = (WNDPROC) SetWindowLong(ref->hList, 
		GWL_WNDPROC, (LONG) ListViewWndSubclassProc); 

	//SetWindowLong(hList, GWL_USERDATA,(LONG)lParam);
	//--------------------------------------------------------------
	HWND hwndHD = ListView_GetHeader(ref->hList); 
	//--------------------------------------------------------------
	// Subclass the control. 
	wpOrigHeaderWndProc = (WNDPROC) SetWindowLong(hwndHD, 
		GWL_WNDPROC, (LONG) HeaderWndSubclassProc); 
	SetWindowLong(hwndHD, GWL_USERDATA,(LONG)ref->cols);

	//--------------------------------------------------------------
    // Load bitmaps. 
    hbmpChecked = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_CHECKED)); 
    hbmpUnchecked = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_UNCHECKED)); 

	printf("hInst = %x hbmpChecked = %x\n",hInst,  hbmpChecked);

	//--------------------------------------------------------------
	//--------------------------------------------------------------

	ref->headers = new vector<string>;
	for (int c = 0; c < ref->cols; c++)
	{
		ref->headers->push_back(string());
		ref->headers->back().resize(32);
		switch(c)
		{
		case 0:
			{
				sprintf((char *)ref->headers->back().data(), "%s", 
					"Elem");				
			}
			break;
		case 1:
			{
				sprintf((char *)ref->headers->back().data(), "%s", 
					"x");				
			}
			break;
		case 2:
			{
				sprintf((char *)ref->headers->back().data(), "%s", 
					"y");				
			}
			break;
		case 3:
			{
				sprintf((char *)ref->headers->back().data(), "%s", 
					"z");				
			}
			break;
		case 4:
			{
				sprintf((char *)ref->headers->back().data(), "%s", 
					"Vx");				
			}
			break;
		case 5:
			{
				sprintf((char *)ref->headers->back().data(), "%s", 
					"Vy");				
			}
			break;
		case 6:
			{
				sprintf((char *)ref->headers->back().data(), "%s", 
					"Vz");				
			}
			break;

		}
	}

	ref->items		= new vector<vector<string> >;
	ref->checkings	= new vector<vector<int> >;
	ref->editings	= new vector<vector<int> >;

	ref->items->		resize(ref->rows);
	ref->checkings->	resize(ref->rows);
	ref->editings->		resize(ref->rows);
	
	atom ** glob_atmtab = ref->mld->eng->GetSetup()->GetAtoms();

	for (int r = 0; r < ref->rows; r++)
	{
		ref->items->operator		[] (r).resize(ref->cols);
		ref->checkings->operator	[] (r).resize(ref->cols);
		ref->editings->operator		[] (r).resize(ref->cols);

		for (int c = 0; c < ref->cols; c++)
		{
			(*ref->items)[r][c] .resize(256);

			switch (c)
			{
			case 0:
				{
					(*ref->checkings)[r][c] = 0;
					(*ref->editings)[r][c] = 0;
					sprintf((char *)(*ref->items)[r][c].data(), "%s", 
						glob_atmtab[r]->el.GetSymbol());
				}
				break;
			case 1:
			case 2:
			case 3:
				{
					(*ref->checkings)[r][c] = 0;
					(*ref->editings)[r][c] = 1;
					sprintf((char *)(*ref->items)[r][c].data(), "%f", 
						ref->mld->eng->crd[r * 3 + c-1]);
				}
				break;
			case 4:
			case 5:
			case 6:
				{
					(*ref->checkings)[r][c] = 0;
					(*ref->editings)[r][c] = 1;
					sprintf((char *)(*ref->items)[r][c].data(), "%f", 
						ref->mld->vel[r * 3 + c-4]);
				}
				break;
			default:
				{
					(*ref->checkings)[r][c] = 0;
					(*ref->editings)[r][c] = 0;
				}
			}
		}
	}

	InitListViewColumns(ref->hList, ref->headers);


	LVITEM lvI;
	// Some code to create the list-view control.
	// Initialize LVITEM members that are common to all
	// items. 
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE; 
	lvI.state = 0; 
	lvI.stateMask = 0; 
	// first two empty rows
	ListView_InsertItem(ref->hList, &lvI);
	ListView_InsertItem(ref->hList, &lvI);
	// Initialize LVITEM members that are different for each item. 
	for (int index = 0; index < ref->rows; index++)
	{
   		lvI.iItem = index;
		lvI.iImage = index;
		lvI.iImage = 0;
		lvI.iSubItem = 0;
		lvI.pszText = LPSTR_TEXTCALLBACK; // sends an
										  // LVN_GETDISP
										  // message.									  
		if(ListView_InsertItem(ref->hList, &lvI) == -1)
		{
			char str[255];
			sprintf(str, "error %d", index);
			MessageBox(0,str,"",0);
			return;
		}
	}
}

void moldyn_atomlist_dialog::OnLVN_GETDISPINFO(WPARAM wParam, LPARAM lParam, void * data)
{
	moldyn_atomlist_dialog * ref = (moldyn_atomlist_dialog *) data;
	NMLVDISPINFO * plvdi = ((NMLVDISPINFO *)lParam);
	plvdi->item.pszText = (char*)(*ref->items)[plvdi->item.iItem][plvdi->item.iSubItem].c_str();
/*
	printf("OnLVN_GETDISPINFO %d %d %s %s\n",
		plvdi->item.iItem,
		plvdi->item.iSubItem, 
		plvdi->item.pszText,
		(*ref->items)[plvdi->item.iItem][plvdi->item.iSubItem].c_str()
		);*/
}

void moldyn_atomlist_dialog::OnLVN_ENDLABELEDIT(WPARAM wParam, LPARAM lParam, void * data)
{
	moldyn_atomlist_dialog * ref = (moldyn_atomlist_dialog *) data;
	NMLVDISPINFO * plvdi = ((NMLVDISPINFO *)lParam);
	if(plvdi->item.mask & LVIF_TEXT && plvdi->item.pszText)
	{
		printf("%d %d %s\n", plvdi->item.iItem, plvdi->item.iSubItem, plvdi->item.pszText);
		(*ref->items)[plvdi->item.iItem][plvdi->item.iSubItem] = plvdi->item.pszText;
	}
}

void moldyn_atomlist_dialog::OnWM_USER_1(WPARAM wParam, LPARAM lParam, void * data)
{
	moldyn_atomlist_dialog * ref = (moldyn_atomlist_dialog *) data;

	char buff[255];
	GetWindowText(ref->hWndEdit, buff, 254);
	if (ref->edit_r >= 0 && ref->edit_r < ref->rows
		&& 
		ref->edit_c >= 0 && ref->edit_c < ref->cols)
	strcpy((char *)(*ref->items)[ref->edit_r][ref->edit_c].data(), buff);

	printf("%d %d %s\n", ref->edit_r, ref->edit_c, buff);
	switch (ref->edit_c)
	{
	case 0:
		{
		}
		break;
	case 1:
	case 2:
	case 3:
		{
			ref->mld->eng->crd[ref->edit_r * 3 + ref->edit_c-1] = atof(buff);
		}
		break;
	case 4:
	case 5:
	case 6:
		{
			ref->mld->vel[ref->edit_r * 3 + ref->edit_c-4] = atof(buff);
		}
		break;
	default:
		{
		}
	}

	DestroyWindow(ref->hWndEdit);
}

void moldyn_atomlist_dialog::OnNM_CLICK(WPARAM wParam, LPARAM lParam, void * data)
{
	moldyn_atomlist_dialog * ref = (moldyn_atomlist_dialog *) data;

	LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;


	int  r = lpnmitem->iItem;
	int  c = lpnmitem->iSubItem;
	bool check_not_clicked = true;

	if (r >= 0 && (*ref->checkings)[r][c])
	{
		RECT rcItem;
		RECT rcIcon;
		
		ListView_GetSubItemRect(ref->hList, r, c,
			LVIR_LABEL,
			&rcItem);

		ListView_GetSubItemRect(ref->hList, r, 0,
			LVIR_ICON,
			&rcIcon);

		if (c == 0 ?
			lpnmitem->ptAction.x <= rcIcon.right 
			&&
			lpnmitem->ptAction.x >= rcIcon.left
			:
			lpnmitem->ptAction.x - rcItem.left <= rcIcon.right - rcIcon.left 
			&&
			lpnmitem->ptAction.x - rcItem.left >= 0
			)
		{
			check_not_clicked = false;
			if ((*ref->checkings)[r][c] == 2)
				(*ref->checkings)[r][c] = 1;
			else
				if ((*ref->checkings)[r][c] == 1)
					(*ref->checkings)[r][c] = 2;
		}

		RECT rcClient;
		GetClientRect(ref->hList, &rcClient);
		InvalidateRect(ref->hList, &rcClient, TRUE);
	}

	if (r >= 0 && (*ref->editings)[r][c] && check_not_clicked )
	{
		RECT rcItem;
		RECT rcList;
		RECT rcEdit;

		ListView_GetSubItemRect(ref->hList, r, c,
			LVIR_LABEL,
			&rcItem);

		GetWindowRect(ref->hList, &rcList);

		//Получаем дескриптор заголовка
		HWND hwndHD = GetDlgItem(ref->hList, 0); 
		
		rcEdit.left = rcList.left + rcItem.left + (c > 0 ? XBITMAP + 6 : 0);
		rcEdit.top = rcList.top + rcItem.top;

		rcEdit.right = rcItem.right - rcItem.left - (c > 0 ? XBITMAP + 6 : 0); //width
		rcEdit.bottom = rcItem.bottom -	rcItem.top;
		
		ref->hWndEdit = CreateWindow ("EDIT", "", 
			WS_TABSTOP | WS_POPUP  | WS_VISIBLE, 
			rcEdit.left, 
			rcEdit.top, // rcItem.bottom + (list view header height)
			rcEdit.right, 
			rcEdit.bottom, 
			ref->hDlg, 
			NULL,
			hInst, NULL); 

		ref->edit_r = r;
		ref->edit_c = c;
		
		ShowWindow(ref->hWndEdit, 1);

		//--------------------------------------------------------------
		// Subclass the control. 
		wpOrigEditWndProc = (WNDPROC) SetWindowLong(ref->hWndEdit, 
			GWL_WNDPROC, (LONG) EditWndSubclassProc); 

		SetWindowLong(ref->hWndEdit, GWL_USERDATA,(LONG)lParam);
		//--------------------------------------------------------------

		SetWindowText(ref->hWndEdit, (*ref->items)[r][c].c_str() );
	}
}

void moldyn_atomlist_dialog::OnWM_DRAWITEM(WPARAM wParam, LPARAM lParam, void * data)
{
	moldyn_atomlist_dialog * ref = (moldyn_atomlist_dialog *) data;
    HDC hdcMem; 
    TEXTMETRIC tm; 
    int y; 
			
    LPDRAWITEMSTRUCT lpdis; 
	lpdis = (LPDRAWITEMSTRUCT) lParam; 
	
	// If there are no list box items, skip this message. 
	if (lpdis->itemID == -1) 
	{ 
		return; 
	} 

	if (lpdis->hwndItem == ref->hList)
	{
		// Draw the bitmap and text for the list box item. Draw a 
		// rectangle around the bitmap if it is selected. 
		
		switch (lpdis->itemAction) 
		{ 
		case ODA_DRAWENTIRE: 
			{
				//Затем в MyListCtrl.cpp в DrawItem (переопределение события WM_DRAWITEM)
				//рисуются все айтемы с сабайтемами, 
				
				// Display the bitmap associated with the item.

				
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 
				//checkbox'ы рисуются так:
				
				hdcMem = CreateCompatibleDC(lpdis->hDC); 

				int r = lpdis->itemID;
				//int ti = SendMessage(hList, LVM_GETTOPINDEX, 0, 0);

				if(r >= 0)
				{

					for (int c = 0; c < ref->cols; c++)
					{
						
						RECT rcItem;
						//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
						//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
						//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@						
						ListView_GetSubItemRect(ref->hList, r, c,
							LVIR_BOUNDS, 
							//LVIR_ICON,
							&rcItem);									
															
						{
							HDITEM hdm; 
							hdm.mask=HDI_LPARAM; 									
							hdm.lParam=0; 
							

							//Получим стиль заголовка колонки 
							HWND hwndHD = ListView_GetHeader(ref->hList); 
							Header_GetItem(hwndHD, c, &hdm);
							
							//Установим стиль заголовка колонки 
							LPCOLOMNDATA coldata = (LPCOLOMNDATA )hdm.lParam;
							COLORREF color = coldata->color;

							HBRUSH hbrush = CreateSolidBrush(color);
							HBRUSH hbrushOld = (HBRUSH__ *)SelectObject(lpdis->hDC, hbrush);

							FillRect(lpdis->hDC, &rcItem, hbrush);

							SelectObject(lpdis->hDC, hbrushOld);
							DeleteObject(hbrush);
						}									
															
						GetTextMetrics(lpdis->hDC, &tm); 

						y = (lpdis->rcItem.bottom + lpdis->rcItem.top - 
							tm.tmHeight) / 2;
						
						TextOut(lpdis->hDC, 
							rcItem.left + XBITMAP + 6,
							y, 
							(*ref->items)[r][c].c_str(), 
							strlen((*ref->items)[r][c].c_str())
							);						

						//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
						//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
						//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
						ListView_GetSubItemRect(ref->hList, r, c,
							//LVIR_BOUNDS, 
							LVIR_ICON,
							&rcItem);

						if ( (*ref->checkings)[r][c] > 0 )
						{
							if ( (*ref->checkings)[r][c] == 1)
								hbmpPicture = hbmpUnchecked;
							else
								hbmpPicture = hbmpChecked;

							hbmpOld = (HBITMAP__ *)SelectObject(hdcMem, hbmpPicture); 

							if(!BitBlt(lpdis->hDC, 
								rcItem.left, rcItem.top, 
								rcItem.right - rcItem.left, 
								rcItem.bottom - rcItem.top, 
								hdcMem, 0, 0, SRCCOPY)) 								
								ErrorExit("Screen to Compat Blt Failed"); 
						}
						//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
						//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
						//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
					}
					
					
					SelectObject(hdcMem, (HBITMAP__ *)hbmpOld); 
					DeleteDC(hdcMem); 

					// Is the item selected? 
					if (lpdis->itemState & ODS_SELECTED)
					{ 
						// Draw a rectangle around bitmap to indicate 
						// the selection. 
						DrawFocusRect(lpdis->hDC, &lpdis->rcItem); 
					} 

				}
#if !HEADER_IN_FIRST_ROW

				if (false)
				{
					RECT rcItem;
					
					ListView_GetSubItemRect(ref->hList, 0, 0,
						LVIR_BOUNDS, 
						&rcItem);
					int HeightHeader = (rcItem.bottom - rcItem.top )*3;
					//Получаем дескриптор заголовка
					HWND hwndHD = ListView_GetHeader(ref->hList); 
					HDITEM hdm; 
					hdm.mask=HDI_FORMAT; 
					for(int i=0; i < ref->cols; i++) 
					{ 
						hdm.fmt=0; 
						//Получим стиль заголовка колонки 
						Header_GetItem(hwndHD,i,&hdm);
						
						if(hdm.fmt&HDF_OWNERDRAW)continue; 
						//Установим стиль заголовка колонки 
						hdm.fmt|=HDF_OWNERDRAW; 
						Header_SetItem(hwndHD,i,&hdm); 
					}
				}
#endif // !HEADER_IN_FIRST_ROW
			}
			break; 
		}
	} 

}

void moldyn_atomlist_dialog::handler_Destroy(HWND, void * data)			// not really needed...
{
	moldyn_atomlist_dialog * ref = (moldyn_atomlist_dialog *) data;
	//cout << "handler_Destroy() : ref = " << ref << endl;
}

void moldyn_atomlist_dialog::handler_ButtonOK(HWND, void * data)
{
	moldyn_atomlist_dialog * ref = (moldyn_atomlist_dialog *) data;
	//cout << "handler_ButtonOK() : ref = " << ref << endl;
	
	// read the user's settings from widgets...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
}

void moldyn_atomlist_dialog::handler_ButtonCancel(HWND, void * data)		// not really needed...
{
	moldyn_atomlist_dialog * ref = (moldyn_atomlist_dialog *) data;
	//cout << "handler_ButtonCancel() : ref = " << ref << endl;
}

/*################################################################################################*/
// CreateListView - creates a list-view control in
// report view. 
// Returns the handle to the new control if successful,
// or NULL otherwise. 
// hWndParent - handle to the control's parent window. 

HWND CreateListView (HWND hwndParent, double part_h, double part_v) 
{ 
	HWND hWndListView;
	RECT rcl; 
	INITCOMMONCONTROLSEX icex;
	
	// Ensure that the common control DLL is loaded. 
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex); 
	// Create the list-view window in report view with label 
	// editing enabled.

	
	GetClientRect (hwndParent, &rcl); 
	//long marge = (rcl.bottom - rcl.top) * double((1.0 - part_v) / 2.0);
	long marge = 10;
	hWndListView = CreateWindowEx (
		WS_EX_CLIENTEDGE /*| LVS_EX_CHECKBOXES */| LVS_EX_FULLROWSELECT, 
		WC_LISTVIEW, 
		"", 
        WS_TABSTOP | WS_CHILD | LVS_REPORT | LVS_ICON | LVS_OWNERDATA 
		| LVS_OWNERDRAWFIXED 
		| LBS_OWNERDRAWVARIABLE,
        marge, marge, 
		part_h * (rcl.right - rcl.left), 
		part_v * (rcl.bottom - rcl.top), 
        hwndParent, 
		(HMENU)IDLIST,
		hInst, NULL); 
	
	if (hWndListView == NULL) 
		return NULL; 
	else
		return hWndListView;
}

// SetView - sets a list-view's window style to change
// the view. 
// hWndListView - handle to the list-view control. 
// dwView - value specifying the new view style.

VOID SetView(HWND hWndListView, DWORD dwView) 
{ 
	// Retrieve the current window style. 
	DWORD dwStyle = GetWindowLong(hWndListView, GWL_STYLE); 
	// Only set the window style if the view bits have
	// changed. 
	
	if ((dwStyle & LVS_TYPEMASK) != dwView) 
		SetWindowLong(hWndListView, GWL_STYLE, 
		(dwStyle & ~LVS_TYPEMASK) | dwView); 
} 
// InitListViewImageLists - creates image lists for a
// list-view control.
// This function only creates image lists. It does not
// insert the items into the control, which is necessary
// for the control to be visible.   
// Returns TRUE if successful, or FALSE otherwise. 

// hWndListView - handle to the list-view control. 

BOOL InitListViewImageLists(HWND hWndListView) 
{ 
    HICON hiconItem;     // icon for list-view items 
    HIMAGELIST hLarge;   // image list for icon view 
    HIMAGELIST hSmall;   // image list for other views 

// Create the full-sized icon image lists. 
    hLarge = ImageList_Create(GetSystemMetrics(SM_CXICON), 
    GetSystemMetrics(SM_CYICON), ILC_MASK, 1, 1); 
    hSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON), 
    GetSystemMetrics(SM_CYSMICON), ILC_MASK, 1, 1); 
	
// Add an icon to each image list.  
    hiconItem = LoadIcon(hInst,
	   MAKEINTRESOURCE(IDI_GHEMICAL)); 
    ImageList_AddIcon(hLarge, hiconItem); 
    DestroyIcon(hiconItem); 


    hiconItem = LoadIcon(hInst,
	   MAKEINTRESOURCE(IDI_SMALL)); 
    ImageList_AddIcon(hSmall, hiconItem); 
     DestroyIcon(hiconItem); 

/******************************************************
Usually you have multiple icons; therefore, the previous
four lines of code can be inside a loop. The following
code shows such a loop. The icons are defined in the
application's header file as resources, which are
numbered consecutively starting with IDS_FIRSTICON. The
number of icons is defined in the header file as
C_ICONS.

for(index = 0; index < C_ICONS; index++)
    {
    hIconItem = LoadIcon (hInst, MAKEINTRESOURCE
	   (IDS_FIRSTICON + index));
    ImageList_AddIcon(hSmall, hIconItem);
    ImageList_AddIcon(hLarge, hIconItem);
    Destroy(hIconItem);
    }
   *******************************************************/
// Assign the image lists to the list-view control. 
    ListView_SetImageList(hWndListView, hLarge, LVSIL_NORMAL); 
    ListView_SetImageList(hWndListView, hSmall, LVSIL_SMALL); 
    return TRUE; 
} 

// InitListViewColumns - adds columns to a list-view
// control. 
// Returns TRUE if successful, or FALSE otherwise. 
// hWndListView - handle to the list-view control. 
BOOL InitListViewColumns(HWND hWndListView, vector<string> * headers) 
{ 
    char szText[256];     // temporary buffer 
    LVCOLUMN lvc; 
    int iCol; 

// Initialize the LVCOLUMN structure.
// The mask specifies that the format, width, text, and
// subitem members of the structure are valid. 
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT |
	  LVCF_SUBITEM; 
	  
// Add the columns. 
    for (iCol = 0; iCol < (*headers).size(); iCol++) 
	{ 
		strcpy(szText, (*headers)[iCol].c_str());
        lvc.iSubItem = iCol;
        lvc.pszText = szText;	
        lvc.cx = 100;     // width of column in pixels

        if ( iCol < 2 )
          {
          lvc.fmt = LVCFMT_LEFT;  // left-aligned column
          }
        else
          { 
          lvc.fmt = LVCFMT_RIGHT; // right-aligned column		                         
          }

		lvc.fmt = LVCFMT_CENTER;

        if (ListView_InsertColumn(hWndListView, iCol,
		        &lvc) == -1) 
            return FALSE; 

		HWND hWndComboBox = NULL;

		//Получаем дескриптор заголовка
		HWND hwndHD = ListView_GetHeader(hWndListView); 

		if (false)
		{
			RECT rcHD;
			Header_GetItemRect(hwndHD, iCol, &rcHD);

			RECT ComboBoxRect = GetComboBoxRect(&rcHD);

			hWndComboBox = CreateWindow ("COMBOBOX", "", 
				WS_CHILD | WS_TABSTOP | WS_VISIBLE | CBS_DROPDOWNLIST, 
				ComboBoxRect.left, 
				ComboBoxRect.top, 
				ComboBoxRect.right, 
				ComboBoxRect.bottom,			
				hwndHD, 
				(HMENU)IDCOMBO,
				hInst, NULL); 


			SendMessage(hWndComboBox, CB_RESETCONTENT, 0, 0);
			SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"not used");
			SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"well ID");
			SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"X");
			SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"Y");
			SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"Ustje Altitude");

			SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"Podoshva Altitude");
			//SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"Podoshva Glubina");
			SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"Zaboj Altitude");
			//SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"Zaboj Glubina");
			SendMessage(hWndComboBox, CB_SETCURSEL, 0, 0);

		}

		LPCOLOMNDATA coldata;

		coldata = new COLOMNDATA;

		coldata->color = RGB(255,255,255);
		coldata->hwndComboBox = hWndComboBox;
					
				
		HDITEM hdm; 
		hdm.mask=HDI_LPARAM; 			
		hdm.lParam=(LPARAM)coldata;				
		Header_SetItem(hwndHD,iCol,&hdm); 

		//--------------------------------------------------------------
		// Subclass the control. 
		wpOrigComboBoxWndProc = (WNDPROC) SetWindowLong(hWndComboBox, 
			GWL_WNDPROC, (LONG) ComboBoxWndSubclassProc);

		SetWindowLong(hWndComboBox, GWL_USERDATA,(LONG)iCol);
		//--------------------------------------------------------------

    } 
	return TRUE; 
} 

int GetItemWorkingArea(HWND hWndListView, int iItem)
{
	UINT     uWorkAreas = 0;
	int      nReturn = -1;
	LPRECT   pRects;
	POINT    pt;
	if(!ListView_GetItemPosition(hWndListView, iItem, &pt))
		return nReturn;
	ListView_GetNumberOfWorkAreas(hWndListView, &uWorkAreas);
	
	if(uWorkAreas)
	{
		pRects = (LPRECT)GlobalAlloc(GPTR, sizeof(RECT) *
			uWorkAreas);
		if(pRects)
		{
			UINT  i;
			
			nReturn = 0;
			
			ListView_GetWorkAreas(hWndListView, uWorkAreas, pRects);
			
			for(i = 0; i < uWorkAreas; i++)
			{
				if(PtInRect((pRects + i), pt))
				{
					nReturn = i;
					break;
				}
			}
			GlobalFree((HGLOBAL)pRects);
		}
	}
	return nReturn;
}


// eof



