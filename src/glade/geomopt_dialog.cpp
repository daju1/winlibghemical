// GEOMOPT_DIALOG.CPP

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "../stdafx.h"

#include "geomopt_dialog.h"

//#include <gtk/gtk.h>

#include <iostream>
#include <strstream>
using namespace std;

/*################################################################################################*/
extern HINSTANCE hInst;								// current instance
BOOL CALLBACK DlgProcGeomOpt(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
	static geomopt_dialog * geomopt_dlg;
    switch (message) 
    {
	case WM_INITDIALOG:
		{
			geomopt_dlg = reinterpret_cast<geomopt_dialog *>(lParam);
			if (geomopt_dlg)
			{
				geomopt_dlg->hDlg = hDlg;
				geomopt_dlg->OnInitDialog();
			}
		}
		break;
    case WM_COMMAND: 
        switch (LOWORD(wParam)) 
        { 
		case IDC_RADIO_BOX_OPT_NO:
			{
				if (geomopt_dlg && geomopt_dlg->param)
					geomopt_dlg->param->box_opt = geomopt_param::box_optimization_type::no;
			}
			break;
		case IDC_RADIO_BOX_OPT_XYZ:
			{
				if (geomopt_dlg && geomopt_dlg->param)
					geomopt_dlg->param->box_opt = geomopt_param::box_optimization_type::xyz;
			}
			break;
		case IDC_RADIO_BOX_OPT_XY:
			{
				if (geomopt_dlg && geomopt_dlg->param)
					geomopt_dlg->param->box_opt = geomopt_param::box_optimization_type::xy;
			}
			break;
		case IDC_RADIO_BOX_OPT_Z:
			{
				if (geomopt_dlg && geomopt_dlg->param)
					geomopt_dlg->param->box_opt = geomopt_param::box_optimization_type::z;
			}
			break;
		case IDOK: 
			{
				if (geomopt_dlg)
					geomopt_dlg->handler_ButtonOK(NULL, geomopt_dlg);
	            EndDialog(hDlg, wParam); 
			}
			break;
        case IDCANCEL: 
			{
				if (geomopt_dlg)
					geomopt_dlg->handler_ButtonCancel(NULL, geomopt_dlg);
				EndDialog(hDlg, wParam); 
			}
        }
		break;
	case WM_DESTROY:
		{
			if (geomopt_dlg)
				delete geomopt_dlg;
		}
    }
    return FALSE; 
} 



/*################################################################################################*/

geomopt_dialog::geomopt_dialog(geomopt_param * p1) : glade_dialog("glade/geomopt_dialog.glade")
{
	param = p1;
	
	//dialog = glade_xml_get_widget(xml, "geomopt_dialog");
	//if (dialog == NULL) { cout << "geomopt_dialog : glade_xml_get_widget() failed!!!" << endl; return; }
	if (DialogBoxParam(hInst, 
         MAKEINTRESOURCE(IDD_DIALOG_GEOMOPT), 
         (HWND)NULL, (DLGPROC)DlgProcGeomOpt, (LPARAM)this)==IDOK) 
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
	// initialize the widgets...
}

void geomopt_dialog::OnInitDialog()
{	
	
	//entry_nsteps = glade_xml_get_widget(xml, "entry_nsteps");
	//entry_grad = glade_xml_get_widget(xml, "entry_grad");
	//entry_delta_e = glade_xml_get_widget(xml, "entry_delta_e");
	
	char buffer[64];
	
	ostrstream str_nsteps(buffer, sizeof(buffer));
	str_nsteps << param->treshold_nsteps << ends;
	//gtk_entry_set_text(GTK_ENTRY(entry_nsteps), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_NSTEPS, str_nsteps.str());
	
	ostrstream str_grad(buffer, sizeof(buffer));
	str_grad << param->treshold_grad << ends;
	//gtk_entry_set_text(GTK_ENTRY(entry_grad), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_GRAD, str_grad.str());
	
	ostrstream str_delta_e(buffer, sizeof(buffer));
	str_delta_e << param->treshold_delta_e << ends;
	//gtk_entry_set_text(GTK_ENTRY(entry_delta_e), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_DELTA_E, str_delta_e.str());

	ostrstream str_delta_e_min_nsteps(buffer, sizeof(buffer));
	str_delta_e_min_nsteps << param->treshold_delta_e_min_nsteps << ends;
	//gtk_entry_set_text(GTK_ENTRY(entry_delta_e), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_DELTA_E_MIN_NSTEPS, str_delta_e_min_nsteps.str());

	CheckDlgButton( hDlg, IDC_CHECK_DELTA_E,
		param->enable_delta_e ? BST_CHECKED : BST_UNCHECKED );

	CheckDlgButton( hDlg, IDC_CHECK_GRAD,
		param->enable_grad ? BST_CHECKED : BST_UNCHECKED );

	CheckDlgButton( hDlg, IDC_CHECK_NSTEPS,
		param->enable_nsteps ? BST_CHECKED : BST_UNCHECKED );

	CheckDlgButton( hDlg, IDC_CHECK_CALC_MIN_MAX_COORDINATES,
		param->recalc_box ? BST_CHECKED : BST_UNCHECKED );
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	CheckDlgButton( hDlg, IDC_CHECK_BOX_OPTIMIZATION,
		param->box_optimization ? BST_CHECKED : BST_UNCHECKED );
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	CheckDlgButton( hDlg, IDC_CHECK_DONT_MOVE_FIXED_ATOMS,
		param->don_t_move_fixed_atoms ? BST_CHECKED : BST_UNCHECKED );

	//##########################################################
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// SET RADIO 
	switch (param->box_opt)
	{
	case geomopt_param::box_optimization_type::no:
		CheckRadioButton( hDlg,
			IDC_RADIO_BOX_OPT_NO,
			IDC_RADIO_BOX_OPT_NO,
			IDC_RADIO_BOX_OPT_Z);
		break;
	case geomopt_param::box_optimization_type::xyz:
		CheckRadioButton( hDlg,
			IDC_RADIO_BOX_OPT_NO,
			IDC_RADIO_BOX_OPT_XYZ,
			IDC_RADIO_BOX_OPT_Z);
		break;
	case geomopt_param::box_optimization_type::xy:
		MessageBox(0, "xy", "check", 0);
		CheckRadioButton( hDlg,
			IDC_RADIO_BOX_OPT_NO,
			IDC_RADIO_BOX_OPT_XY,
			IDC_RADIO_BOX_OPT_Z);
		break;
	case geomopt_param::box_optimization_type::z:
		CheckRadioButton( hDlg,
			IDC_RADIO_BOX_OPT_NO,
			IDC_RADIO_BOX_OPT_Z,
			IDC_RADIO_BOX_OPT_Z);
		break;
	}
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@	// connect the handlers...
	
	//glade_xml_signal_connect_data(xml, "on_dialog_destroy", (GtkSignalFunc) handler_Destroy, (gpointer) this);
	
	//glade_xml_signal_connect_data(xml, "on_button_ok_clicked", (GtkSignalFunc) handler_ButtonOK, (gpointer) this);
	//glade_xml_signal_connect_data(xml, "on_button_cancel_clicked", (GtkSignalFunc) handler_ButtonCancel, (gpointer) this);
	
	//gtk_dialog_run(GTK_DIALOG(dialog));	// MODAL
	//gtk_widget_destroy(dialog);		// MODAL
	
}

