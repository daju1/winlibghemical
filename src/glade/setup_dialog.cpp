// SETUP_DIALOG.CPP

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "../stdafx.h"

#include "setup_dialog.h"

#include "../win32_project.h"

#include "../../libghemical/src/eng1_qm.h"
#include "../../libghemical/src/eng1_qm.h"
#include "../../libghemical/src/eng1_sf.h"
#include "../../libghemical/src/eng2_qm_mm.h"
#include "../../libghemical/src/eng2_mm_sf.h"

//#include <gtk/gtk.h>

#include <iostream>
#include <strstream>
using namespace std;

#define USE_SETUP_DIALOG 0

/*################################################################################################*/
extern HINSTANCE hInst;								// current instance
BOOL CALLBACK DlgProcSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	static setup_dialog * stp_dlg;
    switch (message) 
    { 
	case WM_INITDIALOG:
		{
			stp_dlg = reinterpret_cast<setup_dialog *>(lParam);
			if (stp_dlg) stp_dlg->dialog = hDlg;
		}
		break;
    case WM_COMMAND: 
        switch (LOWORD(wParam)) 
        { 
		case IDOK: 
			{
				if (stp_dlg) stp_dlg->handler_ButtonOK(hDlg, stp_dlg);
				EndDialog(hDlg, wParam);
			}
			break;
        case IDCANCEL: 
 			{
				if (stp_dlg) stp_dlg->handler_ButtonCancel(hDlg, stp_dlg);
				EndDialog(hDlg, wParam); 
			}
			break;
            return TRUE; 
        } 
		break;
	case WM_DESTROY:
		{
			if (stp_dlg) delete stp_dlg;
		}
    } 
    return FALSE; 
} 
#if 0
BOOL CALLBACK Surface3D::DlgProcSurfAlpha( HWND hDlg, UINT uMsg,
                              WPARAM wParam, LPARAM lParam )
{
	static Surface3D * me;
	static int m_Pos[11]; 			// Массив позиций ползунков
	switch( uMsg )
	{
    case WM_INITDIALOG :
		{	


			PROPSHEETPAGE *lPage = (PROPSHEETPAGE*)lParam;
			me = (Surface3D *)lPage->lParam;

			char str[63];
			sprintf(str,"%d",me->m_alpha);
			SetDlgItemText(hDlg, IDC_EDIT_SURF_ALPHA, str);

			SendMessage(hDlg,WM_COMMAND,IDC_INIT_SURF_ALPHA_SLIDER,0);			
		}
		break;
	case WM_HSCROLL :
		{
			if (lParam)
			{
				int nPos = HIWORD (wParam);
				//====== Windows-описатель окна активного ползунка
				HWND hwnd = reinterpret_cast<HWND> (lParam);

				UINT nID;

				//=== Определяем индекс в массиве позиций ползунков
				int num = GetAlphaSliderNum(hwnd, nID);
				int delta, newPos;

				//====== Анализируем код события
				switch ( LOWORD( wParam ) )
				{
				case SB_THUMBTRACK:
				case SB_THUMBPOSITION:		// Управление мышью
					m_Pos[num] = nPos;
					break;
				case SB_LEFT:					// Клавиша Home
					delta = -255;
					goto New_Pos;
				case SB_RIGHT:				// Клавиша End
					delta = +255;
					goto New_Pos;
				case SB_LINELEFT:			// Клавиша <-
					delta = -1;
					goto New_Pos;
				case SB_LINERIGHT:			// Клавиша ->
					delta = +1;
					goto New_Pos;
				case SB_PAGELEFT:			// Клавиша PgUp
					delta = -20;
					goto New_Pos;
				case SB_PAGERIGHT:			// Клавиша PgDn
					delta = +20;
					goto New_Pos;

				New_Pos:						// Общая ветвь
					//====== Устанавливаем новое значение регулятора
					newPos = m_Pos[num] + delta;
					//====== Ограничения
					m_Pos[num] = newPos<0 ? 0 : newPos>255 ? 255 : newPos;
					break;
				case SB_ENDSCROLL:
				default:
					return (TRUE);
				}

				//====== Синхронизируем параметр lp и
				//====== устанавливаем его в положение nPos
				switch (num)
				{
				case 0:
					me->m_alpha = m_Pos[num];
					break;
				}


				//====== Синхронизируем текстовый аналог позиции
				char s[32];
				sprintf (s,"%d",me->m_alpha);
				sprintf (s,"%d",m_Pos[num]);
				SetDlgItemText(hDlg, nID, (LPCTSTR)s);

				// init of zoomCoefZ_XY
				//me->m_pSurfDoc->ZoomView();
				me->m_pSurfDoc->Draw();
				//====== Перерисовываем Вид с учетом изменений
				RECT rect;
				GetClientRect(me->m_pSurfDoc->hSurfWnd,&rect);
				InvalidateRect(me->m_pSurfDoc->hSurfWnd,&rect, true);
				
			}
		}
		break;
	case WM_COMMAND :
		switch( LOWORD( wParam ) )
		{
		case IDC_INIT_SURF_ALPHA_SLIDER :
			{
				//=== Добываем Windows-описатель окна ползунка
				HWND hwnd = GetDlgItem(hDlg, IDC_SLIDER_SURF_ALPHA);

				SendMessage(    // returns LRESULT in lResult
				(HWND) hwnd,       // handle to destination control
				(UINT) TBM_SETRANGE,    // message ID
				(WPARAM) true,          // = (WPARAM) (BOOL) fRedraw
				(LPARAM) MAKELONG (0, 255)              // = (LPARAM) MAKELONG (lMinimum, lMaximum)
				);
				hwnd = GetDlgItem(hDlg, IDC_SLIDER_SURF_ALPHA_PLANE);

				SendMessage(    // returns LRESULT in lResult
				(HWND) hwnd,       // handle to destination control
				(UINT) TBM_SETRANGE,    // message ID
				(WPARAM) true,          // = (WPARAM) (BOOL) fRedraw
				(LPARAM) MAKELONG (0, 255)              // = (LPARAM) MAKELONG (lMinimum, lMaximum)
				);
				//====== Проход по всем регулировкам
				//for (int i=0; i<2; i++)
					//====== Заполняем транспортный массив pPos
					m_Pos[0] = me->m_alpha;

				//====== Массив идентификаторов ползунков
				UINT IDs[] = 
				{
					IDC_SLIDER_SURF_ALPHA,
				};

				//====== Цикл прохода по всем регуляторам
				for (int i=0; i<sizeof(IDs)/sizeof(IDs[0]); i++)
				{
					//=== Добываем Windows-описатель окна ползунка
					HWND hwnd = GetDlgItem(hDlg, IDs[i]);
					UINT nID;

					//====== Определяем его идентификатор
					int num = Surface3D::GetAlphaSliderNum(hwnd, nID);

					// Требуем установить ползунок в положение m_Pos[i]
					::SendMessage(hwnd, TBM_SETPOS, TRUE,
									(LPARAM)m_Pos[i]);
					char s[32];
					//====== Готовим текстовый аналог текущей позиции
					sprintf (s,"%d",m_Pos[i]);
					//====== Помещаем текст в окно справа от ползунка
					SetDlgItemText(hDlg, nID, (LPCTSTR)s);
				}
			}
			break;
		case IDOK :
			{
				char str[63];
				GetDlgItemText(hDlg, IDC_EDIT_SURF_ALPHA,str,62);
				int alpha = atoi (str);
				if (alpha >= 0.0)
					me->m_alpha = alpha;
				else
					MessageBox(0, "Enter a valid value", "Alpha dialog", 0);

//				GetDlgItemText(hDlg, IDC_EDIT_SURF_RAZLOM_THICKNESS,str,62);
//				double thickness = atof (str);
				//if (thickness >= 0.0)
//					me->m_pSurfDoc->m_razlomThickness = thickness;
				//else
				//	MessageBox(0, "Enter a valid value", "Alpha dialog", 0);

				SendMessage(hDlg,WM_COMMAND,IDC_INIT_SURF_ALPHA_SLIDER,0);

				
				// init of zoomCoefZ_XY
				me->m_pSurfDoc->ZoomView();


				me->m_pSurfDoc->Draw();

				//====== Перерисовываем Вид с учетом изменений
				RECT rect;
				GetClientRect(me->m_pSurfDoc->hSurfWnd,&rect);
				InvalidateRect(me->m_pSurfDoc->hSurfWnd,&rect, true);

			}
			break;
			
		case IDCANCEL :
			EndDialog( hDlg, IDCANCEL );
			break;
		}
		break;
		
		default :
			
			return( FALSE );
	}
	return( TRUE );
}

 
BOOL CALLBACK  Surface3D::DlgProcSurfPalette( HWND hDlg, UINT uMsg,
                              WPARAM wParam, LPARAM lParam )
{
//	static LPSURFDLGDATA lpSurfDlgMem;
	static Surface3D * me;
    TCHAR tchBuffer[MAX_PATH]; 
    static HWND hListBox; 
    TEXTMETRIC tm; 
    int y; 
    LPMEASUREITEMSTRUCT lpmis; 
    LPDRAWITEMSTRUCT lpdis; 
	LONG itemdata;
	switch( uMsg )
	{
    case WM_INITDIALOG :
		{
			PROPSHEETPAGE *lPage = (PROPSHEETPAGE*)lParam;
			me = (Surface3D *)lPage->lParam;

			//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			CheckDlgButton( hDlg, IDC_CHECK_SURF_INVERSE_COLORMAP,
				me->m_bInverseColormap ? BST_CHECKED : BST_UNCHECKED );
			//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			CheckDlgButton( hDlg, IDC_CHECK_SURF_LOCAL_COLORMAP,
				me->m_bLocalColormap ? BST_CHECKED : BST_UNCHECKED );
			//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

			SendMessage(hDlg,WM_COMMAND,INIT_COMBO_IDC_COMBO_SURF_COLORMAP,0);
            hListBox = GetDlgItem(hDlg, IDC_LIST_SURF_CREATE_PALETTE); 
 
            wpOrigListBoxProc2 = (WNDPROC) SetWindowLongPtr(hListBox, 
				GWLP_WNDPROC, (LONG_PTR) ListBoxPaletteSubclassProc); 

			SendMessage(hDlg, WM_COMMAND, INIT_SURF_PALETTE, 0);
		}
		break;
	case WM_MEASUREITEM:
		{ 
            lpmis = (LPMEASUREITEMSTRUCT) lParam; 
            // Set the height of the List box items. 
            lpmis->itemHeight = YBITMAP2; 
		}
		break;
	case WM_DRAWITEM: 
		{
            lpdis = (LPDRAWITEMSTRUCT) lParam; 
 
            // If there are no List box items, skip this message. 
            if (lpdis->itemID == -1) 
            { 
                break; 
            } 
            // Draw the bitmap and text for the List box item. Draw a 
            // rectangle around the bitmap if it is selected. 
 
            switch (lpdis->itemAction) 
            { 
			case ODA_SELECT:
			case ODA_FOCUS:
            case ODA_DRAWENTIRE: 
				{ 
                    // Display the bitmap associated with the item. 
 
                    itemdata = SendMessage(lpdis->hwndItem, 
                        LB_GETITEMDATA, lpdis->itemID, (LPARAM) 0); 

					HBRUSH brush =  CreateSolidBrush(
					  (COLORREF) itemdata   // brush color value
					);
					FillRect(lpdis->hDC, &lpdis->rcItem, brush);
					DeleteObject(brush);

                    // Display the text associated with the item. 
 
                    SendMessage(lpdis->hwndItem, LB_GETTEXT, 
                        lpdis->itemID, (LPARAM) tchBuffer); 
 
                    GetTextMetrics(lpdis->hDC, &tm); 
 
                    y = (lpdis->rcItem.bottom + lpdis->rcItem.top - 
                        tm.tmHeight) / 2;
						
 
                    TextOut(lpdis->hDC, 
                        2, 
                        y, 
                        tchBuffer, 
                        strlen(tchBuffer)); 						
 
                    // Is the item selected? 
 
                    if (lpdis->itemState & ODS_SELECTED) 
                    { 
                        // Draw a rectangle around bitmap to indicate 
                        // the selection. 
 
                        DrawFocusRect(lpdis->hDC, &lpdis->rcItem); 
                    } 
				}
                break; 
 
            } 
		}
		break;
 
	case WM_COMMAND :
		switch( LOWORD( wParam ) )
		{
		case INIT_SURF_PALETTE:
			{				
				SendMessage(hListBox, LB_RESETCONTENT, 0, 0); 
				// with each List box item. 
				char item_str[255];
				int palette_len = me->m_palette.GetLen();
				int j = 0;
				if (me->m_bInverseColormap)
				{
					for (UINT r = palette_len - 1, j = 0; j < palette_len ; r--, j++)
					{
						sprintf(item_str, "%d", j);
						COLORREF itemcolor = me->m_palette.GetColor(r);
						AddCheckedPaletteItem(hListBox, 
							item_str, LONG(itemcolor));
					}
				}
				else
				{
					for (UINT r = 0; r < palette_len; r++)
					{
						sprintf(item_str, "%d", r);
						COLORREF itemcolor = me->m_palette.GetColor(r);
						AddCheckedPaletteItem(hListBox, 
							item_str, LONG(itemcolor));
					}
				}
				SetFocus(hListBox); 
				SendMessage(hListBox, LB_SETCURSEL, 0, 0); 
			}
			break;
		case IDC_BUTTON_INTERPOLATE:
			{
				LONG itemdata = SendMessage(hListBox, LB_GETITEMDATA, 0, 0);
				me->m_palette.InitFirst(COLORREF (itemdata) ); 
				LONG count_of_items = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
				itemdata = SendMessage(hListBox, LB_GETITEMDATA, count_of_items - 1, 0);
				me->m_palette.InitLast(COLORREF (itemdata) ); 
				me->m_palette.Interpolate();
				SendMessage(hDlg, WM_COMMAND, IDOK, 1);	
			}
			break;
		case IDC_BUTTON_SURF_PALETTE_APPLY:
			{
				LONG count_of_items = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
				me->m_palette.Free();
				me->m_palette.Allocate(count_of_items);
				for ( int i = 0; i < count_of_items; i++)
				{
					LONG itemdata = SendMessage(hListBox, LB_GETITEMDATA, i, 0);
					me->m_palette.InitItem(i,COLORREF (itemdata) );
				}
				SendMessage(hDlg, WM_COMMAND, IDOK, 1);	
			}
			break;
		case IDC_BUTTON_SURF_PALETTE_PLOT:
			{
				int len = me->m_palette.GetLen();
				Vector<vdouble> vx(3), vy(3);
				for (int iv = 0; iv < 3; iv++)
				{
					vx[iv].resize(len);
					vy[iv].resize(len);
					for ( UINT i = 0; i < len; i++)
					{
						COLORREF color = me->m_palette.GetColor(i);
						vx[iv][i] = i;
						switch (iv)
						{
						case 0:
							vy[iv][i] = GetRValue(color);
							break;
						case 1:
							vy[iv][i] = GetGValue(color);
							break;
						case 2:
							vy[iv][i] = GetBValue(color);
							break;
						}
					}
				}
				WinPlot(vx,vy);
			}
			break;
		case IDC_BUTTON_SURF_PALETTE_EXTRACT:
			{
				if (me->m_palette.GetLen() < 2048)
				{
					me->m_palette.Extract(2);
					SendMessage(hDlg, WM_COMMAND, IDOK, 1);
					char str[255];
					sprintf(str, "palette length = %d", me->m_palette.GetLen());
					SetDlgItemText(hDlg,IDC_STATIC_SURF_PALETTE_LEN, str);
				}
			}
			break;
		case IDC_CHECK_SURF_LOCAL_COLORMAP:
			{
				me->m_bLocalColormap = 
					IsDlgButtonChecked( hDlg, IDC_CHECK_SURF_LOCAL_COLORMAP) == BST_CHECKED;
				SendMessage(hDlg, WM_COMMAND, IDOK, 1);
			}
			break;
		case IDC_CHECK_SURF_INVERSE_COLORMAP:
			{
				me->m_bInverseColormap = 
					IsDlgButtonChecked( hDlg, IDC_CHECK_SURF_INVERSE_COLORMAP) == BST_CHECKED;
				SendMessage(hDlg, WM_COMMAND, IDOK, 1);
			}
			break;
		case IDC_COMBO_SURF_COLORMAP:
			{
				int nColormap = 
					SendDlgItemMessage( hDlg, IDC_COMBO_SURF_COLORMAP, 
					CB_GETCURSEL,0,(LPARAM)0 );
				if (nColormap >= 0 && nColormap < 21)
				{
					me->m_nColormap = UINT(nColormap);

					char str[255];
					sprintf(str, "%u", me->m_nColormap);
					SetDlgItemText(hDlg, IDC_STATIC_SURF_N_COLORMAP, str);

					//SendMessage(hDlg, WM_COMMAND, IDOK, 0);					
				}
			}
			break;
		case INIT_COMBO_IDC_COMBO_SURF_COLORMAP:
			{
				SendDlgItemMessage( hDlg, IDC_COMBO_SURF_COLORMAP, CB_RESETCONTENT, 0,0);
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
				for (int i = 0; i < 21; i++)
				{					
					SendDlgItemMessage( hDlg, 
						IDC_COMBO_SURF_COLORMAP, CB_ADDSTRING, 0,
						(LPARAM)(LPCTSTR)colormap_strings[i]);
				}
				
				SendDlgItemMessage( hDlg, 
					IDC_COMBO_SURF_COLORMAP, CB_SETCURSEL, 0, me->m_nColormap);
			}
			break;
		case IDOK:
			{
				if (lParam != 1)
					CPalette::InitPalette(&me->m_palette, me->m_nColormap);

				SendMessage(hDlg, WM_COMMAND, INIT_SURF_PALETTE, 0);
				me->m_pSurfDoc->ZoomView();
				
				me->m_pSurfDoc->Draw();

				//====== Перерисовываем Вид с учетом изменений
				RECT rect;
				GetClientRect(me->m_pSurfDoc->hSurfWnd,&rect);
				InvalidateRect(me->m_pSurfDoc->hSurfWnd,&rect, true);

			}
			break;
			
		case IDCANCEL :
			EndDialog( hDlg, IDCANCEL );
			break;
		}
		break;
		
		default :
			
			return( FALSE );
	}
	return( TRUE );
}