geomopt_dialog::~geomopt_dialog(void)
{
}

void geomopt_dialog::handler_Destroy(HWND, void * data)		// not really needed...
{
	geomopt_dialog * ref = (geomopt_dialog *) data;
	//cout << "handler_Destroy() : ref = " << ref << endl;
}

void geomopt_dialog::handler_ButtonOK(HWND, void * data)
{
	geomopt_dialog * ref = (geomopt_dialog *) data;
	
	// read the user's settings from widgets...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	char buffer[256];
	
	GetDlgItemText(ref->hDlg,IDC_EDIT_NSTEPS, buffer, 63);
	istrstream istr1(buffer, strlen(buffer) + 1);
	istr1 >> ref->param->treshold_nsteps;
	
	GetDlgItemText(ref->hDlg,IDC_EDIT_GRAD, buffer, 63);
	istrstream istr2(buffer, strlen(buffer) + 1);
	istr2 >> ref->param->treshold_grad;
	
	GetDlgItemText(ref->hDlg,IDC_EDIT_DELTA_E, buffer, 63);
	istrstream istr3(buffer, strlen(buffer) + 1);
	istr3 >> ref->param->treshold_delta_e;

	GetDlgItemText(ref->hDlg,IDC_EDIT_DELTA_E_MIN_NSTEPS, buffer, 63);
	istrstream istr4(buffer, strlen(buffer) + 1);
	istr4 >> ref->param->treshold_delta_e_min_nsteps;

	ref->param->enable_delta_e	= IsDlgButtonChecked( ref->hDlg, IDC_CHECK_DELTA_E) 
				== BST_CHECKED;
	ref->param->enable_grad		= IsDlgButtonChecked( ref->hDlg, IDC_CHECK_GRAD) 
				== BST_CHECKED;
	ref->param->enable_nsteps	= IsDlgButtonChecked( ref->hDlg, IDC_CHECK_NSTEPS) 
				== BST_CHECKED;
	ref->param->recalc_box = IsDlgButtonChecked( ref->hDlg, IDC_CHECK_CALC_MIN_MAX_COORDINATES) 
				== BST_CHECKED;
	ref->param->box_optimization = IsDlgButtonChecked( ref->hDlg, IDC_CHECK_BOX_OPTIMIZATION) 
				== BST_CHECKED;		
	ref->param->don_t_move_fixed_atoms = IsDlgButtonChecked( ref->hDlg, IDC_CHECK_DONT_MOVE_FIXED_ATOMS) 
				== BST_CHECKED;

	if (!ref->param->enable_nsteps && !ref->param->enable_grad && !ref->param->enable_delta_e)
	{
		ref->param->enable_nsteps = true;
		cout << "WARNING: all termination tests were disabled!!! the nsteps test is forced." << endl;
	}
	
	ref->param->confirm = true;
}

void geomopt_dialog::handler_ButtonCancel(HWND, void * data)		// not really needed...
{
	geomopt_dialog * ref = (geomopt_dialog *) data;
	//cout << "handler_ButtonCancel() : ref = " << ref << endl;
}
	


/*################################################################################################*/

// eof