void Surface3D::PropertiesDialog()
{
    // Ensure that the common control DLL is loaded. 
	// InitCommonControls(); 
	// Инициализируем страницы блокнота  
	psheetPage[0].dwSize = sizeof(PROPSHEETPAGE);
	psheetPage[0].hInstance = hInst;
//	psheetPage[0].pszIcon = MAKEINTRESOURCE(IDI_TAB);
	psheetPage[0].pszIcon = NULL;
	psheetPage[0].dwFlags = PSP_USETITLE/* | PSP_USEICONID*/;
	psheetPage[0].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_SURF_ALPHA);
	psheetPage[0].pfnDlgProc = Surface3D::DlgProcSurfAlpha;
	psheetPage[0].pszTitle = "Alpha";
	psheetPage[0].lParam = (LPARAM)this;

	// Добавляем страницу в блокнот, сохраняя ее
	// идентификатор в массиве hPage
	hPage[0] = CreatePropertySheetPage(&psheetPage[0]);

	psheetPage[1].dwSize = sizeof(PROPSHEETPAGE);
	psheetPage[1].hInstance = hInst;
//	psheetPage[1].pszIcon = MAKEINTRESOURCE(IDI_TAB);
	psheetPage[1].pszIcon = NULL;
	psheetPage[1].dwFlags = PSP_USETITLE /*| PSP_USEICONID*/;
	psheetPage[1].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_SURF_PALETTE);
	psheetPage[1].pfnDlgProc = Surface3D::DlgProcSurfPalette;
	psheetPage[1].pszTitle = "Palette";
	psheetPage[1].lParam = (LPARAM)this;
	hPage[1] = CreatePropertySheetPage(&psheetPage[1]);

#if 0
	psheetPage[2].dwSize = sizeof(PROPSHEETPAGE);
	psheetPage[2].hInstance = hInst;
//	psheetPage[2].pszIcon = MAKEINTRESOURCE(IDI_KEYWORD);
	psheetPage[2].dwFlags = PSP_USETITLE /*| PSP_USEICONID*/;
	psheetPage[2].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG3);
	psheetPage[2].pfnDlgProc = DlgProc3;
	psheetPage[2].pszTitle = "Keyword";
	psheetPage[2].lParam = 0;
	hPage[2] = CreatePropertySheetPage(&psheetPage[2]);
#endif

	// Инициализируем заголовок блокнота
	psheetHeader.dwSize = sizeof(PROPSHEETHEADER);
	psheetHeader.hInstance = hInst;
	psheetHeader.pszIcon = MAKEINTRESOURCE(IDI_APPICONSM);
//	psheetHeader.dwFlags = PSH_MODELESS | PSH_USEICONID;
	psheetHeader.dwFlags = PSH_USEICONID;
	psheetHeader.hwndParent = this->m_pSurfDoc->hwndTV;
	psheetHeader.hwndParent = this->m_pSurfDoc->hSurfWnd;
	psheetHeader.hwndParent = NULL;
	psheetHeader.pszCaption = "Surface Properties";
	psheetHeader.nPages = 
	sizeof(psheetPage) / sizeof(PROPSHEETPAGE);
	psheetHeader.phpage = (HPROPSHEETPAGE FAR  *)&hPage[0];

	// Создаем и отображаем блокнот
	PropertySheet(&psheetHeader);

	//For a modeless property sheet, your message loop should use 
	//PSM_ISDIALOGMESSAGE to pass messages to the property sheet dialog box. 
	//Your message loop should use PSM_GETCURRENTPAGEHWND to determine when 
	//to destroy the dialog box. When the user clicks the OK or Cancel button, 
	//PSM_GETCURRENTPAGEHWND returns NULL. You can then use the DestroyWindow 
	//function to destroy the dialog box.


}


#endif
setup_dialog::setup_dialog(win_project * p1) : glade_dialog("glade/setup_dialog.glade")
{
	prj = p1;

	if (DialogBoxParam(hInst, 
         MAKEINTRESOURCE(IDD_DIALOG_SETUP), 
         (HWND)NULL, (DLGPROC)DlgProcSetup, (LPARAM)this)==IDOK) 
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


#if USE_SETUP_DIALOG
	
	dialog = glade_xml_get_widget(xml, "setup_dialog");
	if (dialog == NULL) { cout << "setup_dialog : glade_xml_get_widget() failed!!!" << endl; return; }
	
	// initialize the widgets...
	// initialize the widgets...
	// initialize the widgets...
	
	GtkWidget * notebook_su = glade_xml_get_widget(xml, "notebook_su");
	GtkWidget * optmenu; GtkWidget * submenu; GtkWidget * item; i32u eng_index;
	
	GtkWidget * saved_optmenus[5];	// these are for setting the default engine...
	
	// setup1_qm
	// ^^^^^^^^^
	
	optmenu = glade_xml_get_widget(xml, "optionmenu_allQM_eng");
	submenu = gtk_menu_new(); saved_optmenus[0] = optmenu;
	
	for (eng_index = 0; eng_index < setup1_qm::static_GetEngineCount(); eng_index++)
	{
		item = gtk_menu_item_new_with_label(setup1_qm::static_GetEngineName(eng_index));
		gtk_widget_show(item); gtk_menu_append(GTK_MENU(submenu), item);
	}
	
	gtk_option_menu_set_menu(GTK_OPTION_MENU(optmenu), submenu);
	
	// setup1_mm
	// ^^^^^^^^^
	
	optmenu = glade_xml_get_widget(xml, "optionmenu_allMM_eng");
	submenu = gtk_menu_new(); saved_optmenus[1] = optmenu;
	
	for (eng_index = 0; eng_index < setup1_mm::static_GetEngineCount(); eng_index++)
	{
		item = gtk_menu_item_new_with_label(setup1_mm::static_GetEngineName(eng_index));
		gtk_widget_show(item); gtk_menu_append(GTK_MENU(submenu), item);
	}
	
	gtk_option_menu_set_menu(GTK_OPTION_MENU(optmenu), submenu);
	
	// setup1_sf
	// ^^^^^^^^^
	
	optmenu = glade_xml_get_widget(xml, "optionmenu_allSF_eng");
	submenu = gtk_menu_new(); saved_optmenus[2] = optmenu;
	
	for (eng_index = 0; eng_index < setup1_sf::static_GetEngineCount(); eng_index++)
	{
		item = gtk_menu_item_new_with_label(setup1_sf::static_GetEngineName(eng_index));
		gtk_widget_show(item); gtk_menu_append(GTK_MENU(submenu), item);
	}
	
	gtk_option_menu_set_menu(GTK_OPTION_MENU(optmenu), submenu);
	
	// setup2_qm_mm
	// ^^^^^^^^^^^^
	
	optmenu = glade_xml_get_widget(xml, "optionmenu_QMMM_eng");
	submenu = gtk_menu_new(); saved_optmenus[3] = optmenu;
	
	for (eng_index = 0; eng_index < setup2_qm_mm::static_GetEngineCount(); eng_index++)
	{
		item = gtk_menu_item_new_with_label(setup2_qm_mm::static_GetEngineName(eng_index));
		gtk_widget_show(item); gtk_menu_append(GTK_MENU(submenu), item);
	}
	
	gtk_option_menu_set_menu(GTK_OPTION_MENU(optmenu), submenu);
	
	// setup2_mm_sf
	// ^^^^^^^^^^^^
	
	optmenu = glade_xml_get_widget(xml, "optionmenu_MMSF_eng");
	submenu = gtk_menu_new(); saved_optmenus[4] = optmenu;
	
	for (eng_index = 0; eng_index < setup2_mm_sf::static_GetEngineCount(); eng_index++)
	{
		item = gtk_menu_item_new_with_label(setup2_mm_sf::static_GetEngineName(eng_index));
		gtk_widget_show(item); gtk_menu_append(GTK_MENU(submenu), item);
	}
	
	gtk_option_menu_set_menu(GTK_OPTION_MENU(optmenu), submenu);
	
	// detect the type of model::current_setup object, and setup the default values...
	// detect the type of model::current_setup object, and setup the default values...
	// detect the type of model::current_setup object, and setup the default values...
	
	setup1_qm * su_allqm = dynamic_cast<setup1_qm *>(prj->GetCurrentSetup());
	setup1_mm * su_allmm = dynamic_cast<setup1_mm *>(prj->GetCurrentSetup());
	setup1_sf * su_allsf = dynamic_cast<setup1_sf *>(prj->GetCurrentSetup());
	setup2_qm_mm * su_qmmm = dynamic_cast<setup2_qm_mm *>(prj->GetCurrentSetup());
	setup2_mm_sf * su_mmsf = dynamic_cast<setup2_mm_sf *>(prj->GetCurrentSetup());
	
	// the defaults for the subpages...
	// the defaults for the subpages...
	// the defaults for the subpages...
	
	if (su_allqm == NULL)
	{
		GtkWidget * entry_totchrg = glade_xml_get_widget(xml, "entry_allQM_totchrg");		// total charge
		gtk_entry_set_text(GTK_ENTRY(entry_totchrg), "+0");
		
		GtkWidget * entry_spinmult = glade_xml_get_widget(xml, "entry_allQM_spinmult");		// spin multiplicity
		gtk_entry_set_text(GTK_ENTRY(entry_spinmult), "1");
	}
	
	if (su_allmm == NULL)
	{
		//GtkWidget * entry_dimx = glade_xml_get_widget(xml, "entry_allmm_dimx");	// pbc dim x
		//gtk_entry_set_text(GTK_ENTRY(entry_dimx), "1.500");
		//GtkWidget * entry_dimy = glade_xml_get_widget(xml, "entry_allmm_dimy");	// pbc dim y
		//gtk_entry_set_text(GTK_ENTRY(entry_dimy), "1.500");
		//GtkWidget * entry_dimz = glade_xml_get_widget(xml, "entry_allmm_dimz");	// pbc dim z
		//gtk_entry_set_text(GTK_ENTRY(entry_dimz), "1.500");
	}
	
	if (su_allsf == NULL)
	{
	}
	
	if (su_qmmm == NULL)
	{
	}
	
	if (su_mmsf == NULL)
	{
	}
	
	// the settings specific to the model::current_setup object...
	// the settings specific to the model::current_setup object...
	// the settings specific to the model::current_setup object...

	if (su_allqm != NULL)
	{
		gtk_option_menu_set_history(GTK_OPTION_MENU(saved_optmenus[0]), su_allqm->current_eng_index);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook_su), 0);
		
		char buffer[64];
		
		GtkWidget * entry_totchrg = glade_xml_get_widget(xml, "entry_allQM_totchrg");		// total charge
		ostrstream str_tc(buffer, sizeof(buffer));
		str_tc.setf(ios::showpos); str_tc << prj->GetQMTotalCharge() << ends;
		gtk_entry_set_text(GTK_ENTRY(entry_totchrg), buffer);
		
		GtkWidget * entry_spinmult = glade_xml_get_widget(xml, "entry_allQM_spinmult");		// spin multiplicity
	// todo!!! todo!!! todo!!! todo!!! todo!!! todo!!! todo!!!
	// todo!!! todo!!! todo!!! todo!!! todo!!! todo!!! todo!!!
		gtk_entry_set_text(GTK_ENTRY(entry_spinmult), "???");
	}
	else if (su_allmm != NULL)
	{
		gtk_option_menu_set_history(GTK_OPTION_MENU(saved_optmenus[1]), su_allmm->current_eng_index);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook_su), 1);
		
		GtkWidget * checkbutton_amber = glade_xml_get_widget(xml, "checkbutton_allMM_amber");		// exceptions flag
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_amber), (su_allmm->exceptions ? TRUE : FALSE));
		
		//GtkWidget * entry_dimx = glade_xml_get_widget(xml, "entry_allmm_dimx");	// pbc dim x
		//gtk_entry_set_text(GTK_ENTRY(entry_dimx), "???");
		//GtkWidget * entry_dimy = glade_xml_get_widget(xml, "entry_allmm_dimy");	// pbc dim y
		//gtk_entry_set_text(GTK_ENTRY(entry_dimy), "???");
		//GtkWidget * entry_dimz = glade_xml_get_widget(xml, "entry_allmm_dimz");	// pbc dim z
		//gtk_entry_set_text(GTK_ENTRY(entry_dimz), "???");
	}
	else if (su_allsf != NULL)
	{
		gtk_option_menu_set_history(GTK_OPTION_MENU(saved_optmenus[2]), su_allsf->current_eng_index);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook_su), 2);
		
		GtkWidget * checkbutton_expsolv = glade_xml_get_widget(xml, "checkbutton_allSF_expsolv");	// exp/imp solv flag
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_expsolv), (su_allsf->exp_solv ? TRUE : FALSE));
	}
	else if (su_qmmm != NULL)
	{
	//	gtk_option_menu_set_history(GTK_OPTION_MENU(saved_optmenus[3]), su_qmmm->current_eng_index);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook_su), 3);
	}
	else if (su_mmsf != NULL)
	{
	//	gtk_option_menu_set_history(GTK_OPTION_MENU(saved_optmenus[4]), su_mmsf->current_eng_index);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook_su), 4);
	}
	else
	{
		prj->ErrorMessage("BUG: Bad cast was detected at setup_dialog ctor.\nThe program will terminate.");
		exit(EXIT_FAILURE);
	}
	
	// connect the handlers...
	
	glade_xml_signal_connect_data(xml, "on_dialog_destroy", (GtkSignalFunc) handler_Destroy, (gpointer) this);
	
	glade_xml_signal_connect_data(xml, "on_button_ok_clicked", (GtkSignalFunc) handler_ButtonOK, (gpointer) this);
	glade_xml_signal_connect_data(xml, "on_button_cancel_clicked", (GtkSignalFunc) handler_ButtonCancel, (gpointer) this);
	
	gtk_dialog_run(GTK_DIALOG(dialog));	// MODAL
	gtk_widget_destroy(dialog);		// MODAL
#endif /*USE_SETUP_DIALOG*/
}

setup_dialog::~setup_dialog(void)
{
}

void setup_dialog::handler_Destroy(HWND, void * data)			// not really needed...
{
	setup_dialog * ref = (setup_dialog *) data;
	//cout << "handler_Destroy() : ref = " << ref << endl;
}

void setup_dialog::handler_ButtonOK(HWND, void * data)
{
	setup_dialog * ref = (setup_dialog *) data;
	//cout << "handler_ButtonOK() : ref = " << ref << endl;
	
	// read in and process the user's settings...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#if USE_SETUP_DIALOG
	
	GtkWidget * notebook_su = glade_xml_get_widget(ref->xml, "notebook_su");
	GtkWidget * optmenu; GtkWidget * submenu; GtkWidget * item; int index = NOT_DEFINED;
	
	const gint current_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook_su));
	
	if (current_page == 0)
	{
		optmenu = glade_xml_get_widget(ref->xml, "optionmenu_allQM_eng");
		submenu = gtk_option_menu_get_menu(GTK_OPTION_MENU(optmenu));
		item = gtk_menu_get_active(GTK_MENU(submenu));
		
		index = g_list_index(GTK_MENU_SHELL(submenu)->children, item);
		
		delete ref->prj->current_setup;						// get rid of the old setup...
		ref->prj->current_setup = new setup1_qm(ref->prj);			// ...and make a new one.
		ref->prj->GetCurrentSetup()->current_eng_index = index;			// set the requested engine.
		
		// handle the total charge
		
		GtkWidget * entry_totchrg = glade_xml_get_widget(ref->xml, "entry_allQM_totchrg");
		const gchar * buffer = gtk_entry_get_text(GTK_ENTRY(entry_totchrg));
		istrstream istr(buffer, strlen(buffer) + 1);
		i32s value; istr >> value;
		ref->prj->qm_total_charge = value;
		
		// TODO : handle the multiplicity...
	}
	else if (current_page == 1)
	{
		optmenu = glade_xml_get_widget(ref->xml, "optionmenu_allMM_eng");
		submenu = gtk_option_menu_get_menu(GTK_OPTION_MENU(optmenu));
		item = gtk_menu_get_active(GTK_MENU(submenu));
		
		index = g_list_index(GTK_MENU_SHELL(submenu)->children, item);
		
		delete ref->prj->current_setup;						// get rid of the old setup...
		ref->prj->current_setup = new setup1_mm(ref->prj);			// ...and make a new one.
		ref->prj->GetCurrentSetup()->current_eng_index = index;			// set the requested engine.
		
		// handle the exceptions flag
		
		GtkWidget * checkbutton_amber = glade_xml_get_widget(ref->xml, "checkbutton_allMM_amber");
		bool exceptions = (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_amber)) == TRUE ? true : false);
		dynamic_cast<setup1_mm *>(ref->prj->current_setup)->exceptions = exceptions;
	}
	else if (current_page == 2)
	{
		optmenu = glade_xml_get_widget(ref->xml, "optionmenu_allSF_eng");
		submenu = gtk_option_menu_get_menu(GTK_OPTION_MENU(optmenu));
		item = gtk_menu_get_active(GTK_MENU(submenu));
		
		index = g_list_index(GTK_MENU_SHELL(submenu)->children, item);
		
		delete ref->prj->current_setup;						// get rid of the old setup...
		ref->prj->current_setup = new setup1_sf(ref->prj, !index);		// ...and make a new one.
		ref->prj->GetCurrentSetup()->current_eng_index = index;			// set the requested engine.
		
		// handle the exp/imp solv flag
		
		GtkWidget * checkbutton_expsolv = glade_xml_get_widget(ref->xml, "checkbutton_allSF_expsolv");
		bool exp_solv = (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_expsolv)) == TRUE ? true : false);
		dynamic_cast<setup1_sf *>(ref->prj->current_setup)->exp_solv = exp_solv;
	}
	else if (current_page == 3)
	{
	/*	optmenu = glade_xml_get_widget(ref->xml, "optionmenu_QMMM_eng");
		submenu = gtk_option_menu_get_menu(GTK_OPTION_MENU(optmenu));
		item = gtk_menu_get_active(GTK_MENU(submenu));
		
		index = g_list_index(GTK_MENU_SHELL(submenu)->children, item);
		
		delete ref->prj->current_setup;						// get rid of the old setup...
		ref->prj->current_setup = new setup2_qm_mm(ref->prj);			// ...and make a new one.
		ref->prj->GetCurrentSetup()->current_eng_index = index;			// set the requested engine.	*/
	}
	else if (current_page == 4)
	{
	/*	optmenu = glade_xml_get_widget(ref->xml, "optionmenu_MMSF_eng");
		submenu = gtk_option_menu_get_menu(GTK_OPTION_MENU(optmenu));
		item = gtk_menu_get_active(GTK_MENU(submenu));
		
		index = g_list_index(GTK_MENU_SHELL(submenu)->children, item);
		
		delete ref->prj->current_setup;						// get rid of the old setup...
		ref->prj->current_setup = new setup2_mm_sf(ref->prj);			// ...and make a new one.
		ref->prj->GetCurrentSetup()->current_eng_index = index;			// set the requested engine.	*/
	}
	else
	{
		ref->prj->ErrorMessage("BUG: Invalid current page at setup_dialog::handler_ButtonOK().");
		exit(EXIT_FAILURE);
	}
#endif /*USE_SETUP_DIALOG*/
	
	// report the new settings to log...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	char mbuff1[256];
	ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << "Changed the Setup for calculations ";
	str1 << "(setup = " << ref->prj->GetCurrentSetup()->GetClassName();
	str1 << ", engine = " << ref->prj->GetCurrentSetup()->GetEngineName(ref->prj->GetCurrentSetup()->GetCurrEngIndex());
	str1 << ")." << endl << ends;
	ref->prj->PrintToLog(mbuff1);
}

void setup_dialog::handler_ButtonCancel(HWND, void * data)		// not really needed...
{
	setup_dialog * ref = (setup_dialog *) data;
	//cout << "handler_ButtonCancel() : ref = " << ref << endl;
}

/*################################################################################################*/

// eof
